// (c) bernhard schupp 1997 - 1999
#ifndef GITTER_PLL_LDB_H_INCLUDED
#define GITTER_PLL_LDB_H_INCLUDED

#include <map>
#include <set>
#include <vector>

namespace ALUGrid
{

  //! type of coordinate storage
  typedef double alucoord_t;

} // namespace ALUGrid

#if HAVE_ZOLTAN
#define GRAPHVERTEX_WITH_CENTER
#endif

#include "../serial/serialize.h"
#include "../serial/key.h"
#include "../serial/myalloc.h"
#include "../serial/parallel.h"
#include "mpAccess.h"

namespace ALUGrid
{

  class LoadBalancer
  {
    public :
      static inline bool debugOption (int = 0);
    public :
      class GraphEdge
      : public MyAlloc
      {
        int _leftNode;
        int _rightNode;
        int _weight;
        int _leftMaster;
        int _rightMaster;
      public :
          static const int sizeOfData = 5 * sizeof( int );
          explicit GraphEdge ( ObjectStream& os );
          inline GraphEdge (int,int,int,int,int);
          inline int leftNode () const;
          inline int rightNode () const;
          inline int weight () const;
          inline bool operator < (const GraphEdge &) const;
          inline bool operator == (const GraphEdge &) const;
          inline GraphEdge operator - () const;
          inline bool isValid () const;
          inline bool readFromStream (ObjectStream &);
          inline void writeToStream  (ObjectStream &) const;
          inline int leftMaster () const;
          inline int rightMaster () const;
        private:
      };

      class GraphVertex : public MyAlloc
      {
        typedef alucoord_t center_t[ 3 ];

        int _index;   // global graph index
        int _weight;  // weight of vertex

#ifdef GRAPHVERTEX_WITH_CENTER
        const MacroGridMoverIF* _element ; // for computing the bary center if necessary
#endif

        public :
          static const int sizeOfData = 2 * sizeof(int);

          explicit GraphVertex( ObjectStream& os );
          // contructor taking linked object (for barycenter calculation)
          inline GraphVertex (int,int, const MacroGridMoverIF& );
          inline GraphVertex (int,int);
          // constructor without center is initializing center and weight to zero
          inline GraphVertex (int);
          inline int index () const;
          inline int weight () const;
          inline void computeBaryCenter( center_t& center ) const;
          inline bool operator < (const GraphVertex &) const;
          inline bool operator == (const GraphVertex &) const;
          inline bool isValid () const;
          inline bool readFromStream (ObjectStream &);
          inline void writeToStream  (ObjectStream &) const;
      };

    public:
      class DataBase
      {
        public :
          typedef std::map< GraphVertex, int > ldb_vertex_map_t;
          typedef std::set< GraphEdge >        ldb_edge_set_t;
          typedef std::vector< int >           ldb_vector_t;

        public :
          class AccVertexLoad
          {
          public :
            int operator () ( int, const std::pair< const GraphVertex, int > & ) const;
          };

          class AccEdgeLoad
          {
          public :
            int operator () ( int, const GraphEdge & ) const;
          };

          typedef std::set< int > ldb_connect_set_t;

        private :
          int _minFaceLoad;
          int _maxFaceLoad;
          int _minVertexLoad;
          int _maxVertexLoad;
          ldb_connect_set_t  _connect;
          ldb_edge_set_t     _edgeSet;
          mutable ldb_vertex_map_t   _vertexSet;
          const ldb_vertex_map_t :: iterator _vertexSetEnd ;

          // contains the sizes of the partition (vertices and edges of each proc)
          // if this is zero, then the sizes will be communicated
          std::vector< int > _graphSizes;
          std::vector< int > _elementCuts;
          // true if no periodic faces are present
          mutable bool _noPeriodicFaces;
        public :
          enum method {
            // no load balancing
            NONE = 0,

            // collect all to rank 0
            COLLECT = 1,

