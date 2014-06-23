/** include config file generated by configure
 *  (i.e., know what grids are present, etc)
 *  this should always be included first */
#include <config.h>
/** standard headers **/
#include <iostream>
/** dune (mpi, field-vector and grid type for dgf) **/
#include <dune/common/fvector.hh>        
#include <dune/common/timer.hh>        

/** numerical scheme **/
#include "piecewisefunction.hh"
#include "fvscheme.hh"

/** adaptation scheme **/
#include "adaptation.hh"

/** pde and problem description **/
#include "problem.hh"
#include "problem-transport.hh"
#include "problem-ball.hh"
#include "problem-euler.hh"

#include "diagnostics.hh"
#include "paralleldgf.hh"

// method
// ------
void method ( int problem, int startLvl, int maxLvl, 
              const char* outpath, const int mpiSize  )
{
  typedef Dune::GridSelector::GridType Grid;

  const int startLevel = startLvl * Dune :: DGFGridInfo< Grid > :: refineStepsForHalf();
  const int maxLevel   = maxLvl   * Dune :: DGFGridInfo< Grid > :: refineStepsForHalf();

  /** type of pde to solve **/
#if TRANSPORT
  typedef TransportModel< Grid::dimensionworld > ModelType;
#elif BALL
  typedef BallModel< Grid::dimensionworld > ModelType;
#elif EULER
  typedef EulerModel< Grid::dimensionworld > ModelType;
#endif
  ModelType model( problem );

  /* Grid construction ... */
  std::string name = model.problem().gridFile( "./", mpiSize );
  // create grid pointer and release to free memory of GridPtr
  Grid* gridPtr = Dune::CreateParallelGrid< Grid >::create( name ).release();

  Grid &grid = *gridPtr;
  grid.loadBalance();
  //grid.finalizeGridCreation() ;
  const bool verboseRank = grid.comm().rank() == 0 ;

  std::string outPath( outpath );

  // create the diagnostics object 
  Dune::Diagnostics< Grid> diagnostics( grid.comm(), 1);

  /* ... some global refinement steps */
  if( verboseRank ) 
    std::cout << "globalRefine: " << startLevel << std::endl;
  grid.globalRefine( startLevel );

  /* get view to leaf grid */
  typedef Grid::Partition< Dune::Interior_Partition >::LeafGridView GridView;
  GridView gridView = grid.leafGridView< Dune::Interior_Partition >();

  /* construct data vector for solution */
  typedef PiecewiseFunction< GridView, Dune::FieldVector< double, ModelType::dimRange > > DataType;
  DataType solution( gridView );
  /* initialize data */
  solution.initialize( model.problem() );

  /* create finite volume scheme */
  typedef FiniteVolumeScheme< DataType, ModelType > FVScheme;
  FVScheme scheme( gridView, model );

  /* create VTK writer for data sequqnce */
  Dune::VTKSequenceWriter< GridView >* vtkOut = 0 ;
  if( outPath != "none" )
  {
    vtkOut = new Dune::VTKSequenceWriter< GridView >(  gridView, "solution", outPath, ".", Dune::VTK::nonconforming );
#if ! BALL
    VTKData< DataType >::addTo( solution, *vtkOut );
#endif
    VTKData< DataType >::addPartitioningData( grid.comm().rank(), *vtkOut );
  }

  /* create adaptation method */
  typedef LeafAdaptation< Grid, DataType > AdaptationType;
  AdaptationType adaptation( grid, model.problem().balanceStep() );

  for( int i = 0; i <= maxLevel; ++i )
  {
    // mark grid for initial refinement
    GridMarker< Grid > gridMarker( grid, startLevel, maxLevel );
    scheme.mark( 0, solution, gridMarker );
    // adapt grid 
    if( gridMarker.marked() )
      adaptation( solution );
    // initialize solution for new grid
    solution.initialize( model.problem() );
  }

  if( vtkOut ) 
  {
    /* output the initial grid and the solution */
    vtkOut->write( 0.0 );
  }

  /* prepare for time stepping scheme */
  /* final time for simulation */
  const double endTime = model.problem().endTime();
  /* interval for saving data */
  const double saveInterval = model.problem().saveInterval();     
  /* first point where data is saved */
  double saveStep = saveInterval;
  /* cfl number */
  double cfl = 0.15;
  /* vector to store update */
  DataType update( gridView );

  /* print info about initialization */
  if ( verboseRank )  
    std::cout << "Intialization done!" << std::endl;

  /* now do the time stepping */
  unsigned int step = 0;
  double time = 0.0;
  const unsigned int maxTimeSteps = model.problem().maxTimeSteps();
  while ( time < endTime ) 
  {
    Dune::Timer overallTimer ;

    // update vector might not be of the right size if grid has changed
    update.resize();
    // set update to zero 
    update.clear();
    double dt;

    Dune :: Timer solveTimer ;
    {
      // apply the spacial operator
      dt = scheme.border( time, solution, update );
      // want to have
      // auto comm = update.communicate();
      // the following is taken from piecewisefunction.hh to avoid copy constructer...
      const Dune::InterfaceType interface = Dune::InteriorBorder_All_Interface;
      const Dune::CommunicationDirection direction = Dune::ForwardCommunication;
      typename DataType::CommDataHandle handle( update );
      // auto commObject = grid.communicate( handle, interface, direction );
      grid.communicate( handle, interface, direction );
      dt = std::min(dt , scheme( time, solution, update ) );
      // multiply time step by CFL number
    }
    dt *= cfl;
    // stop time 
    const double solveTime = solveTimer.elapsed(); 

    Dune :: Timer commTimer ;
    // minimize time step over all processes
    dt = solution.gridView().comm().min( dt );
    // communicate update
    const double commTime = commTimer.elapsed();

    // update solution
    solution.axpy( dt, update );

    /* augment time */
    time += dt;
    ++step;

    /* mark the grid for adaptation */
    GridMarker< Grid > gridMarker( grid, startLevel, maxLevel );
    size_t elements = scheme.mark( time, solution, gridMarker );

#ifndef NO_OUTPUT
    /* check if data should be written */
    if( time >= saveStep )
    {
      if( vtkOut ) 
      {
        /* visualize with VTK */
        vtkOut->write( time );
      }
      /* set saveStep for next save point */
      saveStep += saveInterval;

      size_t sumElements = gridView.grid().comm().sum( elements );
      size_t minElements = gridView.grid().comm().min( elements );
      size_t maxElements = gridView.grid().comm().max( elements );
      double imbalance = double(maxElements)/double(minElements);

      /* print info about time, timestep size and counter */
      if ( verboseRank )  
      {
        std::cout << "elements = " << sumElements ;
        std::cout << " ("<<minElements << "," << maxElements << "," << imbalance << ")";
        std::cout << "   maxLevel = " << grid.maxLevel();
        std::cout << "   step = " << step;
        std::cout << "   time = " << time;
        std::cout << "   dt = " << dt;
        std::cout << std::endl;
      }
    }
#endif

    /* call adaptation algorithm */
    if( gridMarker.marked() )
      adaptation( solution );

    {
      // write times to run file 
      diagnostics.write( time, dt,                     // time and time step
                         elements,                     // number of elements
                         ModelType::dimRange,          // number of dofs per element (max)
                         solveTime,                    // time for operator evaluation 
                         commTime + adaptation.communicationTime(), // communication time  
                         adaptation.adaptationTime(),  // time for adaptation 
                         adaptation.loadBalanceTime(), // time for load balance
                         overallTimer.elapsed(),       // time step overall time
                         getMemoryUsage() );                   // memory usage

    }

    // abort when maximal number of time steps is reached (default is disabled)
    if( step >= maxTimeSteps ) 
      break ;
  }           

  if( vtkOut ) 
  {
    /* output final result */
    vtkOut->write( time );
  }

  // flush diagnostics 
  diagnostics.flush();

  delete vtkOut ;
  // delete grid 
  delete gridPtr ;
}
/***************************************************
 ** main program with parameters:                 **
 ** 1) number of problem to use (initial data...) **
 ** 2) number of global refinement steps          **
 ** 3) maximal level to use during refinement     **
 ***************************************************/
