// (c) bernhard schupp, 1997 - 1998
#ifndef PARALLEL_H_INCLUDED
#define PARALLEL_H_INCLUDED

typedef class VertexPllXIF  VertexPllXIF_t ;
typedef class EdgePllXIF    EdgePllXIF_t ;
typedef class FacePllXIF    FacePllXIF_t ;
typedef class ElementPllXIF ElementPllXIF_t ;

//////////////////////////////////////////////////////////////////////////////
//
//
//  Interfaces for elements, faces, edges, and vertices for parallel computations 
//
//
//////////////////////////////////////////////////////////////////////////////

  // Das 'MacroGridMoverIF' mu"s von den Parallelerweiterungen der 
  // Knoten, Kanten, Fl"achen und Elemente des Grobgitters implementiert
  // werden, damit der Lastverteiler diese Objekte zuweisen, einpacken
  // und rekonstruieren kann.

class MacroGridMoverIF {
  protected :
    MacroGridMoverIF () {}
    virtual ~MacroGridMoverIF () {}
  public :
    enum { VERTEX = 1, EDGE1, FACE3, FACE4, 
           HEXA, TETRA, PERIODIC3, PERIODIC4=-65, 
           HBND3EXT, HBND4EXT, HBND3INT, HBND4INT = -22 ,
           ENDMARKER , NO_POINT = -777, POINTTRANSMITTED=-888 } ;
    virtual void attach2 (int) = 0 ;
    virtual void unattach2 (int) = 0 ;
    virtual bool packAll (vector < ObjectStream > &) = 0 ;
    virtual bool dunePackAll (vector < ObjectStream > &, GatherScatterType & ) { return false; }
    virtual void unpackSelf (ObjectStream &,bool) = 0 ;
    virtual void duneUnpackSelf (ObjectStream &,GatherScatterType &,bool) {};
} ;

class MacroGridMoverDefault : public MacroGridMoverIF {
  protected :
    MacroGridMoverDefault () {}
    virtual ~MacroGridMoverDefault () {}
  public :
    virtual void attach2 (int) { assert(false);abort(); }
    virtual void unattach2 (int) { assert(false);abort(); }
    virtual bool packAll (vector < ObjectStream > &) { assert(false);abort(); }
    virtual bool dunePackAll (vector < ObjectStream > &, GatherScatterType & ) { return false; }
    virtual void unpackSelf (ObjectStream &,bool) { assert(false);abort(); }
    virtual void duneUnpackSelf (ObjectStream &,GatherScatterType &,bool) {};
} ;

  // LinkedObjekt ist die Schnittstelle, die im parallelen Gitter zur
  // Identifikation ben"otigt wird. Das Identifikationsmodul wendet
  // sich an diese Schnittstelle, um die Schl"ussel f"ur die Objekte
  // des Gitters und eine obere Absch"atzung f"ur deren Verbindungsstern
  // zu erhalten. Diese Abschh"atzung kann auch die globale Verbindung
  // sein, d.h. der Vektor enth"alt alle Gebietsnummern, dann wird aber
  // die Effizienz des Identifikationsmoduls schlecht.

  // Note: The derivation from MacroGridMoverIF is artificial. Since all
  //       implementations of LinkedObject also derive from MacroGridMoverIf,
  //       this saves the additional pointer to the vtbl of MacroGridMoverIf.

class LinkedObject : public MacroGridMoverDefault
{
  public :
  
  // Der Identifier wird f"ur alle Gitterobjekte einheitlich verwendet.
  // Er ist der Schl"ussel f"ur die Identifikation der mehrdeutigen
  // Gitterobjekte. Kanten benutzen eine Schl"ussell"ange von zwei,
  // Fl"achen eine von drei und Elemente eine von vier. Wird nicht der
  // gesamte Schl"ussel benutzt, werden die "ubrigen Eintr"age mit
  // -1 gepaddet.
  // Die Schnittstelle wird von den Parallelerweiterungen der Knoten
  // Kanten, Fl"achen und (sp"ater auch) Elemente implementiert.
  
    class Identifier : public Serializable 
    {
      int _i1, _i2, _i3, _i4 ;
    public :
      inline Identifier (int = -1, int = -1, int = -1, int = -1) ;
      inline Identifier (const Identifier &) ;
      inline const Identifier & operator = (const Identifier &) ;
      inline bool operator < (const Identifier &) const ;
      inline bool operator == (const Identifier &) const ;
      inline void read (vector < int > :: const_iterator &, const vector < int > :: const_iterator &) ;
      inline void write (vector < int > &) const ;
      inline bool readObject (ObjectStream &) ;
      inline void writeObject (ObjectStream &) const ;
      inline bool isValid () const ;
    } ;

  public :
    virtual ~LinkedObject () {}
    virtual Identifier getIdentifier () const = 0 ;
    virtual vector < int > estimateLinkage () const = 0 ;
} ;

