#include <config.h>

#include <iostream>
#include <sstream>
#include <string>

#include <dune/common/parallel/mpihelper.hh>

#include <dune/grid/common/rangegenerators.hh>
#include <dune/grid/io/file/vtk/vtksequencewriter.hh>
#include "../../examples/piecewisefunction.hh"

#include <dune/alugrid/dgf.hh>
#include <dune/alugrid/grid.hh>


template< class GridType >
void estimateClosure ( GridType &grid )
{
  const std::size_t macroSize =grid.levelGridView(0).size(0);

  typedef typename GridType::LevelIndexSet LevelIndexSetType;
  typedef typename LevelIndexSetType::IndexType IdType;

  const LevelIndexSetType & macroIdSet = grid.levelIndexSet(0);
  std::map< IdType, size_t > elementClosure;

  typedef typename GridType::LeafGridView  LeafGridViewType;
  Dune::VTKSequenceWriter< LeafGridViewType > vtkout(  grid.leafGridView(), "solution", "./", ".", Dune::VTK::nonconforming );

  std::shared_ptr< VolumeData< LeafGridViewType >  > ptr( new VolumeData< LeafGridViewType > ( ) );
  vtkout.addCellData( ptr );

  vtkout.write(0.0);

  size_t maxClosure = 0;
  //loop over all macro elements.
  for( const auto & macroEntity : Dune::elements( grid.levelGridView(0) ) )
  {
    //if elementClosure calculated - continue
    const IdType macroId =  macroIdSet.index(macroEntity);
    //mark for refinement
    grid.mark( 1, macroEntity );
    //adapt
    grid.preAdapt();
    grid.adapt();
    grid.postAdapt();
    //loop over macro elements
    size_t closure = grid.leafGridView().size(0) - macroSize;
    elementClosure[ macroId ] = closure;
    if(closure > maxClosure)
    {
      maxClosure = closure;
      vtkout.write( double( closure ) );
    }
    while( grid.leafGridView().size(0) != macroSize )
    {
      for( const auto & entity : Dune::elements( grid.leafGridView() ) )
      {
        if(entity.level() > 0 )
        {
          grid.mark( -1, entity );
        }
      }
      grid.preAdapt();
      grid.adapt();
      grid.postAdapt();
    }
  }

  size_t minClosure = 1e10;
  size_t avgClosure = 0;

  for(auto closure : elementClosure )
  {
    maxClosure = std::max( closure.second , maxClosure );
    minClosure = std::min( closure.second , minClosure );
    avgClosure += closure.second ;
  }
  avgClosure /=  macroSize;

  std::cout << "Closure (min, max, avg): " << minClosure << " " << maxClosure << " " << avgClosure << std::endl << std::endl;
}


int main (int argc , char **argv) {

  // this method calls MPI_Init, if MPI is enabled
  Dune::MPIHelper::instance( argc, argv );

  try {
    // 3-3 conform
    {
      typedef Dune::ALUGrid< 3, 3, Dune::simplex, Dune::conforming > GridType;
      Dune::GridPtr< GridType > gridPtr( "../../examples/dgf/input3.dgf" );
      GridType & grid = *gridPtr;
      grid.loadBalance();
      estimateClosure( grid );
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