int main ( int argc , char **argv )
try
{
  /* initialize MPI, finalize is done automatically on exit */
  Dune::MPIHelper &mpi = Dune::MPIHelper::instance( argc, argv );
  
  if( argc < 2 )
  {
    /* display usage */
    if( mpi.rank() == 0 )
      std::cout << "Usage: " << argv[ 0 ] << " [problem-nr] [startLevel] [maxLevel]" << std::endl;
    return 0;
  }

#if HAVE_ALUGRID
  if( mpi.rank() == 0 ) 
    std::cout << "WARNING: Using old ALUGrid version from dune-grid." << std::endl;
#endif
     
  // meassure program time 
  Dune::Timer timer ;

  /* create problem */
  const int problem = (argc > 1 ? atoi( argv[ 1 ] ) : 0);

  /* get level to use for computationa */
  const int startLevel = (argc > 2 ? atoi( argv[ 2 ] ) : 0);
  const int maxLevel = (argc > 3 ? atoi( argv[ 3 ] ) : startLevel);

  const char* path = (argc > 4) ? argv[ 4 ] : "./";
  method( problem, startLevel, maxLevel, path, mpi.size() );

#ifdef HAVE_MPI 
  MPI_Barrier ( MPI_COMM_WORLD );
#endif

  if( mpi.rank() == 0 ) 
    std::cout << "Program finished: CPU time = " << timer.elapsed() << " sec." << std::endl;

  /* done */
  return 0;
}
catch( const std::exception &e )
{
  std::cout << "STL ERROR: " << e.what() << std::endl;
  return 1;
}
catch( const Dune::Exception &e )
{
  std::cout << "DUNE ERROR: " << e << std::endl;
  return 1;
}
#if ! HAVE_ALUGRID
catch( const ALUGrid::ALUGridException& e )
{
  std::cout << "ALUGrid ERROR: " << e.what() << std::endl;
  return 1;
}
#endif
catch( ... )
{
  std::cout << "Unknown ERROR" << std::endl;
  return 1;
}