class LinkedObjectDefault : public LinkedObject
{
  public :
    virtual ~LinkedObjectDefault () {}
    virtual Identifier getIdentifier () const { assert(false);abort(); return Identifier(); }
    virtual vector < int > estimateLinkage () const { assert(false);abort(); return vector<int> (); }
} ;

  // Die Schnittstelle 'RefineableObject' ist diejenige, an die sich
  // der parallele Verfeinerer wendet, um z.B. die Requests heraus-
  // zufinden und zu setzen. Die Requests werden einfach auf den Strom
  // geschrieben, und sollten beim einlesen auf ihre G"ultigkeit
  // getestet werden. Die Schnittstelle wird von den Parallelerweiterungen
  // der Kanten und der Fl"achen implementiert.

  // Note: The derivation from LinkedObject is artificial. Since all
  //       implementations of RefineableObject also derive from LinkedObject,
  //       this saves the additional pointer to the vtbl of LinkedObject.

class RefineableObject : public LinkedObjectDefault
{
  protected :
    RefineableObject () {}
    virtual ~RefineableObject () {}
  public :
    virtual void getRefinementRequest (ObjectStream &) const = 0 ;
    virtual bool setRefinementRequest (ObjectStream &) = 0 ;
} ;


class RefineableObjectDefault : public RefineableObject
{
  protected :
    RefineableObjectDefault () {}
    virtual ~RefineableObjectDefault () {}
  public :
    virtual void getRefinementRequest (ObjectStream &) const { assert(false);abort(); }
    virtual bool setRefinementRequest (ObjectStream &) { assert(false);abort(); return false ;}
} ;


class VertexPllXIF : public LinkedObjectDefault //, public MacroGridMoverIF
{
  protected :
    virtual ~VertexPllXIF () {}
  public :
    virtual bool setLinkage (vector < int >) = 0 ;
} ;

class VertexPllXDefault : public VertexPllXIF
{
  protected :
    virtual ~VertexPllXDefault () {}
  public :
    virtual bool setLinkage (vector < int >) { assert(false);abort(); return false ; } 
} ;


class EdgePllXIF : public RefineableObjectDefault //, public LinkedObject, public MacroGridMoverIF
{
  protected :
    virtual ~EdgePllXIF () {}
  public :
    virtual bool lockAndTry () = 0 ;
    virtual bool unlockAndResume (bool) = 0 ;
    virtual bool lockedAgainstCoarsening () const = 0 ;
} ;

// default implementation 
class EdgePllXDefault : public EdgePllXIF 
{
  protected :
    virtual ~EdgePllXDefault () {}
  private:  
    virtual bool lockAndTry () { assert(false);abort(); return false ; }
    virtual bool unlockAndResume (bool) { assert(false);abort(); return false ; }
    virtual bool lockedAgainstCoarsening () const { assert(false);abort(); return false ; }
} ;


class FacePllXIF : public LinkedObjectDefault //, public MacroGridMoverIF
{
  protected :
    virtual ~FacePllXIF () {}
  public :
    virtual vector < int > checkParallelConnectivity () const = 0 ;
    virtual pair < ElementPllXIF_t *, int > accessOuterPllX () = 0 ;
    virtual pair < const ElementPllXIF_t *, int > accessOuterPllX () const = 0 ;
    virtual pair < ElementPllXIF_t *, int > accessInnerPllX () = 0 ;
    virtual pair < const ElementPllXIF_t *, int > accessInnerPllX () const = 0 ;

  public :
    virtual void writeStaticState (ObjectStream &) const = 0 ;
    virtual void readStaticState (ObjectStream &) = 0 ;
  public :
    virtual bool ldbUpdateGraphEdge (LoadBalancer :: DataBase &) = 0 ;
} ;

// default implementation (should not be called) 
class FacePllXDefault : public FacePllXIF 
{
  protected :
    virtual ~FacePllXDefault () {}
  private:
    virtual vector < int > checkParallelConnectivity () const { assert( false ); abort(); return vector<int> (); }
    virtual pair < ElementPllXIF_t *, int > accessOuterPllX () { assert( false ); abort(); return pair< ElementPllXIF_t *, int > ( (ElementPllXIF_t *) 0, -1); }
    virtual pair < const ElementPllXIF_t *, int > accessOuterPllX () const  { assert( false); abort(); return pair< ElementPllXIF_t *, int > ( (ElementPllXIF_t *) 0, -1); }
    virtual pair < ElementPllXIF_t *, int > accessInnerPllX ()  { assert( false); abort(); return pair< ElementPllXIF_t *, int > ( (ElementPllXIF_t *) 0, -1); }
    virtual pair < const ElementPllXIF_t *, int > accessInnerPllX () const { assert( false); abort(); return pair< ElementPllXIF_t *, int > ( (ElementPllXIF_t *) 0, -1); }

    virtual void writeStaticState (ObjectStream &) const { assert(false);abort(); }
    virtual void readStaticState (ObjectStream &) { assert(false);abort(); }
    virtual bool ldbUpdateGraphEdge (LoadBalancer :: DataBase &) { assert(false);abort(); return false ; }
};

