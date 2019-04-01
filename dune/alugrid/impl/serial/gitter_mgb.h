// (c) bernhard schupp 1997 - 1998
// modifications for dune
// (c) Robert Kloefkorn 2004 - 2005
#ifndef GITTER_MGB_H_INCLUDED
#define GITTER_MGB_H_INCLUDED

#include <list>
#include <map>
#include <utility>

#include "key.h"
#include "gitter_sti.h"
#include "ghost_info.h"

namespace ALUGrid
{

  // re-implementation of std::rotate due to compiler bug in clang
  inline void rotateVec( int* first, int* middle, int* last)
  {
    int * next = middle;
    while (first!=next)
    {
      std::swap (*first++,*next++);
      if (next==last)
        next=middle;
      else if (first==middle)
        middle=next;
    }
  }

  // cyclic re-ordering of v and computing of twist
  inline int cyclicReorder( int *begin, int *end )
  {
    int* middle = std::min_element( begin, end );

    int twst = 0;
    if( middle != begin )
    {
      rotateVec( begin, middle, end );
      twst = end - middle;
    }

    if( *(begin + 1) < *(end - 1) )
    {
      return twst;
    }
    else
    {
      std::reverse( begin, end );
      rotateVec( begin, end - 1, end );
      return -twst - 1;
    }
  }

  // sorts v in ascending order and returns twist
  template <int dim>
  inline int cyclicReorder( int (&v)[ dim ] )
  {
    return cyclicReorder( v, v+dim );
  }

