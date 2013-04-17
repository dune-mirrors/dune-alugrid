// (c) bernhard schupp 1997 - 1998
#include <config.h>

#include <iomanip>

#include "../serial/gatherscatter.hh"
#include "../serial/serialize.h"
#include "../serial/gitter_mgb.h"

#include "gitter_pll_sti.h"
#include "gitter_pll_mgb.h"

#define USE_NONBLOCKING_SEND_PGM

namespace ALUGrid
{

  ParallelGridMover::ParallelGridMover ( BuilderIF &b )
  : MacroGridBuilder( b, false ) 
  {
    // lock MyAlloc so that objects are not freed  
    // because we want to reuse them  
    MyAlloc::lockFree ((void *) this);

    // initialize all lists 
    initialize();
  }

  // overloaded, because here we use the new insertInternal method 
  void ParallelGridMover::initialize ()
  {
    {
      BuilderIF::vertexlist_t& _vertexList = myBuilder ()._vertexList;
      const BuilderIF::vertexlist_t::iterator _vertexListend  = _vertexList.end ();
      for (BuilderIF::vertexlist_t::iterator i = _vertexList.begin ();
        i != _vertexListend; _vertexList.erase (i ++)) 
          _vertexMap [(*i)->ident ()] = (*i);
    }
    {
      BuilderIF::hedge1list_t& _hedge1List = myBuilder ()._hedge1List;
      const BuilderIF::hedge1list_t::iterator _hedge1Listend = _hedge1List.end ();
      for (BuilderIF::hedge1list_t::iterator i = _hedge1List.begin ();
           i != _hedge1Listend; _hedge1List.erase (i ++)) 
      {
        int k = (*i)->myvertex (0)->ident (), l = (*i)->myvertex (1)->ident ();
        _edgeMap [edgeKey_t (k < l ? k : l, k < l ? l : k)] = (*i);
      }
    }
    {
      BuilderIF::hface3list_t& _hface3List = myBuilder ()._hface3List;
      const BuilderIF::hface3list_t::iterator _hface3Listend = _hface3List.end ();
      for (BuilderIF::hface3list_t::iterator i = _hface3List.begin (); 
           i != _hface3Listend; _hface3List.erase (i ++)) 
      {
        _face3Map [faceKey_t ((*i)->myvertex (0)->ident (),(*i)->myvertex (1)->ident (), (*i)->myvertex (2)->ident ())] = (*i);
      }
    }
    {
      BuilderIF::hface4list_t& _hface4List = myBuilder ()._hface4List;
      const BuilderIF::hface4list_t::iterator _hface4Listend = _hface4List.end ();
      for (BuilderIF::hface4list_t::iterator i = _hface4List.begin (); 
           i != _hface4Listend; _hface4List.erase (i ++)) 
        _face4Map [faceKey_t ((*i)->myvertex (0)->ident (),(*i)->myvertex (1)->ident (), (*i)->myvertex (2)->ident ())] = (*i);
    }

    // all periodic elements (need to be removed before hbndseg and elements) 
    {
      BuilderIF::periodic3list_t& _periodic3List = myBuilder ()._periodic3List; 
      const BuilderIF::periodic3list_t::iterator _periodic3Listend = _periodic3List.end ();
      for (BuilderIF::periodic3list_t::iterator i = _periodic3List.begin (); 
           i != _periodic3Listend; _periodic3List.erase (i++)) 
      {
        // note: the last vertex is flipped because of confusion with tetra keys
        _periodic3Map [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
             (*i)->myvertex (2)->ident (), -((*i)->myvertex (3)->ident ())-1)] = (*i);
      }
      alugrid_assert ( _periodic3List.size() == 0 );
    }


