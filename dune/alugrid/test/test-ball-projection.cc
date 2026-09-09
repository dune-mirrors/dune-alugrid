#include <config.h>

// iostream includes
#include <iostream>

// grid includes
#include <dune/alugrid/grid.hh>
#include <dune/alugrid/dgf.hh>

#include <dune/grid/common/partitionset.hh>
#include <dune/grid/common/rangegenerators.hh>

#include <dune/grid/io/file/vtk/vtkwriter.hh>
#include <dune/grid/io/file/vtk/vtksequencewriter.hh>


//#include <dune/grid/albertagrid.hh>
//#include <dune/grid/albertagrid/dgfparser.hh>

template <class HGridType >
void algorithm ( HGridType &grid, const int step, const bool writeVTK = false )
{
  int n = 0;
  double volume = 0;

  const auto gridView = grid.leafGridView();

  for( const auto& entity : Dune::elements( gridView, Dune::Partitions::interior ) )
  {
    volume += entity.geometry().volume();
    ++n;
  }

  volume = gridView.comm().sum( volume );
  n = gridView.comm().sum( n );

  if( gridView.comm().rank() == 0 )
  {
    std::cout << "level: " << step
              << " elements: " << n
              << " volume: " << volume
              << " error: " << std::abs( volume - 4.0 * M_PI / 3.0 )
              << std::endl;
  }

}

template <class HGridType >
void refineRank ( HGridType &grid, const int refRank )
{
  const auto gridView = grid.leafGridView();
  for( const auto& entity : Dune::elements( gridView, Dune::Partitions::interior ) )
  {
    const auto center = entity.geometry().center();
    if( center[0] > 0 && center[1] > 0)
      grid.mark( 1, entity );
  }

  grid.preAdapt();
  grid.adapt();
  grid.postAdapt();
  grid.loadBalance();
}

template <class GridViewType>
class PartitioningData
  : public Dune::VTKFunction< GridViewType >
{
  typedef PartitioningData   ThisType;

public:
  typedef typename GridViewType :: template Codim< 0 >::Entity EntityType;
  typedef typename EntityType::Geometry::LocalCoordinate LocalCoordinateType;

  //! constructor taking discrete function
  PartitioningData( const int rank ) : rank_( rank ) {}

  //! virtual destructor
  virtual ~PartitioningData () {}

  //! return number of components
  virtual int ncomps () const { return 1; }

  //! evaluate single component comp in
  //! the entity
  virtual double evaluate ( int comp, const EntityType &e, const LocalCoordinateType &xi ) const
  {
    return double( rank_ );
  }

  //! get name
  virtual std::string name () const
  {
    return std::string( "rank" );
  }

private:
  const int rank_;
};



// main
// ----

int main ( int argc, char **argv )
try
{
  Dune::MPIHelper &mpihelper = Dune::MPIHelper::instance( argc, argv );

  // create grid from DGF file
  static const int dim = 2;
  const std::string gridFile = ( dim == 3 ) ? "dgf/ball.dgf" : "dgf/circ.dgf";

  {
    // type of hierarchical grid
    typedef Dune :: ALUGrid< dim, dim, Dune::simplex, Dune::conforming > HGridType;
    typedef typename HGridType::LeafGridView GridView;

    std::cout << "P[ " << mpihelper.rank() << " ]:  Dune :: ALUGrid< 3, 3, Dune::simplex, Dune::conforming >" << std::endl;

    // the method rank and size from MPIManager are static
    std::cout << "P[ " << mpihelper.rank() << " ]:  Loading macro bulk grid: " << gridFile << std::endl;

    // construct macro using the DGF Parser
    Dune::GridPtr< HGridType > gridPtr( gridFile );
    HGridType& grid = *gridPtr ;

    // do initial load balance
    grid.loadBalance();

    // Write VTK
    std::ostringstream vtkName;
    vtkName << "test-ball-ref" ;
    auto gridView = grid.leafGridView();
    Dune::VTKSequenceWriter< GridView > vtkWriter( gridView, vtkName.str(), "./", "" );
    vtkWriter.addCellData( std::shared_ptr< PartitioningData< GridView > > ( new PartitioningData< GridView >(gridView.comm().rank()) ) );

    const int refineStepsForHalf = Dune::DGFGridInfo< HGridType >::refineStepsForHalf();

    for( int step = 0; step < 6; ++step )
    {
      // refine globally such that grid with is bisected
      // and all memory is adjusted correctly
      if( dim == 2 )
        refineRank( grid, 0 );
      else
      {
        grid.globalRefine( refineStepsForHalf );
      }

      algorithm( grid, step );
      vtkWriter.write( step );
    }
  }
#if 0
  {
    typedef Dune::AlbertaGrid< 3 > HGridType;
    std::cout << "Dune :: AlbertaGrid< 3 >" << std::endl;

    // the method rank and size from MPIManager are static
    std::cout << "Loading macro bulk grid: " << gridFile << std::endl;

    // construct macro using the DGF Parser
    Dune::GridPtr< HGridType > gridPtr( gridFile );
    HGridType& grid = *gridPtr ;

    // do initial load balance
    grid.loadBalance();

    const int refineStepsForHalf = Dune::DGFGridInfo< HGridType >::refineStepsForHalf();

    for( int step = 0; step <= 5; ++step )
    {
      // refine globally such that grid with is bisected
      // and all memory is adjusted correctly
      grid.globalRefine( refineStepsForHalf );

      algorithm( grid, step );
    }
  }
#endif

  return 0;
}
catch( const Dune::Exception &exception )
{
  std::cerr << "Error: " << exception << std::endl;
  return 1;
}