  class MacroGridBuilder
  : protected Gitter::Geometric
  {

    protected:
    // stores a hface3 and the other point needed to build a tetra
    class Hbnd3IntStorage : public MyAlloc
    {
      // info about ghost element, see ghost_info.h
      MacroGhostInfoTetra * _ptr;
      // internal face
      hface3_GEO * _first;
      // twist of face
      int _second;
      // ldb vertex index
      int _ldbVertexIndex;
      int _master;

      // prohibit copying
      Hbnd3IntStorage( const Hbnd3IntStorage& );

    public:
      // destructor deleting _ptr if not zero
      ~Hbnd3IntStorage();

      // store point and face and twist
      Hbnd3IntStorage( hface3_GEO * f, int tw, int ldbVertexIndex, int master, const tetra_GEO * tetra, int fce);

      // store point and face and twist
      Hbnd3IntStorage( hface3_GEO * f, int tw, int ldbVertexIndex, int master, MacroGhostInfoTetra* p);

      // store face and twist and set point to default
      Hbnd3IntStorage( hface3_GEO * f, int tw, int ldbVertexIndex, int master );

      // release internal MacroGhostInfoTetra pointer
      MacroGhostInfoTetra* release ();

      // release internal MacroGhostInfoTetra pointer
      MacroGhostInfoTetra* ghInfo ();

      // this two method are just like in pair
      hface3_GEO * first  () const { return _first;  }
      int          second () const { return _second; }

      // return ldb vertex index
      int ldbVertexIndex() const { return _ldbVertexIndex; }
      int master() const { return _master; }
    };

    // stores a hface4 and the other points needed to build a hexa
    class Hbnd4IntStorage : public MyAlloc
    {
      // info about ghost element, see ghost_info.h
      MacroGhostInfoHexa * _ptr;
      // internal face
      hface4_GEO * _first;
      // twist of face
      int _second;
      // ldb vertex index
      int _ldbVertexIndex;
      int _master;

      // prohibit copying
      Hbnd4IntStorage( const Hbnd4IntStorage& );

    public:
      // destructor deleting _ptr if not zero
      ~Hbnd4IntStorage ();

      // store point and face and twist
      Hbnd4IntStorage( hface4_GEO * f, int tw, int ldbVertexIndex, int master, const hexa_GEO * hexa, int fce);

      // store point and face and twist
      Hbnd4IntStorage( hface4_GEO * f, int tw, int ldbVertexIndex, int master, MacroGhostInfoHexa* );

      // store face and twist and set point to default
      Hbnd4IntStorage( hface4_GEO * f, int tw, int ldbVertexIndex, int master );

      // release internal ghost info pointer
      MacroGhostInfoHexa* ghInfo ();

      // release internal ghost info pointer
      MacroGhostInfoHexa* release ();

      // this two method are just like in pair
      hface4_GEO * first  () const { return _first;  }
      int          second () const { return _second; }

      // return ldb vertex index
      int ldbVertexIndex() const { return _ldbVertexIndex; }
      int master() const { return _master; }
    };

    public :
      enum ElementRawID {TETRA_RAW=4, HEXA_RAW=8, PERIODIC3_RAW=33, PERIODIC4_RAW=44};
    protected :
      typedef BuilderIF::hexalist_t  hexalist_t;
      typedef BuilderIF::tetralist_t tetralist_t;

      typedef long    vertexKey_t;
      typedef std::pair< int, int >   edgeKey_t;
      typedef Key3 < int >  faceKey_t;
      typedef Key4 < int >  elementKey_t;

      typedef std::map< vertexKey_t, VertexGeo * > vertexMap_t;
      typedef std::map< edgeKey_t, hedge1_GEO * > edgeMap_t;
      typedef std::map< faceKey_t, void * > faceMap_t;
      typedef std::map< elementKey_t, void * > elementMap_t;

      typedef std::map< faceKey_t, Hbnd3IntStorage* > hbnd3intMap_t;
      typedef std::map< faceKey_t, Hbnd4IntStorage* > hbnd4intMap_t;

      vertexMap_t  _vertexMap;
      edgeMap_t    _edgeMap;

      faceMap_t    _face4Map, _face3Map, _hbnd3Map, _hbnd4Map;

      // new type here, so we dont have to cast to void *
      hbnd3intMap_t _hbnd3Int;
      hbnd4intMap_t _hbnd4Int;

      elementMap_t _hexaMap, _tetraMap, _periodic3Map, _periodic4Map;

      inline BuilderIF & myBuilder ();
      inline const BuilderIF & myBuilder () const;
      void removeElement (const elementKey_t & );
    public :
      virtual std::pair< VertexGeo *, bool >     InsertUniqueVertex (double, double, double, int);
      virtual std::pair< hedge1_GEO *, bool >    InsertUniqueHedge (int,int);
      virtual std::pair< hedge1_GEO *, bool >    InsertUniqueHedge1 (int a, int b) { return InsertUniqueHedge( a, b); }
      virtual std::pair< hface3_GEO *, bool >    InsertUniqueHface3 (int (&v)[3]) { return InsertUniqueHface( v ); }
      virtual std::pair< hface4_GEO *, bool >    InsertUniqueHface4 (int (&v)[4]) { return InsertUniqueHface( v ); }
      virtual std::pair< hface3_GEO *, bool >    InsertUniqueHface (int (&)[3]);
      virtual std::pair< hface4_GEO *, bool >    InsertUniqueHface (int (&)[4]);

      virtual std::pair< tetra_GEO *, bool >     InsertUniqueTetra (int (&v)[4] ) { return InsertUniqueTetra( v, SimplexTypeFlag(0,0) ); }
      virtual std::pair< tetra_GEO *, bool >     InsertUniqueTetra (int (&)[4], SimplexTypeFlag );
      virtual std::pair< hexa_GEO *, bool >      InsertUniqueHexa (int (&)[8]);

      virtual std::pair< periodic3_GEO *, bool > InsertUniquePeriodic (int (&)[6], const Gitter::hbndseg::bnd_t (&)[2]);
      virtual std::pair< periodic4_GEO *, bool > InsertUniquePeriodic (int (&)[8], const Gitter::hbndseg::bnd_t (&)[2]);
      virtual std::pair< periodic3_GEO *, bool >
      InsertUniquePeriodic3 (int (&v)[6], const Gitter::hbndseg::bnd_t (&bnd)[2]) { return InsertUniquePeriodic( v, bnd ); }
      virtual std::pair< periodic4_GEO *, bool >
      InsertUniquePeriodic4 (int (&v)[8], const Gitter::hbndseg::bnd_t (&bnd)[2]) { return InsertUniquePeriodic( v, bnd ); }

      // old version setting default boundary ids
      std::pair< periodic3_GEO *, bool > InsertUniquePeriodic3 (int (&v)[6] )
      {
        Gitter::hbndseg::bnd_t bnd[ 2 ] =
          { Gitter::hbndseg::periodic, Gitter::hbndseg::periodic };
        return InsertUniquePeriodic( v, bnd );
      }

      // old version setting default boundary ids
      std::pair< periodic4_GEO *, bool > InsertUniquePeriodic4 (int (&v)[8] )
      {
        Gitter::hbndseg::bnd_t bnd[ 2 ] =
          { Gitter::hbndseg::periodic, Gitter::hbndseg::periodic };
        return InsertUniquePeriodic( v, bnd );
      }

      virtual bool InsertUniqueHbnd3 (int (&)[3], Gitter::hbndseg::bnd_t, int,int, const ProjectVertexPtr& pv);
      virtual bool InsertUniqueHbnd4 (int (&)[4], Gitter::hbndseg::bnd_t, int,int, const ProjectVertexPtr& pv);

      virtual bool InsertUniqueHbnd3 (int (&v)[3], Gitter::hbndseg::bnd_t bt, const ProjectVertexPtr& pv )
      {
        // ldbVertexIndex = -1
        return InsertUniqueHbnd3( v, bt, int(-1), int(-1), pv );
      }
      virtual bool InsertUniqueHbnd4 (int (&v)[4], Gitter::hbndseg::bnd_t bt, const ProjectVertexPtr& pv )
      {
        // ldbVertexIndex = -1
        return InsertUniqueHbnd4( v, bt, int(-1), int(-1), pv );
      }

    public :
      static bool debugOption (int);

      MacroGridBuilder (BuilderIF &, const bool init = true);
      virtual ~MacroGridBuilder ();

      template <class stream_t>
      void inflateMacroGrid ( stream_t&, int );

      // former constructor
      void initialize ();
      // former destructor
      void finalize ();
    protected:
      bool _initialized;
      bool _finalized;

      void computeVertexElementLinkage( elementMap_t& elementMap,
                                        Gitter::ElementPllXIF::vertexelementlinkage_t& vxElemLinkage );

      // insert all hexas from elemMap into elemList
      void hexaMapToList( elementMap_t& elemMap, hexalist_t& elemList, const bool setIndex  );

      // insert all hexas from elemMap into elemList
      void tetraMapToList( elementMap_t& elemMap, tetralist_t& elemList, const bool setIndex  );

      // insert all element from elemMap into elemList
      template<class elemlist_t>
      void elementMapToList( elementMap_t& elemMap, elemlist_t& elemList, const bool setIndex  );

      // reserve memory for container for additional 'size' elements
      template <class A>
      void reserve( std::vector< A >& container, size_t size ) const
      {
        container.reserve( container.size() + size );
      }

      // reserve memory for container, nothing to do for list
      template <class A>
      void reserve( std::list< A >& container, size_t size ) const
      {
      }

      // reserve memory for container for additional 'size' elements
      template <class container_t>
      void clear( container_t& container ) const
      {
        // clear container and also free memory
        container_t().swap( container );
      }

    private:
      BuilderIF & _mgb;
  };


  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //
  inline Gitter::Geometric::BuilderIF & MacroGridBuilder::myBuilder () {
    return _mgb;
  }

