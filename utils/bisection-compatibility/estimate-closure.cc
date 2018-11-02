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
  typedef typename GridType::template Codim< 0 >::Entity EntityType;

  const IndexSetType & macroIdSet = grid.globalIdSet();
  std::map< IdType, size_t > elementClosure;

  typedef typename GridType::LeafGridView  LeafGridViewType;
  const LeafGridViewType & leafGridView = grid.leafGridView();
  Dune::VTKSequenceWriter< LeafGridViewType > vtkout(  grid.leafGridView(), "solution" + std::to_string(level), "./", ".", Dune::VTK::nonconforming );

  std::shared_ptr< VolumeData< LeafGridViewType >  > ptr( new VolumeData< LeafGridViewType > ( ) );
  vtkout.addCellData( ptr );

  vtkout.write(0.0);

  size_t maxClosure = 0;
  //loop over all macro elements.
  size_t done = 0;
  while(  done < macroSize )
  {
    std::cerr << elementClosure.size() << " " ;
    EntityType macroEntity;
    IdType macroId;
    //find new elements
    for(const auto & entity : Dune::elements(leafGridView ) )
    {
      const IdType id =  macroIdSet.id( entity );
      if( elementClosure.find( id ) == elementClosure.end() )
      {
        macroEntity = entity;
        macroId = id;
        break;
      }
    }
    if( macroEntity == EntityType() )
      break;
    ++done;

    //mark for refinement
    grid.mark( 1, macroEntity );
    //adapt
    grid.preAdapt();
    grid.adapt();
    grid.postAdapt();
    //loop over macro elements
    size_t closure = leafGridView.size(0) - macroSize;
    for( const auto & entity : Dune::elements( grid.levelGridView( level ) ) )
    {
      if( ! entity.isLeaf() )
      {
        const auto id = macroIdSet.id( entity );
        auto clIt = elementClosure.find( id );
        if( clIt == elementClosure.end() )
          elementClosure.insert( std::make_pair ( id, closure ) );
        else
          clIt->second = std::min( clIt->second, closure );
      }
    }
    elementClosure[ macroId ] = closure;
    if(closure > maxClosure)
    {
      maxClosure = closure;
      std::cout << "New maximum: " << maxClosure << std::endl;
      vtkout.write( double( closure ) );
    }
    //coarsen down to level
    while(grid.maxLevel() > level)
    {
      for( const auto & entity : Dune::elements( leafGridView ) )
      {
        // if level -1  => infinite loop ...
        if(entity.level() > level - 2)
        {
          grid.mark( -1, entity );
        }
      }
      grid.preAdapt();
      grid.adapt();
      grid.postAdapt();
    }
    //refine up to level
    for(int  i = 0; i < level ; ++i)
    {
      for( const auto & entity : Dune::elements( leafGridView ) )
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
      estimateClosure( grid, 3 );
      estimateClosure( grid, 6 );
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
