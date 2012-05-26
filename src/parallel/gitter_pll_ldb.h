// (c) bernhard schupp 1997 - 1999
#ifndef GITTER_PLL_LDB_H_INCLUDED
#define GITTER_PLL_LDB_H_INCLUDED
 
#include "key.h"
#include "mpAccess.h"

class LoadBalancer {
  public :
    static inline bool debugOption (int = 0) ;
  public :
    class GraphEdge : public Serializable 
    {
      int _leftNode, _rightNode, _weight ;
      public :
        inline GraphEdge () ;
        inline GraphEdge (int,int,int) ;
        inline virtual ~GraphEdge () ;
        inline int leftNode () const ;
        inline int rightNode () const ;
        inline int weight () const ;
        inline bool operator < (const GraphEdge &) const ;
        inline bool operator == (const GraphEdge &) const ;
        inline GraphEdge operator - () const ;
        inline bool isValid () const ;
        inline bool readObject (ObjectStream &) ;
        inline void writeObject (ObjectStream &) const ;
    } ;

    class GraphVertex : public Serializable 
    {
#ifdef GRAPHVERTEX_WITH_CENTER
      alucoord_t  _center [3] ;  // Schwerpunktskoordinaten
#endif
      int _index, _weight ; // globale Nummer, Gewicht
      public :
        inline GraphVertex () ;
        inline GraphVertex (int,int,const alucoord_t (&)[3]) ;
        inline GraphVertex (int,int) ;
        // constructor without center is initializing center and weight to zero 
        inline GraphVertex (int) ;
        inline virtual ~GraphVertex () ;
        inline int index () const ;
        inline int weight () const ;
#ifdef GRAPHVERTEX_WITH_CENTER
        inline const alucoord_t (&center () const)[3] ;
#endif
        inline bool operator < (const GraphVertex &) const ;
        inline bool operator == (const GraphVertex &) const ;
        inline bool isValid () const ;
        inline bool readObject (ObjectStream &) ;
        inline void writeObject (ObjectStream &) const ;
    } ;

  public :
    class DataBase {
      public :
        typedef map < GraphVertex, int, less < GraphVertex > >  ldb_vertex_map_t ;
        typedef set < GraphEdge, less < GraphEdge > >           ldb_edge_set_t ;
        typedef vector< int >                                   ldb_vector_t ;
      public :
        class AccVertexLoad {
          public :
            inline int operator () (int,const pair < const GraphVertex, int > &) const ;
        } ;
        class AccEdgeLoad {
          public :
            inline int operator () (int, const GraphEdge &) const ;
        } ;
      private :
        int _minFaceLoad ;
        int _maxFaceLoad ;
        int _minVertexLoad ;
        int _maxVertexLoad ;
        set < int, less < int > > _connect ;
        ldb_edge_set_t   _edgeSet ;
        ldb_vertex_map_t _vertexSet ;
      public :
        enum method { 
          // no load balancing 
          NONE = 0,

          // collect all to rank 0 
          COLLECT = 1,

          // METIS method for graph partitioning 
          METIS_PartGraphKway = 11,
          METIS_PartGraphRecursive = 12,

          // parallel partkway
          ParMETIS_V3_PartKway = 13,

          // ParMETIS method for adaptive repartition 
          ParMETIS_V3_AdaptiveRepart = 14   
        } ;
      private :
        template <class idx_t>
        void graphCollect (const MpAccessGlobal &,insert_iterator < ldb_vertex_map_t >,
                           insert_iterator < ldb_edge_set_t >,
                           idx_t* , const bool ) const ;

        template <class idx_t>
        void graphCollectAllgather (const MpAccessGlobal &,insert_iterator < ldb_vertex_map_t >,
                                    insert_iterator < ldb_edge_set_t >,
                                    idx_t* , const bool ) const ;

        template <class idx_t>
        void graphCollectBcast (const MpAccessGlobal &,insert_iterator < ldb_vertex_map_t >,
                                insert_iterator < ldb_edge_set_t >,
                                idx_t* , const bool ) const ;
      public :
        static const char * methodToString (method) ;
        inline DataBase () ;
        inline DataBase (const DataBase &) ;
        inline virtual ~DataBase () ;
        inline int nEdges () const ;
        inline int nVertices () const ;
        void edgeUpdate (const GraphEdge &) ;
        void vertexUpdate (const GraphVertex &) ;
        void printLoad () const ;
        int accVertexLoad ()const ;
        int accEdgeLoad () const ;
        inline int maxVertexLoad () const ;
      public :
        int getDestination (int) const ;
        set < int, less < int > > scan () const ;
        // original repartition method for ALUGrid 
        bool repartition (MpAccessGlobal &, method) ;
        // repartition to be called from outside (communicator ALU2d)
        vector< int > repartition (MpAccessGlobal &, 
                                   method,
                                   const int ) ;
      protected:  
        // implementation of repartition 
        bool repartition (MpAccessGlobal &, method,
                          vector< int >& , const int ) ;
    } ;
} ;

  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //

inline bool LoadBalancer :: debugOption (int level) {
  return (getenv ("VERBOSE_LDB") ? ( atoi (getenv ("VERBOSE_LDB")) > level ? true : (level == 0)) : false) ;
}

inline LoadBalancer :: GraphEdge :: GraphEdge () : _leftNode (-1), _rightNode (-1), _weight (0) {
}

inline LoadBalancer :: GraphEdge :: GraphEdge (int i, int j, int w) : _leftNode (i), _rightNode (j), _weight (w) {
}

inline LoadBalancer :: GraphEdge :: ~GraphEdge () {
}