  inline const Gitter::Geometric::BuilderIF & MacroGridBuilder::myBuilder () const {
    return _mgb;
  }

  inline bool MacroGridBuilder::debugOption (int level) {
#ifdef ALUGRIDDEBUG
    return (getenv ("VERBOSE_MGB") ? ( atoi (getenv ("VERBOSE_MGB")) > level ? true : (level == 0)) : false);
#else
    return false;
#endif
  }

  //- Hbnd3IntStorage
  inline MacroGridBuilder::Hbnd3IntStorage::
  Hbnd3IntStorage( hface3_GEO * f, int tw, int ldbVertexIndex, int master, const tetra_GEO * tetra, int fce)
   : _ptr(new MacroGhostInfoTetra(tetra,fce))
   , _first(f) , _second(tw), _ldbVertexIndex( ldbVertexIndex ), _master(master)
  {
    alugrid_assert ( _ldbVertexIndex >= 0 );
  }

  inline MacroGridBuilder::Hbnd3IntStorage::
  Hbnd3IntStorage( hface3_GEO * f, int tw, int ldbVertexIndex, int master, MacroGhostInfoTetra *p)
   : _ptr(p) , _first(f) , _second(tw), _ldbVertexIndex( ldbVertexIndex ), _master(master)
  {
    alugrid_assert ( _ldbVertexIndex >= 0 );
    alugrid_assert ( _ptr );
  }

