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
    class AccessPllException : public ALUGridException 
    {
    public:
      virtual std::string what () const { return "AccessPllException"; }
    };
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

      virtual bool packAll  ( std::vector< ObjectStream > & ) = 0;
      virtual moveto_t* moveToMap () = 0 ;
      virtual bool dunePackAll ( std::vector< ObjectStream > &, GatherScatterType & ) = 0;
      virtual void unpackSelf ( ObjectStream &, bool ) = 0;
      virtual void duneUnpackSelf ( ObjectStream &, bool, GatherScatterType * ) = 0;
      virtual void computeBaryCenter( double (&center)[3] ) const = 0;
  };

  class MacroGridMoverDefault : public MacroGridMoverIF {
    protected :
      MacroGridMoverDefault () {}
      virtual ~MacroGridMoverDefault () {}
    public :
      typedef MacroGridMoverIF :: moveto_t moveto_t ;

      virtual void attach2   (int) { alugrid_assert (false);abort(); }
      virtual void unattach2 (int) { alugrid_assert (false);abort(); }

      virtual bool packAll ( std::vector< ObjectStream > &) { alugrid_assert (false); abort(); }
      virtual moveto_t* moveToMap () { alugrid_assert (false); abort(); return ((moveto_t *) 0); }
      virtual bool dunePackAll ( std::vector< ObjectStream > &, GatherScatterType & ) { alugrid_assert (false); return false; }
      virtual void unpackSelf ( ObjectStream &, bool ) { alugrid_assert (false); abort(); }
      virtual void duneUnpackSelf (ObjectStream &, const bool, GatherScatterType *) {alugrid_assert (false);}
      virtual void computeBaryCenter( double (&center)[3] ) const { center[ 0 ] = center[ 1 ] = center[ 2 ] = 0; }
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
      int _i[ 4 ];
      static const int _endOfStream = -128 ; // must be a negative value 
    public :
      inline Identifier (int = -1, int = -1, int = -1, int = -1) ;
      inline Identifier (const Identifier &) ;
      inline const Identifier & operator = (const Identifier &) ;
      inline bool operator < (const Identifier &) const ;
      inline bool operator == (const Identifier &) const ;
      // read identifier from stream and return true if successful 
      bool read ( ObjectStream& );
      bool read ( ObjectStream&, const int );
      void write ( ObjectStream& ) const ;
      void write ( ObjectStream&, const int ) const ;
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
    virtual void checkAndAddLinkage ( const int ) = 0;
  };

  class LinkedObjectDefault
  : public LinkedObject
  {
    public :
      virtual ~LinkedObjectDefault () {}
      virtual Identifier getIdentifier () const { alugrid_assert (false);abort(); return Identifier(); }
      virtual std::vector< int > estimateLinkage () const { alugrid_assert (false); abort(); return std::vector< int >(); }
      virtual void checkAndAddLinkage ( const int ) { alugrid_assert (false); abort(); }
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
      virtual void getRefinementRequest (ObjectStream &) const { alugrid_assert (false);abort(); }
      virtual bool setRefinementRequest (ObjectStream &) { alugrid_assert (false);abort(); return false ;}
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
    return _i[ 0 ] == -1 ? false : true ;
  }

  inline LinkedObject :: Identifier :: Identifier (int a, int b, int c, int d) 
  {
    _i[ 0 ] = a;
    _i[ 1 ] = b;
    _i[ 2 ] = c;
    _i[ 3 ] = d;
  }

  inline LinkedObject :: Identifier :: Identifier (const Identifier & x) 
  {
    for( int i=0; i<4; ++i ) _i[ i ] = x._i[ i ];
  }

  inline const LinkedObject :: Identifier & LinkedObject :: Identifier :: operator = (const Identifier & x) 
  {
    alugrid_assert (x.isValid ()) ;
    for( int i=0; i<4; ++i ) _i[ i ] = x._i[ i ];
    return * this ;
  }

  inline bool LinkedObject :: Identifier :: operator < (const Identifier & x) const {
    alugrid_assert (isValid () && x.isValid ()) ;
    return (_i[ 0 ] < x._i[ 0 ]) ? true : (_i[ 0 ] == x._i[ 0 ] ? (_i[ 1 ] < x._i[ 1 ] ? true : 
        (_i[ 1 ] == x._i[ 1 ] ? (_i[ 2 ] < x._i[ 2 ] ? true : (_i[ 2 ] == x._i[ 2 ] ? 
      (_i[ 3 ] < x._i[ 3 ] ? true : false) : false )) : false )) : false ) ;
  }

  inline bool LinkedObject :: Identifier :: operator == (const Identifier & x) const {
    return (_i[ 0 ] == x._i[ 0 ] && _i[ 1 ] == x._i[ 1 ] && _i[ 2 ] == x._i[ 2 ] && _i[ 3 ] == x._i[ 3 ]) ? true : false ;
  }

  // read identifier and return true if successful 
  inline bool LinkedObject::Identifier::read ( ObjectStream& os ) 
  {
    // if the next entry is end of stream do nothing more 
    os.readObject( _i[ 0 ] );
    if( _i[ 0 ] == _endOfStream ) 
      return false ;

    os.readObject( _i[ 1 ] );
    os.readObject( _i[ 2 ] );
    os.readObject( _i[ 3 ] );
    return true ;
  }

  inline void LinkedObject::Identifier::write ( ObjectStream& os ) const
  {
    // write object to stream 
    os.writeObject( _i[ 0 ] );
    os.writeObject( _i[ 1 ] );
    os.writeObject( _i[ 2 ] );
    os.writeObject( _i[ 3 ] );
  }

  // read identifier and return true if successful 
  inline bool LinkedObject::Identifier::read ( ObjectStream& os, const int cnt ) 
  {
    // if the next entry is end of stream do nothing more 
    os.readObject( _i[ 0 ] );
    if( _i[ 0 ] == _endOfStream ) 
      return false ;

    for( int i=1; i<cnt; ++i )
      os.readObject( _i[ i ] );
    return true ;
  }

  inline void LinkedObject::Identifier::write ( ObjectStream& os, const int cnt ) const
  {
    // write object to stream 
    for( int i=0; i<cnt; ++i )
      os.writeObject( _i[ i ] );
  }

} // namespace ALUGrid

#endif // #ifndef PARALLEL_H_INCLUDED