inline int LoadBalancer :: GraphEdge :: leftNode () const {
  return _leftNode ;
}

inline int LoadBalancer :: GraphEdge :: rightNode () const {
  return _rightNode ;
} 

inline int LoadBalancer :: GraphEdge :: weight () const {
  return _weight ;
}

inline bool LoadBalancer :: GraphEdge :: isValid () const {
  return !(_leftNode < 0 || _rightNode < 0 || _weight <= 0) ;
}

inline bool LoadBalancer :: GraphEdge :: operator < (const GraphEdge & x) const {
  return _leftNode < x._leftNode ? true : (_leftNode == x._leftNode ? (_rightNode < x._rightNode ? true : false) : false) ;
}

inline bool LoadBalancer :: GraphEdge :: operator == (const GraphEdge & x) const {
  return _leftNode == x._leftNode  && _rightNode == x._rightNode ;
}

inline LoadBalancer :: GraphEdge LoadBalancer :: GraphEdge :: operator - () const {
  return GraphEdge (_rightNode, _leftNode, _weight) ;
} 

inline bool LoadBalancer :: GraphEdge :: readObject (ObjectStream & os) {
  os.readObject (_leftNode) ;
  os.readObject (_rightNode) ;
  os.readObject (_weight) ;
  return true ;
}

inline void LoadBalancer :: GraphEdge :: writeObject (ObjectStream & os) const {
  os.writeObject (_leftNode) ;
  os.writeObject (_rightNode) ;
  os.writeObject (_weight) ;
  return ;
}

inline LoadBalancer :: GraphVertex :: ~GraphVertex () {
}

inline LoadBalancer :: GraphVertex :: GraphVertex () 
  : _index (-1) , _weight (0) {
#ifdef GRAPHVERTEX_WITH_CENTER
  _center [0] = _center [1] = _center [2] = 0.0 ;
#endif
  return ;
}

inline LoadBalancer :: GraphVertex :: GraphVertex (int i, int w, const alucoord_t (&p)[3]) 
: _index (i), _weight (w) 
{
#ifdef GRAPHVERTEX_WITH_CENTER
  _center [0] = p [0] ;
  _center [1] = p [1] ;
  _center [2] = p [2] ;
#endif
  return ;
}

inline LoadBalancer :: GraphVertex :: GraphVertex (int i, int w)
: _index (i), _weight (w) 
{
#ifdef GRAPHVERTEX_WITH_CENTER
  _center [0] = _center [1] = _center [2] = 0.0 ;
#endif
  return ;
}

inline LoadBalancer :: GraphVertex :: GraphVertex (int i) 
  : _index (i), _weight (0) 
{
#ifdef GRAPHVERTEX_WITH_CENTER
  _center [0] = _center [1] = _center [2] = 0.0 ;
#endif
  return ;
}

inline int LoadBalancer :: GraphVertex :: index () const {
  return _index ;
}

inline int LoadBalancer :: GraphVertex :: weight () const {
  return _weight ;
}

#ifdef GRAPHVERTEX_WITH_CENTER
inline const alucoord_t (& LoadBalancer :: GraphVertex :: center () const)[3] {
  return _center ;
}
#endif

inline bool LoadBalancer :: GraphVertex :: isValid () const {
  return !(_index < 0 || _weight <= 0) ;
}

inline bool LoadBalancer :: GraphVertex :: operator < (const GraphVertex & x) const {
  return _index < x._index ;
}

inline bool LoadBalancer :: GraphVertex :: operator == (const GraphVertex & x) const {
  return _index == x._index ;
}

inline bool LoadBalancer :: GraphVertex :: readObject (ObjectStream & os) {
  os.readObject (_index) ;
  os.readObject (_weight) ;
#ifdef GRAPHVERTEX_WITH_CENTER
  os.readObject (_center [0]) ;
  os.readObject (_center [1]) ;
  os.readObject (_center [2]) ;
#endif
  return true ;
}

inline void LoadBalancer :: GraphVertex :: writeObject (ObjectStream & os) const 
{
  os.writeObject (_index) ;
  os.writeObject (_weight) ;
#ifdef GRAPHVERTEX_WITH_CENTER
  os.writeObject (_center [0]) ;
  os.writeObject (_center [1]) ;
  os.writeObject (_center [2]) ;
#endif
  return ;
}

inline LoadBalancer :: DataBase :: DataBase () : _minFaceLoad (0), _maxFaceLoad (0), _minVertexLoad (0), _maxVertexLoad (0), 
  _edgeSet (), _vertexSet () {
  return ;
}

inline LoadBalancer :: DataBase :: DataBase (const DataBase & b) : _minFaceLoad (b._minFaceLoad), _maxFaceLoad (b._maxFaceLoad), 
    _minVertexLoad (b._minVertexLoad), _maxVertexLoad (b._maxVertexLoad), 
    _edgeSet (b._edgeSet), _vertexSet (b._vertexSet) {
  return ;
}

inline LoadBalancer :: DataBase :: ~DataBase () {
  return ;
}

inline int LoadBalancer :: DataBase :: nEdges () const {
  return _edgeSet.size () ;
}

inline int LoadBalancer :: DataBase :: nVertices () const {
  return _vertexSet.size () ;
}
 
inline int LoadBalancer :: DataBase :: AccVertexLoad :: operator () (int s,const pair < const GraphVertex, int > & p) const {
  return s + p.first.weight () ;
}

inline int LoadBalancer :: DataBase :: AccEdgeLoad :: operator () (int s, const GraphEdge & e) const {
  return s + e.weight () ;
}

inline int LoadBalancer :: DataBase :: maxVertexLoad () const {
  return _maxVertexLoad ;
}

#endif

