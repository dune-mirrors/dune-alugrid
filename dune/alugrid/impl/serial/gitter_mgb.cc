// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface
// (c) Robert Kloefkorn 2004 - 2005
#include <config.h>

#include <sstream>

#include <dune/alugrid/impl/macrofileheader.hh>

#include "gitter_sti.h"
#include "gitter_mgb.h"
#include "gitter_impl.h"

namespace ALUGrid
{
  std::pair< Gitter::Geometric::VertexGeo *, bool > MacroGridBuilder::
  InsertUniqueVertex (double x, double y, double z, int i) {
    std::pair< vertexMap_t::iterator, bool > result = _vertexMap.insert( std::make_pair( i, static_cast< VertexGeo * >( 0 ) ) );
    if( result.second )
      result.first->second = myBuilder().insert_vertex( x, y, z, i );
    return std::make_pair( result.first->second, result.second );
  }

  std::pair< Gitter::Geometric::hedge1_GEO *, bool > MacroGridBuilder::
  InsertUniqueHedge (int l, int r) {
    if (l > r) {
      int i = l; l = r; r = i;
    }
    edgeKey_t key (l,r);
    std::pair< edgeMap_t::iterator, bool > result = _edgeMap.insert( std::make_pair( key, static_cast< hedge1_GEO * >( 0 ) ) );
    if( result.second )
    {
      vertexMap_t::const_iterator a = _vertexMap.find (l), b = _vertexMap.find (r);

      alugrid_assert ( a != _vertexMap.end() );
      alugrid_assert ( b != _vertexMap.end() );

      result.first->second = myBuilder ().insert_hedge1 ((*a).second,(*b).second);
    }
    return std::make_pair( result.first->second, result.second );
  }

  std::pair< Gitter::Geometric::hface3_GEO *, bool > MacroGridBuilder::
  InsertUniqueHface (int (&v)[3]) {
    cyclicReorder (v);
    faceKey_t key (v[0],v[1],v[2]);
    std::pair< faceMap_t::iterator, bool > result = _face3Map.insert( std::make_pair( key, static_cast< void * >( 0 ) ) );
    if( result.second )
    {
      hedge1_GEO * edge [3];
      int dire [3] = { 0, 0, 1 };
      edge [0] = InsertUniqueHedge (v[0],v[1]).first;
      edge [1] = InsertUniqueHedge (v[1],v[2]).first;
      edge [2] = InsertUniqueHedge (v[2],v[0]).first;
      result.first->second = myBuilder ().insert_hface3 (edge,dire);
    }
    return std::make_pair( static_cast< hface3_GEO * >( result.first->second ), result.second );
  }

  std::pair< Gitter::Geometric::hface4_GEO *, bool > MacroGridBuilder::InsertUniqueHface (int (&v)[4]) {
    cyclicReorder (v);
    faceKey_t key (v[0],v[1],v[2]);
    std::pair< faceMap_t::iterator, bool > result = _face4Map.insert( std::make_pair( key, static_cast< void * >( 0 ) ) );
    if( result.second )
    {
      hedge1_GEO * edge [4];
      int dire [4];
      edge [0] = InsertUniqueHedge (v[0],v[1]).first;
      edge [1] = InsertUniqueHedge (v[1],v[2]).first;
      edge [2] = InsertUniqueHedge (v[2],v[3]).first;
      edge [3] = InsertUniqueHedge (v[3],v[0]).first;
      dire [0] = v[0] < v[1] ? 0 : 1;
      dire [1] = v[1] < v[2] ? 0 : 1;
      dire [2] = v[2] < v[3] ? 0 : 1;
      dire [3] = v[3] < v[0] ? 0 : 1;
      result.first->second = myBuilder ().insert_hface4 (edge,dire);
    }
    return std::make_pair( static_cast< hface4_GEO * >( result.first->second ), result.second );
  }

  std::pair< Gitter::Geometric::tetra_GEO *, bool > MacroGridBuilder::
  InsertUniqueTetra (int (&v)[4], SimplexTypeFlag elementType)
  {
    elementKey_t key (v [0], v [1], v [2], v [3]);
    std::pair< elementMap_t::iterator, bool > result = _tetraMap.insert( std::make_pair( key, static_cast< void * >( 0 ) ) );
    if( result.second )
    {
      hface3_GEO * face [4];
      int twst [4];
      for (int fce = 0; fce < 4; ++fce )
      {
        int x [3];
        x [0] = v [Tetra::prototype [fce][0]];
        x [1] = v [Tetra::prototype [fce][1]];
        x [2] = v [Tetra::prototype [fce][2]];
        twst [fce] = cyclicReorder (x);
        face [fce] =  InsertUniqueHface (x).first;
      }
      result.first->second = myBuilder ().insert_tetra (face,twst, elementType);
      alugrid_assert( result.first->second );
    }
    return std::make_pair( static_cast< tetra_GEO * >( result.first->second ), result.second );
  }