/////////////////////////////////////////////////////////////////////////////
//
//  Parallel If extension 
//
/////////////////////////////////////////////////////////////////////////////

class Parallel {
  public :

    class CommunicationBuffer 
    {
      protected:
        typedef vector < SmallObjectStream > BufferType; 
        BufferType * _buff; 

        CommunicationBuffer () : _buff(0) {}
        ~CommunicationBuffer () 
        { 
          if(_buff) delete _buff; 
          _buff = 0;  
        }
      public:   
        void reserveBuffer (const size_t size)
        {
          if(_buff) 
          {
            // adjust size 
            if( size > _buff->size() ) _buff->resize( size );
          }
          else 
          {
            _buff = new BufferType( size ); 
          }
        }
        BufferType & commBuffer () { assert(_buff); return *_buff; }
        const BufferType & commBuffer () const { assert(_buff); return *_buff; }
    };
    
    class AccessPllException {} ;
  
    class VertexIF : public VertexPllXDefault
#ifdef ALUGRID_USE_COMM_BUFFER_IN_ITEM
      : public CommunicationBuffer 
#endif
    {
      public :
        virtual ~VertexIF () {}
        typedef class Key1SLZ identifier_t ;
        inline virtual VertexPllXIF_t & accessPllX () throw (AccessPllException) ;
        inline virtual const VertexPllXIF_t & accessPllX () const throw (AccessPllException) ;
        inline virtual void detachPllXFromMacro () throw (AccessPllException) ;
    } ;

    class EdgeIF : public EdgePllXDefault 
#ifdef ALUGRID_USE_COMM_BUFFER_IN_ITEM
      : public CommunicationBuffer 
#endif
    {
      public :
        virtual ~EdgeIF () {}
        typedef class Key2SLZ identifier_t ;
        inline virtual EdgePllXIF_t & accessPllX () throw (AccessPllException) ;
        inline virtual const EdgePllXIF_t & accessPllX () const throw (AccessPllException) ;
        inline virtual void detachPllXFromMacro () throw (AccessPllException) ;
    } ;
    class FaceIF : public FacePllXDefault {
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
///////////////////////////////////////////////////////////////////
//
//  --LinkedObject
//
///////////////////////////////////////////////////////////////////

inline bool LinkedObject :: Identifier :: isValid () const {
  return _i1 == -1 ? false : true ;
}

inline LinkedObject :: Identifier :: Identifier (int a, int b, int c, int d) 
  : _i1 (a), _i2 (b), _i3 (c), _i4 (d) {
}

inline LinkedObject :: Identifier :: Identifier (const Identifier & x) 
  : _i1 (x._i1), _i2 (x._i2), _i3 (x._i3), _i4 (x._i4) {
}

inline const LinkedObject :: Identifier & LinkedObject :: Identifier :: operator = (const Identifier & x) {
  assert (x.isValid ()) ;
  _i1 = x._i1 ;
  _i2 = x._i2 ;
  _i3 = x._i3 ;
  _i4 = x._i4 ;
  return * this ;
}

inline bool LinkedObject :: Identifier :: operator < (const Identifier & x) const {
  assert (isValid () && x.isValid ()) ;
  return (_i1 < x._i1) ? true : (_i1 == x._i1 ? (_i2 < x._i2 ? true : 
      (_i2 == x._i2 ? (_i3 < x._i3 ? true : (_i3 == x._i3 ? 
    (_i4 < x._i4 ? true : false) : false )) : false )) : false ) ;
}

inline bool LinkedObject :: Identifier :: operator == (const Identifier & x) const {
  return (_i1 == x._i1 && _i2 == x._i2 && _i3 == x._i3 && _i4 == x._i4) ? true : false ;
}

inline void LinkedObject :: Identifier :: read (vector < int > :: const_iterator & pos,
  const vector < int > :: const_iterator & end) {
  assert (pos != end ) ; _i1 = * pos ++ ;
  assert (pos != end ) ; _i2 = * pos ++ ; 
  assert (pos != end ) ; _i3 = * pos ++ ;
  assert (pos != end ) ; _i4 = * pos ++ ;
  return ;  
}

inline void LinkedObject :: Identifier :: write (vector < int > & v) const {
  v.push_back (_i1) ;
  v.push_back (_i2) ;
  v.push_back (_i3) ;
  v.push_back (_i4) ;
  return ;
}

inline bool LinkedObject :: Identifier :: readObject (ObjectStream & os) {
  os.readObject (_i1) ;
  os.readObject (_i2) ;
  os.readObject (_i3) ;
  os.readObject (_i4) ;
  return true ;
}

inline void LinkedObject :: Identifier :: writeObject (ObjectStream & os) const {
  os.writeObject (_i1) ;
  os.writeObject (_i2) ;
  os.writeObject (_i3) ;
  os.writeObject (_i4) ;
  return ;
}

///////////////////////////////////////////////////////////////////
//
//  --Parallel
//
///////////////////////////////////////////////////////////////////


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
