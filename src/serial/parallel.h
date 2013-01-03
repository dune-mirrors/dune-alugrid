#ifndef PARALLEL_H_INCLUDED
#define PARALLEL_H_INCLUDED

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

  private:
    // type of move to map, derive from MyAlloc 
    class MoveTo :
      public MyAlloc, 
      public map < int, int, less < int > > 
    {
      typedef map < int, int, less < int > >  base_t ;
    public:
      MoveTo () :  base_t () {}
    };
  public :
    typedef MoveTo moveto_t ;

    enum { VERTEX = 1, EDGE1, FACE3, FACE4, 
           HEXA, TETRA, PERIODIC3, PERIODIC4=-65, 
           HBND3EXT, HBND4EXT, HBND3INT, HBND4INT = -22 ,
           ENDMARKER , ENDSTREAM,  NO_POINT = -777, POINTTRANSMITTED=-888 } ;
    virtual void attach2   (int) = 0 ;
    virtual void unattach2 (int) = 0 ;

    virtual bool packAll (vector < ObjectStream > &) = 0 ;
    virtual bool dunePackAll (vector < ObjectStream > &, GatherScatterType & ) = 0; 
    virtual void unpackSelf (ObjectStream &,bool) = 0 ;
    virtual void duneUnpackSelf (ObjectStream &, const bool, GatherScatterType* ) = 0; 
} ;

class MacroGridMoverDefault : public MacroGridMoverIF {
  protected :
    MacroGridMoverDefault () {}
    virtual ~MacroGridMoverDefault () {}
  public :
    typedef MacroGridMoverIF :: moveto_t moveto_t ;

    virtual void attach2   (int) { assert(false);abort(); }
    virtual void unattach2 (int) { assert(false);abort(); }

    virtual bool packAll (vector < ObjectStream > &) { assert(false);abort(); }
    virtual bool dunePackAll (vector < ObjectStream > &, GatherScatterType & ) { assert(false);return false; }
    virtual void unpackSelf (ObjectStream &,bool) { assert(false);abort(); }
    virtual void duneUnpackSelf (ObjectStream &, const bool, GatherScatterType *) {assert(false);}
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
  
    class Identifier
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
#endif