  std::pair< Gitter::Geometric::hexa_GEO *, bool > MacroGridBuilder::InsertUniqueHexa (int (&v)[8])
  {
    elementKey_t key (v [0], v [1], v [3], v[4]);
    std::pair< elementMap_t::iterator, bool > result = _hexaMap.insert( std::make_pair( key, static_cast< void * >( 0 ) ) );
    if( result.second )
    {
      hface4_GEO * face [6];
      int twst [6];
      for (int fce = 0; fce < 6; ++fce)
      {
        int x [4];
        x [0] = v [Hexa::prototype [fce][0]];
        x [1] = v [Hexa::prototype [fce][1]];
        x [2] = v [Hexa::prototype [fce][2]];
        x [3] = v [Hexa::prototype [fce][3]];
        twst [fce] = cyclicReorder (x);
        face [fce] =  InsertUniqueHface (x).first;
      }
      result.first->second = myBuilder ().insert_hexa (face,twst);
    }
    return std::make_pair( static_cast< hexa_GEO * >( result.first->second ), result.second );
  }

  bool MacroGridBuilder::
  InsertUniqueHbnd3 (int (&v)[3],Gitter::hbndseg_STI ::bnd_t bt, int ldbVertexIndex, int master, const ProjectVertexPtr& pv )
  {
    int twst = cyclicReorder (v);
    faceKey_t key (v [0], v [1], v [2]);
    if (bt == Gitter::hbndseg_STI::closure)
    {
      if (_hbnd3Int.find (key) == _hbnd3Int.end ()) {
        hface3_GEO * face =  InsertUniqueHface (v).first;
        _hbnd3Int [key] = new Hbnd3IntStorage (face, twst, ldbVertexIndex, master);
        return true;
      }
    }
    else
    {
      if (_hbnd3Map.find (key) == _hbnd3Map.end ())
      {
        hface3_GEO * face  = InsertUniqueHface (v).first;
        hbndseg3_GEO * hb3 = myBuilder ().insert_hbnd3 (face,twst,bt);
        hb3->setLoadBalanceVertexIndex( ldbVertexIndex );
        hb3->setMaster( master );
        hb3->setBoundaryProjection( pv );
        _hbnd3Map [key] = hb3;
        return true;
      }
    }
    return false;
  }

  bool MacroGridBuilder::
  InsertUniqueHbnd4 (int (&v)[4], Gitter::hbndseg_STI ::bnd_t bt, int ldbVertexIndex, int master, const ProjectVertexPtr& pv )
  {
    int twst = cyclicReorder (v);
    faceKey_t key (v [0], v [1], v [2]);
    if (bt == Gitter::hbndseg_STI::closure)
    {
      if (_hbnd4Int.find (key) == _hbnd4Int.end ()) {
        hface4_GEO * face =  InsertUniqueHface (v).first;
        _hbnd4Int [key] = new Hbnd4IntStorage (face, twst, ldbVertexIndex, master );
        return true;
      }
    }
    else
    {
      if (_hbnd4Map.find (key) == _hbnd4Map.end ())
      {
        hface4_GEO * face =  InsertUniqueHface (v).first;
        hbndseg4_GEO * hb4 = myBuilder ().insert_hbnd4 (face,twst,bt);
        hb4->setLoadBalanceVertexIndex( ldbVertexIndex );
        hb4->setMaster( master );
        hb4->setBoundaryProjection( pv );
        _hbnd4Map [key] = hb4;
        return true;
      }
    }
    return false;
  }

  std::pair< Gitter::Geometric::periodic3_GEO *, bool > MacroGridBuilder::
  InsertUniquePeriodic (int (&v)[6], const Gitter::hbndseg_STI ::bnd_t (&bt)[2] )
  {

    // Vorsicht: Der Schl"ussel f"ur das periodische Randelement wird
    // dummerweise mit dem eines Hexaeders verwechselt, falls nicht
    // der letzte Knoten negativ (mit umgekehrtem Vorzeichen) in die
    // Schl"ussel eingef"ugt wird.

    elementKey_t key (v [0], v [1], v [2], -(v [3])-1);
    elementMap_t::const_iterator hit = _periodic3Map.find (key);
    if (hit == _periodic3Map.end ()) {
      hface3_GEO * face [2];
      int twst [2];
      for (int fce = 0; fce < 2; ++fce )
      {
        int x [3];
        x [0] = v [Periodic3::prototype [fce][0]];
        x [1] = v [Periodic3::prototype [fce][1]];
        x [2] = v [Periodic3::prototype [fce][2]];
        twst [fce] = cyclicReorder (x);
        face [fce] = InsertUniqueHface (x).first;
      }
      periodic3_GEO * t = myBuilder ().insert_periodic3 (face,twst,bt);
      alugrid_assert (t);
      _periodic3Map [key] = t;
      return std::pair< periodic3_GEO *, bool > (t,true);
    } else {
      return std::pair< periodic3_GEO *, bool > ((periodic3_GEO *)(*hit).second,false);
    }
  }

