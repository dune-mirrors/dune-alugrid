// (c) Robert Kloefkorn 2010
#include <config.h>

#include <dune/alugrid/common/alugrid_assert.hh>

#include "gitter_tetra_top_pll.h"
#include "../serial/gitter_tetra_top.cc"

namespace ALUGrid
{

  template < class A, class X, class MX >
  void Hbnd3PllInternal < A, X, MX >::HbndPll::splitGhost ( GhostChildrenInfo_t &info )
  {
    if(_ghostPair.first)
    {
      // get the childs
      typedef typename Gitter::Geometric::tetra_GEO  tetra_GEO;
      typedef typename Gitter::Geometric::hface3_GEO hface3_GEO;

      // ghostpair.second is the internal face number of the face
      // connected to the interior of the process
      // in case of bisection count can be zero since the face might have not been split

      GhostTetra_t* ghost = static_cast<GhostTetra_t *> (_ghostPair.first);

      if( ! ghost->down() )
      {
        ghost->tagForGlobalRefinement();
        ghost->refine();
      }

      typedef std::pair< Gitter::Geometric::hasFace3 *, int > neigh_t;

      hface3_GEO * orgFace = ghost->myhface( _ghostPair.second );
      hface3_GEO * face    = orgFace->down();

#ifdef ALUGRIDDEBUG
      int breakCount = 0;
#endif
      while( ! face )
      {
        neigh_t neighbour = orgFace->nb.front();
        // this is true for the boundaries of ghost elements (see null face3Neighbour)
        if( neighbour.second < 0 )
        {
          alugrid_assert ( neighbour.first->isboundary() );
          neighbour = orgFace->nb.rear();
        }

        tetra_GEO* elem = static_cast<tetra_GEO *> (neighbour.first);
        // make sure that cast worked
        alugrid_assert ( dynamic_cast<tetra_GEO *> (neighbour.first) );
        // refine element with suitable refinement rule
        elem->tagForGlobalRefinement();
        elem->refine();

        face = orgFace->down();
        alugrid_assert ( breakCount++ < 5 );
      }

      // find new ghost elements
      {
        alugrid_assert ( face );
        int count = 0;
        for(; face; face = face->next() )
        {
          alugrid_assert (face);

          // check neighbours
          neigh_t neighbour = face->nb.front();
          // if nb is boundary take other neighbour
          if( neighbour.second < 0 )
          {
            alugrid_assert ( neighbour.first->isboundary() );
            neighbour = face->nb.rear();
          }

          alugrid_assert ( ! neighbour.first->isboundary () );
          tetra_GEO* ghch = static_cast<tetra_GEO *> (neighbour.first);
          alugrid_assert ( dynamic_cast<tetra_GEO *> (neighbour.first) );
          // check father only for non-conforming refinement
          alugrid_assert ( ghost->getrule().bisection() ? true : ghch->up() == ghost );

          // set element pointer and local face number
          info.setGhostPair( ghostpair_STI( ghch, neighbour.second ) , count );

          ++count;
        }
        //this assert fails for 2d, because iso2 is not a bisection refinement
        alugrid_assert ( (ghost->getrule().bisection() || ghost->is2d() ) ? count == 2 : count == 4);
      }
    }
  }

  template < class A, class X, class MX >
  void Hbnd3PllInternal < A, X, MX >::HbndPll::
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
  void Hbnd3PllInternal < A, X, MX >::HbndPll:: coarseGhost ()
  {
    if(_ghostPair.first)
    {
      helement_STI& ghost = (*_ghostPair.first);
      if( ghost.leaf() ) return;

      // remove all descendents if possible
      removeDescendents( ghost );
    }
  }

  template < class A, class X, class MX >
  void Hbnd3PllInternal < A, X, MX >::HbndPll::
  setGhost ( const ghostpair_STI & gpair )
  {
    if(gpair.first)
    {
      _ghostPair = gpair;
      alugrid_assert ( _ghostPair.first );

      // copy indices from internal boundry to myhface(.) of ghost
      _ghostPair.first->setIndicesAndBndId ( *(this->myhface(0)) , _ghostPair.second );
    }
    else
    {
      _ghostPair.first  =  0;
      _ghostPair.second = -1;
    }
  }

  //***************************************************************************************
  //  --HbndPllMacro
  //***************************************************************************************
  template < class A, class X, class MX >
  const MacroGhostInfo_STI* Hbnd3PllInternal < A, X, MX >::
  HbndPllMacro::buildGhostCell(ObjectStream& os, int fce)
  {
    alugrid_assert ( _gm == 0 );
    int code = MacroGridMoverIF::ENDMARKER;
    os.readObject (code);
    alugrid_assert ( code == MacroGridMoverIF::HBND3INT );

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

      int v [3] = { -1, -1, -1 };
      os.readObject (v[0]);
      os.readObject (v[1]);
      os.readObject (v[2]);

      const signed char pointTransmitted = os.get();

      // the following makes only sense if information has been transmitted
      if( pointTransmitted != MacroGridMoverIF::POINTTRANSMITTED )
      {
        std::cerr << "ERROR: No point transmitted, building ghost cells impossible in " << __FILE__ << ", " << __LINE__ << std::endl;
        abort();
      }

      // create macro ghost cell
      {
        // create ghost info and read from stream
        MacroGhostInfoTetra* ghInfo = new MacroGhostInfoTetra( os );

        myhface3_t * f = this->myhface(0);
        alugrid_assert ( f );

        // ghInfo is stored inside MacroGhostHexa
        _gm = new MacroGhostTetra( _mgb , ghInfo,  f );
        this->setGhost ( _gm->getGhost() );
      }
    }

    alugrid_assert ( _gm );
    return _gm->getGhostInfo();
  }

  // template instantiation
  typedef GitterBasis::Objects::Hbnd3Default Hbnd3DefaultType;
  template class Hbnd3PllInternal < Hbnd3DefaultType ,
                                    BndsegPllBaseXClosure < Hbnd3DefaultType > ,
                                    BndsegPllBaseXMacroClosure < Hbnd3DefaultType > >;

  // from serial part with different template argument
  template class Hface3Top< GitterBasisPll::ObjectsPll::Hface3EmptyPll >;
  template class TetraTop< GitterBasisPll::ObjectsPll::TetraEmptyPll >;
  template class Periodic3Top < GitterBasisPll::ObjectsPll::Periodic3EmptyPll >;

} // namespace ALUGrid