    {
      BuilderIF::periodic4list_t& _periodic4List = myBuilder ()._periodic4List;
      const BuilderIF::periodic4list_t::iterator _periodic4Listend = _periodic4List.end ();
      for (BuilderIF::periodic4list_t::iterator i = _periodic4List.begin (); 
           i != _periodic4Listend; _periodic4List.erase (i++)) 
      {
        // note: the last vertex is flipped because of confusion with hexa keys
        _periodic4Map [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
             (*i)->myvertex (3)->ident (), -((*i)->myvertex (4)->ident ())-1)] = (*i);
      }
      alugrid_assert ( _periodic4List.size() == 0 );
    }


    // all boundary segments 
    typedef std::vector< Gitter::hbndseg_STI * > hbndvector_t;
    hbndvector_t  toDeleteHbnd;
    { 
      BuilderIF::hbndseg4list_t& _hbndseg4List = myBuilder ()._hbndseg4List;
      toDeleteHbnd.reserve( _hbndseg4List.size() );

      const BuilderIF::hbndseg4list_t::iterator _hbndseg4Listend = _hbndseg4List.end ();
      for (BuilderIF::hbndseg4list_t::iterator i = _hbndseg4List.begin (); 
           i != _hbndseg4Listend; _hbndseg4List.erase (i++)) 
      {
        typedef Gitter::Geometric::hface4_GEO hface4_GEO;
        hface4_GEO * face = (*i)->myhface4 (0);
        alugrid_assert ( face );
        faceKey_t key (face->myvertex (0)->ident (), 
                       face->myvertex (1)->ident (), 
                       face->myvertex (2)->ident ());

        // if internal face 
        if ((*i)->bndtype () == Gitter::hbndseg_STI::closure) 
        {
          typedef Gitter::ghostpair_STI ghostpair_STI;
          typedef Gitter::Geometric::hexa_GEO  hexa_GEO;

          ghostpair_STI gpair = (*i)->getGhost();
          hexa_GEO * gh = dynamic_cast<hexa_GEO *> (gpair.first);
          if( gh )
          {
            _hbnd4Int [key] = new Hbnd4IntStorage (face , (*i)->twist (0), 
                                                   (*i)->ldbVertexIndex(),
                                                   (*i)->master(),
                                                   gh, gpair.second );
          }
          else 
            _hbnd4Int [key] = new Hbnd4IntStorage (face ,(*i)->twist (0),
                                                   (*i)->ldbVertexIndex(), (*i)->master() );

          toDeleteHbnd.push_back( (*i ) );
        } 
        else 
        {
          _hbnd4Map [key] = (*i);
        }
      }
    }
    
    {
      BuilderIF::hbndseg3list_t& _hbndseg3List = myBuilder ()._hbndseg3List; 
      toDeleteHbnd.reserve( toDeleteHbnd.size() + _hbndseg3List.size() );

      const BuilderIF::hbndseg3list_t::iterator _hbndseg3Listend = _hbndseg3List.end ();
      for (BuilderIF::hbndseg3list_t::iterator i = _hbndseg3List.begin (); 
           i != _hbndseg3Listend; _hbndseg3List.erase (i++)) 
      {
        typedef Gitter::Geometric::hface3_GEO hface3_GEO;
        hface3_GEO * face = (*i)->myhface3 (0);
        alugrid_assert ( face );
        faceKey_t key ( face->myvertex (0)->ident (), face->myvertex (1)->ident (), face->myvertex (2)->ident ());
        // if internal face 
        if ((*i)->bndtype () == Gitter::hbndseg_STI::closure) 
        {
          // check for ghost element 
          typedef Gitter::ghostpair_STI ghostpair_STI;
          ghostpair_STI gpair = (*i)->getGhost();

          typedef Gitter::Geometric::tetra_GEO  tetra_GEO;
          tetra_GEO * gh = dynamic_cast<tetra_GEO *> (gpair.first);
          if( gh )
          {
            // insert new internal storage 
            _hbnd3Int [key] = new Hbnd3IntStorage ( face , (*i)->twist (0), 
                                                    (*i)->ldbVertexIndex(), (*i)->master(),
                                                    gh , gpair.second );
          }
          // until here
          else 
            _hbnd3Int [key] = new Hbnd3IntStorage ( face , (*i)->twist (0),
                                                    (*i)->ldbVertexIndex(), (*i)->master() );
          
          toDeleteHbnd.push_back( (*i) );
        } 
        else 
        {
          _hbnd3Map [key] = (*i);
        }
      }
    }

    // all elements 
    {
      BuilderIF::tetralist_t& _tetraList = myBuilder ()._tetraList; 
      const BuilderIF::tetralist_t::iterator _tetraListend = _tetraList.end ();
      for (BuilderIF::tetralist_t::iterator i = _tetraList.begin (); 
           i != _tetraListend; _tetraList.erase (i++)) 
      {
        _tetraMap [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
             (*i)->myvertex (2)->ident (), (*i)->myvertex (3)->ident ())] = (*i);
      } 
    }
    {
      BuilderIF::hexalist_t& _hexaList = myBuilder()._hexaList;
      const BuilderIF::hexalist_t::iterator _hexaListend = _hexaList.end ();
      for (BuilderIF::hexalist_t::iterator i = _hexaList.begin (); 
           i != _hexaListend; _hexaList.erase (i++)) 
      {
        _hexaMap [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
                  (*i)->myvertex (3)->ident (), (*i)->myvertex (4)->ident ())] = (*i);
      }
    }

    /////////////////////////////////////////

    // from constructor ParallelGridMover 
    std::vector< elementKey_t > toDelete;

    // reserve memory 
    toDelete.reserve( _hexaMap.size() + _tetraMap.size() );
    
    {
      const elementMap_t::iterator _hexaMapend = _hexaMap.end ();
      for (elementMap_t::iterator i = _hexaMap.begin (); i != _hexaMapend; ++i)
      {
        if( ((hexa_GEO *) (*i).second)->erasable () ) 
        {
          toDelete.push_back ((*i).first);
        }
      }
    }
    {
      const elementMap_t::iterator _tetraMapend = _tetraMap.end ();
      for (elementMap_t::iterator i = _tetraMap.begin (); i != _tetraMapend; ++i)
      {
        if ( ((tetra_GEO *)(*i).second)->erasable () ) 
        {
          toDelete.push_back ((*i).first);
        }
      }
    }

    // delete all internal boundaries 
    { 
      typedef hbndvector_t::iterator  iterator; 
      const iterator endi = toDeleteHbnd.end();
      for( iterator i = toDeleteHbnd.begin(); i != endi; ++i ) 
        delete (*i);
    }

    // delete all periodic boundaries 
    {
      const elementMap_t::iterator _periodic3Mapend = _periodic3Map.end ();
      for (elementMap_t::iterator i = _periodic3Map.begin (); i != _periodic3Mapend; ++i)
      {
        if ( ((periodic3_GEO *)(*i).second)->erasable () ) 
        {
          // false means periodic element 
          removeElement ((*i).first, false );
        }
      }

      const elementMap_t::iterator _periodic4Mapend = _periodic4Map.end ();
      for (elementMap_t::iterator i = _periodic4Map.begin (); i != _periodic4Mapend; ++i)
      {
        if ( ((periodic4_GEO *)(*i).second)->erasable ()) 
        {
          // false means periodic element 
          removeElement ((*i).first, false );
        }
      }
    }

    // delete all elements at last
    {
      const std::vector< elementKey_t >::iterator toDeleteend = toDelete.end (); 
      for (std::vector< elementKey_t >::iterator i = toDelete.begin (); i != toDeleteend; ++i )
      {
        // true means we have a real element 
        removeElement (*i, true);
      }
    }

    this->_initialized = true;
    return; 
  }

  // overloaded, because here we use the new insertInternal method 
  void ParallelGridMover::finalize ()
  {
    // copy elements from hexaMap to hexaList by 
    // respecting the order given by ldbVertexIndex 
    // false indicates that the vertex should not be set
    MacroGridBuilder::hexaMapToList( _hexaMap, myBuilder ()._hexaList, false );

    // copy elements from tetraMap to tetraList by 
    // respecting the order given by ldbVertexIndex 
    // false indicates that the vertex should not be set
    MacroGridBuilder::tetraMapToList( _tetraMap, myBuilder ()._tetraList, false );

    {
      BuilderIF::periodic3list_t& _periodic3List = myBuilder ()._periodic3List; 
      const elementMap_t::iterator _periodic3Mapend = _periodic3Map.end ();
      for (elementMap_t::iterator i = _periodic3Map.begin (); i != _periodic3Mapend; _periodic3Map.erase (i++))
      {
        // if the periodic element is only there without connections 
        // to real elements then delete it (check why this can happen)
        periodic3_GEO * periodic = (periodic3_GEO *) (*i).second;
        if( periodic->myhface3( 0 )->ref == 1 && periodic->myhface3( 1 )->ref == 1 ) 
        {
          delete periodic;
        }
        else 
        {
          alugrid_assert ( periodic->myhface3( 0 )->ref == 2 );
          alugrid_assert ( periodic->myhface3( 1 )->ref == 2 );
          _periodic3List.push_back ( periodic );
        }
      }
    }
    
    {
      BuilderIF::periodic4list_t& _periodic4List = myBuilder ()._periodic4List;
      const elementMap_t::iterator _periodic4Mapend =  _periodic4Map.end ();
      for (elementMap_t::iterator i = _periodic4Map.begin (); i != _periodic4Mapend; _periodic4Map.erase (i++))
      {
        // if the periodic element is only there without connections 
        // to real elements then delete it (check why this can happen)
        periodic4_GEO * periodic = (periodic4_GEO *) (*i).second;
        if( periodic->myhface4( 0 )->ref == 1 && periodic->myhface4( 1 )->ref == 1 ) 
        {
          delete periodic;
        }
        else 
        {
          alugrid_assert ( periodic->myhface4( 0 )->ref == 2 );
          alugrid_assert ( periodic->myhface4( 1 )->ref == 2 );
          _periodic4List.push_back ( periodic );
        }
      }
    }

    {
      BuilderIF::hbndseg4list_t& _hbndseg4List = myBuilder ()._hbndseg4List;
      const faceMap_t::iterator _hbnd4Mapend = _hbnd4Map.end ();
      for (faceMap_t::iterator i = _hbnd4Map.begin (); i != _hbnd4Mapend; )
      {
        hbndseg4_GEO * hbnd = (hbndseg4_GEO *)(*i).second;
        if ( hbnd->myhface4 (0)->ref == 1 ) 
        {
          delete hbnd;
          _hbnd4Map.erase (i++);
        } 
        else 
        {
          _hbndseg4List.push_back ( hbnd );
          ++ i;
        }
      }
    }

    {
      BuilderIF::hbndseg3list_t& _hbndseg3List = myBuilder ()._hbndseg3List;
      const faceMap_t::iterator _hbnd3Mapend = _hbnd3Map.end ();
      for (faceMap_t::iterator i = _hbnd3Map.begin (); i != _hbnd3Mapend; )
      {
        hbndseg3_GEO * hbnd = (hbndseg3_GEO *)(*i).second;
        if ( hbnd->myhface3 (0)->ref == 1) 
        {
          delete hbnd;
          _hbnd3Map.erase (i++);
        } 
        else 
        {
          _hbndseg3List.push_back ( hbnd );
          ++ i;
        }
      }
    }

    {
      BuilderIF::hbndseg4list_t& _hbndseg4List = myBuilder ()._hbndseg4List; 
      const hbnd4intMap_t::iterator _hbnd4Intend = _hbnd4Int.end ();
      for (hbnd4intMap_t::iterator i = _hbnd4Int.begin (); i != _hbnd4Intend; ++i) 
      {
        Hbnd4IntStorage* p = (*i).second;
        if (p->first()->ref == 1) 
        {
          // get ghost info from storage and release pointer 
          MacroGhostInfoHexa* ghInfo = p->release();

          hbndseg4_GEO * hb4 = myBuilder ().
                insert_hbnd4 (p->first(), p->second(), 
                              Gitter::hbndseg_STI::closure, ghInfo );
          hb4->setLoadBalanceVertexIndex( p->ldbVertexIndex() );
          hb4->setMaster( p->master() );
          _hbndseg4List.push_back (hb4);
        }
        delete p; 
      } 
    }

    // here the internal boundary elements are created 
    {
      BuilderIF::hbndseg3list_t& _hbndseg3List = myBuilder ()._hbndseg3List; 
      const hbnd3intMap_t::iterator _hbnd3Intend = _hbnd3Int.end ();
      for (hbnd3intMap_t::iterator i = _hbnd3Int.begin (); i != _hbnd3Intend; ++i ) 
      {
        Hbnd3IntStorage* p = (*i).second;
        if (p->first()->ref == 1) 
        {
          // get ghost info from storage and release pointer 
          MacroGhostInfoTetra* ghInfo = p->release();

          hbndseg3_GEO * hb3 = myBuilder().insert_hbnd3( p->first(), p->second(),
                                                         Gitter::hbndseg_STI::closure, 
                                                         ghInfo );
          alugrid_assert ( p->ldbVertexIndex() >= 0 );
          hb3->setLoadBalanceVertexIndex( p->ldbVertexIndex() );
          hb3->setMaster( p->master() );

          // insert to list 
          _hbndseg3List.push_back (hb3);
        }
        delete p; 
      }
    }
    {
      BuilderIF::hface4list_t& _hface4List = myBuilder ()._hface4List;
      const faceMap_t::iterator _face4Mapend = _face4Map.end ();
      for (faceMap_t::iterator i = _face4Map.begin (); i != _face4Mapend; )
      {
        hface4_GEO * face = (hface4_GEO * ) (*i).second;
        if (! face->ref ) 
        {
          delete face;
          _face4Map.erase ( i++ );
        } 
        else 
        {
          alugrid_assert ( face->ref == 2 );
          _hface4List.push_back ( face );
          ++ i;
        }
      }
    }
    {
      BuilderIF::hface3list_t& _hface3List = myBuilder ()._hface3List;
      const faceMap_t::iterator _face3Mapend = _face3Map.end ();
      for (faceMap_t::iterator i = _face3Map.begin (); i != _face3Mapend; ) 
      {
        hface3_GEO * face = (hface3_GEO *) (*i).second;
        if (! face->ref ) 
        {
          delete face;
          _face3Map.erase (i++);
        } 
        else 
        {
           alugrid_assert ( face->ref == 2 ); 
          _hface3List.push_back ( face );
          ++ i;
        }
      }
    }
    {
      BuilderIF::hedge1list_t& _hedge1List = myBuilder ()._hedge1List;
      const edgeMap_t::iterator _edgeMapend = _edgeMap.end ();
      for (edgeMap_t::iterator i = _edgeMap.begin (); i != _edgeMapend; )
      {
        hedge1_GEO* edge = (hedge1_GEO *) (*i).second;
        if (! edge->ref )
        {
          delete edge;
          _edgeMap.erase (i++);
        } 
        else 
        {
          alugrid_assert ( edge->ref >= 1 );
          _hedge1List.push_back ( edge );
          ++ i;
        }
      }
    }
    {
      BuilderIF::vertexlist_t& _vertexList = myBuilder ()._vertexList;
      const vertexMap_t::iterator _vertexMapend = _vertexMap.end ();
      for (vertexMap_t::iterator i = _vertexMap.begin (); i != _vertexMapend; )
      {
        vertex_GEO* vertex = (vertex_GEO *) (*i).second;
        if ( ! vertex->ref ) 
        {
          delete vertex;
          _vertexMap.erase (i++);
        } 
        else 
        {
          alugrid_assert ( vertex->ref >= 2);
          _vertexList.push_back ( vertex );
          ++i;
        }
      }
    }

    this->_finalized = true;
    return;
  }

  ParallelGridMover::~ParallelGridMover () 
  {
    alugrid_assert (_initialized);

    if(!_finalized)
    {   
      // compress index manager before new elements are created 
      myBuilder().compressIndexManagers();
      
      // finalize mover 
      finalize();
    }     

    // unlock MyAlloc so that objects can be freed again 
    MyAlloc::unlockFree ((void *) this);

    return;
  }

  void ParallelGridMover::unpackVertex ( ObjectStream &os )
  {
    int id;
    double x, y, z;
    os.readObject (id);
    os.readObject (x);
    os.readObject (y);
    os.readObject (z);
    std::pair< VertexGeo *, bool > p = InsertUniqueVertex (x,y,z,id);
    p.first->accessPllX ().unpackSelf (os,p.second);
  }

  void ParallelGridMover::unpackHedge1 (ObjectStream & os) {
    int left, right;
    os.readObject (left);
    os.readObject (right);
    std::pair< hedge1_GEO *, bool > p = InsertUniqueHedge (left,right);
    p.first->unpackSelf (os,p.second);
    return;
  }

  void ParallelGridMover::unpackHface3 (ObjectStream & os) 
  {
    int v [3];
    os.readObject (v[0]);
    os.readObject (v[1]);
    os.readObject (v[2]);
    std::pair< hface3_GEO *, bool > p = InsertUniqueHface (v);
    p.first->unpackSelf (os,p.second);
    return;
  }

  void ParallelGridMover::unpackHface4 (ObjectStream & os) 
  {
    int v [4];
    os.readObject (v[0]);
    os.readObject (v[1]);
    os.readObject (v[2]);
    os.readObject (v[3]);
    std::pair< hface4_GEO *, bool > p = InsertUniqueHface (v);
    p.first->unpackSelf (os,p.second);
    return;
  }

  void ParallelGridMover::unpackTetra (ObjectStream & os, GatherScatterType* gs ) 
  {
    int ldbVertexIndex = -1;
    int v [4];
    os.readObject (ldbVertexIndex); 
    os.readObject (v[0]);
    os.readObject (v[1]);
    os.readObject (v[2]);
    os.readObject (v[3]);
    int orientation = 0;
    os.readObject( orientation ); 
    std::pair< tetra_GEO *, bool > p = InsertUniqueTetra (v, orientation);
    // set unique element number 
    p.first->setLoadBalanceVertexIndex( ldbVertexIndex );
    p.first->accessPllX ().duneUnpackSelf (os, p.second, gs);
    return;
  }

  void ParallelGridMover::unpackPeriodic3 (ObjectStream & os) 
  {
    // read boundary ids 
    int bnd[ 2 ];
    os.readObject (bnd[ 0 ]);
    os.readObject (bnd[ 1 ]);
    Gitter::hbndseg::bnd_t b[ 2 ] = { 
      (Gitter::hbndseg::bnd_t) bnd[ 0 ],
      (Gitter::hbndseg::bnd_t) bnd[ 1 ] };

    // read vertex ids 
    int v [6];
    os.readObject (v[0]);
    os.readObject (v[1]);
    os.readObject (v[2]);
    os.readObject (v[3]);
    os.readObject (v[4]);
    os.readObject (v[5]);

    std::pair< periodic3_GEO *, bool > p = InsertUniquePeriodic (v, b);
    p.first->accessPllX ().unpackSelf (os,p.second);
    return;
  } 

  void ParallelGridMover::unpackPeriodic4 (ObjectStream & os) 
  {
    // read boundary ids 
    int bnd[ 2 ];
    os.readObject (bnd[ 0 ]);
    os.readObject (bnd[ 1 ]);
    Gitter::hbndseg::bnd_t b[ 2 ] = { 
      (Gitter::hbndseg::bnd_t) bnd[ 0 ],
      (Gitter::hbndseg::bnd_t) bnd[ 1 ] };

    // read vertex ids 
    int v [8];
    os.readObject (v[0]);
    os.readObject (v[1]);
    os.readObject (v[2]);
    os.readObject (v[3]);
    os.readObject (v[4]);
    os.readObject (v[5]);
    os.readObject (v[6]);
    os.readObject (v[7]);
    std::pair< periodic4_GEO *, bool > p = InsertUniquePeriodic (v, b);
    p.first->accessPllX ().unpackSelf (os,p.second);
    return;
  }

  void ParallelGridMover::unpackHexa (ObjectStream & os, GatherScatterType* gs) 
  {
    int ldbVertexIndex = -1;
    int v [8];
    os.readObject (ldbVertexIndex); 
    os.readObject (v[0]);
    os.readObject (v[1]);
    os.readObject (v[2]);
    os.readObject (v[3]);
    os.readObject (v[4]);
    os.readObject (v[5]);
    os.readObject (v[6]);
    os.readObject (v[7]);
    std::pair< hexa_GEO *, bool > p = InsertUniqueHexa (v);
    // set unique element number 
    p.first->setLoadBalanceVertexIndex( ldbVertexIndex );
    p.first->accessPllX ().duneUnpackSelf (os, p.second, gs );
    return;
  }

  // new method that gets coord of ghost point 
  bool ParallelGridMover::
  InsertUniqueHbnd3_withPoint (int (&v)[3],         
                               Gitter::hbndseg_STI ::bnd_t bt, 
                               int ldbVertexIndex,
                               int master,
                               MacroGhostInfoTetra * ghInfo) 
  {
    int twst = cyclicReorder (v,v+3);
    faceKey_t key (v [0], v [1], v [2]);
    alugrid_assert ( bt == Gitter::hbndseg_STI::closure ); 
    if (_hbnd3Int.find (key) == _hbnd3Int.end ()) 
    {
      alugrid_assert ( ghInfo );
      hface3_GEO * face =  InsertUniqueHface3 (v).first;
      // here the point is stored 
      _hbnd3Int [key] = new Hbnd3IntStorage (face,twst, ldbVertexIndex, master, ghInfo);
      return true;
    }
    return false;
  }

  // new method that gets coord of ghost point 
  bool ParallelGridMover::
  InsertUniqueHbnd4_withPoint (int (&v)[4],         
                               Gitter::hbndseg_STI ::bnd_t bt, 
                               int ldbVertexIndex,
                               int master,
                               MacroGhostInfoHexa* ghInfo) 
  {
    int twst = cyclicReorder (v,v+4);
    faceKey_t key (v [0], v [1], v [2]);
    alugrid_assert ( bt == Gitter::hbndseg_STI::closure );
    if (_hbnd4Int.find (key) == _hbnd4Int.end ()) 
    {
      alugrid_assert ( ghInfo );
      hface4_GEO * face =  InsertUniqueHface4 (v).first;
      _hbnd4Int [key] = new Hbnd4IntStorage (face, twst, ldbVertexIndex, master, ghInfo);
      return true;
    }
    return false;
  }

  // overloaded method because here we call insertion with point 
  inline void ParallelGridMover::unpackHbnd3Int (ObjectStream & os) 
  {
    // see also gitter_{tetra,hexa}_pll_top.* for methods buildGhost 
    int bfake, v [3];
    os.readObject (bfake);
    Gitter::hbndseg::bnd_t b = (Gitter::hbndseg::bnd_t) bfake;

    int ldbVertexIndex,master;
    os.readObject( ldbVertexIndex );
    os.readObject( master );
    
    os.readObject (v[0]);
    os.readObject (v[1]);
    os.readObject (v[2]);

    int readPoint = 0; 
    os.readObject( readPoint ); 

    MacroGhostInfoTetra * ghInfo = 0;
    if( readPoint == MacroGridMoverIF::POINTTRANSMITTED ) 
    {
      // read ghost data from stream in any case 
      ghInfo = new MacroGhostInfoTetra( os ); 
    }

    // if internal boundary, create internal bnd face 
    if( b == Gitter::hbndseg::closure && ghInfo )
    {
      // ghInfo is stored in the macro ghost created internally
      const bool inserted = InsertUniqueHbnd3_withPoint (v, b, ldbVertexIndex, master, ghInfo );

      // if inserted then clear pointer to avoid deleting it 
      if( inserted ) ghInfo = 0;
    }
    else 
    {
      // create normal bnd face, and make sure that no Point was send
      alugrid_assert ( readPoint == MacroGridMoverIF::NO_POINT );
      // old method defined in base class 
      InsertUniqueHbnd3 (v, b, ldbVertexIndex, master );
    }

    // delete to avoid memory leak 
    if( ghInfo ) delete ghInfo;
  }

  // overloaded method because here we call insertion with point 
  inline void ParallelGridMover::unpackHbnd4Int (ObjectStream & os) 
  {
    // see also gitter_{tetra,hexa}_pll_top.* for methods buildGhost 
    int bfake, v [4] = {-1,-1,-1,-1};

    os.readObject (bfake);
    Gitter::hbndseg::bnd_t b = (Gitter::hbndseg::bnd_t) bfake;

    int ldbVertexIndex,master;
    os.readObject( ldbVertexIndex );
    os.readObject( master );
    
    os.readObject (v[0]);
    os.readObject (v[1]);
    os.readObject (v[2]);
    os.readObject (v[3]);

    int readPoint = 0; 
    os.readObject( readPoint ); 
    
    MacroGhostInfoHexa* ghInfo = 0;
    if( readPoint == MacroGridMoverIF::POINTTRANSMITTED ) 
    {
      // read ghost data from stream 
      ghInfo = new MacroGhostInfoHexa(os); 
    }

    // if internal boundary, create internal bnd face 
    if(b == Gitter::hbndseg::closure && ghInfo )
    {
      // ghInfo is stored in the macro ghost created internally
      const bool inserted = InsertUniqueHbnd4_withPoint (v, b, ldbVertexIndex, master, ghInfo );

      // if inserted then clear pointer to avoid deleting it 
      if( inserted ) ghInfo = 0;
    }
    else 
    {
      // create normal bnd face, and make sure that no Point was send
      alugrid_assert ( readPoint == MacroGridMoverIF::NO_POINT );
      // old method defined in base class 
      InsertUniqueHbnd4 (v, b, ldbVertexIndex, master );
    }

    // delete to avoid memory leak 
    if( ghInfo ) delete ghInfo;
  }

  void ParallelGridMover::unpackHbnd3Ext (ObjectStream & os) 
  {
    int b, v [3];
    os.readObject (b);
    os.readObject (v[0]);
    os.readObject (v[1]);
    os.readObject (v[2]);
    int ldbVertexIndex = -1;
    int master = -1;
    InsertUniqueHbnd3 (v, Gitter::hbndseg::bnd_t (b), ldbVertexIndex, master);
    return;
  }
    
  void ParallelGridMover::unpackHbnd4Ext (ObjectStream & os) {
    int b, v [4];
    os.readObject (b);
    os.readObject (v[0]);
    os.readObject (v[1]);
    os.readObject (v[2]);
    os.readObject (v[3]);
    int ldbVertexIndex = -1;
    int master = -1;
    InsertUniqueHbnd4 (v, Gitter::hbndseg::bnd_t (b), ldbVertexIndex, master);
    return;
  }

  void ParallelGridMover::
  unpackAll( ObjectStream& os, GatherScatterType* gs) 
  {
    int code = MacroGridMoverIF::ENDMARKER;
    for (os.readObject (code); code != MacroGridMoverIF::ENDMARKER; os.readObject (code)) 
    {
      switch (code) {
      case MacroGridMoverIF:: VERTEX :
        {
          unpackVertex (os);
          break;
        }
      case MacroGridMoverIF::EDGE1 :
        {
          unpackHedge1 (os);
          break;
        }
      case MacroGridMoverIF::FACE3 :
        {
          unpackHface3 (os);
          break;
        }
      case MacroGridMoverIF::FACE4 :
        {
          unpackHface4 (os);
          break;
        }
      case MacroGridMoverIF::TETRA :
        {
          unpackTetra (os, gs);
          break;
        }
      case MacroGridMoverIF::HEXA :
        {
          unpackHexa (os, gs);
          break;
        }
      case MacroGridMoverIF::PERIODIC3 :
        {
          unpackPeriodic3 (os);
          break;
        }
      case MacroGridMoverIF::PERIODIC4 :
        {
          unpackPeriodic4 (os);
          break;
        }
      case MacroGridMoverIF::HBND3INT :
        {
          unpackHbnd3Int (os);
          break;
        }
      case MacroGridMoverIF::HBND3EXT :
        {
          unpackHbnd3Ext (os);
          break;
        }
      case MacroGridMoverIF::HBND4INT :
        {
          unpackHbnd4Int (os);
          break; 
        }
      case MacroGridMoverIF::HBND4EXT :
        {
          unpackHbnd4Ext (os);
          break;
        }
      default :
        std::cerr << "**FEHLER (FATAL) Unbekannte Gitterobjekt-Codierung gelesen [" << code << "] on p = " << __STATIC_myrank << "\n";
        std::cerr << "  Weitermachen unm\"oglich. In " << __FILE__ << " " << __LINE__ << std::endl;
        alugrid_assert (false);
        abort ();
        break;
      }
    }  
  }

  class UnpackLBData : public MpAccessLocal::NonBlockingExchange::DataHandleIF
  {
    GitterPll::MacroGitterPll& _containerPll;
    ParallelGridMover*  _pgm;
    GatherScatterType* _gs; 

#ifdef USE_NONBLOCKING_SEND_PGM
    std::vector< std::vector< MacroGridMoverIF* > > _vertices;
    std::vector< std::vector< MacroGridMoverIF* > > _edges;
    std::vector< std::vector< MacroGridMoverIF* > > _faces;
    const int _estimateSize;
#endif

    UnpackLBData( const UnpackLBData& );
  public:
    // constructor 
    UnpackLBData( GitterPll::MacroGitterPll& containerPll, 
                  const int nLinks,
                  GatherScatterType* gs ) 
      : _containerPll( containerPll ),
        _pgm( 0 ),
        _gs( gs )
#ifdef USE_NONBLOCKING_SEND_PGM
        , _vertices( nLinks ),
        _edges( nLinks ),
        _faces( nLinks ),
        _estimateSize( typename AccessIterator < Gitter::hface_STI >::Handle(_containerPll).size() )
    {
      // create lists of items that should be packed
      addItems< Gitter::vertex_STI > ( _vertices );
      addItems< Gitter::hedge_STI  > ( _edges );
      addItems< Gitter::hface_STI >  ( _faces );
    }
#else 
    {}
#endif

    // destructor deleting parallel macro grid mover
    ~UnpackLBData() { delete _pgm; }

#ifdef USE_NONBLOCKING_SEND_PGM
    template <class item_STI>
    void addItems( std::vector< std::vector< MacroGridMoverIF* > >& items ) 
    {
      typename AccessIterator < item_STI >::Handle w ( _containerPll );
      const int nLinks = items.size();
      for( int i=0; i<nLinks; ++i ) items[ i ].reserve( _estimateSize );
      for (w.first (); ! w.done (); w.next ()) 
      {
        w.item().addAll( items );
      }
    }

    void packItems( std::vector< MacroGridMoverIF* >& items, const int link, ObjectStream& os ) 
    {
      typedef typename std::vector< MacroGridMoverIF* > :: iterator iterator ;
      const iterator end = items.end();
      for( iterator it = items.begin(); it != end ; ++it ) 
      {
        (*it)->packLink( link, os );
      }
    }
#endif

    void pack( const int link, ObjectStream& os ) 
    {
#ifdef USE_NONBLOCKING_SEND_PGM
      // pack vertices 
      packItems( _vertices[ link ], link, os );
      // pack edges  
      packItems( _edges[ link ], link, os );
      // pack faces 
      packItems( _faces[ link ], link, os );

      // pack elements
      {
        AccessIterator < Gitter::helement_STI >::Handle w ( _containerPll );
        for (w.first (); ! w.done (); w.next ()) w.item ().packLink( link, os, _gs );
      }
      // pack periodic elements 
      {
        AccessIterator < Gitter::hperiodic_STI >::Handle w ( _containerPll );
        for (w.first (); ! w.done (); w.next ()) w.item ().packLink( link, os );
      }

      // write end marker 
      os.writeObject (MacroGridMoverIF::ENDMARKER);
#else
      std::cerr << "ERROR: UnpackLBData::pack should not be called!" << std::endl;
      abort();
#endif
    }

    // work that can be done between send and receive, 
    // such as the construction of the ParallelGridMover 
    void meantimeWork () 
    {
      // create ParallelGridMover when all data was packed, otherwise the link packing
      // will fail since this will modify the macro grid, since the 
      // parallel macro grid mover clears the lists of macro elements 
      if( ! _pgm ) _pgm = new ParallelGridMover( _containerPll );
    }

    void unpack( const int link, ObjectStream& os ) 
    {
      alugrid_assert ( _pgm );
      // unpack data for given stream 
      _pgm->unpackAll( os, _gs );
    }
  };

  // method was overloaded because here we use our DuneParallelGridMover 
  void GitterPll::doRepartitionMacroGrid ( LoadBalancer::DataBase &db, GatherScatterType *gatherScatter )
  {
    // in case gatherScatter is given check for overloaded partitioning 
    const bool userDefinedPartitioning = gatherScatter && gatherScatter->userDefinedPartitioning();

    // default partitioning method 
    // for user defined paritioning gatherScatter.partitioning() was called in gitter_pll_sti.cc before 
    // calling this method and returned true - that method should thus already have compute the 
    // new partitioning. For the internal partitioner, repartition is called heer and
    // could still lead to no repartitioning being carried out
    const bool doRepartition = userDefinedPartitioning ? true : 
      db.repartition (mpAccess (), LoadBalancer::DataBase::method (_ldbMethod), _ldbOver );

    // get graph sizes from data base, this is only used for the serial partitioners
    if( ! userDefinedPartitioning ) 
    {
      _graphSizes = db.graphSizes();
    }
      
    // time meassure 
    const long start = clock ();
    long lap1 (start), lap2 (start), lap3 (start), lap4 (start);

    // if partitining should be done, erase linkage and setup moveto directions 
    if( doRepartition ) 
    {
      typedef LoadBalancer::DataBase::ldb_connect_set_t  ldb_connect_set_t;
      ldb_connect_set_t connect;
      const ldb_connect_set_t* connectScan = &connect;

      // setup connection set in case if user defined partitioning 
      if( userDefinedPartitioning ) 
      {
        // attach all elements to their new destinations 
        AccessIterator < helement_STI >::Handle w (containerPll ());
        for (w.first (); ! w.done (); w.next ()) 
        {
          connect.insert( gatherScatter->destination( w.item() ) );
        }
      }
      else // take connections from db in default version
        connectScan = & db.scan();

      mpAccess ().removeLinkage ();
      mpAccess ().insertRequestSymetric ( *connectScan );

      const int me = mpAccess ().myrank (); 
      const int nl = mpAccess ().nlinks (); 
      {
        if ( ! userDefinedPartitioning )
        {
          // iterate over all periodic elements and set 'to' of first neighbour
          AccessIterator < hperiodic_STI >::Handle w (containerPll ());
          for (w.first (); ! w.done (); w.next ())
          {
            // get both ldbVertices from the elements of a periodic closure 
            std::pair< int, int > ldbVx = w.item().insideLdbVertexIndex();

            int moveTo = me;
            // check destinations of both elements 
            // of a periodic element 
            if( ldbVx.first >= 0 ) 
            {
              const int to = db.destination ( ldbVx.first );
              if( to != me ) 
                moveTo = to;
            }

            if( ldbVx.second >= 0 && moveTo == me ) 
            {
              const int to = db.destination ( ldbVx.second );
              if( to != me ) 
                moveTo = to;
            }

            // if moveTo is not me than attach periodic element 
            // and all connected real elements 
            if( moveTo != me ) 
            {
              w.item ().attach2 ( mpAccess ().link( moveTo ) );
            }
          }
        }

        // attach all elements to their new destinations 
        { 
          // if not userDefinedPartitioning, then pass null pointer 
          GatherScatterType* gsDestination = userDefinedPartitioning ? gatherScatter : 0;
          AccessIterator < helement_STI >::Handle w (containerPll ());
          for (w.first (); ! w.done (); w.next ()) 
          {
            helement_STI& item = w.item();
            // moveTo < 0 means the element has not been assigned yet
            if( item.moveTo() < 0 ) 
            { 
              const int to = db.destination ( item, gsDestination );
              if (me != to)
              {
                item.attach2 ( mpAccess ().link (to) );
              }
            }
          }
        }
      }
      lap1 = clock ();

#ifndef USE_NONBLOCKING_SEND_PGM
      // message buffers 
      std::vector< ObjectStream > osv( nl );

      // pack vertices 
      {
        AccessIterator < vertex_STI >::Handle w (containerPll ());
        for (w.first (); ! w.done (); w.next ()) w.item().packAll( osv );
      }
      // pack edges 
      {
        AccessIterator < hedge_STI >::Handle w (containerPll ());
        for (w.first (); ! w.done (); w.next ()) w.item().packAll( osv );
      }
      // pack faces  
      {
        AccessIterator < hface_STI >::Handle w (containerPll ());
        for (w.first (); ! w.done (); w.next ()) w.item().packAll( osv );
      }
      // pack elements 
      {
        AccessIterator < helement_STI >::Handle w (containerPll ());
        if( gatherScatter ) 
        {
          GatherScatterType& gs = *gatherScatter ;
          for (w.first (); ! w.done (); w.next ()) w.item().dunePackAll( osv, gs );
        }
        else 
          for (w.first (); ! w.done (); w.next ()) w.item().packAll( osv );
      }
      // pack periodic elements
      {
        AccessIterator < hperiodic_STI >::Handle w (containerPll ());
        for (w.first (); ! w.done (); w.next ()) w.item().packAll( osv );
      }

      // write end marker to terminate stream 
      for( int link=0; link<nl; ++link ) 
        osv[ link ].writeObject( MacroGridMoverIF::ENDMARKER );
#endif

      lap2 = clock ();
      
      {
        // data handle  
        UnpackLBData data( containerPll (), nl, gatherScatter );

        // pack, exchange, and unpack data 
#ifndef USE_NONBLOCKING_SEND_PGM
        mpAccess ().exchange ( osv, data );
#else 
        mpAccess ().exchange ( data );
#endif
      }

      lap3 = clock ();
#ifdef ALUGRIDDEBUG
      if( conformingClosureNeeded() ) 
      {
        // check that all leaf elements are in conforming status (bisection only)
        bool x = false ;
        LeafIterator< helement_STI > i( *this );
        for( i->first(); ! i->done(); i->next()) { x &= i->item().markForConformingClosure (); }
        alugrid_assert ( x == false );
      }
#endif

      // result 
      lap4 = clock ();
      if (MacroGridBuilder::debugOption (20)) {
        std::cout << "**INFO GitterPll["<<me<<"]::doRepartitionMacroGrid () [ass|pck|exc|upk|all] ";
        std::cout << std::setw (5) << (float)(lap1 - start)/(float)(CLOCKS_PER_SEC) << " ";
        std::cout << std::setw (5) << (float)(lap2 - lap1)/(float)(CLOCKS_PER_SEC) << " ";
        std::cout << std::setw (5) << (float)(lap3 - lap2)/(float)(CLOCKS_PER_SEC) << " ";
        std::cout << std::setw (5) << (float)(lap4 - lap3)/(float)(CLOCKS_PER_SEC) << " ";
        std::cout << std::setw (5) << (float)(lap4 - start)/(float)(CLOCKS_PER_SEC) << " sec." << std::endl;
      }
    }
    return;
  }

  void GitterPll::repartitionMacroGrid (LoadBalancer::DataBase & db) 
  {
    // call repartition of macro grid without gather-scatter object 
    doRepartitionMacroGrid( db, (GatherScatterType *) 0 );
  }

  // dune  version 
  void GitterPll::
  duneRepartitionMacroGrid (LoadBalancer::DataBase & db, GatherScatterType & gs) 
  {
    // call reparition implementation 
    doRepartitionMacroGrid( db, &gs );
  }

} // namespace ALUGrid