  std::pair< Gitter::Geometric::periodic4_GEO *, bool > MacroGridBuilder::
  InsertUniquePeriodic (int (&v)[8], const Gitter::hbndseg_STI ::bnd_t (&bt)[2] )
  {

    // Vorsicht: Der Schl"ussel f"ur das periodische Randelement wird
    // dummerweise mit dem eines Hexaeders verwechselt, falls nicht
    // der letzte Knoten negativ (mit umgekehrtem Vorzeichen) in die
    // Schl"ussel eingef"ugt wird.

    elementKey_t key (v [0], v [1], v [3], -(v [4])-1);
    elementMap_t::const_iterator hit = _periodic4Map.find (key);
    if (hit == _periodic4Map.end ()) {
      hface4_GEO * face [2];
      int twst [2];
      for (int fce = 0; fce < 2; ++fce )
      {
        int x [4];
        x [0] = v [Periodic4::prototype [fce][0]];
        x [1] = v [Periodic4::prototype [fce][1]];
        x [2] = v [Periodic4::prototype [fce][2]];
        x [3] = v [Periodic4::prototype [fce][3]];
        twst [fce] = cyclicReorder (x);
        face [fce] = InsertUniqueHface (x).first;
      }
      periodic4_GEO * t = myBuilder ().insert_periodic4 (face,twst,bt);
      alugrid_assert (t);
      _periodic4Map [key] = t;
      return std::pair< periodic4_GEO *, bool > (t,true);
    }
    else
    {
      return std::pair< periodic4_GEO *, bool > ((periodic4_GEO *)(*hit).second,false);
    }
  }
  // Ende - Neu am 23.5.02 (BS)

  void MacroGridBuilder::removeElement (const elementKey_t & k )
  {
    // the key should only exist in exactly one map

    alugrid_assert ((_hexaMap.find (k) == _hexaMap.end ()  ? 0 : 1)
                  + (_tetraMap.find(k) == _tetraMap.end () ? 0 : 1) );

    if( ! _tetraMap.empty() )
    {
      elementMap_t::iterator hit = _tetraMap.find (k);
      if (hit != _tetraMap.end ())
      {
        tetra_GEO * tr = (tetra_GEO *)(*hit).second;
        int ldbVertexIndex = tr->ldbVertexIndex();
        int master = tr->master();

        typedef hbnd3intMap_t::iterator iterator;
        const iterator end = _hbnd3Int.end();
        for (int i = 0; i < 4; ++i)
        {
          // for periodic neighbours we do not create internal storages
          if( tr->myneighbour( i ).first->isperiodic() )
            continue;

          hface3_GEO* face = tr->myhface3 (i);
          faceKey_t key (face->myvertex (0)->ident (),
                         face->myvertex (1)->ident (),
                         face->myvertex (2)->ident ());

          // if the face does not exist in the map of internal boundaries
          // we need to insert this
          iterator hbndit = _hbnd3Int.find( key );
          if( hbndit == end )
          {
            Hbnd3IntStorage* hbnd =
              new Hbnd3IntStorage (face, tr->twist (i), ldbVertexIndex, master, tr , i );
            _hbnd3Int.insert( std::make_pair( key, hbnd ) );
          }
          // if the face already exists this means we can delete it,
          // since both adjacent element will disappear
          else
          {
            Hbnd3IntStorage* hbnd = (*hbndit).second;
            _hbnd3Int.erase( hbndit );
            delete hbnd;
          }
        }

        delete tr;
        _tetraMap.erase (hit);

        return;
      }
    }

    if( ! _hexaMap.empty() )
    {
      elementMap_t::iterator hit = _hexaMap.find (k);
      if (hit != _hexaMap.end ())
      {
        hexa_GEO * hx = (hexa_GEO *)(*hit).second;
        int ldbVertexIndex = hx->ldbVertexIndex();
        int master = hx->master();

        typedef hbnd4intMap_t::iterator iterator;
        const iterator end = _hbnd4Int.end();
        for (int i = 0; i < 6; ++i)
        {
          // for periodic neighbours we do not create internal storages
          if( hx->myneighbour( i ).first->isperiodic() )
            continue;

          hface4_GEO* face = hx->myhface4 (i);
          faceKey_t key (face->myvertex (0)->ident (),
                         face->myvertex (1)->ident (),
                         face->myvertex (2)->ident ());

          iterator hbndit = _hbnd4Int.find( key );
          // if the face does not exist in the map of internal boundaries
          // we need to insert this
          if( hbndit == end )
          {
            Hbnd4IntStorage* hbnd =
              new Hbnd4IntStorage ( face, hx->twist (i), ldbVertexIndex, master, hx, i );

            _hbnd4Int.insert( std::make_pair( key, hbnd ) );
          }
          // if the face already exists this means we can delete it,
          // since both adjacent element will disappear
          else
          {
            Hbnd4IntStorage* hbnd = (*hbndit).second;
            _hbnd4Int.erase( hbndit );
            delete hbnd;
          }
        }

        delete hx;
        _hexaMap.erase (hit);

        return;
      }
    }

    std::abort ();
    return;
  }

  // default of init == true
  MacroGridBuilder::MacroGridBuilder (BuilderIF & b, const bool init)
   : _initialized(false)
   , _finalized(false)
   , _mgb (b)
  {
    if(init) initialize();
  }

