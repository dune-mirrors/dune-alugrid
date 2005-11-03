// (c) bernhard schupp, 1997 - 1998
//
#ifndef PARALLEL_H_INCLUDED
#define PARALLEL_H_INCLUDED

typedef class VertexPllXIF  VertexPllXIF_t ;
typedef class EdgePllXIF    EdgePllXIF_t ;
typedef class FacePllXIF    FacePllXIF_t ;
typedef class ElementPllXIF ElementPllXIF_t ;

class Parallel {
  public :

    class AccessPllException {} ;
  
    class VertexIF {
      public :
        virtual ~VertexIF () {}
        typedef class Key1SLZ identifier_t ;
        inline virtual VertexPllXIF_t & accessPllX () throw (AccessPllException) ;
        inline virtual const VertexPllXIF_t & accessPllX () const throw (AccessPllException) ;
	inline virtual void detachPllXFromMacro () throw (AccessPllException) ;
    } ;
    class EdgeIF {
      public :
        virtual ~EdgeIF () {}
        typedef class Key2SLZ identifier_t ;
        inline virtual EdgePllXIF_t & accessPllX () throw (AccessPllException) ;
        inline virtual const EdgePllXIF_t & accessPllX () const throw (AccessPllException) ;
	inline virtual void detachPllXFromMacro () throw (AccessPllException) ;
    } ;
    class FaceIF {
      public :
        virtual ~FaceIF () {}
        typedef class Key3SLZ identifier_t ;
        inline virtual FacePllXIF_t & accessPllX () throw (AccessPllException) ;
        inline virtual const FacePllXIF_t & accessPllX () const throw (AccessPllException) ;
	inline virtual void detachPllXFromMacro () throw (AccessPllException) ;
    } ;
    class ElementIF {
      public :
        virtual ~ElementIF () {}
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
