// (c) Robert Kloefkorn 2004 - 2005 
#ifndef GITTER_DUNE_PLL_MGB_CC_INCLUDED
#define GITTER_DUNE_PLL_MGB_CC_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif
  
#include <assert.h>
#include <time.h>
#include <stdio.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
  #include <iomanip>
  #include <algorithm>
#else
  #include <iostream.h>
  #include <iomanip.h>
  #include <algo.h>
#endif

#include "serialize.h"
#include "gitter_mgb.h"

#include "gitter_pll_sti.h"
#include "gitter_pll_mgb.h"

class DuneParallelGridMover : public ParallelGridMover {
  public :
    DuneParallelGridMover (BuilderIF &i); 

    // if not finalized yet, finalize is called 
    ~DuneParallelGridMover () ;
    inline void initialize (); //former constructor
    inline void finalize ();   //former destructor
    
    // overlaoded, because calles unpackHbnd3 
    inline void unpackAll (vector < ObjectStream > &) ;
    inline void duneUnpackAll (vector < ObjectStream > &, GatherScatterType & ) ;

  protected :
    void duneUnpackTetra (ObjectStream &,GatherScatterType &) ;
    void duneUnpackHexa (ObjectStream &,GatherScatterType &) ;
    // overloaded, because calles InsertUniqueHbnd3_withPoint
    inline void unpackHbnd3Int (ObjectStream & os); 
    
    // overloaded, because calles InsertUniqueHbnd4_withPoint
    inline void unpackHbnd4Int (ObjectStream & os); 

    // creates Hbnd3IntStorage with ghost info if needed 
    bool InsertUniqueHbnd3_withPoint (int (&)[3], 
                              Gitter :: hbndseg :: bnd_t, 
                              MacroGhostInfoTetra* ) ;

    // creates Hbnd4IntStorage with ghost info if needed 
    bool InsertUniqueHbnd4_withPoint (int (&)[4], Gitter :: hbndseg ::
            bnd_t, MacroGhostInfoHexa* );
        
};

// new method that gets coord of ghost point 
bool DuneParallelGridMover :: InsertUniqueHbnd3_withPoint (int (&v)[3],         
      Gitter :: hbndseg_STI ::bnd_t bt, MacroGhostInfoTetra * ghInfo) 
{
  int twst = cyclicReorder (v,v+3) ;
  faceKey_t key (v [0], v [1], v [2]) ;
  if (bt == Gitter :: hbndseg_STI :: closure) 
  {
    if (_hbnd3Int.find (key) == _hbnd3Int.end ()) 
    {
      assert( ghInfo );
      hface3_GEO * face =  InsertUniqueHface3 (v).first ;
      // here the point is stored 
      _hbnd3Int [key] = new Hbnd3IntStorage (face,twst,ghInfo) ;
      return true ;
    }
  } 
  else 
  {
    if (_hbnd3Map.find (key) == _hbnd3Map.end ()) 
    {
      hface3_GEO * face =  InsertUniqueHface3 (v).first ;
      hbndseg3_GEO * hb3 = myBuilder ().insert_hbnd3 (face,twst,bt) ;
      _hbnd3Map [key] = hb3 ;
      return true ;
    }
  }
  return false ;
}

// new method that gets coord of ghost point 
bool DuneParallelGridMover :: InsertUniqueHbnd4_withPoint (int (&v)[4],         
      Gitter :: hbndseg_STI ::bnd_t bt, 
      MacroGhostInfoHexa* ghInfo) 
{
  int twst = cyclicReorder (v,v+4) ;
  faceKey_t key (v [0], v [1], v [2]) ;
  if (bt == Gitter :: hbndseg_STI :: closure) 
  {
    if (_hbnd4Int.find (key) == _hbnd4Int.end ()) 
    {
      assert( ghInfo );
      hface4_GEO * face =  InsertUniqueHface4 (v).first ;
      _hbnd4Int [key] = new Hbnd4IntStorage (face,twst,ghInfo) ;
      return true ;
    }
  } 
  else 
  {
    if (_hbnd4Map.find (key) == _hbnd4Map.end ()) 
    {
      hface4_GEO * face =  InsertUniqueHface4 (v).first ;
      hbndseg4_GEO * hb4 = myBuilder ().insert_hbnd4 (face,twst,bt) ;
      _hbnd4Map [key] = hb4 ;
      return true ;
    }
  }
  return false ;
}

