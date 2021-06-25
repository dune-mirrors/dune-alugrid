#ifndef DUNE_ALUGRID_FROMTOGRIDFACTORY_HH
#define DUNE_ALUGRID_FROMTOGRIDFACTORY_HH

#include <map>
#include <vector>

#include <dune/common/version.hh>

#include <dune/grid/common/gridfactory.hh>
#include <dune/grid/common/exceptions.hh>

#include <dune/alugrid/common/alugrid_assert.hh>
#include <dune/alugrid/common/declaration.hh>

namespace Dune
{

  // External Forward Declarations
  // -----------------------------

  template< class ToGrid >
  class FromToGridFactory;


  // FromToGridFactory for ALUGrid
  // -----------------------------

  template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refineType, class Comm >
  class FromToGridFactory< ALUGrid< dim, dimworld, eltype, refineType, Comm > >
  {
  public:
    // type of grid that is converted to
    typedef ALUGrid< dim, dimworld, eltype, refineType, Comm > Grid;
  protected:
    typedef FromToGridFactory< Grid > This;

    // typedef grid pointer type based on what the grid factory interface defines
    typedef decltype(std::declval< Dune::GridFactoryInterface< Grid >* >()->createGrid())  GridPtrType;

    std::vector< unsigned int > ordering_ ;

  public:
    template <class FromGrid, class Vector>
    GridPtrType convert( const FromGrid& grid, Vector& cellData, std::vector<unsigned int>& ordering )
    {
      int rank = 0;
#if HAVE_MPI
      MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#endif

      // create ALUGrid GridFactory
      GridFactory< Grid > factory;

      // only insert elements on rank 0
      if( rank == 0 )
      {
        typedef typename FromGrid  :: LeafGridView GridView ;
        typedef typename GridView  :: IndexSet  IndexSet ;
        typedef typename IndexSet  :: IndexType IndexType ;
        typedef typename GridView  :: template Codim< 0 > :: Iterator ElementIterator ;
        typedef typename ElementIterator::Entity  Entity ;
        typedef typename GridView :: IntersectionIterator     IntersectionIterator ;
        typedef typename IntersectionIterator :: Intersection Intersection ;

        GridView gridView = grid.leafGridView();
        const IndexSet &indexSet = gridView.indexSet();

        // map global vertex ids to local ones
        std::map< IndexType, unsigned int > vtxMap;

        const int numVertices = (1 << dim);
        std::vector< unsigned int > vertices( numVertices );
        typedef std::pair< Dune::GeometryType, std::vector< unsigned int > > ElementPair;
        std::vector< ElementPair > elements;
        if( ! ordering.empty() )
          elements.resize( ordering.size() );

        int nextElementIndex = 0;
        const ElementIterator end = gridView.template end< 0 >();
        for( ElementIterator it = gridView.template begin< 0 >(); it != end; ++it )
        {
          const Entity &entity = *it;

          // insert vertices and element
          const typename Entity::Geometry geo = entity.geometry();
          alugrid_assert( numVertices == geo.corners() );
          for( int i = 0; i < numVertices; ++i )
          {
            const IndexType vtxId = indexSet.subIndex( entity, i, dim );
            std::pair< typename std::map< IndexType, unsigned int >::iterator, bool > result
              = vtxMap.insert( std::make_pair( vtxId, vtxMap.size() ) );
            if( result.second )
              factory.insertVertex( geo.corner( i ), vtxId );
            vertices[ i ] = result.first->second;
          }
          if( ordering.empty() )
          {
            factory.insertElement( entity.type(), vertices );
          }
          else
          {
            // store element applying the reordering
            elements[ ordering[ nextElementIndex++ ] ] = ElementPair( entity.type(), vertices ) ;
          }
        }

        if( ! ordering.empty() )
        {
          // insert elements using reordered list
          for( auto it = elements.begin(), end = elements.end(); it != end; ++it )
          {
            factory.insertElement( (*it).first, (*it).second );
          }
        }

        nextElementIndex = 0;
        for( ElementIterator it = gridView.template begin< 0 >(); it != end; ++it )
        {
          const Entity &entity = *it;

          const int elementIndex = ordering.empty() ? nextElementIndex++ : ordering[ nextElementIndex++ ];
          const IntersectionIterator iend = gridView.iend( entity );
          for( IntersectionIterator iit = gridView.ibegin( entity ); iit != iend; ++iit )
          {
            const Intersection &intersection = *iit;
            const int faceNumber = intersection.indexInInside();
            // insert boundary face in case of domain boundary
            if( intersection.boundary() )
              factory.insertBoundary( elementIndex, faceNumber );

            // for parallel grids we can check if we are at a process border
            if( intersection.neighbor() &&
                intersection.outside().partitionType() != InteriorEntity )
            {
              // insert process boundary if the neighboring element has a different rank
              factory.insertProcessBorder( elementIndex, faceNumber );
            }
          }
        }
      }

      // create grid pointer (behaving like a shared_ptr)
      GridPtrType newgrid = factory.createGrid( true, true, std::string("FromToGrid") );

      if( ! cellData.empty() )
      {
        Vector oldCellData( cellData );
        auto macroView = newgrid->levelGridView( 0 );
        size_t idx = 0;
        for( auto it = macroView.template begin<0>(), end = macroView.template end<0>();
             it != end; ++it, ++idx )
        {
          const int insertionIndex = ordering.empty() ?
            factory.insertionIndex( *it ) : ordering[ factory.insertionIndex( *it ) ];                                                        ;
          cellData[ idx ] = oldCellData[ insertionIndex ] ;
        }
      }

      // store the ordering from the factory, if it was not provided
      if( ordering.empty() )
        ordering = factory.ordering();

#if HAVE_MPI
      MPI_Barrier( MPI_COMM_WORLD );
#endif

      return newgrid;
    }

    template <class FromGrid, class Vector>
    GridPtrType convert( const FromGrid& fromGrid, Vector& cellData )
    {
      return convert( fromGrid, cellData, ordering_ );
    }

    template <class FromGrid>
    GridPtrType convert( const FromGrid& fromGrid )
    {
      std::vector<int> dummy(0);
      return convert( fromGrid, dummy, ordering_ );
    }
  protected:
    template <int codim, class Entity>
    int subEntities ( const Entity& entity ) const
    {
      return entity.subEntities( codim );
    }
  };

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_STRUCTUREDGRIDFACTORY_HH
