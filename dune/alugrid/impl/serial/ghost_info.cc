// (c) Robert Kloefkorn 2010
#include <config.h>

#include "ghost_info.h"

namespace ALUGrid
{

  template<int points>
  void MacroGhostInfoStorage<points>::
  doInlineGhostElement(ObjectStream & os ) const
  {
    // local face number
    os.put( _fce );

    // global vertex number of the elements vertices
    for(int i=0; i<noVx; ++i) os.write( _vx[i] );

    // global vertex numbers of the face not existing on this partition
    for(int i=0; i<noFaceVx; ++i)
    {
      os.write( _vxface[i] );
      os.write( _p[i][0] );
      os.write( _p[i][1] );
      os.write( _p[i][2] );
    }
  }

  template<int points>
  void MacroGhostInfoStorage<points>::
  doReadData(ObjectStream & os )
  {
    // read local face number
    _fce = os.get();

    // read vertices of element
    for(int i=0; i<noVx; ++i)
    {
      os.read( _vx[i] );
    }

#ifdef ALUGRIDDEBUG
    for( int i=0; i<noVx; ++i )
    {
      for( int j=0; j<noVx; ++j)
      {
        if( j == i ) continue;
        alugrid_assert ( _vx[ i ] != _vx[ j ] );
      }
    }
#endif

    // read vertices of face an coordinates
    for(int i=0; i<noFaceVx; ++i)
    {
      os.read( _vxface[i] );
      alucoord_t (&pr) [3] = _p[i];

      os.read(pr[0]);
      os.read(pr[1]);
      os.read(pr[2]);
    }

    alugrid_assert ( _fce != invalidFace );
  }

  MacroGhostInfoHexa::
  MacroGhostInfoHexa(const Gitter::Geometric::hexa_GEO * hexa,
                     const int fce)
  {
    alugrid_assert ( points == this->nop() );
    int oppFace = Gitter::Geometric::hexa_GEO::oppositeFace[fce];
    for(int vx=0; vx<points; vx++)
    {
      const Gitter::Geometric::VertexGeo * vertex = hexa->myvertex(oppFace,vx);
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

  void MacroGhostInfoHexa::
  writeGhostInfo( ObjectStream& os,
                  const int fce,
                  const Gitter::Geometric::hexa_GEO& hexa )
  {
    signed char face = fce;
    // local face number as char
    os.put( face );

    // global vertex number of the elements vertices
    for(int k=0; k<noVx; ++k)
    {
      int vx = hexa.myvertex (k)->ident ();
      os.write( vx );
    }

    const int oppFace = Gitter::Geometric::Hexa::oppositeFace[fce];
    for(int vx=0; vx<noFaceVx; ++vx)
    {
      const Gitter::Geometric::VertexGeo * vertex = hexa.myvertex(oppFace,vx);
      os.writeObject( vertex->ident() );
      const alucoord_t (&p)[3] = vertex->Point();
      os.writeObject ( p[0] );
      os.writeObject ( p[1] );
      os.writeObject ( p[2] );
    }
  }


  MacroGhostInfoTetra::MacroGhostInfoTetra ( const Gitter::Geometric::tetra_GEO *tetra, const int fce )
  {
    _simplexTypeFlag = tetra->simplexTypeFlag();
    alugrid_assert ( points == this->nop() );
    const Gitter::Geometric::VertexGeo * vertex = tetra->myvertex( fce );
    alugrid_assert ( vertex );
    for(int vx=0; vx<points; ++vx)
    {
      this->_vxface[vx] = vertex->ident();
      const alucoord_t (&p) [3] = vertex->Point();
      this->_p[vx][0] = p[0];
      this->_p[vx][1] = p[1];
      this->_p[vx][2] = p[2];
    }

    for( int i = 0; i < noVx; ++i )
      this->_vx[i] = tetra->myvertex(i)->ident();

    this->_fce = tetra->simplexTypeFlag().orientation() ? -fce-1 : fce;
  }

  void MacroGhostInfoTetra::
  writeGhostInfo( ObjectStream& os,
                  const int fce,
                  const Gitter::Geometric::tetra_GEO& tetra )
  {
    signed char face = fce;
    // local face number as char
    os.put( face );

    // global vertex number of the elements vertices
    for(int k=0; k<noVx; ++k)
    {
      int vx = tetra.myvertex (k)->ident ();
      os.write( vx );
    }

    {
      const Gitter::Geometric::VertexGeo * vertex = tetra.myvertex(fce);
      alugrid_assert ( vertex );

      // know identifier of transmitted point
      int vxId = vertex->ident () ;
      os.write( vxId );

      // store the missing point to form a tetra
      const alucoord_t (&p)[3] = vertex->Point();
      os.writeObject ( p[0] );
      os.writeObject ( p[1] );
      os.writeObject ( p[2] );
    }

    // write simplex type flag
    tetra.simplexTypeFlag().write( os );
  }



  // Template Instantiation
  // ----------------------

  template class MacroGhostInfoStorage< 1 >;
  template class MacroGhostInfoStorage< 4 >;
  class MacroGhostInfoHexa;
  class MacroGhostInfoTetra;

} // namespace ALUGrid
