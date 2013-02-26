#ifndef ALUGRID_PARALLEL_DGF_HH
#define ALUGRID_PARALLEL_DGF_HH

#include <dune/grid/io/file/dgfparser/dgfs.hh>
#include <dune/alugrid/dgf.hh>

namespace Dune 
{
  template <class Grid> 
  class CreateParallelGrid ;  

  template < int dim, int dimworld, ALUGridElementType eltype, 
             ALUGridRefinementType refineType, class Comm > 
  class CreateParallelGrid< ALUGrid< dim, dimworld, eltype, refineType, Comm > >
  {
    typedef ALUGrid< dim, dimworld, eltype, refineType, Comm > Grid ;

    // SimplePartitioner
    // -----------------
    template< class GV, PartitionIteratorType pitype, class IS = typename GV::IndexSet >
    class SimplePartitioner 
    {
      typedef SimplePartitioner< GV, pitype, IS > This;

    public:
      typedef GV GridView;
      typedef typename GridView::Grid Grid;

      typedef IS IndexSet;

    protected:
      typedef typename IndexSet::IndexType IndexType;

      static const int dimension = Grid::dimension;

      typedef typename Grid::template Codim< 0 >::Entity Element;
      typedef typename Grid::template Codim< 0 >::EntityPointer ElementPointer;

      // type of communicator 
      typedef Dune :: CollectiveCommunication< typename MPIHelper :: MPICommunicator >
        CollectiveCommunication ;

    public:
      SimplePartitioner ( const GridView &gridView )
      : comm_( MPIHelper :: getCommunicator() ),
        gridView_( gridView ),
        indexSet_( gridView_.indexSet() )
      {
        // per default every entity is on rank 0 
        partition_.resize( indexSet_.size( 0 ) );
        std::fill( partition_.begin(), partition_.end(), 0 );
        // compute decomposition 
        calculatePartitioning();
      }

    public:
      template< class Entity >
      int rank( const Entity &entity ) const
      {
        assert( Entity::codimension == 0 );
        return rank( (int)indexSet_.index( entity ) );
      }

      int rank( int index ) const
      {
        return partition_[ index ];
      }

    protected:
      void calculatePartitioning()
      {

        const size_t partSize = indexSet_.size( 0 );

        // get number of MPI processes  
        const int procSize = comm_.size();

        // get minimal number of entities per process 
        const size_t minPerProc = (double(partSize) / double( procSize ));
        size_t maxPerProc = minPerProc ;
        if( partSize % procSize != 0 )
          ++ maxPerProc ;

        // calculate percentage of elements with larger number 
        // of elements per process 
        double percentage = (double(partSize) / double( procSize ));
        percentage -= minPerProc ;
        percentage *= procSize ;

        // per default every entity is on rank 0 
        partition_.resize( indexSet_.size( 0 ) );
        std::fill( partition_.begin(), partition_.end(), 0 );

        int rank = 0;
        size_t elementCount  = maxPerProc ;
        size_t elementNumber = 0;
        size_t localElementNumber = 0;
        const int lastRank = procSize - 1;
        // create the space filling curve iterator 
        typedef typename GridView::template Codim< 0 >::Iterator Iterator;
        const Iterator end = gridView_.template end< 0 > ();
        for( Iterator it = gridView_.template begin< 0 > (); it != end; ++it ) 
        {
          const Element &element = *it ;
          if( localElementNumber >= elementCount ) 
          {
            // increase rank 
            if( rank < lastRank ) ++ rank;

            // reset local number 
            localElementNumber = 0;

            // switch to smaller number if red line is crossed 
            if( elementCount == maxPerProc && rank >= percentage ) 
              elementCount = minPerProc ;
          }

          const size_t index = indexSet_.index( element );
          assert( rank < procSize );
          partition_[ index ] = rank ;

          // increase counters 
          ++elementNumber;
          ++localElementNumber; 
        }
      }
      
      CollectiveCommunication comm_;

      const GridView& gridView_;
      const IndexSet &indexSet_;

      // load balancer bounds 
      std::vector< int > partition_;
    };