// overloaded method because here we call insertion with point 
inline void DuneParallelGridMover :: unpackHbnd3Int (ObjectStream & os) 
{
  int bfake, v [3] ;
  os.readObject (bfake) ;
  Gitter :: hbndseg :: bnd_t b = (Gitter :: hbndseg :: bnd_t) bfake;
  
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;

  int readPoint = 0; 
  os.readObject( readPoint ); 

  MacroGhostInfoTetra * ghInfo = 0;
  if( readPoint == MacroGridMoverIF :: POINTTRANSMITTED ) 
  {
    // read ghost data from stream 
    ghInfo = new MacroGhostInfoTetra(os); 
  }

  // if internal boundary, create internal bnd face 
  if(b == Gitter :: hbndseg :: closure)
  {
    assert( ghInfo );
    InsertUniqueHbnd3_withPoint (v, b, ghInfo ) ;
  }
  else
  {
    // delete ghost info not needed any longer 
    if( ghInfo ) delete ghInfo;

    // create normal bnd face, and make sure that no Point was send
    assert(readPoint == MacroGridMoverIF :: NO_POINT );
    // old method defined in base class 
    InsertUniqueHbnd3 (v, b ) ;
  }

  return ;
}

// overloaded method because here we call insertion with point 
inline void DuneParallelGridMover :: unpackHbnd4Int (ObjectStream & os) 
{
  int bfake, v [4] = {-1,-1,-1,-1};

  os.readObject (bfake) ;
  Gitter :: hbndseg :: bnd_t b = (Gitter :: hbndseg :: bnd_t) bfake;

  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  os.readObject (v[3]) ;

  int readPoint = 0; 
  os.readObject( readPoint ); 
  
  MacroGhostInfoHexa* ghInfo = 0;
  if( readPoint == MacroGridMoverIF :: POINTTRANSMITTED ) 
  {
    // read ghost data from stream 
    ghInfo = new MacroGhostInfoHexa(os); 
  }

  // if internal boundary, create internal bnd face 
  if(b == Gitter :: hbndseg :: closure)
  {
    assert( ghInfo );
    InsertUniqueHbnd4_withPoint (v, b, ghInfo ) ;
  }
  else
  {
    // delete ghost info not needed any longer 
    if( ghInfo ) delete ghInfo;

    // create normal bnd face, and make sure that no Point was send
    assert(readPoint == MacroGridMoverIF :: NO_POINT );
    // old method defined in base class 
    InsertUniqueHbnd4 (v, b ) ;
  }
  return ;
}


void DuneParallelGridMover :: duneUnpackTetra (ObjectStream & os, GatherScatterType & gs) 
{
  int v [4] ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  os.readObject (v[3]) ;
  pair < tetra_GEO *, bool > p = InsertUniqueTetra (v) ;
  p.first->accessPllX ().duneUnpackSelf (os,gs,p.second) ;
  return ;
}

void DuneParallelGridMover :: duneUnpackHexa (ObjectStream & os, GatherScatterType & gs) {
  int v [8] ;
  os.readObject (v[0]) ;
  os.readObject (v[1]) ;
  os.readObject (v[2]) ;
  os.readObject (v[3]) ;
  os.readObject (v[4]) ;
  os.readObject (v[5]) ;
  os.readObject (v[6]) ;
  os.readObject (v[7]) ;
  pair < hexa_GEO *, bool > p = InsertUniqueHexa (v) ;
  p.first->accessPllX ().duneUnpackSelf (os,gs,p.second) ;
  return ;
}

