// (c) Robert Kloefkorn 2010
#include "gitter_tetra_top_pll.h"

// avoid linker errors by quadratures 
//#define MAPP_CUBE_3D_H_INCLUDED
#include "../serial/gitter_tetra_top.cc"

template < class A, class X, class MX > 
void Hbnd3PllInternal < A, X, MX > :: HbndPll ::  splitGhost
(GhostChildrenInfo_t & info ) 
{
  if(_ghostPair.first)
  {
    GhostTetra_t & ghost = static_cast<GhostTetra_t &> (*_ghostPair.first); 
    if( ! ghost.down() )
    {
      ghost.tagForGlobalRefinement();
      ghost.refine();
    }

    // get the childs 
    typedef typename Gitter :: Geometric :: tetra_GEO  tetra_GEO;
    typedef typename Gitter :: Geometric :: hface3_GEO hface3_GEO;

    // ghostpair.second is the internal face number of the face 
    // connected to the interior of the process 
    hface3_GEO * face = ghost.myhface3( _ghostPair.second ); 
    int count = 0;
    for(face = face->down(); face; face = face->next())
    {
      assert(face);

      // NOTE: we need here tetra_GEO because we cannot cast from hasFace
      // to tetra_IMPL (GhostTetra_t)
      tetra_GEO * ghch = 0;

      typedef pair < Gitter :: Geometric :: hasFace3 *, int > neigh_t;
      neigh_t neighbour = face->nb.front();
      if( ! neighbour.first->isboundary ())
      {
        ghch = dynamic_cast<tetra_GEO *> (neighbour.first);
        assert(ghch);
        assert( ghch->up() == &ghost );
      }
      else 
      {
        neighbour = face->nb.rear();
        assert( ! neighbour.first->isboundary () );
        ghch = dynamic_cast<tetra_GEO *> (neighbour.first);
      }
      
      assert(ghch);
      assert(ghch->up() == &ghost );
     
      // set element pointer and local face number 
      info.setGhostPair( ghostpair_STI( ghch, neighbour.second ) , count );

      ++count ;
    }
  }
}

template < class A, class X, class MX > 
void Hbnd3PllInternal < A, X, MX > :: HbndPll :: 
removeDescendents( helement_STI & elem ) 
{
  elem.resetRefinementRequest(); 
  // check all children first 
  for( helement_STI* child = elem.down(); child; child = child->next() )
  {
    // if child is not leaf coarse childs first 
    if( ! child->leaf() )
      removeDescendents( *child );
    
    // if something went wrong, return ghosts are removed later 
    if( ! child->leaf () ) return ;

    // mark child for coarsening 
    child->tagForGlobalCoarsening(); 
  }

  // if element is not already leaf call coarse 
  if( ! elem.leaf () )
  {
    elem.coarse();
  }
}

template < class A, class X, class MX > 
void Hbnd3PllInternal < A, X, MX > :: HbndPll ::  coarseGhost () 
{
  if(_ghostPair.first)
  {
    helement_STI& ghost = (*_ghostPair.first); 
    if( ghost.leaf() ) return ;

    // remove all descendents if possible 
    removeDescendents( ghost );
  }
}

template < class A, class X, class MX > 
void Hbnd3PllInternal < A, X, MX > :: HbndPll ::  
setGhost ( const ghostpair_STI & gpair ) 
{
  if(gpair.first)
  {
    _ghostPair = gpair; 
    assert( _ghostPair.first );
    
    // copy indices from internal boundry to myhface3(.) of ghost
    _ghostPair.first->setIndicesAndBndId ( *(this->myhface3(0)) , _ghostPair.second );
  }
  else 
  {
    _ghostPair.first  =  0 ;
    _ghostPair.second = -1 ;
  }
}

//***************************************************************************************
//  --HbndPllMacro
//***************************************************************************************
template < class A, class X, class MX > 
const MacroGhostInfo_STI* Hbnd3PllInternal < A, X, MX > :: 
HbndPllMacro :: buildGhostCell(ObjectStream& os, int fce)
{
  assert( _gm == 0 ); 
  int code = MacroGridMoverIF :: ENDMARKER ;
  os.readObject (code); 
  assert( code == MacroGridMoverIF :: HBND3INT );

  {
    int bfake, v [3] ;
    os.readObject (bfake) ;
#ifndef NDEBUG 
    Gitter :: hbndseg :: bnd_t b = (Gitter :: hbndseg :: bnd_t) bfake;
    assert( b == Gitter :: hbndseg :: closure );
#endif

    os.readObject (v[0]) ;
    os.readObject (v[1]) ;
    os.readObject (v[2]) ;

    int readPoint = 0;
    os.readObject( readPoint );

    MacroGhostInfoTetra * ghInfo = 0;
    if( readPoint == MacroGridMoverIF :: POINTTRANSMITTED )
    {
      // create ghInfo and read data from stream 
      ghInfo = new MacroGhostInfoTetra(os);
    }

    {
      assert( ghInfo );
      myhface3_t * f = this->myhface3(0);
      assert( f );

      _gm = new MacroGhostTetra( _mgb , ghInfo,  f );
      this->setGhost ( _gm->getGhost() );   
    }
  }

  assert( _gm );
  return _gm->getGhostInfo();
}

// template instantiation 
typedef GitterBasis :: Objects :: Hbnd3Default Hbnd3DefaultType;
template class Hbnd3PllInternal < Hbnd3DefaultType , 
                                  BndsegPllBaseXClosure < Hbnd3DefaultType > ,
                                  BndsegPllBaseXMacroClosure < Hbnd3DefaultType > > ;

// from serial part with different template argument 
template class Hface3Top< GitterBasisPll :: ObjectsPll :: Hface3EmptyPll >; 
template class TetraTop< GitterBasisPll :: ObjectsPll :: TetraEmptyPll >; 
template class Periodic3Top < GitterBasisPll :: ObjectsPll :: Periodic3EmptyPll >;
