#ifndef DUNE_ALUGRID_STRUCTUREDGRIDFACTORY_HH
#define DUNE_ALUGRID_STRUCTUREDGRIDFACTORY_HH

#include <vector>

#include <dune/common/array.hh>
#include <dune/common/version.hh>

#include <dune/common/classname.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/shared_ptr.hh>

#include <dune/grid/common/gridfactory.hh>
#include <dune/grid/common/exceptions.hh>

#include <dune/alugrid/common/alugrid_assert.hh>
#include <dune/alugrid/common/declaration.hh>

#include <dune/alugrid/common/hsfc.hh>

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
    // mygrid_ptr in GridPtr is a derived from std::shared_ptr and implements a method release
    typedef typename Dune::GridPtr< Grid > :: mygrid_ptr  SharedPtrType;

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

      typedef typename Element::Geometry::GlobalCoordinate VertexType;

      // type of communicator
      typedef Dune :: CollectiveCommunication< typename MPIHelper :: MPICommunicator >
        CollectiveCommunication ;

#ifdef USE_ALUGRID_SFC_ORDERING
      typedef SpaceFillingCurveOrdering< VertexType >  SpaceFillingCurveOrderingType;
#endif

    public:
      SimplePartitioner ( const GridView &gridView, const CollectiveCommunication& comm,
                          const VertexType& lowerLeft, const VertexType& upperRight )
      : comm_( comm ),
        gridView_( gridView ),
        indexSet_( gridView_.indexSet() ),
        pSize_( comm_.size() ),
        elementCuts_( pSize_, -1 ),
#ifdef USE_ALUGRID_SFC_ORDERING
        sfc_( lowerLeft, upperRight, comm_ ),
#endif
        maxIndex_( double(indexSet_.size(0)-1) )
      {
        // compute decomposition of sfc
        calculateElementCuts();
      }

    public:
      template< class Entity >
      int rank( const Entity &entity ) const
      {
        alugrid_assert ( Entity::codimension == 0 );
#ifdef USE_ALUGRID_SFC_ORDERING
        // get center of entity's geometry
        VertexType center = entity.geometry().center();
        // get hilbert index in [0,1]
        const double hidx = sfc_.index( center );
        // transform to element index
        const long int index = (hidx * maxIndex_);
#else
        const long int index = indexSet_.index( entity );
#endif
        return rank( index );
      }

    protected:
      int rank( long int index ) const
      {
        if( index < elementCuts_[ 0 ] ) return 0;
        for( int p=1; p<pSize_; ++p )
        {
          if( index >= elementCuts_[ p-1 ] && index < elementCuts_[ p ] )
            return p;
        }
        return pSize_-1;
      }

    protected:
      void calculateElementCuts()
      {
        const size_t nElements = indexSet_.size( 0 );

        // get number of MPI processes
        const int nRanks = pSize_;

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

        int rank = 0;
        size_t elementCount  = maxPerProc ;
        size_t elementNumber = 0;
        size_t localElementNumber = 0;
        const int lastRank = nRanks - 1;

        const size_t size = indexSet_.size( 0 );
        for( size_t i=0; i<size; ++i )
        {
          if( localElementNumber >= elementCount )
          {
            elementCuts_[ rank ] = i ;

            // increase rank
            if( rank < lastRank ) ++ rank;

            // reset local number
            localElementNumber = 0;

            // switch to smaller number if red line is crossed
            if( elementCount == maxPerProc && rank >= percentage )
              elementCount = minPerProc ;
          }

          // increase counters
          ++elementNumber;
          ++localElementNumber;
        }

        // set cut for last process
        elementCuts_[ lastRank ] = size ;

        //for( int p=0; p<pSize_; ++p )
        //  std::cout << "P[ " << p << " ] = " << elementCuts_[ p ] << std::endl;
      }

      const CollectiveCommunication& comm_;

      const GridView& gridView_;
      const IndexSet &indexSet_;

      const int pSize_;
      std::vector< long int > elementCuts_ ;

#ifdef USE_ALUGRID_SFC_ORDERING
      // get element to hilbert index mapping
      SpaceFillingCurveOrdering< VertexType > sfc_;