void DuneParallelGridMover :: unpackAll (vector < ObjectStream > & osv) 
{
  for (vector < ObjectStream > :: iterator j = osv.begin () ; j != osv.end () ; j ++) 
  {
    ObjectStream & os (*j) ;
    int code = MacroGridMoverIF :: ENDMARKER ;
    for (os.readObject (code) ; code != MacroGridMoverIF :: ENDMARKER ; os.readObject (code)) {
      switch (code) {
      case MacroGridMoverIF:: VERTEX :
        unpackVertex (os) ;
        break ;
      case MacroGridMoverIF :: EDGE1 :
        unpackHedge1 (os) ;
        break ;
      case MacroGridMoverIF :: FACE3 :
        unpackHface3 (os) ;
        break ;
      case MacroGridMoverIF :: FACE4 :
        unpackHface4 (os) ;
        break ;
      case MacroGridMoverIF :: TETRA :
        unpackTetra (os) ;
        break ;
      case MacroGridMoverIF :: HEXA :
        unpackHexa (os) ;
        break ;
      case MacroGridMoverIF :: PERIODIC3 :
        unpackPeriodic3 (os) ;
        break ;
      case MacroGridMoverIF :: PERIODIC4 :
        unpackPeriodic4 (os) ;
        break ;
      case MacroGridMoverIF :: HBND3INT :
        unpackHbnd3Int (os) ;
        break ;
      case MacroGridMoverIF :: HBND3EXT :
        unpackHbnd3Ext (os) ;
        break ;
      case MacroGridMoverIF :: HBND4INT :
        unpackHbnd4Int (os) ;
        break; 
      case MacroGridMoverIF :: HBND4EXT :
        unpackHbnd4Ext (os) ;
        break ;
      default :
  cerr << "**FEHLER (FATAL) Unbekannte Gitterobjekt-Codierung gelesen [" << code << "]\n" ;
  cerr << "  Weitermachen unm\"oglich. In " << __FILE__ << " " << __LINE__ << endl ;
  abort () ;
        break ;
      }
    }
  }  
  return ;
}

void DuneParallelGridMover :: 
duneUnpackAll (vector < ObjectStream > & osv, GatherScatterType & gs) 
{
  for (vector < ObjectStream > :: iterator j = osv.begin () ; j != osv.end () ; j ++) 
  {
    ObjectStream & os (*j) ;
    int code = MacroGridMoverIF :: ENDMARKER ;
    for (os.readObject (code) ; code != MacroGridMoverIF :: ENDMARKER ; os.readObject (code)) 
    {
      switch (code) {
      case MacroGridMoverIF:: VERTEX :
        {
          unpackVertex (os) ;
          break ;
        }
      case MacroGridMoverIF :: EDGE1 :
        {
          unpackHedge1 (os) ;
          break ;
        }
      case MacroGridMoverIF :: FACE3 :
        {
          unpackHface3 (os) ;
          break ;
        }
      case MacroGridMoverIF :: FACE4 :
        {
          unpackHface4 (os) ;
          break ;
        }
      case MacroGridMoverIF :: TETRA :
        {
          duneUnpackTetra (os,gs) ;
          break ;
        }
      case MacroGridMoverIF :: HEXA :
        {
          duneUnpackHexa (os,gs) ;
          break ;
        }
      case MacroGridMoverIF :: PERIODIC3 :
        {
          unpackPeriodic3 (os) ;
          break ;
        }
      case MacroGridMoverIF :: PERIODIC4 :
        {
          unpackPeriodic4 (os) ;
          break ;
        }
      case MacroGridMoverIF :: HBND3INT :
        {
          unpackHbnd3Int (os) ;
          break ;
        }
      case MacroGridMoverIF :: HBND3EXT :
        {
          unpackHbnd3Ext (os) ;
          break ;
        }
      case MacroGridMoverIF :: HBND4INT :
        {
          unpackHbnd4Int (os) ;
          break; 
        }
      case MacroGridMoverIF :: HBND4EXT :
        {
          unpackHbnd4Ext (os) ;
          break ;
        }
      default :
        cerr << "**FEHLER (FATAL) Unbekannte Gitterobjekt-Codierung gelesen [" << code << "] on p = " << __STATIC_myrank << "\n" ;
        cerr << "  Weitermachen unm\"oglich. In " << __FILE__ << " " << __LINE__ << endl ;
        assert(false);
        abort () ;
        break ;
      }
    }
  }  
  return ;
}

// calles initialize, former Constructor of MacroGridBuilder
DuneParallelGridMover :: DuneParallelGridMover (BuilderIF & i) : ParallelGridMover (i,false) 
{
  initialize();
}