            // assuming the elements to be ordered by a
            // space filling curve approach
            // here, the edges in the graph are neglected
            // parallel version
            ALUGRID_SpaceFillingCurveLinkage = 4,
            // serial version that requires the whole graph to be avaiable
            ALUGRID_SpaceFillingCurveSerialLinkage = 5,

            // METIS method for graph partitioning (with linkage storage)
            //METIS_PartGraphKwayLinkage      = 6,
            //METIS_PartGraphRecursiveLinkage = 7,

            // ALU sfc without linkage
            ALUGRID_SpaceFillingCurve       = 9,
            ALUGRID_SpaceFillingCurveSerial = 10,

            // METIS method for graph partitioning
            METIS_PartGraphKway = 11,
            METIS_PartGraphRecursive = 12,

            // ZOLTAN partitioning
            ZOLTAN_LB_HSFC = 13 ,
            ZOLTAN_LB_GraphPartitioning = 14 ,
            ZOLTAN_LB_PARMETIS = 15
          };
        private :
          void graphCollect ( const MpAccessGlobal &,
                              std::insert_iterator< ldb_edge_set_t > );

          void graphCollectAllgather ( const MpAccessGlobal &,
                                       std::insert_iterator< ldb_edge_set_t > );

          void graphCollectBcast ( const MpAccessGlobal &,
                                   std::insert_iterator < ldb_edge_set_t > );
        public :
          // ALUGRID_SpaceFillingCurveLinkage,
          // ALUGRID_SpaceFillingCurveSerialLinkage, and
          // ALUGRID_SpaceFillingCurve do not work when periodic
          // boundaries are present, one needs to select one of the other
          // methods, see above
          bool methodConsitentWithPeriodicBnd( const method mth )
          {
            return mth < ALUGRID_SpaceFillingCurveLinkage ||
                   mth > ALUGRID_SpaceFillingCurve;
          }

          const std::vector< int > &graphSizes () const { return _graphSizes; }

          void clearGraphSizesVector ()
          {
            // clear graph size and also deallocate memory
            std::vector< int >().swap( _graphSizes );
            _noPeriodicFaces = false;
          }

          //! return true if mth specifies a serial partitioner
          static bool serialPartitionerUsed ( const method mth )
          {
            return mth < ZOLTAN_LB_HSFC ;
          }

          //! return true if linkage should be stored in vertices
          static bool storeLinkageInVertices ( const method mth )
          {
            return mth < ALUGRID_SpaceFillingCurve ;
          }

          //! return true if mth specifies a serial partitioner
          static bool graphEdgesNeeded ( const method mth )
          {
            return //mth == METIS_PartGraphKwayLinkage ||
                   //mth == METIS_PartGraphRecursiveLinkage ||
                   mth > ALUGRID_SpaceFillingCurveSerial &&
                   mth != ZOLTAN_LB_HSFC;
          }

          static bool initializeZoltan( const method mth );

          static std::string methodToString( method );

          DataBase ();
          explicit DataBase ( const std::vector< int > &graphSizes,
                              std::vector< int > &elementCuts );
          DataBase ( const DataBase & );

          virtual ~DataBase ();

          int nEdges () const;
          int nVertices () const;
          void edgeUpdate (const GraphEdge &);
          void vertexUpdate (const GraphVertex &);
          void insertVertex (const GraphVertex &, const int );
          void printLoad () const;
          int accVertexLoad ()const;
          int accEdgeLoad () const;
          inline int maxVertexLoad () const;
        public :
          template <class helement_t, class gatherscatter_t >
          int destination (helement_t& elem, gatherscatter_t* gs ) const
          {
            // if gs is given use this to obtain detination
            return ( gs ) ? gs->destination( elem ) :
                            destination( elem.ldbVertexIndex() );
          }

          int destination (int) const;

          const ldb_connect_set_t& scan () const { return _connect; }

          // original repartition method for ALUGrid
          bool repartition (MpAccessGlobal &, method, const double tolerance = 1.2 );

          std::vector< int > repartition ( MpAccessGlobal &, method, int, const double tolerance = 1.2  );