#endif
      const double maxIndex_ ;
    };

  public:
    typedef typename Grid::ctype ctype;
    typedef typename MPIHelper :: MPICommunicator MPICommunicatorType ;

    // type of communicator
    typedef Dune :: CollectiveCommunication< MPICommunicatorType >
        CollectiveCommunication ;

    static SharedPtrType
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

    static SharedPtrType
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
        GridPtr< Grid > grid( input, mpiComm );
        return SharedPtrType ( grid.release() );
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
    static SharedPtrType
    createSimplexGrid ( const FieldVector<ctype,dimworld>& lowerLeft,
                        const FieldVector<ctype,dimworld>& upperRight,
                        const array< int_t, dim>& elements,
                        MPICommunicatorType mpiComm = MPIHelper :: getCommunicator() )
    {
      // create DGF interval block and use DGF parser to create simplex grid
      std::stringstream dgfstream;
      dgfstream << "DGF" << std::endl;
      dgfstream << "Interval" << std::endl;
      dgfstream << lowerLeft  << std::endl;
      dgfstream << upperRight << std::endl;
      for( int i=0; i<dim; ++ i)
        dgfstream << elements[ i ] << " ";
      dgfstream << std::endl;
      dgfstream << "#" << std::endl;
      dgfstream << "Simplex" << std::endl;
      dgfstream << "#" << std::endl;

      Dune::GridPtr< Grid > grid( dgfstream, mpiComm );
      return std::make_shared( grid.release() );
    }

    template < class int_t >
    static SharedPtrType
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
    template <int codim, class Entity>
    int subEntities ( const Entity& entity ) const
    {
#if DUNE_VERSION_NEWER(DUNE_GRID,2,4)
      return entity.subEntities( codim );
#else
      return entity.template count< codim > ();
#endif
    }

    template < class int_t >
    static SharedPtrType
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
      typedef typename GridView :: IntersectionIterator     IntersectionIterator ;
      typedef typename IntersectionIterator :: Intersection Intersection ;

      GridView gridView = sgrid.leafGridView();
      const IndexSet &indexSet = gridView.indexSet();

      // get decompostition of the marco grid
      SimplePartitioner< GridView, InteriorBorder_Partition > partitioner( gridView, comm, lowerLeft, upperRight );

      // create ALUGrid GridFactory
      GridFactory< Grid > factory;

      // map global vertex ids to local ones
      std::map< IndexType, unsigned int > vtxMap;

      const int numVertices = (1 << dim);
      std::vector< unsigned int > vertices( numVertices );

      int nextElementIndex = 0;
      //const auto end = gridView.template end< 0 >();
      //for( auto it = gridView.template begin< 0 >(); it != end; ++it )
      const ElementIterator end = gridView.template end< 0 >();
      for( ElementIterator it = gridView.template begin< 0 >(); it != end; ++it )
      {
        const Entity &entity = *it;
        // if the element does not belong to our partition, continue
        if( partitioner.rank( entity ) != myrank )
          continue;

        // insert vertices and element
        const typename Entity::Geometry geo = entity.geometry();
        alugrid_assert( numVertices == geo.corners() );
        for( int i = 0; i < numVertices; ++i )
        {
          const IndexType vtxId = indexSet.subIndex( entity, i, dim );
          //auto result = vtxMap.insert( std::make_pair( vtxId, vtxMap.size() ) );
          std::pair< typename std::map< IndexType, unsigned int >::iterator, bool > result
            = vtxMap.insert( std::make_pair( vtxId, vtxMap.size() ) );
          if( result.second )
            factory.insertVertex( geo.corner( i ), vtxId );
          vertices[ i ] = result.first->second;
        }
        factory.insertElement( entity.type(), vertices );
        const int elementIndex = nextElementIndex++;

        //const auto iend = gridView.iend( entity );
        //for( auto iit = gridView.ibegin( entity ); iit != iend; ++iit )
        const IntersectionIterator iend = gridView.iend( entity );
        for( IntersectionIterator iit = gridView.ibegin( entity ); iit != iend; ++iit )
        {
          const Intersection &isec = *iit;
          const int faceNumber = isec.indexInInside();
          // insert boundary face in case of domain boundary
          if( isec.boundary() )
            factory.insertBoundary( elementIndex, faceNumber );
          // insert process boundary if the neighboring element has a different rank
#if DUNE_VERSION_NEWER(DUNE_GRID,2,4)
          if( isec.neighbor() && (partitioner.rank( isec.outside() ) != myrank) )
#else
          if( isec.neighbor() && (partitioner.rank( *isec.outside() ) != myrank) )
#endif
            factory.insertProcessBorder( elementIndex, faceNumber );
        }
      }

      // create shared grid pointer
      return SharedPtrType( factory.createGrid( true, true, name ) );
    }
  };

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_STRUCTUREDGRIDFACTORY_HH
