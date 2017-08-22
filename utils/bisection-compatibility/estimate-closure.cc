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
void estimateClosure ( GridType &grid , int level = 0 )
{
  //refine up to level
  for(int  i = 0; i < level; ++i)
  {
    for( const auto & entity : Dune::elements( grid.leafGridView( ) ) )
    {
      if( entity.level() < level )
      {
        grid.mark( 1, entity );
      }
      else if( entity.level() > level )
      {
        grid.mark( -1, entity );
      }
    }
    grid.preAdapt();
    grid.adapt();
    grid.postAdapt();
  }
  const size_t macroSize = grid.leafGridView().size( 0 );

  typedef typename GridType::GlobalIdSet IndexSetType;
  typedef typename IndexSetType::IdType IdType;

  const IndexSetType & macroIdSet = grid.globalIdSet();
  std::map< IdType, size_t > elementClosure;


  typedef typename GridType::LeafGridView  LeafGridViewType;
  Dune::VTKSequenceWriter< LeafGridViewType > vtkout(  grid.leafGridView(), "solution" + level, "./", ".", Dune::VTK::nonconforming );

  std::shared_ptr< VolumeData< LeafGridViewType >  > ptr( new VolumeData< LeafGridViewType > ( ) );
  vtkout.addCellData( ptr );

  vtkout.write(0.0);

  size_t maxClosure = 0;
  //loop over all macro elements.
  for( const auto & macroEntity : Dune::elements( grid.leafGridView( ) ) )
  {
    //if elementClosure calculated - continue
    const IdType macroId =  macroIdSet.id( macroEntity );
    if( elementClosure.find( macroId ) != elementClosure.end() )
    {
      continue;
    }
    //mark for refinement
    grid.mark( 1, macroEntity );
    //adapt
    grid.preAdapt();
    grid.adapt();
    grid.postAdapt();
    //loop over macro elements
    size_t closure = grid.leafGridView().size(0) - macroSize;
    for( const auto & entity : Dune::elements( grid.leafGridView( ) ) )
    {
      const auto id = macroIdSet.id( entity );
      if( ! entity.isLeaf() )
      {
        elementClosure[ id ] = elementClosure[ id ] == 0 ? closure : std::min( closure, elementClosure[ id ] );
      }
    }
    elementClosure[ macroId ] = closure;
    if(closure > maxClosure)
    {
      maxClosure = closure;
      std::cout << "New maximum: " << maxClosure << std::endl;
      vtkout.write( double( closure ) );
    }
    std::cout << "Before: "<< grid.leafGridView().size(0) << ", after: ";
    //refine up to level
    for(int  i = 0; i <  closure *  (level + 1); ++i)
    {
      if( grid.leafGridView().size(0) == macroSize ) break;
      for( const auto & entity : Dune::elements( grid.leafGridView( ) ) )
      {
        grid.mark( -1, entity );
      }
      grid.preAdapt();
      grid.adapt();
      grid.postAdapt();
    }
    //refine up to level
    for(int  i = 0; i < level ; ++i)
    {
      for( const auto & entity : Dune::elements( grid.leafGridView( ) ) )
      {
        if( entity.level() < level )
        {
          grid.mark( 1, entity );
        }
      }
      grid.preAdapt();
      grid.adapt();
      grid.postAdapt();
    }
    std::cout << grid.leafGridView().size(0) << std::endl;
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
      estimateClosure( grid, 3);
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