// overloaded, because here we use the new insertInternal method 
void DuneParallelGridMover :: initialize ()
{
  {
    for (list < VertexGeo * > :: iterator i = myBuilder ()._vertexList.begin () ;
      i != myBuilder ()._vertexList.end () ; myBuilder ()._vertexList.erase (i ++)) 
        _vertexMap [(*i)->ident ()] = (*i) ;
  }
  {
    for (list < hedge1_GEO * > :: iterator i = myBuilder ()._hedge1List.begin () ;
      i != myBuilder ()._hedge1List.end () ; myBuilder ()._hedge1List.erase (i ++)) {
      long k = (*i)->myvertex (0)->ident (), l = (*i)->myvertex (1)->ident () ;
      _edgeMap [edgeKey_t (k < l ? k : l, k < l ? l : k)] = (*i) ;
    }
  }
  {for (list < hface3_GEO * > :: iterator i = myBuilder ()._hface3List.begin () ; i != myBuilder ()._hface3List.end () ;
     myBuilder ()._hface3List.erase (i ++)) {
      _face3Map [faceKey_t ((*i)->myvertex (0)->ident (),(*i)->myvertex (1)->ident (), (*i)->myvertex (2)->ident ())] = (*i) ;
  }}
  {
    for (list < hface4_GEO * > :: iterator i = myBuilder ()._hface4List.begin () ; i != myBuilder ()._hface4List.end () ; 
      myBuilder ()._hface4List.erase (i ++)) _face4Map [faceKey_t ((*i)->myvertex (0)->ident (),(*i)->myvertex (1)->ident (),
        (*i)->myvertex (2)->ident ())] = (*i) ;
  }
  { for (list < hbndseg4_GEO * > :: iterator i = myBuilder ()._hbndseg4List.begin () ; i != myBuilder ()._hbndseg4List.end () ; myBuilder ()._hbndseg4List.erase (i++)) 
    {
      typedef Gitter :: Geometric :: hface4_GEO hface4_GEO;
      hface4_GEO * face = (*i)->myhface4 (0);
      assert( face );
      faceKey_t key (face->myvertex (0)->ident (), 
                     face->myvertex (1)->ident (), 
                     face->myvertex (2)->ident ()) ;
      // if internal face 
      if ((*i)->bndtype () == Gitter :: hbndseg_STI :: closure) 
      {
        typedef Gitter :: ghostpair_STI ghostpair_STI;
        typedef Gitter :: Geometric :: hexa_GEO  hexa_GEO;

        ghostpair_STI gpair = (*i)->getGhost();
        hexa_GEO * gh = dynamic_cast<hexa_GEO *> (gpair.first);
        if( gh )
        {
          _hbnd4Int [key] = new Hbnd4IntStorage (face , (*i)->twist (0), 
                                                 gh, gpair.second ) ;
        }
        else 
          _hbnd4Int [key] = new Hbnd4IntStorage (face ,(*i)->twist (0)) ;
        delete (*i) ;
      } 
      else 
      {
        _hbnd4Map [key] = (*i) ;
      }
    }
  }
  {for (list < hbndseg3_GEO * > :: iterator i = myBuilder ()._hbndseg3List.begin () ; i != myBuilder ()._hbndseg3List.end () ;
    myBuilder ()._hbndseg3List.erase (i++)) 
  {
    typedef Gitter :: Geometric :: hface3_GEO hface3_GEO;
    hface3_GEO * face = (*i)->myhface3 (0);
    assert( face );
    faceKey_t key ( face->myvertex (0)->ident (), face->myvertex (1)->ident (), face->myvertex (2)->ident ()) ;
    // if internal face 
    if ((*i)->bndtype () == Gitter :: hbndseg_STI :: closure) 
    {
      // check for ghost element 
      typedef Gitter :: ghostpair_STI ghostpair_STI;
      ghostpair_STI gpair = (*i)->getGhost();

      typedef Gitter :: Geometric :: tetra_GEO  tetra_GEO;
      tetra_GEO * gh = dynamic_cast<tetra_GEO *> (gpair.first);
      if( gh )
      {
        // insert new internal storage 
        _hbnd3Int [key] = new Hbnd3IntStorage ( face , (*i)->twist (0), 
                                               gh , gpair.second ) ;
      }
      // until here
      else 
        _hbnd3Int [key] = new Hbnd3IntStorage ( face , (*i)->twist (0)) ;
      
      delete (*i) ;
    } 
    else 
    {
      _hbnd3Map [key] = (*i) ;
    }
  }}

  {for (list < tetra_GEO * > :: iterator i = myBuilder ()._tetraList.begin () ; i != myBuilder ()._tetraList.end () ; 
      myBuilder ()._tetraList.erase (i++)) {
      _tetraMap [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
           (*i)->myvertex (2)->ident (), (*i)->myvertex (3)->ident ())] = (*i) ;
  }}
  {for (list < periodic3_GEO * > :: iterator i = myBuilder ()._periodic3List.begin () ; i != myBuilder ()._periodic3List.end () ; 
      myBuilder ()._periodic3List.erase (i++)) {
      _periodic3Map [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
           (*i)->myvertex (2)->ident (), -((*i)->myvertex (3)->ident ())-1)] = (*i) ;
  }}
  {for (list < periodic4_GEO * > :: iterator i = myBuilder ()._periodic4List.begin () ; i != myBuilder ()._periodic4List.end () ; 
      myBuilder ()._periodic4List.erase (i++)) {
      _periodic4Map [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
           (*i)->myvertex (3)->ident (), -((*i)->myvertex (4)->ident ())-1)] = (*i) ;
  }}
  {
    for (list < hexa_GEO * > :: iterator i = myBuilder ()._hexaList.begin () ; i != myBuilder ()._hexaList.end () ; 
      myBuilder ()._hexaList.erase (i++)) _hexaMap [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
                  (*i)->myvertex (3)->ident (), (*i)->myvertex (4)->ident ())] = (*i) ;
  }

  // from constructor ParallelGridMover 
  vector < elementKey_t > toDelete ;
  {for (elementMap_t :: iterator i = _hexaMap.begin () ; i != _hexaMap.end () ; i ++)
    if (Gitter :: InternalElement ()(*((hexa_GEO *)(*i).second)).accessPllX ().erasable ()) {
      toDelete.push_back ((*i).first) ;
    }
  }
  {for (elementMap_t :: iterator i = _tetraMap.begin () ; i != _tetraMap.end () ; i ++)
    if (Gitter :: InternalElement ()(*((tetra_GEO *)(*i).second)).accessPllX ().erasable ()) {
      toDelete.push_back ((*i).first) ;
    }
  }
  {for (elementMap_t :: iterator i = _periodic3Map.begin () ; i != _periodic3Map.end () ; i ++)
    if (Gitter :: InternalElement ()(*((periodic3_GEO *)(*i).second)).accessPllX ().erasable ()) {
      toDelete.push_back ((*i).first) ;
    }
  }
  {for (elementMap_t :: iterator i = _periodic4Map.begin () ; i != _periodic4Map.end () ; i ++)
    if (Gitter :: InternalElement ()(*((periodic4_GEO *)(*i).second)).accessPllX ().erasable ()) {
      toDelete.push_back ((*i).first) ;
    }
  }
  {for (vector < elementKey_t > :: iterator i = toDelete.begin () ; i != toDelete.end () ; i ++ )
    removeElement (*i) ;
  }

  this->_initialized = true;
  return ; 
}