          void printVertexSet() const;

          // store internal element cuts in elementCuts
          void storeElementCuts( std::vector<int>& elementCuts )
          {
            elementCuts.swap( _elementCuts );
          }
        protected:
          bool repartition ( MpAccessGlobal &, method, std::vector< int > &, const int, const double );
      };
  };

    //
    //    #    #    #  #          #    #    #  ######
    //    #    ##   #  #          #    ##   #  #
    //    #    # #  #  #          #    # #  #  #####
    //    #    #  # #  #          #    #  # #  #
    //    #    #   ##  #          #    #   ##  #
    //    #    #    #  ######     #    #    #  ######
    //

  inline bool LoadBalancer::debugOption (int level) {
#ifdef ALUGRIDDEBUG
    return (getenv ("VERBOSE_LDB") ? ( atoi (getenv ("VERBOSE_LDB")) > level ? true : (level == 0)) : false);
#else
    return false ;
#endif
  }

  inline LoadBalancer::GraphEdge::GraphEdge ( ObjectStream& os )
  {
    readFromStream( os );
  }

  inline LoadBalancer::GraphEdge::GraphEdge (int i, int j, int w, int lmaster, int rmaster)
    : _leftNode (i), _rightNode (j), _weight (w),
      _leftMaster(lmaster), _rightMaster(rmaster)
  {
    alugrid_assert ( _weight >= 0 );
  }

  inline int LoadBalancer::GraphEdge::leftNode () const {
    return _leftNode;
  }

  inline int LoadBalancer::GraphEdge::rightNode () const {
    return _rightNode;
  }

  inline int LoadBalancer::GraphEdge::weight () const {
    return _weight;
  }

  inline int LoadBalancer::GraphEdge::leftMaster () const {
    return _leftMaster;
  }
  inline int LoadBalancer::GraphEdge::rightMaster () const {
    return _rightMaster;
  }

  inline bool LoadBalancer::GraphEdge::isValid () const
  {
    return ( _leftNode >= 0 ) && ( _rightNode >= 0 ) && ( _weight > 0 );
  }

  inline bool LoadBalancer::GraphEdge::operator < (const GraphEdge & x) const {
    return _leftNode < x._leftNode ? true : (_leftNode == x._leftNode ? (_rightNode < x._rightNode ? true : false) : false);
  }

  inline bool LoadBalancer::GraphEdge::operator == (const GraphEdge & x) const {
    return _leftNode == x._leftNode  && _rightNode == x._rightNode;
  }

  inline LoadBalancer::GraphEdge LoadBalancer::GraphEdge::operator - () const {
    return GraphEdge (_rightNode, _leftNode, _weight, _rightMaster, _leftMaster);
  }

  inline bool LoadBalancer::GraphEdge::readFromStream (ObjectStream & os)
  {
    os.readObject ( _leftNode  );
    os.readObject ( _rightNode );
    os.readObject ( _weight );
    os.readObject( _leftMaster );
    os.readObject( _rightMaster );
    return true;
  }

  inline void LoadBalancer::GraphEdge::writeToStream (ObjectStream & os) const {
    os.writeObject (_leftNode);
    os.writeObject (_rightNode);
    os.writeObject (_weight);
    os.writeObject( _leftMaster );
    os.writeObject( _rightMaster );
    return;
  }

  inline LoadBalancer::GraphVertex::GraphVertex ( ObjectStream& os )
  {
    readFromStream( os );
  }

  inline LoadBalancer::GraphVertex::GraphVertex (int i, int w, const MacroGridMoverIF& obj )
  : _index (i), _weight (w)
#ifdef GRAPHVERTEX_WITH_CENTER
    , _element( &obj )
#endif
  {
    alugrid_assert ( _weight > 0 );
  }

  inline LoadBalancer::GraphVertex::GraphVertex (int i, int w)
  : _index (i), _weight (w)
#ifdef GRAPHVERTEX_WITH_CENTER
    , _element( 0 )
#endif
  {
    alugrid_assert ( _weight > 0 );
  }