  inline MacroGridBuilder::Hbnd3IntStorage::
  Hbnd3IntStorage( hface3_GEO * f, int tw, int ldbVertexIndex, int master )
   : _ptr(0), _first(f) , _second(tw), _ldbVertexIndex( ldbVertexIndex ), _master(master)
  {
    // for this constructor we need to allow ldbVertexIndex < 0
    // since this can happen on parallel construction via several macro files.
    // alugrid_assert ( _ldbVertexIndex >= 0 );
  }

  inline MacroGridBuilder::Hbnd3IntStorage::~Hbnd3IntStorage ()
  {
    if( _ptr )
    {
      delete _ptr;
      _ptr = 0;
    }
  }

  inline MacroGhostInfoTetra* MacroGridBuilder::Hbnd3IntStorage::ghInfo ()
  {
    return _ptr;
  }

  inline MacroGhostInfoTetra* MacroGridBuilder::Hbnd3IntStorage::release ()
  {
    MacroGhostInfoTetra* p = _ptr;
    _ptr = 0;
    return p;
  }

  //- Hbnd4IntStorage
  inline MacroGridBuilder::Hbnd4IntStorage::
  Hbnd4IntStorage( hface4_GEO * f, int tw, int ldbVertexIndex, int master, const hexa_GEO * hexa, int fce)
   : _ptr( new MacroGhostInfoHexa(hexa,fce) ), _first(f) , _second(tw), _ldbVertexIndex( ldbVertexIndex ), _master(master)
  {
    alugrid_assert ( _ldbVertexIndex >= 0 );
  }

  // hface4 storage
  inline MacroGridBuilder::Hbnd4IntStorage::
  Hbnd4IntStorage( hface4_GEO * f, int tw, int ldbVertexIndex, int master, MacroGhostInfoHexa* p)
   : _ptr(p) , _first(f) , _second(tw), _ldbVertexIndex( ldbVertexIndex ), _master(master)
  {
    alugrid_assert ( _ldbVertexIndex >= 0 );
    alugrid_assert ( _ptr );
  }

  inline MacroGridBuilder::Hbnd4IntStorage::
  Hbnd4IntStorage( hface4_GEO * f, int tw, int ldbVertexIndex, int master )
   : _ptr(0) , _first(f) , _second(tw), _ldbVertexIndex( ldbVertexIndex ), _master(master)
  {
    // for this constructor we need to allow ldbVertexIndex < 0
    // since this can happen on parallel construction via several macro files.
    // alugrid_assert ( _ldbVertexIndex >= 0 );
  }

  inline MacroGridBuilder::Hbnd4IntStorage::~Hbnd4IntStorage ()
  {
    if( _ptr )
    {
      delete _ptr;
      _ptr = 0;
    }
  }

  inline MacroGhostInfoHexa* MacroGridBuilder::Hbnd4IntStorage::ghInfo()
  {
    return _ptr;
  }

  inline MacroGhostInfoHexa* MacroGridBuilder::Hbnd4IntStorage::release()
  {
    MacroGhostInfoHexa* p = _ptr;
    _ptr = 0;
    return p;
  }

} // namespace ALUGrid

#endif // #ifndef GITTER_MGB_H_INCLUDED
