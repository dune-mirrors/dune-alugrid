	// (c) bernhard schupp, 1997 - 1998
	//
	// $Source$
	// $Revision$
	// $Name$
	// $State$
	// $Date$

/* $Id$
 * $Log$
 * Revision 1.1  2004/10/25 16:39:54  robertk
 * Some off the headers are old and changed from .hh to .h.
 * All changes are made in the headers aswell.
 *
 * Some new file are the parallel grid files mostly have a _pll_ in then name.
 * There some Constructors of Tetra and Hbdn3Top had to be adapted.
 *
 * Revision 1.1  2004/10/15 09:48:38  robertk
 * Inititial version. Some extenxions for Dune made. Schould be compatible
 * with all other applications done so far.
 *
 * Revision 1.2  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

#ifndef PARALLEL_H_INCLUDED
#define PARALLEL_H_INCLUDED

static volatile char RCSId_parallel_h [] = "$Id$" ;

typedef class VertexPllXIF  VertexPllXIF_t ;
typedef class EdgePllXIF    EdgePllXIF_t ;
typedef class FacePllXIF    FacePllXIF_t ;
typedef class ElementPllXIF ElementPllXIF_t ;

class Parallel {
  public :

    class AccessPllException {} ;
  
    class VertexIF {
      public :
        typedef class Key1SLZ identifier_t ;
        inline virtual VertexPllXIF_t & accessPllX () throw (AccessPllException) ;
        inline virtual const VertexPllXIF_t & accessPllX () const throw (AccessPllException) ;
	inline virtual void detachPllXFromMacro () throw (AccessPllException) ;
    } ;
    class EdgeIF {
      public :
        typedef class Key2SLZ identifier_t ;
        inline virtual EdgePllXIF_t & accessPllX () throw (AccessPllException) ;
        inline virtual const EdgePllXIF_t & accessPllX () const throw (AccessPllException) ;
	inline virtual void detachPllXFromMacro () throw (AccessPllException) ;
    } ;
    class FaceIF {
      public :
        typedef class Key3SLZ identifier_t ;
        inline virtual FacePllXIF_t & accessPllX () throw (AccessPllException) ;
        inline virtual const FacePllXIF_t & accessPllX () const throw (AccessPllException) ;
	inline virtual void detachPllXFromMacro () throw (AccessPllException) ;
    } ;
    class ElementIF {
      public :
        inline virtual ElementPllXIF_t & accessPllX () throw (AccessPllException) ;
        inline virtual const ElementPllXIF_t & accessPllX () const throw (AccessPllException) ;
	inline virtual void detachPllXFromMacro () throw (AccessPllException) ;
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


inline VertexPllXIF_t & Parallel :: VertexIF :: accessPllX () throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ;
}

inline const VertexPllXIF_t & Parallel :: VertexIF :: accessPllX () const throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ;
}

inline void Parallel :: VertexIF :: detachPllXFromMacro () throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ;
}

inline EdgePllXIF_t & Parallel :: EdgeIF :: accessPllX () throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ;
}

inline const EdgePllXIF_t & Parallel :: EdgeIF :: accessPllX () const throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ; 
}

inline void Parallel :: EdgeIF :: detachPllXFromMacro () throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ;
}

inline FacePllXIF_t &  Parallel :: FaceIF :: accessPllX () throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ;
}

inline const FacePllXIF_t & Parallel :: FaceIF :: accessPllX () const throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ;
}

inline void Parallel :: FaceIF :: detachPllXFromMacro () throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ;
}

inline ElementPllXIF_t &  Parallel :: ElementIF :: accessPllX () throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ;
}

inline const ElementPllXIF_t & Parallel :: ElementIF :: accessPllX () const throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ;
}

inline void Parallel :: ElementIF :: detachPllXFromMacro () throw (AccessPllException) {
  assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
  throw AccessPllException () ;
}
#endif