  inline LoadBalancer::GraphVertex::GraphVertex (int i)
    : _index (i), _weight (1)
#ifdef GRAPHVERTEX_WITH_CENTER
      , _element( 0 )
#endif
  {
    alugrid_assert ( _weight > 0 );
  }

  inline int LoadBalancer::GraphVertex::index () const {
    return _index;
  }

  inline int LoadBalancer::GraphVertex::weight () const {
    alugrid_assert ( _weight > 0 );
    return _weight;
  }

  inline void LoadBalancer::GraphVertex::computeBaryCenter( center_t& center ) const
  {
#ifdef GRAPHVERTEX_WITH_CENTER
    assert( _element ) ;
    _element->computeBaryCenter( center );
#else
    center[ 2 ] = center[ 1 ] = center[ 0 ] = 0;
#endif
  }

  inline bool LoadBalancer::GraphVertex::isValid () const {
    return (_index >= 0 ) && ( _weight > 0 );
  }

  inline bool LoadBalancer::GraphVertex::operator < (const GraphVertex & x) const {
    return _index < x._index;
  }

  inline bool LoadBalancer::GraphVertex::operator == (const GraphVertex & x) const {
    return _index == x._index;
  }

  inline bool LoadBalancer::GraphVertex::readFromStream (ObjectStream & os)
  {
    os.readObject (_index);
    os.readObject (_weight);
    return true;
  }

  inline void LoadBalancer::GraphVertex::writeToStream (ObjectStream & os) const
  {
    os.writeObject (_index);
    os.writeObject (_weight);
    return;
  }

  inline LoadBalancer::DataBase::DataBase ()
    : _minFaceLoad (0), _maxFaceLoad (0), _minVertexLoad (0), _maxVertexLoad (0),
      _edgeSet (), _vertexSet (), _vertexSetEnd( _vertexSet.end() ),
      _graphSizes(), _noPeriodicFaces( true )
  {
  }

  inline LoadBalancer::DataBase::DataBase ( const std::vector< int > &graphSizes,
                                            std::vector< int > &elementCuts )
  : _minFaceLoad (0), _maxFaceLoad (0), _minVertexLoad (0), _maxVertexLoad (0),
    _edgeSet (), _vertexSet (), _vertexSetEnd( _vertexSet.end() ),
    _graphSizes( graphSizes ),
    _elementCuts(), _noPeriodicFaces( true )
  {
    // swap memory
    _elementCuts.swap( elementCuts );
  }

  inline LoadBalancer::DataBase::DataBase (const DataBase & b)
    : _minFaceLoad (b._minFaceLoad), _maxFaceLoad (b._maxFaceLoad),
      _minVertexLoad (b._minVertexLoad), _maxVertexLoad (b._maxVertexLoad),
      _edgeSet (b._edgeSet), _vertexSet (b._vertexSet) ,
      _vertexSetEnd( _vertexSet.end() ),
      _graphSizes( b._graphSizes ),
      _elementCuts( b._elementCuts ),
      _noPeriodicFaces( b._noPeriodicFaces )
  {
  }

  inline LoadBalancer::DataBase::~DataBase () {}

  inline int LoadBalancer::DataBase::nEdges () const { return _edgeSet.size(); }

  inline int LoadBalancer::DataBase::nVertices () const { return _vertexSet.size(); }

  inline int LoadBalancer::DataBase::AccVertexLoad::operator() ( int s, const std::pair< const GraphVertex, int > &p ) const
  {
    return s + p.first.weight();
  }

  inline int LoadBalancer::DataBase::AccEdgeLoad::operator () (int s, const GraphEdge & e) const {
    return s + e.weight ();
  }

  inline int LoadBalancer::DataBase::maxVertexLoad () const
  {
    return _maxVertexLoad;
  }

} // namespace ALUGrid

#endif // #ifndef GITTER_PLL_LDB_H_INCLUDED
