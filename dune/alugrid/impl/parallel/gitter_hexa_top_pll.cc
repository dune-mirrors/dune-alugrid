// (c) Robert Kloefkorn 2010
#include <config.h>

#include "gitter_hexa_top_pll.h"
#include "../serial/gitter_hexa_top.cc"

namespace ALUGrid
{

  template < class A, class X, class MX >
  void Hbnd4PllInternal < A, X, MX >::HbndPll ::
  setGhost ( const ghostpair_STI & gpair )
  {
    if(gpair.first)
    {
      _ghostPair = gpair;
      alugrid_assert ( _ghostPair.first );
      // copy indices from internal boundry to myhface(.) of ghost
      _ghostPair.first->setIndicesAndBndId( *this->myhface(0), _ghostPair.second );
    }
    else
    {
      _ghostPair.first  =  0;
      _ghostPair.second = -1;
    }
  }

  template < class A, class X, class MX >
  void Hbnd4PllInternal < A, X, MX >::HbndPll::
  splitGhost ( GhostChildrenInfo_t & info )
  {
    if(_ghostPair.first)
    {
      typedef typename Gitter::Geometric::hexa_GEO  hexa_GEO;
      typedef typename Gitter::Geometric::hface4_GEO hface4_GEO;

      hexa_GEO & ghost = static_cast<hexa_GEO &> (*(_ghostPair.first));
      if(!ghost.down())
      {
        ghost.tagForGlobalRefinement();
        ghost.refine();
      }

      int gFaceNum = _ghostPair.second;
      alugrid_assert ( gFaceNum >= 0 );
      alugrid_assert ( gFaceNum < 6 );

      hface4_GEO * face = ghost.myhface( gFaceNum );
      alugrid_assert ( face );

      int count = 0;
      for(face = face->down(); face; face = face->next() )
      {
        alugrid_assert (face);
        hexa_GEO * ghch = 0;

        typedef std::pair< Gitter::Geometric::hasFace4 *, int > neigh_t;
        neigh_t neighbour = face->nb.front();

        if( ! neighbour.first->isboundary ())
        {
          ghch = dynamic_cast<hexa_GEO *> (neighbour.first);
          alugrid_assert (ghch);
          alugrid_assert ( ghch->up() == &ghost );
        }
        else
        {
          neighbour = face->nb.rear();
          alugrid_assert ( ! neighbour.first->isboundary () );
          ghch = dynamic_cast<hexa_GEO *> (neighbour.first);
        }

        alugrid_assert (ghch);
        alugrid_assert (ghch->up() == &ghost );

        // set element pointer and local face number
        info.setGhostPair( ghostpair_STI( ghch, neighbour.second ) , count );

        ++count;
      }
    }
  }

  template < class A, class X, class MX >
  void Hbnd4PllInternal < A, X, MX >::HbndPll ::
  removeDescendents( helement_STI & elem )
  {
    elem.resetRefinementRequest();

    // check all children first
    for( helement_STI * child = elem.down(); child; child = child->next() )
    {
      // if child is not leaf coarse childs first
      if( ! child->leaf() )
        removeDescendents( *child );

      // if something went wrong, return ghosts are removed later
      if( ! child->leaf () ) return;

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
  void Hbnd4PllInternal < A, X, MX >::HbndPll:: coarseGhost ()
  {
    if(_ghostPair.first)
    {
      helement_STI & ghost = (*_ghostPair.first);
      if( ghost.leaf() ) return;

      // remove all descendents if possible
      removeDescendents( ghost );
    }
  }

  ////////////////////////////////////////////////////////////////////
  //
  //  --HbndPllMacro
  //
  ////////////////////////////////////////////////////////////////////
  template < class A, class X, class MX >
  const MacroGhostInfo_STI* Hbnd4PllInternal < A, X, MX > ::
  HbndPllMacro::buildGhostCell(ObjectStream& os, int fce)
  {
    alugrid_assert ( _gm == 0 );
    int code = MacroGridMoverIF::ENDMARKER;
    os.readObject (code);
    alugrid_assert ( code == MacroGridMoverIF::HBND4INT );

    {
      int bfake;
      os.readObject (bfake);
#ifdef ALUGRIDDEBUG
      Gitter::hbndseg::bnd_t b = (Gitter::hbndseg::bnd_t) bfake;
      alugrid_assert ( b == Gitter::hbndseg::closure );
#endif

      // read global graph vertex index
      int ldbVertexIndex = -1;
      int master = -1;
      os.readObject( ldbVertexIndex );
      os.readObject( master );

      int v [4] = {-1,-1,-1,-1};
      os.readObject (v[0]);
      os.readObject (v[1]);
      os.readObject (v[2]);
      os.readObject (v[3]);

      const signed char pointTransmitted = os.get();

      // the following makes only sense if information has been transmitted
      if( pointTransmitted != MacroGridMoverIF::POINTTRANSMITTED )
      {
        std::cerr << "ERROR (fatal): No point transmitted, building ghost cells impossible." << std::endl;
        abort();
      }

      // create macro ghost cell
      {
        // create ghost info and read from stream
        MacroGhostInfoHexa* ghInfo = new MacroGhostInfoHexa( os );

        myhface4_t * f = this->myhface(0);
        alugrid_assert ( f );

        // ghInfo is stored inside MacroGhostHexa
        _gm = new MacroGhostHexa( _mgb , ghInfo, f );
        this->setGhost ( _gm->getGhost() );
      }
    }

    alugrid_assert ( _gm );
    return _gm->getGhostInfo();
  }


  // template instantiation
  typedef GitterBasis::Objects::Hbnd4Default Hbnd4DefaultType;
  template class Hbnd4PllInternal < Hbnd4DefaultType ,
                                    BndsegPllBaseXClosure < Hbnd4DefaultType > ,
                                    BndsegPllBaseXMacroClosure < Hbnd4DefaultType > >;

  // from serial part with different template argument
  template class Hedge1Top< GitterBasisPll::ObjectsPll::Hedge1EmptyPll >;
  template class Hface4Top< GitterBasisPll::ObjectsPll::Hface4EmptyPll >;
  template class HexaTop< GitterBasisPll::ObjectsPll::HexaEmptyPll >;
  template class Periodic4Top < GitterBasisPll::ObjectsPll::Periodic4EmptyPll >;

} // namespace ALUGrid
