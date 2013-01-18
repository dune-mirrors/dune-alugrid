// (c) Robert Kloefkorn 2010
#include <config.h>

#include "ghost_info.h"

template<int points>
void MacroGhostInfoStorage<points> :: 
inlineGhostElement(ObjectStream & os ) const
{
 // local face number 
  os.writeObject( _fce );

  // global vertex number of the hexas vertices  
  for(int i=0; i<noVx; ++i) os.writeObject( _vx[i] );
  
  // global vertex numbers of the face not existing on this partition  
  for(int i=0; i<noFaceVx; ++i) 
  {
    os.writeObject( _vxface[i] );
    os.writeObject( _p[i][0] ); 
    os.writeObject( _p[i][1] ); 
    os.writeObject( _p[i][2] ); 
  }
}

template<int points>
void MacroGhostInfoStorage<points> :: 
readData(ObjectStream & os ) 
{
  // read local face number
  os.readObject ( _fce );

  // read vertices of element
  for(int i=0; i<noVx; ++i)
  {
    os.readObject ( _vx[i] );
  }

#ifndef NDEBUG 
  for( int i=0; i<noVx; ++i )
  {
    for( int j=0; j<noVx; ++j)
    {
      if( j == i ) continue ;
      assert( _vx[ i ] != _vx[ j ] );
    }
  }
#endif

  // read vertices of face an coordinates 
  for(int i=0; i<noFaceVx; ++i)
  {
    os.readObject ( _vxface[i] );
    alucoord_t (&pr) [3] = _p[i];

    os.readObject (pr[0]) ;
    os.readObject (pr[1]) ;
    os.readObject (pr[2]) ;
  }

  assert( _fce != invalidFace );
}

MacroGhostInfoHexa :: 
MacroGhostInfoHexa(const Gitter :: Geometric :: hexa_GEO * hexa, 
                   const int fce)
{
  assert( points == this->nop() );
  int oppFace = Gitter :: Geometric :: hexa_GEO :: oppositeFace[fce];
  for(int vx=0; vx<points; vx++)
  {
    const Gitter :: Geometric :: VertexGeo * vertex = hexa->myvertex(oppFace,vx);
    this->_vxface[vx] = vertex->ident();
    const alucoord_t (&p) [3] = vertex->Point();
    this->_p[vx][0] = p[0];
    this->_p[vx][1] = p[1];
    this->_p[vx][2] = p[2];
  }
  
  for(int i=0; i<noVx; i++) 
  {
    this->_vx[i] = hexa->myvertex(i)->ident();
  }
  this->_fce = fce;
}

MacroGhostInfoTetra :: 
MacroGhostInfoTetra(const Gitter :: Geometric :: tetra_GEO * tetra, 
                    const int fce)
{
  assert( points == this->nop() );
  const Gitter :: Geometric :: VertexGeo * vertex = tetra->myvertex( fce );
  assert( vertex );
  for(int vx=0; vx<points; ++vx)
  {
    this->_vxface[vx] = vertex->ident();
    const alucoord_t (&p) [3] = vertex->Point();
    this->_p[vx][0] = p[0];
    this->_p[vx][1] = p[1];
    this->_p[vx][2] = p[2];
  }
  
  for(int i=0; i<noVx; ++i) 
  {
    this->_vx[i] = tetra->myvertex(i)->ident();
  }

  this->_fce = tetra->orientation() ? -fce-1 : fce;
}

// template instantiation 

template class MacroGhostInfoStorage< 1 >;
template class MacroGhostInfoStorage< 4 >;
class MacroGhostInfoHexa ; 
class MacroGhostInfoTetra ; 