  void MacroGridBuilder::initialize ()
  {
    {
      BuilderIF::vertexlist_t& _vertexList = myBuilder ()._vertexList;
      typedef BuilderIF::vertexlist_t::iterator  iterator;
      const iterator vertexListEnd = _vertexList.end ();
      // copy list entries to map
      for ( iterator i = _vertexList.begin (); i != vertexListEnd; ++i )
        _vertexMap [ (*i)->ident ()] = (*i);

      // clear list
      clear( _vertexList );
    }
    {
      BuilderIF::hedge1list_t& _hedge1List = myBuilder ()._hedge1List;
      typedef BuilderIF::hedge1list_t::iterator  iterator;
      const iterator hedge1ListEnd = _hedge1List.end ();
      // copy list entries to map
      for ( iterator i = _hedge1List.begin (); i != hedge1ListEnd; ++i )
      {
        long k = (*i)->myvertex (0)->ident (), l = (*i)->myvertex (1)->ident ();
        _edgeMap [edgeKey_t (k < l ? k : l, k < l ? l : k)] = (*i);
      }
      // clear list
      clear( _hedge1List );
    }
    {
      BuilderIF::hface3list_t& _hface3List = myBuilder ()._hface3List;
      typedef BuilderIF::hface3list_t::iterator  iterator;
      const iterator  hface3ListEnd = _hface3List.end ();
      // copy list entries to map
      for ( iterator i = _hface3List.begin (); i != hface3ListEnd; ++i )
      {
        _face3Map [faceKey_t ((*i)->myvertex (0)->ident (),(*i)->myvertex (1)->ident (), (*i)->myvertex (2)->ident ())] = (*i);
      }
      // clear list
      clear( _hface3List );
    }
    {
      BuilderIF::hface4list_t& _hface4List = myBuilder ()._hface4List;
      typedef BuilderIF::hface4list_t::iterator  iterator;
      const iterator hface4ListEnd = _hface4List.end ();
      // copy list entries to map
      for ( iterator i = _hface4List.begin (); i != hface4ListEnd; ++i )
      {
        _face4Map [faceKey_t ((*i)->myvertex (0)->ident (),(*i)->myvertex (1)->ident (), (*i)->myvertex (2)->ident ())] = (*i);
      }

      // clear list
      clear( _hface4List );
    }
    {
      BuilderIF::hbndseg4list_t& _hbndseg4List = myBuilder ()._hbndseg4List;
      typedef BuilderIF::hbndseg4list_t::iterator  iterator;
      const iterator hbndseg4ListEnd = _hbndseg4List.end ();
      // copy entries to map
      for ( iterator i = myBuilder ()._hbndseg4List.begin (); i != hbndseg4ListEnd; ++i )
      {
        faceKey_t key ((*i)->myhface4 (0)->myvertex (0)->ident (), (*i)->myhface4 (0)->myvertex (1)->ident (), (*i)->myhface4 (0)->myvertex (2)->ident ());
        if ((*i)->bndtype () == Gitter::hbndseg_STI::closure) {
          _hbnd4Int [key] = new Hbnd4IntStorage ((*i)->myhface4 (0),(*i)->twist (0),(*i)->ldbVertexIndex(),(*i)->master());
          delete (*i);
        }
        else
        {
          _hbnd4Map [key] = (*i);
        }
      }
      // clear list
      clear( _hbndseg4List );
    }
    {
      BuilderIF::hbndseg3list_t& _hbndseg3List = myBuilder ()._hbndseg3List;
      typedef BuilderIF::hbndseg3list_t::iterator iterator;
      const iterator hbndseg3ListEnd = _hbndseg3List.end ();
      // copy entries to map
      for ( iterator i = _hbndseg3List.begin (); i != hbndseg3ListEnd; ++i )
      {
        faceKey_t key ((*i)->myhface3 (0)->myvertex (0)->ident (), (*i)->myhface3 (0)->myvertex (1)->ident (), (*i)->myhface3 (0)->myvertex (2)->ident ());
        if ((*i)->bndtype () == Gitter::hbndseg_STI::closure)
        {
          _hbnd3Int [key] = new Hbnd3IntStorage ((*i)->myhface3 (0), (*i)->twist (0),(*i)->ldbVertexIndex(),(*i)->master());
          delete (*i);
        }
        else
        {
          _hbnd3Map [key] = (*i);
        }
      }
      // clear list
      clear( _hbndseg3List );
    }
    {
      BuilderIF::tetralist_t& _tetraList = myBuilder ()._tetraList;
      typedef BuilderIF::tetralist_t::iterator  iterator;
      const iterator tetraListEnd = _tetraList.end ();
      // copy entries to map
      for ( iterator i = _tetraList.begin (); i != tetraListEnd; ++i )
      {
        _tetraMap [elementKey_t ( (*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (),
                                  (*i)->myvertex (2)->ident (), (*i)->myvertex (3)->ident ())] = (*i);
      }
      // clear list
      clear( _tetraList );
    }
    {
      BuilderIF::periodic3list_t& _periodic3List = myBuilder ()._periodic3List;
      typedef BuilderIF::periodic3list_t::iterator iterator;
      const iterator periodic3ListEnd = _periodic3List.end ();
      // copy entries to map
      for ( iterator i = _periodic3List.begin (); i != periodic3ListEnd; ++i )
      {
        _periodic3Map [elementKey_t ( (*i)->myvertex (0)->ident (),  (*i)->myvertex (1)->ident (),
                                      (*i)->myvertex (2)->ident (), -((*i)->myvertex (3)->ident ())-1)] = (*i);
      }
      // clear list
      clear( _periodic3List );
    }
    {
      BuilderIF::periodic4list_t& _periodic4List = myBuilder ()._periodic4List;
      typedef BuilderIF::periodic4list_t::iterator  iterator;
      const iterator periodic4ListEnd = _periodic4List.end ();
      // copy entries to map
      for ( iterator i = _periodic4List.begin (); i != periodic4ListEnd; ++i )
      {
        _periodic4Map [elementKey_t ( (*i)->myvertex (0)->ident (),  (*i)->myvertex (1)->ident (),
                                      (*i)->myvertex (3)->ident (), -((*i)->myvertex (4)->ident ())-1)] = (*i);
      }
      // clear list
      clear( _periodic4List );
    }
    {
      BuilderIF::hexalist_t& _hexaList = myBuilder ()._hexaList;
      typedef BuilderIF::hexalist_t::iterator  iterator;
      const iterator  hexaListEnd = _hexaList.end ();
      // copy entries to map
      for ( iterator i = _hexaList.begin (); i != hexaListEnd; ++i )
        _hexaMap [elementKey_t ( (*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (),
                                 (*i)->myvertex (3)->ident (), (*i)->myvertex (4)->ident ())] = (*i);
      // clear list
      clear( _hexaList );
    }

    _initialized = true;
    return;
  }

  MacroGridBuilder::~MacroGridBuilder ()
  {
    // _finalized is true if the method was called in inherited classes
    if(!_finalized) finalize();
  }

  void MacroGridBuilder::
  hexaMapToList( elementMap_t& elementMap, hexalist_t& elemList, const bool setIndex  )
  {
    elementMapToList( elementMap, elemList, setIndex );
  }

  void MacroGridBuilder::
  tetraMapToList( elementMap_t& elementMap, tetralist_t& elemList, const bool setIndex  )
  {
    elementMapToList( elementMap, elemList, setIndex );
  }

  template< class elemlist_t >
  void MacroGridBuilder::
  elementMapToList( elementMap_t& elementMap, elemlist_t& elemList, const bool setIndex  )
  {
    // elem_GEO_ptr is either hexa_GEO* or tetra_GEO*
    typedef typename elemlist_t :: value_type elem_GEO_ptr;
    {
      // sort by element numbering which is unique for macro elements
      typedef std::map< int, elem_GEO_ptr > elemmap_t;
      elemmap_t elemMap;
      {
        typedef typename elementMap_t::iterator  iterator;
        const iterator elementMapEnd = elementMap.end();
        for (iterator i = elementMap.begin ();
             i != elementMapEnd; elementMap.erase (i++) )
        {
          elem_GEO_ptr elem = (elem_GEO_ptr)(*i).second;
          // if ldbVertexIndex still needs to be set (in case of initial read)
          if( setIndex )
          {
            elem->setLoadBalanceVertexIndex( elem->getIndex() );
          }
          // ldbVertexIndex provides the unique index of the element across processes
          elemMap[ elem->ldbVertexIndex() ] = elem;
        }
      }
      {
        int elemCount = 0;

        // reserve memory for container in case it's vector
        reserve( elemList, elemMap.size() );

        typedef typename elemmap_t::iterator  iterator;
        const iterator iend = elemMap.end();
        for ( iterator i = elemMap.begin (); i != iend; ++ i, ++elemCount )
        {
          elem_GEO_ptr elem = (elem_GEO_ptr)(*i).second;
          // make sure that the insertion order
          // in the list is reflected by getIndex
          alugrid_assert ( setIndex ? (elem->getIndex() == elemCount) : true );
          // insert into macro element list
          elemList.push_back ( elem );
        }
      }
    }
  }

  // clean the map tables
  void MacroGridBuilder::finalize ()
  {
    alugrid_assert (_initialized);

    // copy elements from hexa map to hexa list respecting the insertion order
    hexaMapToList( _hexaMap, myBuilder()._hexaList, true );

    // copy elements from tetra map to tetra list respecting the insertion order
    tetraMapToList( _tetraMap, myBuilder()._tetraList, true );

    {
      // reserve memory for container in case it's vector
      reserve( myBuilder ()._periodic3List, _periodic3Map.size() );

      typedef elementMap_t::iterator  iterator;
      const iterator periodic3MapEnd = _periodic3Map.end ();
      for (elementMap_t::iterator i = _periodic3Map.begin (); i != periodic3MapEnd; ++i )
        myBuilder ()._periodic3List.push_back ((periodic3_GEO *)(*i).second);
      // clear mpa
      _periodic3Map.clear();
    }

    {
      // reserve memory for container in case it's vector
      reserve( myBuilder ()._periodic4List, _periodic4Map.size() );

      typedef elementMap_t::iterator  iterator;
      const iterator periodic4MapEnd = _periodic4Map.end ();
      for (elementMap_t::iterator i = _periodic4Map.begin (); i != periodic4MapEnd; ++i )
        myBuilder ()._periodic4List.push_back ((periodic4_GEO *)(*i).second);
      // clear map
      _periodic4Map.clear();
    }

    {
      // reserve memory for container in case it's vector
      reserve( myBuilder ()._hbndseg4List, _hbnd4Map.size() );

      typedef faceMap_t::iterator iterator;
      const iterator hbnd4MapEnd =  _hbnd4Map.end ();
      for (faceMap_t::iterator i = _hbnd4Map.begin (); i != hbnd4MapEnd; )
      {
        if (((hbndseg4_GEO *)(*i).second)->myhface4 (0)->ref == 1)
        {
          delete (hbndseg4_GEO *)(*i).second;
          _hbnd4Map.erase (i++);
        }
        else
        {
          myBuilder ()._hbndseg4List.push_back ((hbndseg4_GEO *)(*i ++).second);
        }
      }
      // clear map
      _hbnd4Map.clear();
    }
    {
      // reserve memory for container in case it's vector
      reserve( myBuilder ()._hbndseg3List, _hbnd3Map.size() );

      typedef faceMap_t::iterator iterator;
      const iterator hbnd3MapEnd = _hbnd3Map.end ();
      for (faceMap_t::iterator i = _hbnd3Map.begin (); i != hbnd3MapEnd; )
      {
        if (((hbndseg3_GEO *)(*i).second)->myhface3 (0)->ref == 1) {
          delete (hbndseg3_GEO *)(*i).second;
          _hbnd3Map.erase (i++);
        }
        else
        {
          myBuilder ()._hbndseg3List.push_back ((hbndseg3_GEO *)(*i ++).second);
        }
      }
    }
    {
      // reserve memory for container in case it's vector
      reserve( myBuilder ()._hbndseg4List, _hbnd4Int.size() );

      typedef hbnd4intMap_t::iterator iterator;
      const iterator hbnd4IntEnd = _hbnd4Int.end ();
      for (hbnd4intMap_t::iterator i = _hbnd4Int.begin (); i != hbnd4IntEnd; ++i)
      {
        const Hbnd4IntStorage & p = * ((*i).second);
        if (p.first()->ref == 1)
        {
          hbndseg4_GEO * hb4 =
             myBuilder ().insert_hbnd4 (p.first(), p.second(),
                                        Gitter::hbndseg_STI::closure);
          myBuilder ()._hbndseg4List.push_back (hb4);
        }
        delete (*i).second;
      }
    }

    // here the internal boundary elements are created
    {
      // reserve memory for container in case it's vector
      reserve( myBuilder ()._hbndseg3List, _hbnd3Int.size() );

      typedef hbnd3intMap_t::iterator  iterator;
      const iterator hbnd3IntEnd = _hbnd3Int.end ();
      for (hbnd3intMap_t::iterator i = _hbnd3Int.begin (); i != hbnd3IntEnd; ++i)
      {
        const Hbnd3IntStorage & p = * ((*i).second);
        if (p.first()->ref == 1)
        {
          hbndseg3_GEO * hb3 =
            myBuilder ().insert_hbnd3 (p.first(),p.second(), Gitter::hbndseg_STI::closure);
          myBuilder ()._hbndseg3List.push_back (hb3);
        }
        delete (*i).second;
      }
    }
    {
      // reserve memory for container in case it's vector
      reserve( myBuilder ()._hface4List, _face4Map.size() );

      typedef faceMap_t::iterator iterator;
      const iterator face4MapEnd = _face4Map.end ();
      for (faceMap_t::iterator i = _face4Map.begin (); i != face4MapEnd; )
      if (!((hface4_GEO *)(*i).second)->ref)
      {
        delete (hface4_GEO *)(*i).second;
        _face4Map.erase (i++);
      }
      else
      {
        alugrid_assert (((hface4_GEO *)(*i).second)->ref == 2);
        myBuilder ()._hface4List.push_back ((hface4_GEO *)(*i ++).second );
      }
    }
    {
      // reserve memory for container in case it's vector
      reserve( myBuilder ()._hface3List, _face3Map.size() );

      typedef faceMap_t::iterator iterator;
      const iterator face3MapEnd = _face3Map.end ();
      for (faceMap_t::iterator i = _face3Map.begin (); i != face3MapEnd; )
      {
        if (!((hface3_GEO *)(*i).second)->ref)
        {
          delete (hface3_GEO *)(*i).second;
          _face3Map.erase (i++);
        }
        else
        {
          alugrid_assert (((hface3_GEO *)(*i).second)->ref == 2);
          myBuilder ()._hface3List.push_back ((hface3_GEO *)(*i ++).second );
        }
      }
    }
    {
      // reserve memory for container in case it's vector
      reserve( myBuilder ()._hedge1List, _edgeMap.size() );

      typedef edgeMap_t::iterator iterator;
      const iterator edgeMapEnd = _edgeMap.end ();
      for (edgeMap_t::iterator i = _edgeMap.begin (); i != edgeMapEnd; )
      {
        if (!(*i).second->ref)
        {
          delete (*i).second;
          _edgeMap.erase (i++);
        }
        else
        {
          alugrid_assert ((*i).second->ref >= 1);
          myBuilder ()._hedge1List.push_back ((*i ++).second);
        }
      }
    }
    {
      // reserve memory for container in case it's vector
      reserve( myBuilder ()._vertexList, _vertexMap.size() );

      typedef vertexMap_t::iterator  iterator;
      const iterator vertexMapEnd = _vertexMap.end ();
      for (vertexMap_t::iterator i = _vertexMap.begin (); i != vertexMapEnd; )
      {
        if (!(*i).second->ref)
        {
          delete (*i).second;
          _vertexMap.erase (i++);
        }
        else {
          alugrid_assert ((*i).second->ref >= 2);
          myBuilder ()._vertexList.push_back ((*i ++).second);
        }
      }
    }
    _finalized = true;
    return;
  }

  void MacroGridBuilder::
  computeVertexElementLinkage( elementMap_t& elementMap,
                               Gitter::ElementPllXIF::vertexelementlinkage_t& vxElemLinkage )
  {
    typedef elementMap_t::iterator  iterator;
    const iterator elementMapEnd = elementMap.end();
    for (iterator i = elementMap.begin (); i != elementMapEnd; ++i )
    {
      Gitter::helement_STI* elem = (Gitter::helement_STI*) (*i).second;
      elem->computeVertexLinkage( vxElemLinkage );
    }
  }

  template <class stream_t>
  void MacroGridBuilder::inflateMacroGrid ( stream_t& in, int type )
  {
    const int start = clock ();
    int nv = 0;
    in >> nv;
    for (int i = 0; i < nv; ++i )
    {
      int id;
      double x, y, z;
      in >> id ;
      in >> x ;
      in >> y ;
      in >> z ;
      InsertUniqueVertex (x, y, z, id);
    }

    int ne = 0;
    in >> ne ;

    if( type == HEXA_RAW )
    {
      int v [8];
      for (int i = 0; i<ne; ++i )
      {
        for( int k=0; k<8; ++k )
        {
          in >> v[ k ] ;
        }
        InsertUniqueHexa (v);
      }
    }
    else if( type == TETRA_RAW )
    {
      int v [4];
      for (int i = 0; i < ne; ++i )
      {
        for( int j=0; j<4; ++j )
        {
          in >> v[ j ] ;
        }
        int orientation = i%2;
        int elementType = 0;
        InsertUniqueTetra (v, SimplexTypeFlag(orientation, elementType) );
      }
    }

    // read number of periodic and other boundary elements
    int nper;
    in >> nper ;
    int nb ;
    in >> nb ;

    if( type == HEXA_RAW )
    {
      int vp[ 8 ];
      for( int i=0; i<nper; ++i )
      {
        for( int j=0; j<8; ++j )
          in >> vp[ j ] ;

        Gitter::hbndseg::bnd_t bndId[ 2 ] = { Gitter::hbndseg::periodic, Gitter::hbndseg::periodic };
        InsertUniquePeriodic (vp, bndId );
      }

      int bt ;
      int v[ 4 ];
      for( int i=0; i<nb ; ++i )
      {
        in >> bt ;
        int k = 0 ;
        if( bt < 0 ) // exterior bnd
        {
          bt = -bt; // use positive value
          if( !Gitter::hbndseg_STI::bndRangeCheck( bt ) )
          {
            std::cerr << "ERROR (fatal): Boundary id = " << bt << " out of range (valid are " << Gitter::hbndseg_STI::validRanges() << ")." << std::endl;
            abort();
          }
        }
        else // interior parallel bnd
        {
          v[ k++ ] = bt ;
          bt = Gitter::hbndseg_STI::closure ;
        }

        // read remaining vertices
        for( ; k<4; ++k )
        {
          in >> v[ k ];
        }
        ProjectVertexPtr pv; // empty projection
        // insert bnd object
        InsertUniqueHbnd4 (v, Gitter::hbndseg::bnd_t(bt), pv);
      }
    }
    else if ( type == TETRA_RAW )
    {
      int vp[ 6 ];
      for( int i=0; i<nper; ++i )
      {
        for( int j=0; j<6; ++j )
          in >> vp[ j ] ;

        Gitter::hbndseg::bnd_t bndId[ 2 ] = { Gitter::hbndseg::periodic, Gitter::hbndseg::periodic };
        InsertUniquePeriodic (vp, bndId );
      }

      int bt ;
      int v[ 3 ];
      for( int i=0; i<nb ; ++i )
      {
        in >> bt ;
        int k = 0 ;

        if( bt < 0 ) // exterior bnd
        {
          bt = -bt; // use positive value
          if( !Gitter::hbndseg_STI::bndRangeCheck( bt ) )
          {
            std::cerr << "ERROR (fatal): Boundary id = " << bt << " out of range (valid are " << Gitter::hbndseg_STI::validRanges() << ")." << std::endl;
            abort();
          }
        }
        else // interior parallel bnd
        {
          v[ k++ ] = bt ;
          bt = Gitter::hbndseg_STI::closure ;
        }

        // read remaining vertices
        for( ; k<3; ++k )
        {
          in >> v[ k ];
        }
        ProjectVertexPtr pv; // empty projection
        // insert bnd object
        InsertUniqueHbnd3 (v,Gitter::hbndseg::bnd_t(bt), pv);
      }
    }

    int linkagePatternSize ;
    in >> linkagePatternSize ;
    // is special situations we need to set linkagePatternSize
    //if(retur ()) linkagePatternSize = 1;

    // if linkage was writte, restore vertex linkage
    if( linkagePatternSize > 0 )
    {
      ++linkagePatternSize ; // include null pattern (which is the first entry)

      // mark linkage as computed (to avoid costly rebuild)
      myBuilder().linkageComputed();

      // read linkage combinations
      std::vector< linkagePattern_t > patterns( linkagePatternSize, linkagePattern_t() );
      // don't read null pattern (i=1)
      for( int i=1; i<linkagePatternSize; ++i )
      {
        int n;
        in >> n;
        if( n )
        {
          linkagePattern_t& pattern = patterns[ i ];
          pattern.resize( n );
          for( int k=0; k<n; ++k )
          {
            int rank ;
            in >> rank ;
            pattern[ k ] = rank ;
          }
        }
      }

      int hasElementLink = 0 ;
      in >> hasElementLink ;
      const bool hasElementLinkage = (hasElementLink == 1);

      typedef Gitter :: ElementPllXIF :: vertexelementlinkage_t vertexelementlinkage_t;
      vertexelementlinkage_t vxElemLinkage ;

      if( hasElementLinkage )
      {
        // compuate vertex-element linkage for hexas and tetras
        computeVertexElementLinkage( _hexaMap,  vxElemLinkage );
        computeVertexElementLinkage( _tetraMap, vxElemLinkage );

        // mark element linkage as computed (if available)
        myBuilder().notifyVertexElementLinkageComputed();
      }

      int idx = 0;
      // read position in linkage vector
      int vxId;
      in >> vxId;
      // set vertex linkage according to stores position
      vertexMap_t::iterator i = _vertexMap.begin ();
      while( vxId != -1 )
      {
        // advance iterator until pos is reached
        while( idx != vxId )
        {
          ++i;
          ++idx ;
        }

        int pos;
        in >> pos;
        alugrid_assert( pos < int(patterns.size()) );

        // vertex pointer
        Gitter::vertex_STI* vertex = (*i).second;

        if( hasElementLinkage )
        {
          std::set<int> elements;
          int size ;
          in >> size;
          for( int k=0; k<size; ++k )
          {
            int el;
            in >> el;
            elements.insert( el );
          }
          vertex->insertLinkedElements( elements );

          // erase computed linkage to avoid reinsertion
          vxElemLinkage.erase( vertex );
        }

        // set vertex linkage if not nullPattern
        if( patterns[ pos ].size() )
          vertex->setLinkageSorted( patterns[ pos ] );

        // read position in linkage vector
        in >> vxId;
      } // end while( vxId != -1 )

      if( hasElementLinkage )
      {
        // insert vertex-element linkage for remaining vertices (interior)
        typedef vertexelementlinkage_t :: iterator iterator ;
        const iterator end = vxElemLinkage.end();
        for( iterator i = vxElemLinkage.begin(); i != end; ++ i )
        {
          (*i).first->insertLinkedElements( (*i).second );
        }
      }
    }

    // get last std::endl character (from backup to make stream consistent)
    if( !in.eof() ) in.get();

    if( debugOption( 3 ) )
      std::cout << "INFO: MacroGridBuilder::inflateMacroGrid() used " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " s." << std::endl;
  }

  void Gitter::Geometric::BuilderIF::macrogridBuilder ( std::istream &in )
  {
    MacroFileHeader header;
    if( !header.read( in, true ) )
    {
      std::cerr << "ERROR (fatal): Unable to read macro grid header." << std::endl;
      std::abort();
    }

    MacroGridBuilder mm (*this);
    const int type = (header.type() == MacroFileHeader::tetrahedra ? MacroGridBuilder::TETRA_RAW : MacroGridBuilder::HEXA_RAW);
    if( header.isBinary() )
    {
      if( header.byteOrder() == MacroFileHeader::native || header.byteOrder() == systemByteOrder() )
      {
        ObjectStream os;
        ALUGrid::readBinary( in, os, header );
        mm.inflateMacroGrid( os, type );
      }
      else if( header.byteOrder() == MacroFileHeader::bigendian )
      {
        BigEndianObjectStream os;
        ALUGrid::readBinary( in, os, header );
        mm.inflateMacroGrid( os, type );
      }
      else if ( header.byteOrder() == MacroFileHeader::littleendian )
      {
        LittleEndianObjectStream os;
        ALUGrid::readBinary( in, os, header );
        mm.inflateMacroGrid( os, type );
      }
      else
      {
        std::cerr << "ERROR (fatal): byte order not available" << std::endl;
        std::abort();
      }
    }
    else
      mm.inflateMacroGrid( in, type );
  }

} // namespace ALUGrid
