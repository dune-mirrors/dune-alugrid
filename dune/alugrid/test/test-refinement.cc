#define DISABLE_DEPRECATED_METHOD_CHECK 1
//#define DUNE_GRID_CHECK_USE_DEPRECATED_ENTITY_AND_INTERSECTION_INTERFACE 1

#include <config.h>

#ifndef NDEBUG
#ifndef DUNE_DEVEL_MODE
#define DUNE_DEVEL_MODE
#endif
#define DUNE_INTERFACECHECK
#endif

// #define NO_2D
// #define NO_3D

#include <iostream>
#include <sstream>
#include <string>

#include <dune/common/version.hh>
#include <dune/common/tupleutility.hh>
#include <dune/common/tuples.hh>
#include <dune/common/parallel/mpihelper.hh>

#include <dune/geometry/referenceelements.hh>
#include <dune/geometry/genericgeometry/codimtable.hh>

#include <dune/grid/io/file/dgfparser/dgfwriter.hh>
#include <dune/grid/io/file/vtk/vtkwriter.hh>

#if DUNE_VERSION_NEWER(DUNE_GRID,2,4)
#include <dune/grid/test/gridcheck.hh>
#include <dune/grid/test/checkgeometryinfather.hh>
#include <dune/grid/test/checkintersectionit.hh>
#include <dune/grid/test/checkiterators.hh>
#include <dune/grid/test/checkcommunicate.hh>
#else
#include <dune/grid/test/gridcheck.cc>
#include <dune/grid/test/checkgeometryinfather.cc>
#include <dune/grid/test/checkintersectionit.cc>
#include <dune/grid/test/checkiterators.cc>
#include <dune/grid/test/checkcommunicate.cc>
#endif
//#include "checktwists.cc"

#include <dune/alugrid/dgf.hh>

#if ALU3DGRID_PARALLEL && HAVE_MPI
#define USE_PARALLEL_TEST 1
#endif

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


template <class GridType>
void makeNonConfGrid(GridType &grid,int rank,int adapt)
{
  int myrank = grid.comm().rank();

  for (int i=0;i<adapt;i++)
  {
    if (myrank==rank)
    {
      typedef typename GridType::template Codim< 0 >::template Partition< Dune::Interior_Partition >::LeafIterator LeafIterator;

      LeafIterator endit = grid.template leafend< 0, Dune::Interior_Partition >();
      int nr = 0;
      int size = grid.size(0);
      for(LeafIterator it    = grid.template leafbegin< 0, Dune::Interior_Partition >();
          it != endit ; ++it,nr++ )
      {
        grid.mark(1, *it );
        if (nr>size*0.2) break;
      }
    }
    if(myrank == 0)
       std::cout << "Max Level: " << i+1 << " | ";
    grid.adapt();
    grid.postAdapt();
  }
}

template <class GridView>
void writeFile( const GridView& gridView , std::string name )
{
 //Dune::DGFWriter< GridView > writer( gridView );
 // writer.write( "dump.dgf" );

  const  int rank = gridView.grid().comm().rank();

  std::shared_ptr< PartitioningData<GridView> > rankVTK ( new PartitioningData<GridView>( rank ) );

  Dune::VTKWriter< GridView > writer( gridView );
  writer.addCellData( rankVTK );
  writer.write( name );
}

template <class GridType>
void checkALUSerial(GridType & grid, int rank = 10, int mxl = 2, std::string filename = "dumping")
{
  if(grid.comm().size() == 1)
    std::cout << "  CHECKING: grid size = " << grid.size( 0 ) << std::endl;

  //writeFile( grid.leafGridView() , "init-" + filename.substr(6));


  // check also non-conform grids
  makeNonConfGrid(grid,rank,mxl);

  writeFile( grid.leafGridView() , filename.substr(6));
}



int main (int argc , char **argv) {

  // this method calls MPI_Init, if MPI is enabled
  Dune::MPIHelper &mpihelper = Dune::MPIHelper::instance( argc, argv );
  int myrank = mpihelper.rank();
  int mysize = mpihelper.size();

  try {
    // check conform ALUGrid for 2d

    typedef Dune::ALUGrid< 2, 2, Dune::simplex, Dune::conforming > GridType;
    std::string filename = "./dgf/simplex-testgrid-2-2.dgf";
    Dune::GridPtr<GridType> gridPtr( filename );
    GridType & grid = *gridPtr;
    grid.loadBalance();

    {
      checkALUSerial(grid, 19, 20, filename );
    }

  /*  typedef Dune::ALUGrid< 2, 3, Dune::simplex, Dune::conforming > SurfaceGridType;
    std::string surfaceFilename( "./dgf/simplex-testgrid-2-3.dgf" );
    std::cout << "READING from '" << surfaceFilename << "'..." << std::endl;
    Dune::GridPtr< SurfaceGridType > surfaceGridPtr( surfaceFilename );
    SurfaceGridType & surfaceGrid = *surfaceGridPtr ;
    surfaceGrid.loadBalance();
    checkALUSerial( surfaceGrid, 10, 20 ,surfaceFilename);
*/

    filename = "./dgf/unitcube3d.dgf";

    typedef Dune::ALUGrid< 3, 3, Dune::simplex, Dune::conforming > GridType3d;
    Dune::GridPtr< GridType3d > gridPtr3d( filename );
    GridType3d & grid3d = *gridPtr3d;
    grid3d.loadBalance();

    {
      checkALUSerial(grid3d,10, 20, filename);
    }

  }
  catch( Dune::Exception &e )
  {
    std::cerr << e << std::endl;
    return 1;
  }
  catch( ... )
  {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }

  return 0;
}