// destructor calles finalize if not finalized yet 
DuneParallelGridMover :: ~DuneParallelGridMover () 
{
  if(!_finalized) 
  {
    // compress index manager 
    for(int i=0; i<Gitter :: Geometric :: BuilderIF :: numOfIndexManager; ++i)
    {
      this->_mgb.indexManager(i).compress();
    }

    finalize();
  }
}   

// overloaded, because here we use the new insertInternal method 
void DuneParallelGridMover :: finalize ()
{
  {for (elementMap_t :: iterator i = _hexaMap.begin () ; i != _hexaMap.end () ; _hexaMap.erase (i++))
    myBuilder ()._hexaList.push_back ((hexa_GEO *)(*i).second) ;
  }
  {for (elementMap_t :: iterator i = _tetraMap.begin () ; i != _tetraMap.end () ; _tetraMap.erase (i++))
    myBuilder ()._tetraList.push_back ((tetra_GEO *)(*i).second) ;
  }
  {for (elementMap_t :: iterator i = _periodic3Map.begin () ; i != _periodic3Map.end () ; _periodic3Map.erase (i++))
    myBuilder ()._periodic3List.push_back ((periodic3_GEO *)(*i).second) ;
  }
  
  {for (elementMap_t :: iterator i = _periodic4Map.begin () ; i != _periodic4Map.end () ; _periodic4Map.erase (i++))
    myBuilder ()._periodic4List.push_back ((periodic4_GEO *)(*i).second) ;
  }

  {for (faceMap_t :: iterator i = _hbnd4Map.begin () ; i != _hbnd4Map.end () ; )
    if (((hbndseg4_GEO *)(*i).second)->myhface4 (0)->ref == 1) 
    {
      delete (hbndseg4_GEO *)(*i).second ;
      _hbnd4Map.erase (i++) ;
    } else {
      myBuilder ()._hbndseg4List.push_back ((hbndseg4_GEO *)(*i ++).second) ;
    }
  }
  {for (faceMap_t :: iterator i = _hbnd3Map.begin () ; i != _hbnd3Map.end () ; )
    if (((hbndseg3_GEO *)(*i).second)->myhface3 (0)->ref == 1) {
      delete (hbndseg3_GEO *)(*i).second ;
      _hbnd3Map.erase (i++) ;
    } else {
      myBuilder ()._hbndseg3List.push_back ((hbndseg3_GEO *)(*i ++).second) ;
    }
  }
  {
    for (hbnd4intMap_t :: iterator i = _hbnd4Int.begin () ; i != _hbnd4Int.end () ; i ++) 
    {
      Hbnd4IntStorage & p = * ((*i).second) ;
      if (p.first()->ref == 1) 
      {
        // get ghost info from storage and release pointer 
        MacroGhostInfoHexa* ghInfo = p.release();

        hbndseg4_GEO * hb4 = myBuilder ().
              insert_hbnd4 (p.first(),p.second(),Gitter :: hbndseg_STI :: closure, ghInfo );
        myBuilder ()._hbndseg4List.push_back (hb4) ;
      }
      delete (*i).second;
    } 
  }

  // here the internal boundary elements are created 
  {
    for (hbnd3intMap_t :: iterator i = _hbnd3Int.begin () ; i != _hbnd3Int.end () ; i ++) 
    {
      Hbnd3IntStorage & p = *((*i).second);
      if (p.first()->ref == 1) 
      {
        // get ghost info from storage and release pointer 
        MacroGhostInfoTetra* ghInfo = p.release();

        hbndseg3_GEO * hb3 = myBuilder().insert_hbnd3( p.first(),p.second(),
                          Gitter :: hbndseg_STI :: closure , ghInfo );
        myBuilder ()._hbndseg3List.push_back (hb3) ;
      }
      delete (*i).second; 
    }
  }
  {for (faceMap_t :: iterator i = _face4Map.begin () ; i != _face4Map.end () ; )
    if (!((hface4_GEO *)(*i).second)->ref) {
      delete (hface4_GEO *)(*i).second ;
      _face4Map.erase (i++) ;
    } else {
      //assert (((hface4_GEO *)(*i).second)->ref == 2) ;
      myBuilder ()._hface4List.push_back ((hface4_GEO *)(*i ++).second ) ;
    }
  }
  {
    for (faceMap_t :: iterator i = _face3Map.begin () ; i != _face3Map.end () ; ) 
    {
      if (!((hface3_GEO *)(*i).second)->ref) 
      {
        delete (hface3_GEO *)(*i).second ;
        _face3Map.erase (i++) ;
      } 
      else 
      {
        //assert (((hface3_GEO *)(*i).second)->ref == 2) ;
        myBuilder ()._hface3List.push_back ((hface3_GEO *)(*i ++).second ) ;
      }
    }
  }
  {for (edgeMap_t :: iterator i = _edgeMap.begin () ; i != _edgeMap.end () ; )
    if (!(*i).second->ref) {
      delete (*i).second ;
      _edgeMap.erase (i++) ;
    } else {
      assert ((*i).second->ref >= 1) ;
      myBuilder ()._hedge1List.push_back ((*i ++).second) ;
    }
  }
  {
    for (vertexMap_t :: iterator i = _vertexMap.begin () ; i != _vertexMap.end () ; )
    {
      if (!(*i).second->ref) 
      {
        delete (*i).second ;
        _vertexMap.erase (i++) ;
      } 
      else 
      {
        assert ((*i).second->ref >= 2) ;
        myBuilder ()._vertexList.push_back ((*i ++).second) ;
      }
    }
  }
  myBuilder ()._modified = true ; // wichtig !
  this->_finalized = true;
  return ;
}

