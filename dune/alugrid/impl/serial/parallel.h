#ifndef PARALLEL_H_INCLUDED
#define PARALLEL_H_INCLUDED

#include <map>
#include <vector>

#include "myalloc.h"
#include "serialize.h"

namespace ALUGrid
{

  struct GatherScatter;
  typedef GatherScatter GatherScatterType;



  // ParallelException
  // -----------------

  struct ParallelException 
  {
    class AccessPllException {};
  };


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

  class MacroGridMoverIF
  {
    protected :
      MacroGridMoverIF () {}
      virtual ~MacroGridMoverIF () {}

    private:
      // type of move to map, derive from MyAlloc 
      class MoveTo
      : public MyAlloc,
        public std::map< int, int >
      {};

    public :
      typedef MoveTo moveto_t ;

      enum { VERTEX = 1, EDGE1, FACE3, FACE4, 
             HEXA, TETRA, PERIODIC3, PERIODIC4=-65, 
             HBND3EXT, HBND4EXT, HBND3INT, HBND4INT = -22 ,
             ENDMARKER , ENDSTREAM,  NO_POINT = -777, POINTTRANSMITTED=-888 } ;
      virtual void attach2   (int) = 0 ;
      virtual void unattach2 (int) = 0 ;

      virtual bool packAll ( std::vector< ObjectStream > & ) = 0;
      virtual bool dunePackAll ( std::vector< ObjectStream > &, GatherScatterType & ) = 0;
      virtual void unpackSelf ( ObjectStream &, bool ) = 0;
      virtual void duneUnpackSelf ( ObjectStream &, bool, GatherScatterType * ) = 0;
  };

  class MacroGridMoverDefault : public MacroGridMoverIF {
    protected :
      MacroGridMoverDefault () {}
      virtual ~MacroGridMoverDefault () {}
    public :
      typedef MacroGridMoverIF :: moveto_t moveto_t ;

      virtual void attach2   (int) { assert(false);abort(); }
      virtual void unattach2 (int) { assert(false);abort(); }

      virtual bool packAll ( std::vector< ObjectStream > &) { assert(false); abort(); }
      virtual bool dunePackAll ( std::vector< ObjectStream > &, GatherScatterType & ) { assert(false); return false; }
      virtual void unpackSelf ( ObjectStream &, bool ) { assert(false); abort(); }
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

  class LinkedObject
  : public MacroGridMoverDefault
  {
  public:
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
      static const int _endOfStream = -128 ; // must be a negative value 
    public :
      inline Identifier (int = -1, int = -1, int = -1, int = -1) ;
      inline Identifier (const Identifier &) ;
      inline const Identifier & operator = (const Identifier &) ;
      inline bool operator < (const Identifier &) const ;
      inline bool operator == (const Identifier &) const ;
      // read identifier from stream and return true if successful 
      bool read ( ObjectStream& );
      void write ( ObjectStream& ) const ;
      inline bool isValid () const ;
      // read stream termination marker 
      static void endOfStream( ObjectStream& os ) 
      {
        os.writeObject( _endOfStream );
      }

    } ;

  public :
    virtual ~LinkedObject () {}

    virtual Identifier getIdentifier () const = 0 ;
    virtual std::vector< int > estimateLinkage () const = 0;
  };

  class LinkedObjectDefault
  : public LinkedObject
  {
    public :
      virtual ~LinkedObjectDefault () {}
      virtual Identifier getIdentifier () const { assert(false);abort(); return Identifier(); }
      virtual std::vector< int > estimateLinkage () const { assert(false); abort(); return std::vector< int >(); }
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

  inline const LinkedObject :: Identifier & LinkedObject :: Identifier :: operator = (const Identifier & x) 
  {
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

  // read identifier and return true if successful 
  inline bool LinkedObject::Identifier::read ( ObjectStream& os ) 
  {
    // if the next entry is end of stream do nothing more 
    os.readObject( _i1 );
    if( _i1 == _endOfStream ) 
      return false ;

    os.readObject( _i2 );
    os.readObject( _i3 );
    os.readObject( _i4 );
    return true ;
  }

  inline void LinkedObject::Identifier::write ( ObjectStream& os ) const
  {
    // write object to stream 
    os.writeObject( _i1 );
    os.writeObject( _i2 );
    os.writeObject( _i3 );
    os.writeObject( _i4 );
  }

} // namespace ALUGrid

#endif // #ifndef PARALLEL_H_INCLUDED