  public:  
    static GridPtr< Grid > create( const std::string& filename ) 
    {
      // this only works for Cartesian grid using DGF's IntervalBlock
      if( eltype == simplex ) 
        return GridPtr< Grid >( filename );

#if ! HAVE_ALUGRID 
      std::cout << "Create parallel grid" << std::endl;
      // only work for the new ALUGrid version 
      typedef SGrid< dim, dimworld > SGridType ;
      GridPtr< SGridType > sPtr( filename );
      SGridType& sgrid = *sPtr ; 

      typedef typename SGridType :: LeafGridView GridView ;
      typedef typename GridView :: IndexSet  IndexSet ;
      typedef typename IndexSet :: IndexType IndexType ;
      typedef typename GridView :: template Codim< 0 > :: Iterator ElementIterator ;
      typedef typename ElementIterator::Entity  Entity ;
      typedef typename Entity::EntityPointer  EntityPointer ;
      typedef typename GridView :: IntersectionIterator  IntersectionIterator ;

      GridView gridView = sgrid.leafView();
      const IndexSet& indexSet = gridView.indexSet();

      // type of communicator 
      typedef Dune :: CollectiveCommunication< typename MPIHelper :: MPICommunicator >
        CollectiveCommunication ;

      CollectiveCommunication comm( MPIHelper :: getCommunicator() );
      const int myrank = comm.rank();

      // get decompostition of the marco grid 
      SimplePartitioner< GridView, InteriorBorder_Partition > partitioner( gridView );

      // create ALUGrid GridFactory
      typedef GridFactory< Grid > Factory ;
      Factory factory ;

      typedef typename Factory::VertexId VertexId;

      // create new vector holding vetex ids 
      typedef std::map< IndexType, VertexId > VertexMapType ;
      VertexMapType vertexId ;

      const int numVertices = (1 << dim);
      const typename VertexMapType::iterator endVxMap = vertexId.end();
      const ElementIterator end = gridView.template end< 0 >();
      for( ElementIterator it = gridView.template begin< 0 >(); it != end; ++it )
      {
        const Entity &entity = *it;
        // if the element does not belong to our partitioning, continue 
        if( partitioner.rank( entity ) != myrank ) continue ;

        assert( numVertices == entity.template count< dim >() );
        for( int i = 0; i < numVertices; ++i )
        {
          const IndexType idx = indexSet.subIndex( entity, i, dim );
          if( vertexId.find( idx ) == endVxMap ) 
          {
             vertexId[ idx ] = 
               factory.insertVertex( (*entity.template subEntity< dim > ( i )).geometry().center(), idx );
          }
        }
      }

      int elIndex = 0;
      std::vector< VertexId > vertices( numVertices );
      for( ElementIterator it = gridView.template begin< 0 >(); it != end; ++it )
      {
        const Entity &entity = *it;
        // if the element does not belong to our partitioning, continue 
        if( partitioner.rank( entity ) != myrank ) continue ;

        assert( numVertices == entity.template count< dim >() );
        for( int i = 0; i < numVertices; ++i )
          vertices[ i ] = vertexId[ indexSet.subIndex( entity, i, dim ) ];
        std::cout << "P[ " << myrank << " ] insert " << elIndex << std::endl;
        factory.insertElement( entity.type(), vertices );

        const IntersectionIterator iend = gridView.iend( entity );
        for( IntersectionIterator iit = gridView.ibegin( entity ); iit != iend; ++iit )
        {
          const int face = iit->indexInInside();
          // insert boundary face in case of domain boundary 
          if( iit->boundary() )
            factory.insertBoundary( elIndex, face, iit->boundaryId() );
          // insert process boundary in case the neighboring element has a different rank
          if(   iit->neighbor() ) 
          {
            EntityPointer outside = iit->outside();
            if( partitioner.rank( *outside ) != myrank ) 
            {
              std::cout << "P[ " << myrank << " ] insert internal bnd for " << elIndex << " on face " << face << std::endl;
              factory.insertProcessBorder( elIndex, face );
            }
          }
        }
        ++elIndex;
      }

      return GridPtr< Grid> ( factory.createGrid() );
#else 
      return GridPtr< Grid > (filename);
#endif // if ! HAVE_ALUGRID 
    }
  };
}

#endif