//*************************************************************************
//  repartition method of class GitterDunePll 
//*************************************************************************
// method was overloaded because here we use our DuneParallelGridMover 
void GitterDunePll :: repartitionMacroGrid (LoadBalancer :: DataBase & db) 
{
  if (db.repartition (mpAccess (), LoadBalancer :: DataBase :: method (_ldbMethod))) 
  {
    const long start = clock () ;
    long lap1 (start), lap2 (start), lap3 (start), lap4 (start) ;
    mpAccess ().removeLinkage () ;
    mpAccess ().insertRequestSymetric (db.scan ()) ;
    const int me = mpAccess ().myrank (), nl = mpAccess ().nlinks () ;
    {
      AccessIterator < helement_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) {
      int to = db.getDestination (w.item ().accessPllX ().ldbVertexIndex ()) ;
        if (me != to)
          w.item ().accessPllX ().attach2 (mpAccess ().link (to)) ;
      }
    }
    lap1 = clock () ;
    
    // create vector of object streams 
    vector < ObjectStream > osv (nl) ;

    // pack all stuff 
    {
      AccessIterator < vertex_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().packAll (osv) ;
    }
    {
      AccessIterator < hedge_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().packAll (osv) ;
    }
    {
      AccessIterator < hface_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().packAll (osv) ;
    }
    {
      AccessIterator < helement_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().packAll (osv) ;
    }
    {
      for (vector < ObjectStream > :: iterator i = osv.begin () ; i != osv.end () ; 
        (*i++).writeObject (MacroGridMoverIF :: ENDMARKER)) ;
    }

    lap2 = clock () ;
    
    // exchange stuff 
    osv = mpAccess ().exchange (osv) ;
    lap3 = clock () ;
    
    // delete and unpack  
    {
      DuneParallelGridMover pgm (containerPll ()) ;
      pgm.unpackAll (osv) ;
    }

    // result 
    lap4 = clock () ;
    if (MacroGridBuilder :: debugOption (20)) {
      cout << "**INFO GitterDunePll["<<me<<"] :: repartitionMacroGrid () [ass|pck|exc|upk|all] " ;
      cout << setw (5) << (float)(lap1 - start)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap2 - lap1)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap3 - lap2)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap4 - lap3)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap4 - start)/(float)(CLOCKS_PER_SEC) << " sec." << endl ;
    }
  }
  return ;
}
void GitterDunePll :: 
duneRepartitionMacroGrid (LoadBalancer :: DataBase & db, GatherScatterType & gs) 
{
  if (db.repartition (mpAccess (), LoadBalancer :: DataBase :: method (_ldbMethod))) 
  {
    const clock_t start = clock () ;
    clock_t lap1 (start), lap2 (start), lap3 (start), lap4 (start) ;
    
    mpAccess ().removeLinkage () ;
    mpAccess ().insertRequestSymetric (db.scan ()) ;
    const int me = mpAccess ().myrank (), nl = mpAccess ().nlinks () ;
    {
      AccessIterator < helement > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) 
      {
        int to = db.getDestination (w.item ().accessPllX ().ldbVertexIndex ()) ;
        if (me != to)
        {
          w.item ().accessPllX ().attach2 (mpAccess ().link (to)) ;
        }
      }
    }
    lap1 = clock () ;
    
    // create object stream with size 
    vector < ObjectStream > osv (nl) ;

    // pack vertices 
    {
      AccessIterator < vertex_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().packAll (osv) ;
    }
    // pack edges 
    {
      AccessIterator < hedge_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().packAll (osv) ;
    }
    // pack faces 
    {
      AccessIterator < hface_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().packAll (osv) ;
    }
    // pack elements
    {
      AccessIterator < helement_STI > :: Handle w (containerPll ()) ;
      for (w.first () ; ! w.done () ; w.next ()) 
      {
        w.item ().accessPllX ().dunePackAll (osv,gs) ;
      }
    }
    
    // write end marker 
    {
      for (vector < ObjectStream > :: iterator i = osv.begin () ; i != osv.end () ; 
        (*i++).writeObject (MacroGridMoverIF :: ENDMARKER)) ;
    }

    lap2 = clock () ;

    // exchange gathered data 
    osv = mpAccess ().exchange (osv) ;
    
    lap3 = clock () ;

    // unpack all data 
    {
      DuneParallelGridMover pgm (containerPll ()) ;
      pgm.duneUnpackAll (osv,gs) ;
    }
    
    lap4 = clock () ;
    if (MacroGridBuilder :: debugOption (20)) 
    {
      cout << "**INFO GitterDunePll["<<me<<"] :: duneRepartitionMacroGrid () [ass|pck|exc|upk|all] " ;
      cout << setw (5) << (float)(lap1 - start)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap2 - lap1)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap3 - lap2)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap4 - lap3)/(float)(CLOCKS_PER_SEC) << " " ;
      cout << setw (5) << (float)(lap4 - start)/(float)(CLOCKS_PER_SEC) << " "; 
      cout << " sec." << endl ;
    }
  }
  return ;
}

#endif
