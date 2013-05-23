#ifndef DUNE_ALUGRID_STRUCTUREDGRIDFACTORY_HH
#define DUNE_ALUGRID_STRUCTUREDGRIDFACTORY_HH

#include <vector>

#include <dune/common/array.hh>

#include <dune/common/classname.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/shared_ptr.hh>

#include <dune/grid/common/gridfactory.hh>
#include <dune/grid/common/exceptions.hh>

#include <dune/grid/alugrid/common/declaration.hh>

// include DGF parser implementation for SGrid 
#include <dune/grid/io/file/dgfparser/dgfs.hh>

namespace Dune
{

  // External Forward Declarations
  // -----------------------------

  template< class Grid >
  class StructuredGridFactory;



  // StructuredGridFactory for ALUGrid
  // ---------------------------------

  template< int dim, int dimworld, ALUGridElementType eltype, ALUGridRefinementType refineType, class Comm >
  class StructuredGridFactory< ALUGrid< dim, dimworld, eltype, refineType, Comm > >
  {
  public:
    typedef ALUGrid< dim, dimworld, eltype, refineType, Comm > Grid;
  protected:  
    typedef StructuredGridFactory< Grid > This;

  private:
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
      SimplePartitioner ( const GridView &gridView, const CollectiveCommunication& comm )
      : comm_( comm ),
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
        alugrid_assert ( Entity::codimension == 0 );
        return rank( (int)indexSet_.index( entity ) );
      }

      int rank( int index ) const
      {
        return partition_[ index ];
      }

