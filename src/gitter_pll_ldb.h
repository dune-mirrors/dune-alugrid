	// (c) bernhard schupp 1997 - 1999

	// $Source$
	// $Revision$
	// $Name$
	// $State$

/* $Id$
 * $Log$
 * Revision 1.1  2004/10/25 16:39:53  robertk
 * Some off the headers are old and changed from .hh to .h.
 * All changes are made in the headers aswell.
 *
 * Some new file are the parallel grid files mostly have a _pll_ in then name.
 * There some Constructors of Tetra and Hbdn3Top had to be adapted.
 *
 * Revision 1.4  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.3  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

#ifndef GITTER_PLL_LDB_H_INCLUDED
#define GITTER_PLL_LDB_H_INCLUDED
 
#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <assert.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <set>
  #include <map>
  #include <iterator>
#else
  #include <set.h>
  #include <map.h>
  #include <iterator.h>
#endif

#include "key.h"
#include "mpAccess.h"

static volatile char RCSId_gitter_pll_ldb_h [] = "$Id$" ;

class LoadBalancer {
  public :
    static inline bool debugOption (int = 0) ;
  public :
    class GraphEdge : public Serializable {
      int _leftNode, _rightNode, _weight ;
      public :
        inline GraphEdge () ;
        inline GraphEdge (int,int,int) ;
        inline ~GraphEdge () ;
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
    class GraphVertex : public Serializable {
      int _index, _weight ;	// globale Nummer, Gewicht
      double _center [3] ;	// und Schwerpunktskoordinaten
      public :
        inline GraphVertex () ;
        inline GraphVertex (int,int,const double (&)[3]) ;
        inline ~GraphVertex () ;
        inline int index () const ;
        inline int weight () const ;
	inline const double (&center () const)[3] ;
        inline bool operator < (const GraphVertex &) const ;
	inline bool operator == (const GraphVertex &) const ;
        inline bool isValid () const ;
	inline bool readObject (ObjectStream &) ;
	inline void writeObject (ObjectStream &) const ;
    } ;
  public :
    class DataBase {
      public :
        typedef map < GraphVertex, int, less < GraphVertex > > 	ldb_vertex_map_t ;
        typedef set < GraphEdge, less < GraphEdge > > 		ldb_edge_set_t ;
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
        ldb_edge_set_t 	 _edgeSet ;
        ldb_vertex_map_t _vertexSet ;
      private :
	void graphCollect (const MpAccessGlobal &,insert_iterator < ldb_vertex_map_t >,
		insert_iterator < ldb_edge_set_t >) const ;
      public :
        enum method { 
	              NONE = 0,
		      COLLECT = 1,
		      PARTY_helpfulSet = 3, 
	              PARTY_kernighanLin = 4,
		      PARTY_linear = 7,
		      PARTY_gain = 8,
		      PARTY_farhat = 9,
		      METIS_PartGraphKway = 11,
		      METIS_PartGraphRecursive = 12
		    } ;
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
        bool repartition (MpAccessGlobal &, method) ;
	int getDestination (int) const ;
	set < int, less < int > > scan () const ;
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

inline LoadBalancer :: GraphVertex :: GraphVertex () : _index (-1), _weight (0.0) {
  _center [0] = _center [1] = _center [2] = 0.0 ;
  return ;
}

inline LoadBalancer :: GraphVertex :: GraphVertex (int i, int w, const double (&p)[3]) : _index (i), _weight (w) {
  _center [0] = p [0] ;
  _center [1] = p [1] ;
  _center [2] = p [2] ;
  return ;
}

inline int LoadBalancer :: GraphVertex :: index () const {
  return _index ;
}

inline int LoadBalancer :: GraphVertex :: weight () const {
  return _weight ;
}

inline const double (& LoadBalancer :: GraphVertex :: center () const)[3] {
  return _center ;
}

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
  os.readObject (_center [0]) ;
  os.readObject (_center [1]) ;
  os.readObject (_center [2]) ;
  return true ;
}

inline void LoadBalancer :: GraphVertex :: writeObject (ObjectStream & os) const {
  os.writeObject (_index) ;
  os.writeObject (_weight) ;
  os.writeObject (_center [0]) ;
  os.writeObject (_center [1]) ;
  os.writeObject (_center [2]) ;
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