    protected:
      void calculatePartitioning()
      {
        const size_t nElements = indexSet_.size( 0 );

        // get number of MPI processes  
        const int nRanks = comm_.size();

        // get minimal number of entities per process 
        const size_t minPerProc = (double(nElements) / double( nRanks ));
        size_t maxPerProc = minPerProc ;
        if( nElements % nRanks != 0 )
          ++ maxPerProc ;

        // calculate percentage of elements with larger number 
        // of elements per process 
        double percentage = (double(nElements) / double( nRanks ));
        percentage -= minPerProc ;
        percentage *= nRanks ;

        // per default every entity is on rank 0 
        partition_.resize( indexSet_.size( 0 ) );
        std::fill( partition_.begin(), partition_.end(), 0 );

        int rank = 0;
        size_t elementCount  = maxPerProc ;
        size_t elementNumber = 0;
        size_t localElementNumber = 0;
        const int lastRank = nRanks - 1;
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
          alugrid_assert ( rank < nRanks );
          partition_[ index ] = rank ;

          // increase counters 
          ++elementNumber;
          ++localElementNumber; 
        }
      }
      
      const CollectiveCommunication& comm_;

      const GridView& gridView_;
      const IndexSet &indexSet_;

      // load balancer bounds 
      std::vector< int > partition_;
    };

  public:
    typedef typename Grid::ctype ctype;
    typedef typename MPIHelper :: MPICommunicator MPICommunicatorType ;

    // type of communicator 
    typedef Dune :: CollectiveCommunication< MPICommunicatorType >
        CollectiveCommunication ;

    static GridPtr< Grid > 
    createCubeGrid( const std::string& filename,  
                    MPICommunicatorType mpiComm = MPIHelper :: getCommunicator() )       
    {
      std::ifstream file( filename.c_str() );
      if( ! file ) 
      {
        DUNE_THROW(InvalidStateException,"file not found " << filename );
      }
      return createCubeGrid( file, filename, mpiComm );
    }

    static GridPtr< Grid > 
    createCubeGrid( std::istream& input,  
                    const std::string& name, 
                    MPICommunicatorType mpiComm = MPIHelper :: getCommunicator() )       
    {
      CollectiveCommunication comm( MPIHelper :: getCommunicator() );
      const int myrank = comm.rank();

      typedef SGrid< dim, dimworld, ctype > SGridType ;
      // only work for the new ALUGrid version 
      // if creation of SGrid fails the DGF file does not contain a proper
      // IntervalBlock, and thus we cannot create the grid parallel, 
      // we will use the standard technique 
      bool sgridCreated = true ;
      array<int, dim> dims;
      FieldVector<ctype, dimworld> lowerLeft ( 0 ); 
      FieldVector<ctype, dimworld> upperRight( 0 ); 
      if( myrank == 0 ) 
      {
        GridPtr< SGridType > sPtr;
        try 
        { 
          sPtr = GridPtr< SGridType >( input, mpiComm );
        }
        catch ( DGFException & e ) 
        {
          sgridCreated = false ;
          std::cout << "Caught DGFException on creation of SGrid, trying default DGF method!" << std::endl;
        }
        if( sgridCreated ) 
        { 
          SGridType& sgrid = *sPtr ;
          dims = sgrid.dims( 0 );
          lowerLeft  = sgrid.lowerLeft();
          upperRight = sgrid.upperRight();
        }
      }

      // get global min to be on the same path
      sgridCreated = comm.min( sgridCreated );
      if( ! sgridCreated ) 
      {
        // use traditional DGF method
        return GridPtr< Grid >( input, mpiComm );
      }
      else 
      { 
        // broadcast array values 
        comm.broadcast( &dims[ 0 ], dim, 0 );
        comm.broadcast( &lowerLeft [ 0 ], dim, 0 );
        comm.broadcast( &upperRight[ 0 ], dim, 0 );
      }

      std::string nameS( name );
      nameS += " via SGrid";
      typedef StructuredGridFactory< Grid > SGF;
      return SGF :: createCubeGridImpl( lowerLeft, upperRight, dims, comm, nameS );
    }

    template < class int_t >
    static GridPtr< Grid > 
    createCubeGrid ( const FieldVector<ctype,dimworld>& lowerLeft,
                     const FieldVector<ctype,dimworld>& upperRight,
                     const array< int_t, dim>& elements, 
                     MPICommunicatorType mpiComm = MPIHelper :: getCommunicator() )       
    {
      CollectiveCommunication comm( mpiComm );
      std::string name( "Cartesian ALUGrid via SGrid" );
      return createCubeGridImpl( lowerLeft, upperRight, elements, comm, name );
    }

  protected:  
    template < class int_t >
    static GridPtr< Grid > 
    createCubeGridImpl ( const FieldVector<ctype,dimworld>& lowerLeft,
                         const FieldVector<ctype,dimworld>& upperRight,
                         const array< int_t, dim>& elements, 
                         const CollectiveCommunication& comm,
                         const std::string& name ) 
    {
      const int myrank = comm.rank();

      typedef SGrid< dim, dimworld, ctype > SGridType ;
      FieldVector< int, dim > dims; 
      for( int i=0; i<dim; ++i ) dims[ i ] = elements[ i ];

      // create SGrid to partition and insert elements that belong to process directly 
      SGridType sgrid( dims, lowerLeft, upperRight );  

      typedef typename SGridType :: LeafGridView GridView ;
      typedef typename GridView  :: IndexSet  IndexSet ;
      typedef typename IndexSet  :: IndexType IndexType ;
      typedef typename GridView  :: template Codim< 0 > :: Iterator ElementIterator ;
      typedef typename ElementIterator::Entity  Entity ;
      typedef typename Entity::EntityPointer    EntityPointer ;
      typedef typename GridView :: IntersectionIterator     IntersectionIterator ;
      typedef typename IntersectionIterator :: Intersection Intersection ;

      GridView gridView = sgrid.leafView();
      const IndexSet& indexSet = gridView.indexSet();

      // get decompostition of the marco grid 
      SimplePartitioner< GridView, InteriorBorder_Partition > partitioner( gridView, comm );

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

        alugrid_assert ( numVertices == entity.template count< dim >() );
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

        alugrid_assert ( numVertices == entity.template count< dim >() );
        for( int i = 0; i < numVertices; ++i )
          vertices[ i ] = vertexId[ indexSet.subIndex( entity, i, dim ) ];
        factory.insertElement( entity.type(), vertices );

        const IntersectionIterator iend = gridView.iend( entity );
        for( IntersectionIterator iit = gridView.ibegin( entity ); iit != iend; ++iit )
        {
          const Intersection& intersection = *iit ;
          const int face = intersection.indexInInside();
          // insert boundary face in case of domain boundary 
          if( intersection.boundary() )
            factory.insertBoundary( elIndex, face, intersection.boundaryId() );
          // insert process boundary in case the neighboring element has a different rank
          if( intersection.neighbor() ) 
          {
            EntityPointer outside = intersection.outside();
            if( partitioner.rank( *outside ) != myrank ) 
            {
              factory.insertProcessBorder( elIndex, face );
            }
          }
        }
        ++elIndex;
      }

      // create grid pointer (behaving like a shared_ptr) 
      return GridPtr< Grid> ( factory.createGrid( true, true, name ) );
    }
  };

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_STRUCTUREDGRIDFACTORY_HH
