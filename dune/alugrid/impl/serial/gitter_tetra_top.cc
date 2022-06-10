// (c) Robert Kloefkorn 2010
#include <config.h>

#include "mapp_tetra_3d.h"
#include "gitter_tetra_top.h"
#include "gitter_impl.h"

namespace ALUGrid
{

  // #     #                                  #####  #######
  // #     #  ######    ##     ####   ###### #     #    #      ####   #####
  // #     #  #        #  #   #    #  #            #    #     #    #  #    #
  // #######  #####   #    #  #       #####   #####     #     #    #  #    #
  // #     #  #       ######  #       #            #    #     #    #  #####
  // #     #  #       #    #  #    #  #      #     #    #     #    #  #
  // #     #  #       #    #   ####   ######  #####     #      ####   #

  template< class A >
  typename Hface3Top < A >::myvertex_t*
  Hface3Top < A >::vertexNotOnSplitEdge( const int splitEdge )
  {
    const myhedge_t* edge = myhedge( splitEdge );
    const myvertex_t* edgeVx[ 2 ] = { edge->myvertex( 0 ), edge->myvertex( 1 ) };
    int iVx = (splitEdge + 2) % 3;
    myvertex_t* vx = myvertex( iVx );
    while ( vx == edgeVx[ 0 ] || vx == edgeVx[ 1 ] )
    {
      iVx = ( iVx + 1 ) % 3;
      vx = myvertex( iVx );
      alugrid_assert ( iVx != (splitEdge+2)%3 );
    }

    return vx ;
  }

  template< class A > typename Hface3Top < A >::edgepair_t
  Hface3Top < A >::subEdges( myhedge_t* edge, const myvertex_t* vx0, const myvertex_t* vx1 )
  {
    alugrid_assert ( vx0 );
    alugrid_assert ( vx1 );

    // get sub faces
    myhedge_t* subEdge[ 2 ] = { edge->subedge( 0 ), edge->subedge( 1 ) };

    // check face order with vertex0
    int sub0 = 1 ;
    for( int i=0; i<2; ++i )
    {
      if( subEdge[ 0 ]->myvertex( i ) == vx0 )
      {
        sub0 = 0;
        break;
      }
    }

#ifdef ALUGRIDDEBUG
    // make sure the vertex is on the other face
    bool found0 = false ;
    bool found1 = false ;
    // check face order with vertex0
    for( int i=0; i<2; ++i )
    {
      if( subEdge[ sub0 ]->myvertex( i ) == vx0 )
      {
        found0 = true ;
      }
      if( subEdge[ ! sub0 ]->myvertex( i ) == vx1 )
      {
        found1 = true ;
      }
    }

    if( ! found0 || ! found1 )
    {
      std::cout << "Problem: " << edge << std::endl;
      std::cout << " vx0 " << vx0 << std::endl;
      std::cout << " vx1 " << vx1 << std::endl;
      std::cout << "sub0 " << subEdge[ sub0 ] << std::endl;
      std::cout << "sub1 " << subEdge[ ! sub0 ] << std::endl;
    }

    alugrid_assert ( found0 );
    alugrid_assert ( found1 );

#endif
    return edgepair_t( subEdge[ sub0 ], subEdge[ ! sub0 ] );
  }


  template< class A > void Hface3Top < A >::split_e01 ()
  {
    // NOTE: edge numbering is not opposite vertex !!!
    // see gitter_geo.cc
    // the new edge needs to be in the middle (meaning edge 1 out of {0,1,2})

    /*
                      2
                      *\
                    /2 2\
                   /  *  \
                  /       \
                 /    *    \
             2  /           \ 1
               /2    1*2    1\
              /               \
             /        *        \
            /                   \
           /          *          \
          /0    0    1 0    0    1\
          ------------*------------
        0            0               1
      */

    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + level () ;
    myhedge_t* splitEdge = myhedge(0);

    myvertex_t * ev0 = splitEdge->subvertex (0) ;
    myvertex_t * vx2 = vertexNotOnSplitEdge( 0 );

    edgepair_t subEdge = subEdges( splitEdge, myvertex(0), myvertex(1) );
    //myvertex_t * vx2 = myvertex( 2 );
    alugrid_assert (ev0) ;
    inneredge_t * e0 = new inneredge_t (newLevel, ev0, vx2 ) ;
    alugrid_assert ( e0 ) ;
    innerface_t * f0 = new innerface_t (newLevel,
                                        subEdge.first, twist(0),
                                        e0, 0,
                                        myhedge(2), twist(2),
                                        0) ; // child number

    innerface_t * f1 = new innerface_t (newLevel,
                                        subEdge.second, twist(0),
                                        myhedge(1), twist(1),
                                        e0, 1,
                                        1) ; // child number

    //std::cout << "split_e01 " << ev0 << std::endl;
    //std::cout << "Split face " << this << " into " << std::endl;
    //std::cout << "New subface 0" << f0 << std::endl;
    //std::cout << "New subface 1" << f1 << std::endl;

    alugrid_assert (f0 && f1 ) ;
    f0->append(f1) ;

    _inner = new inner_t( f0 , e0 );
    _rule = myrule_t::e01 ;
    return ;
  }

  template< class A >  void Hface3Top < A >::split_e12 ()
  {
    // NOTE: edge numbering is not opposite vertex !!!
    // see gitter_geo.cc
    // the new edge needs to be in the middle (meaning edge 1 out of {0,1,2})

    /*                  2

                       / \
                      / 2 \
                     /     \
                    /     1 \
                   /         \
                  /           \
               2 / 2         1 \  1
                /             * \
               /           *   2 \
              /      0  *         \
             /       *           1 \
            /     *   2             \
           /0  *                     \
          / *0                      1 \
          -----------------------------
        0              0                 1
      */
    alugrid_assert ( _inner == 0 );
    const int newLevel= 1 + level () ;
    myhedge_t* splitEdge = myhedge(1);

    myvertex_t * ev0 = splitEdge->subvertex (0) ;
    alugrid_assert (ev0) ;

    myvertex_t * vxOld = vertexNotOnSplitEdge( 1 );
    //myvertex_t * vxOld = myvertex(0);

    edgepair_t subEdge = subEdges( splitEdge, myvertex(1), myvertex(2) );

    // create new inner edge
    inneredge_t * e0 = new inneredge_t (newLevel, ev0, vxOld ) ;
    alugrid_assert ( e0 ) ;

    //std::cout << "split_e12 " << ev0 << std::endl;
    //std::cout << "new inner edge " << e0 << std::endl;

    innerface_t * f0 = new innerface_t (newLevel, // level
                                        myhedge(0), twist(0),         // edge 0, twist
                                        subEdge.first, twist(1), // edge 0, twist
                                        e0, 0,                         // edge 1, twist
                                        0 ) ; // child number

    innerface_t * f1 = new innerface_t (newLevel, // level
                                        e0, 1,                         // edge 1, twist
                                        subEdge.second, twist(1), // edge 2, twist
                                        myhedge(2), twist(2),         // edge 0, twist
                                        1 ) ; // child number
    alugrid_assert (f0 && f1 ) ;
    f0->append(f1) ;

    //std::cout << "Split face " << this << " into " << std::endl;
    //std::cout << "New subface 0" << f0 << std::endl;
    //std::cout << "New subface 1" << f1 << std::endl;

    _inner = new inner_t( f0 , e0 );
    _rule = myrule_t::e12 ;


    return ;
  }

  template< class A >  void Hface3Top < A >::split_e20 ()
  {
    // NOTE: edge numbering is not opposite vertex !!!
    // see gitter_geo.cc
    // the new edge needs to be in the middle (meaning edge 1 out of {0,1,2})

    /*                  2

                       / \
                      / 2 \
                     /     \
                    /2      \
                   /         \
                  /           \
               2 /0            \  1
                / *            1\
               / 2   *           \
              /         * 0       \
             /2          1 *       \
            /                 *     \
           /                     * 1 \
          / 0          0          1 * \
          -----------------------------
        0              0                 1
      */

    alugrid_assert ( _inner == 0 );
    const int newLevel= 1 + level () ;

    myhedge_t* splitEdge = myhedge( 2 );

    // get vertices of new edge
    myvertex_t * ev0 = splitEdge->subvertex (0) ;
    myvertex_t * vxOld = vertexNotOnSplitEdge( 2 );

    edgepair_t subEdge = subEdges( splitEdge, myvertex(0), myvertex(2) );
    //myvertex_t * vxOld = myvertex( 1 );

    alugrid_assert (ev0) ;
    inneredge_t * e0 = new inneredge_t (newLevel, ev0, vxOld ) ;
    alugrid_assert ( e0 ) ;

    innerface_t * f0 = new innerface_t (newLevel, // level
                                        e0, 0,                         // edge 0, twist
                                        myhedge(1), twist(1),         // edge 1, twist
                                        subEdge.second, twist(2), // edge 2, twist
                                        0 ) ; // child number

    innerface_t * f1 = new innerface_t (newLevel, // level
                                        myhedge(0), twist(0),         // edge 0, twist
                                        e0, 1,                         // edge 1, twist
                                        subEdge.first, twist(2), // edge 2, twist
                                        1 ) ; // child number

    alugrid_assert (f0 && f1 ) ;

    //std::cout << "split_e20 " << ev0 << std::endl;
    //std::cout << "Split face " << this << " into " << std::endl;
    //std::cout << "New subface 0" << f0 << std::endl;
    //std::cout << "New subface 1" << f1 << std::endl;

    f0->append(f1) ;
    _inner = new inner_t( f0 , e0 );
    _rule = myrule_t::e20 ;

    return ;
  }

  template< class A >  void Hface3Top < A >::split_iso4 ()
  {
    alugrid_assert ( _inner == 0 );
    int l = 1 + level () ;
    myvertex_t * ev0 = myhedge(0)->subvertex (0) ;
    myvertex_t * ev1 = myhedge(1)->subvertex (0) ;
    myvertex_t * ev2 = myhedge(2)->subvertex (0) ;
    alugrid_assert (ev0 && ev1 && ev2 ) ;
    inneredge_t * e0 = new inneredge_t (l, ev0, ev1) ;
    inneredge_t * e1 = new inneredge_t (l, ev1, ev2) ;
    inneredge_t * e2 = new inneredge_t (l, ev2, ev0) ;
    alugrid_assert ( e0 && e1 && e2 ) ;
    e0->append(e1) ;
    e1->append(e2) ;
    innerface_t * f0 = new innerface_t (l, this->subedge(0,0), twist(0), e2, 1, this->subedge(2,1), twist(2), 0) ;
    innerface_t * f1 = new innerface_t (l, this->subedge(0,1), twist(0), this->subedge(1,0), twist(1), e0, 1, 1) ;
    innerface_t * f2 = new innerface_t (l, e1, 1, this->subedge(1,1), twist(1), this->subedge(2,0), twist(2), 2) ;
    innerface_t * f3 = new innerface_t (l, e0, 0, e1, 0, e2, 0, 3 ) ;
    alugrid_assert (f0 && f1 && f2 && f3) ;
    f0->append(f1) ;
    f1->append(f2) ;
    f2->append(f3) ;
    _inner = new inner_t( f0 , e0 );
    _rule = myrule_t::iso4 ;
    return ;
  }

  template< class A > void Hface3Top < A >::refineImmediate (myrule_t r)
  {
    if ( r != getrule () )
    {
      alugrid_assert (getrule () == myrule_t::nosplit) ;
      switch(r)
      {
        typedef typename myhedge_t::myrule_t myhedgerule_t;

        // rotate of hedge rule does nothing,
        // so its actually useless

        case myrule_t::e01 :
          myhedge (0)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (0))) ;
          split_e01 () ;
          break ;
        case myrule_t::e12 :
          myhedge (1)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (1))) ;
          split_e12 () ;
          break ;
        case myrule_t::e20 :
          myhedge (2)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (2))) ;
          split_e20 () ;
          break ;
        case myrule_t::iso4 :
          //iso4 on a 2d face is just e12 bisection + setting the rule to iso4
          if(this->is2d()) {
            myhedge (1)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (1))) ;
            split_e12 () ;
            _rule = myrule_t::iso4 ;
            break ;
          }
          myhedge (0)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (0))) ;
          myhedge (1)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (1))) ;
          myhedge (2)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (2))) ;
          split_iso4 () ;
          break ;
        default :
          std::cerr << "**ERROR (FATAL) wrong refinement rule [" << r << "] in " << __FILE__ << " " << __LINE__ << std::endl ;
          abort () ;
          break ;
      }

      // set parent rule
      {
        myrule_t myRule = getrule();
        for (innerface_t * f = dwnPtr() ; f ; f = f->next ())
        {
          f->nb._parRule = myRule;
        }
      }
      this->postRefinement () ;
    }
    return ;
  }

  template< class A > bool Hface3Top < A >::refine (myrule_t r, int twist)
  {
    if (r != getrule ())
    {
      alugrid_assert (getrule () == myrule_t::nosplit ? 1 :
        (std::cerr << "**FEHLER beim Verfeinern mit Regel " << r << " auf " << getrule () << std::endl, 0)) ;
      switch(r)
      {
        case myrule_t::e01 :
        case myrule_t::e12 :
        case myrule_t::e20 :
        case myrule_t::iso4 :
        {
          typedef typename A::face3Neighbour::neighbour_t neighbour_t ;
          // get face neighbour
          neighbour_t neigh = ( twist < 0 ) ? this->nb.front () : this->nb.rear()  ;

          bool a = true;
          if(!r.bisection())
            // check refineBalance
            a = neigh.first->refineBalance (r, neigh.second);

          if (a)
          {
            if (getrule () == myrule_t::nosplit)
            {
              refineImmediate (r) ;
              {
                for (innerface_t * f = dwnPtr() ; f ; f = f->next ())
                {
                  // assign neighbor info to child faces for initialization
                  f->nb.assign( this->nb ) ;
                }
              }
              //for bisection we want no hanging nodes
              if(r.bisection())
              {
                //the neighbour may have changed
                neighbour_t neigh = ( twist < 0 ) ? this->nb.front () : this->nb.rear()  ;
                //refine neighbour as long as it is leaf.
                //We are at the refinement face, so the neighbour of the refined element
                //has to be refined too
                while(neigh.first->nbLeaf())
                {
                  neigh.first->refineBalance (r, neigh.second);
                  neigh = ( twist < 0 ) ? this->nb.front () : this->nb.rear()  ;
                }
                for (innerface_t * f = dwnPtr() ; f ; f = f->next ())
                {
                  // assert faces are leaf - this is not true anymore with weak compatibility
               //   alugrid_assert(f->leaf());
                 // alugrid_assert(f->nb.front().first->nbLeaf());
                  //alugrid_assert(f->nb.rear().first->nbLeaf());
                ;
                }
              }
            }
            else
            {
              // Als Test absichern, da"s die Verfeinerung durchgekommen ist. Im
              // anisotropen Fall darf das so nicht mehr gemacht werden.
              alugrid_assert (getrule () == r) ;
            }

            this->postRefinement () ;
            return true ;
          }
          else
          {
            return false ;
          }
        }
        default :
          std::cerr << "**WARNING (IGNORED) wrong refinement rule: " ;
          std::cerr << "[" << r << "] in " << __FILE__ << " " << __LINE__ << std::endl ;
          return false ;
      }
    }
    return true ;
  }

  template< class A > bool Hface3Top < A >::coarse ()
  {
    innerface_t * f = dwnPtr() ;
    if ( ! f ) return false ;
    bool x = true ;
    do
    {
      // Falls eine Kind-Fl"ache noch referenziert wird, kann
      // nicht auf diesem Level vergr"obert werden.
      // Daher wird nur die nichtkonforme Nachbarschaft ver-
      // vollst"andigt, die eventuell durch Elementvergr"oberung
      // durcheinander gekommen war. Die Vergr"oberung geht dann
      // auf das n"achste Level "uber.
      if (f->ref)
      {
        if (f->ref == 1) f->nb.complete (this->nb) ;
        f->coarse () ;
        x = false ;
      }
    }
    while ( (f = f->next()) ) ;

    //if( inEd()->ref > 0 )
    //  x = inEd()->noCoarsen() ? false : x ;

    if ( x ) // && inEd()->ref <= 2 )
    {
      // Hier wird tats"achlich vergr"obert, d.h. alle Kinder
      // werden beseitigt, und das Bezugsobjekt wird zum neuen
      // Blatt im Baum.

      // std::cout << "inner edge ref = " << inEd()->ref << std::endl;
      delete _inner;
      _inner = 0 ;

      _rule = myrule_t::nosplit ;
      { for (int i = 0 ; i < 3 ; ++i ) myhedge (i)->coarse () ; }
    }
    return x ;
  }

  template< class A >
  void Hface3Top< A >::backup ( std::ostream &os ) const
  {
    doBackup( os );
  }

  template< class A >
  void Hface3Top< A >::backup ( ObjectStream &os ) const
  {
    doBackup( os );
  }

  template< class A >
  template< class OutStream_t>
  void Hface3Top < A >::doBackup (OutStream_t & os) const
  {
    os.put ((char) getrule ()) ;
    {for (const inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->backup (os) ; }
    {for (const innerface_t * c = dwnPtr() ; c ; c = c->next ()) c->backup (os) ; }
    return ;
  }

  template< class A >
  void Hface3Top< A >::restore ( std::istream &is )
  {
    doRestore( is );
  }

  template< class A > void Hface3Top< A >::restore ( ObjectStream &is )
  {
    doRestore( is );
  }

  template< class A > template<class InStream_t>
  void Hface3Top < A >::doRestore (InStream_t & is)
  {
    refineImmediate (myrule_t ((char) is.get ())) ;
    {for (inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->restore (is) ; }
    {for (innerface_t * c = dwnPtr() ; c ; c = c->next ()) c->restore (is) ; }
    return ;
  }

  // #     #                          #####  #######
  // #     #  #####   #    #  #####  #     #    #      ####   #####
  // #     #  #    #  ##   #  #    #       #    #     #    #  #    #
  // #######  #####   # #  #  #    #  #####     #     #    #  #    #
  // #     #  #    #  #  # #  #    #       #    #     #    #  #####
  // #     #  #    #  #   ##  #    # #     #    #     #    #  #
  // #     #  #####   #    #  #####   #####     #      ####   #

  template< class A >  void Hbnd3Top < A >::split_bisection()
  {
    int l = 1 + level () ;

    typedef typename Gitter::GhostChildrenInfo GhostChildrenInfo;
    GhostChildrenInfo ghostInfo;
    // ghostInfo is filled by splitGhost, see gitter_tetra_top_pll.h
    this->splitGhost( ghostInfo );

    //int gFace = this->getGhost().second ;

    innerbndseg_t * b0 = new innerbndseg_t (l, subface (0,0), twist (0), this , _bt, ghostInfo.child(0), ghostInfo.face(0) ) ;
    innerbndseg_t * b1 = new innerbndseg_t (l, subface (0,1), twist (0), this , _bt, ghostInfo.child(1), ghostInfo.face(1) ) ;
    //innerbndseg_t * b0 = new innerbndseg_t (l, subface (0,0), twist (0), this , _bt, 0, gFace ) ;
    //innerbndseg_t * b1 = new innerbndseg_t (l, subface (0,1), twist (0), this , _bt, 0, gFace ) ;

    alugrid_assert (b0 && b1) ;
    b0->append(b1) ;
    _dwn = b0 ;
    return ;
  }

  template< class A >  void Hbnd3Top < A >::split_iso4 ()
  {
    int l = 1 + level () ;

    typedef typename Gitter::GhostChildrenInfo GhostChildrenInfo;
    GhostChildrenInfo ghostInfo;
    // ghostInfo is filled by splitGhost, see gitter_tetra_top_pll.h
    this->splitGhost( ghostInfo );

    innerbndseg_t * b0 = new innerbndseg_t (l, subface (0,0), twist (0), this , _bt, ghostInfo.child(0), ghostInfo.face(0)) ;
    innerbndseg_t * b1 = new innerbndseg_t (l, subface (0,1), twist (0), this , _bt, ghostInfo.child(1), ghostInfo.face(1)) ;
    innerbndseg_t * b2 = new innerbndseg_t (l, subface (0,2), twist (0), this , _bt, ghostInfo.child(2), ghostInfo.face(2)) ;
    innerbndseg_t * b3 = new innerbndseg_t (l, subface (0,3), twist (0), this , _bt, ghostInfo.child(3), ghostInfo.face(3)) ;
    alugrid_assert (b0 && b1 && b2 && b3) ;
    b0->append(b1) ;
    b1->append(b2) ;
    b2->append(b3) ;
    _dwn = b0 ;

    return ;
  }

  template< class A >  bool Hbnd3Top < A >::coarse ()
  {
    innerbndseg_t * b = down () ;
    if (!b) return false ;
    bool x = true ;
    do {
      if( (b->myhface(0)->ref > 1) ) ((b->coarse ()), x = false) ;
    } while ( (b = b->next()) ) ;
    if (x)
    {
      if (!this->lockedAgainstCoarsening ())
      {
        this->preCoarsening () ;

        delete _dwn ; _dwn = 0 ;
        myhface (0)->coarse () ;

        this->coarseGhost();
      }
    }
    return x ;
  }

  template< class A >  bool Hbnd3Top < A >::bndNotifyCoarsen () {
    return coarse () ;
  }

  template< class A >  bool Hbnd3Top < A >::refineBalance (balrule_t r, int b)
  {

    // Die Methode refineBalance () f"uhrt auf dem Randabschluss entweder
    // unbedingt die Verfeinerung durch, da im Verlauf der Verfeinerung keine
    // weiteren Anforerungen mehr an den Randabschluss  gerichtet werden
    // ODER gibt die Verfeinerung als nicht erf"ullt zur"uck: Dann liegt
    // es am Aufrufer die Verfeinerung nochmals anzuforern.

    alugrid_assert (b == 0) ;

    alugrid_assert (this->leaf ()) ;
    if ( ! this->bndNotifyBalance (r,b) )
    {

      // Hier kann der innere Rand [parallel] die Verfeinerung
      // verhindern, damit z.B. das Durchverfeinern im anisotropen
      // Fall erstmal nicht stattfindet, wenn nicht klar ist, wie die
      // weitere Rekursion aussieht. Dazu muss auf dem Niveau der Klasse
      // des Template-Arguments die Methode bndNotifyBalance () "uber-
      // schrieben werden. Die Defaultmethode liefert immer 'true'.

      return false ;
    }
    else
    {
      myhface_t& face (*(myhface(0)));

      // refine face according to rule
      face.refineImmediate (r) ;

      // std::cout << this->level() << " " << face.level() << " " << this->getrule() << " "  << face.getrule() << " " << r <<  " " << &face ;

      if(r == myrule_t::iso4)
      {
        // Der Rand verfeinert unbedingt die anliegende Fl"ache und dann
        // sich selbst, weil die Anforderung durch die Fl"ache kam, und
        // dahinter keine Balancierung stattfinden muss.

        //on the 2d faces we just have a e12 bisection refinement
        if(face.is2d())
          split_bisection();
        else
          split_iso4 () ;
      }
      else if( r.bisection() )
      {
        split_bisection () ;
      }
      else
      {
        std::cerr << "**FEHLER (FATAL, weil nicht vorgesehen) beim Verfeinern am " ;
        std::cerr << "Randst\"uck mit der Regel [" << r << "] in " ;
        std::cerr << __FILE__ << " " << __LINE__ << std::endl ;
        abort () ;
      }

      // postRefinement () gibt die M"oglichkeit auf dem Niveau des
      // Template-Arguments eine Methode aufzurufen, um eventuelle
      // Operationen auf dem verfeinerten Randst"uck durchzuf"uhren.

      this->postRefinement () ;
      return true ;
    }
  }

  template< class A >  bool Hbnd3Top < A >::refineLikeElement (balrule_t r)
  {

    // Mit der Methode refineLikeElement () verh"alt sich ein Randabschluss
    // in der Verfeinerung wie ein Element: Es wird zuerst gepr"uft ob eine
    // Balancierung der Vererfeinerung durch die Fl"ache hindurch erfolgreich
    // ist und nur genau dann die Verfeinerung durchgef"uhrt mit R"uckgabewert
    // 'true'. Diese Methode bedient eigentlich nur die parallele Verfeinerung
    // kann aber auch auf jedem beliebigen Randelement im seriellen Fall auf-
    // gerufen werden ohne Schaden anzurichten: Eine 1-Level Verfeinerung am
    // Rand ist jedoch wirkungslos, da sie beim n"achsten Vergr"obern wieder
    // aufgel"ost ist. Erst die mehrfache Anwendung f"uhrt durch die
    // Balancierung zu einer "Anderung am Elementgitter.

    if (r == myrule_t::nosplit) {

      std::cerr << "**WARNUNG (IGNORIERT) beim Versuch mit nosplit zu Verfeinern" ;
      std::cerr << "  in " << __FILE__ << " " << __LINE__ << std::endl ;

        // Eine Anforderung mit nosplit zu Verfeinern nur erf"ullt,
    // falls die zugeh"orige Fl"achenregel auch nosplit ist, sonst
    // wird die Anforderung als nicht erf"ullt zur"uckgegeben.

      return this->getrule () == balrule_t::nosplit ? true : false ;

    } else
      {
        if (this->getrule () == r) {

        // Alles schon wie es sein soll -> true.

        return true ;
      }
      else {
        myhface_t& face (*(myhface(0)));

        // Der nachfolgende Test bezieht sich auf die Verfeinerungssituation
        // der Fl"ache, da getrule () auf myhface (0)->getrule () umgeleitet
        // ist.
        // if(this->getrule () != myrule_t::nosplit)
        //std::cout << this->level() << " " << face.level() << " " << this->getrule() << " " << face.getrule() << " " << r << " " <<   &face;

        alugrid_assert (this->getrule () == myrule_t::nosplit) ;
        switch (r) {
        case balrule_t::e01 :
        case balrule_t::e12 :
        case balrule_t::e20 :
          //std::cout << "refLikeEl: e01 " << std::endl;
          // if (!myhface (0)->refine (balrule_t (balrule_t::e01).rotate (twist (0)), twist (0))) return false ;
          if (! face.refine (r, twist (0))) return false ;

          split_bisection() ;
          break;
        case balrule_t::iso4 :
          //in the 2d case we just to the same as e12
          if( face.is2d()){
            if (! face.refine (r, twist (0))) return false ;
            split_bisection() ;
            break;
          }
          //if (!myhface (0)->refine (balrule_t (balrule_t::iso4).rotate (twist (0)), twist (0))) return false ;
          if (! face.refine (r, twist (0))) return false ;
          split_iso4 () ;
          break;
        default :
          std::cerr << "**WARNING (ERROR IGNORED) wrong refinement rule [" << this->getrule () ;
          std::cerr << "] (ignored) in " << __FILE__ << " " << __LINE__ << std::endl ;
          return false ;
        }

        // postRefinement () gibt die M"oglichkeit auf dem Niveau des
        // Template-Arguments eine Methode aufzurufen, um eventuelle
        // Operationen auf dem verfeinerten Randst"uck durchzuf"uhren.
        this->postRefinement () ;
        return true ;
      }
    }
  }

  template< class A >  void Hbnd3Top < A >::restoreFollowFace ()
  {
    // retoreFollowFace () veranlasst das Randelement sich am
    // bestehenden Fl"achenbaum wiederherzustellen durch die
    // entsprechende Verfeinerung.

    myhface_t& face (*(myhface (0))) ;
    if (! face.leaf ()) {
      balrule_t r = face.getrule () ;
      switch (r) {
        case myrule_t::e01 :
        case myrule_t::e12 :
        case myrule_t::e20 :
          split_bisection();
          break ;
        case myrule_t::iso4 :
          if( face.is2d() ) {
            split_bisection();
            break;
          }
          split_iso4 () ;
          break ;
        default :
          std::cerr << "**FEHLER (FATAL) beim Verfeinern am Randst\"uck mit der Regel [" << r << "] in " << __FILE__ << " " << __LINE__ << std::endl ;
          abort () ;
          break ;
      }
      this->postRefinement () ;
      {for (innerbndseg_t * b = down () ; b ; b = b->next ()) b->restoreFollowFace () ; }
    }
    return ;
  }

  // #######                                 #######
  //    #     ######   #####  #####     ##      #      ####   #####
  //    #     #          #    #    #   #  #     #     #    #  #    #
  //    #     #####      #    #    #  #    #    #     #    #  #    #
  //    #     #          #    #####   ######    #     #    #  #####
  //    #     #          #    #   #   #    #    #     #    #  #
  //    #     ######     #    #    #  #    #    #      ####   #

  template< class A > TetraTop < A >
  :: TetraTop (int l, myhface_t * f0, int t0,
               myhface_t * f1, int t1, myhface_t * f2, int t2,
               myhface_t * f3, int t3, innertetra_t *up, int nChild, double vol)
    : A (f0, t0, f1, t1, f2, t2, f3, t3),
      _bbb (0), _up(up)
    , _inner( 0 )
    , _volume( vol < 0.0 ? computeVolume() : vol )
    , _lvl (l)
    , _nChild(nChild)
    , _rule (myrule_t::nosplit)
  {
    // vxMap is set by the setNewMapping routine
    _vxMap[ 0 ] = _vxMap[ 1 ] = _vxMap[ 2 ] = _vxMap[ 3 ] = -1;

    // set level
    alugrid_assert ( this->level() == l );

    // set element's index and 2d flag if necessary
    this->setIndexAnd2dFlag( indexManager() );

    // we need boundary id now, for elements is the same as fathers
    this->_bndid = _up->bndId();

#ifdef ALUGRIDDEBUG
    // check that _volume has the correct value
    const double calculatedVolume = computeVolume();
    const double myvol = volume();

    //In the 2d case the 3d volume may be 0, so do not divide by it
    if(this->is2d())
      alugrid_assert ( std::abs( calculatedVolume - myvol ) < 1e-10 );
    else
      alugrid_assert ( std::abs( calculatedVolume - myvol ) / myvol < 1e-10 );
#endif
  }

  // this is the macro element constructor
  template< class A > TetraTop < A >::
  TetraTop (int l,  // level
            myhface_t * f0, int t0, // face, twist
            myhface_t * f1, int t1, // face, twist
            myhface_t * f2, int t2, // face, twist
            myhface_t * f3, int t3, // face, twist
            SimplexTypeFlag simplexType )
    : A (f0, t0, f1, t1, f2, t2, f3, t3)
    , _bbb (0), _up(0)
    , _inner( 0 )
    , _volume( computeVolume() )
    , _lvl (l)
    , _nChild(0)  // we are macro ==> nChild 0
    , _rule (myrule_t::nosplit)
  {
    alugrid_assert ( this->level() == l );

    // set element's index and 2d flag if necessary
    this->setIndexAnd2dFlag( indexManager() );

    // currently, only type 0 and 1 are supported on init
    // more change needs to be done on DuneIndexProvider to support type 2
    //alugrid_assert( simplexType.type() < 2 );

    // if simplex type is not 0 then we assume type 1
    if( simplexType.type() > 1 )
      this->setSimplexTypeFlagOne() ;
    if( simplexType.type() % 2 )
      this->setSimplexTypeFlagZero() ;

    // initial mapping is has to be adjusted according
    // to the make-6 algorithm
    // NOTE: the _vxMap numbers only affect the bisection refinement
    const int mod = 1 - simplexType.orientation() ;
    _vxMap[ 0 ] = 0;
    _vxMap[ 1 ] = 1;
    _vxMap[ 2 ] = 2 + mod ;
    _vxMap[ 3 ] = 3 - mod ;
  }

  template< class A > TetraTop < A >::~TetraTop ()
  {
    this->freeIndex( indexManager() );
    // attachleafs is called in constructor of TetraEmpty
    // if delete is called on macro we only call this method on leaf
    if (! _inner ) this->detachleafs();
    if (_bbb) delete _bbb ;
    if (_inner) delete _inner ;

    // get refinement rule
    const bool bisection = _up && _up->_rule.bisection() ;

    // the following is only needed for bisection refinement
    if( bisection )
    {
      // setup connector containing father element
      std::pair< Gitter::Geometric::hasFace3 *, int > connect( Gitter::Geometric::InternalHasFace3()( _up ), int( -1 ) );
      for( int i=0; i<4; ++i )
      {
        // get face and twist
        myhface_t* face = myhface( i );
        const int twst = twist( i );

        // check whether this face has unresolved previous attached elements
        if( face->moreAttachments( twst ) )
        {
          // search face in father
          for( int ff=0; ff<4; ++ff )
          {
            myhface_t* upFce = _up->myhface( ff );
            // if faces match setup new connector
            if( face == upFce )
            {
              // set face number of face in father
              connect.second = ff ;
              face->detachElement ( twst, connect );
              // break for loop for ff
              break ;
            }
          }
        }
        else
        {
          // normal detachment
          face->detachElement ( twst );
        }
      }
    }
    else
    {
      // the default normal detachment of all faces
      for( int i=0; i<4; ++i )
        myhface( i )->detachElement ( twist( i ) ) ;
    }
  }

  //- --subedge
  template< class A >  typename TetraTop < A >::myhedge_t * TetraTop < A >::subedge (int face, int edge)
  {
    switch ( myhface( face )->getrule() )
    {
    case myhface_t::myrule_t::e01 :
      alugrid_assert ( edge == 0 );  // for bisection we only get one subEdge of a face
      return myhface ( face )->subedge ( edge ) ;
    case myhface_t::myrule_t::e12 :
      alugrid_assert ( edge == 0 );  // for bisection we only get one subEdge of a face
      return myhface ( face )->subedge ( edge ) ;
    case myhface_t::myrule_t::e20 :
      alugrid_assert ( edge == 0 );  // for bisection we only get one subEdge of a face
      return myhface ( face )->subedge ( edge ) ;
    case myhface_t::myrule_t::iso4 :
      {
        myhface_t& myface (*(myhface( face )));
        //for the 2d faces we do the same as e12
        if( myface.is2d()){
          alugrid_assert ( edge == 0 );  // for bisection we only get one subEdge of a face
          return myface.subedge ( edge ) ;
        }
        alugrid_assert ( edge < 3 );
        return ((twist ( face ) < 0) ? myface.subedge ((8 - edge + twist ( face )) % 3) : myface.subedge ((edge + twist ( face )) % 3)) ;
      }
    case myhface_t::myrule_t::nosplit :
      std::cerr << "**ERROR (FATAL): subedge () called on non-refined face. In " << __FILE__ << " " << __LINE__ << std::endl ;
      abort () ;
      return 0 ;
    }
    return 0 ;
  }

  template< class A >  const typename TetraTop < A >::myhedge_t * TetraTop < A >::subedge (int i, int j) const {
    return ((TetraTop < A > *)this)->subedge (i,j) ;
  }


  // --subFaces
  template< class A >
  typename TetraTop < A >::facepair_t
  TetraTop < A >::subFaces ( const int i )
  {
    // get face that we want sub faces from
    myhface_t* face = myhface( i );

    typedef typename myhface_t::myrule_t  myrule_t;
    // get split rule of face
    const myrule_t rule = face->getrule() ;
    alugrid_assert ( rule == myrule_t::e01 ||
            rule == myrule_t::e12 ||
            rule == myrule_t::e20 );
    alugrid_assert ( -3 <= twist( i ) && twist( i ) <= 2 );

#ifdef ALUGRIDDEBUG
    if( rule == myrule_t::iso4 )
    {
      std::cerr << "**ERROR (FATAL): subFaces () not implemented for iso4 " << i << ". In " << __FILE__ << " " << __LINE__ << std::endl ;
      abort () ;
    }
    else if ( rule == myrule_t::nosplit )
    {
      std::cerr << "**ERROR (FATAL): subFaces () called on non-refined face " << i << ". In " << __FILE__ << " " << __LINE__ << std::endl ;
      abort () ;
    }
#endif

    // obtain rule id
    const unsigned int ruleId = int(rule) - 2;
    alugrid_assert ( ruleId <= 2 );

    // std::cout << "subFaces rule " << ruleId << " of face " << i << " with twist " << twist( i ) << std::endl;

    //                                twists  -3  -2  -1   0   1   2
    static const int subFace[ 3 ][ 6 ] = {  {  1,  1,  0,  0,  0,  1  }, // rule e01
                                            {  1,  0,  0,  0,  1,  1  }, // rule e12
                                            {  0,  0,  0,  0,  1,  1  }  // rule e20
                                         };
    // sub face 0 and 1
    unsigned int sub0 = subFace[ ruleId ][ twist( i ) + 3 ];

    /*
    if( elementType () == 0 && ruleId == 2 && twist( i ) == 0 )
    {
      sub0 = 0;
    }
    */
    // return sub face 0 and 1 of face i
    return facepair_t ( face->subface( sub0 ), face->subface( ! sub0 ) );
  }

  // --subFaces
  template< class A >
  typename TetraTop < A >::facepair_t
  TetraTop < A >::subFaces ( const int i,
                               const myvertex_t* vx0,
                               const myvertex_t* vx1 )
  {
    alugrid_assert ( vx0 );
    alugrid_assert ( vx1 );

    // get face that we want sub faces from
    myhface_t* face = myhface( i );

    // get sub faces
    myhface_t* subFce[ 2 ] = { face->subface( 0 ), face->subface( 1 ) };

    // check face order with vertex0
    int sub0 = 1 ;
    for( int i=0; i<3; ++i )
    {
      if( subFce[ 0 ]->myvertex( i ) == vx0 )
      {
        sub0 = 0;
        break;
      }
    }

#ifdef ALUGRIDDEBUG
    // make sure the vertex is on the other face
    bool found0 = false ;
    bool found1 = false ;
    // check face order with vertex0
    for( int i=0; i<3; ++i )
    {
      if( subFce[ sub0 ]->myvertex( i ) == vx0 )
      {
        found0 = true ;
      }
      if( subFce[ ! sub0 ]->myvertex( i ) == vx1 )
      {
        found1 = true ;
      }
    }
    if( ! found0 || ! found1 )
    {
      std::cout << "Problem: " << face << std::endl;
      std::cout << " vx0 " << vx0 << std::endl;
      std::cout << " vx1 " << vx1 << std::endl;
      std::cout << "sub0 " << subFce[ sub0 ] << std::endl;
      std::cout << "sub1 " << subFce[ ! sub0 ] << std::endl;
    }
    alugrid_assert ( found0 );
    alugrid_assert ( found1 );

#endif
    return facepair_t( subFce[ sub0 ], subFce[ ! sub0 ] );
  }

  // --subface
  template< class A >  typename TetraTop < A >:: myhface_t * TetraTop < A >::subface (int i, int j)
  {
    switch ( myhface( i )->getrule() )
    {
    case myhface_t::myrule_t::e01 :
      alugrid_assert ( j < 2 );
      if ( twist(i) == 0 ||  twist(i) == 1 ||  twist(i) == -1 )
        return myhface(i)->subface( j ) ;
      if ( twist(i) == 2 ||  twist(i) == -2 || twist(i) == -3 )
        return myhface(i)->subface(!j) ;
      std::cerr << __FILE__ << " " << __LINE__ << "myhface(i)->subface()" << std::endl;
      alugrid_assert ( false );
      return 0;
    case myhface_t::myrule_t::e12 :
      alugrid_assert ( j < 2 );
      if ( twist(i) == 0 ||  twist(i) == 2 ||  twist(i) == -3 )
        return myhface(i)->subface(j) ;
      if ( twist(i) == -1 || twist(i) == 1 ||  twist(i) == -2 )
        return myhface(i)->subface(!j) ;
      std::cerr << __FILE__ << " " << __LINE__ << "myhface(i)->subface()" << std::endl;
      return 0;
    case myhface_t::myrule_t::e20 :
      alugrid_assert ( j < 2 );
      if ( twist(i) == 1 ||  twist(i) == 2 ||  twist(i) == -2 )
        return myhface(i)->subface(j) ;
      if ( twist(i) == 0 ||  twist(i) == -1 || twist(i) == -3 )
        return myhface(i)->subface(!j) ;
      std::cerr << __FILE__ << " " << __LINE__ << "myhface(i)->subface()" << std::endl;
      return 0;
    case myhface_t::myrule_t::iso4 :
      {
        myhface_t& myface (*(myhface(i)));
        //for the 2d faces we do the same as for e12
        if(myface.is2d()){
           alugrid_assert ( j < 2 );
            if ( twist(i) == 0 ||  twist(i) == 2 ||  twist(i) == -3 )
              return myface.subface(j) ;
            if ( twist(i) == -1 || twist(i) == 1 ||  twist(i) == -2 )
              return myface.subface(!j) ;
            std::cerr << __FILE__ << " " << __LINE__ << "myhface(i)->subface()" << std::endl;
            return 0;
        }
        alugrid_assert ( j < 4 );
        if ( j == 3 )
        {
          return myface.subface(3);
        }
        else // ( j < 3 )
        {
          alugrid_assert ( j < 3 );
          return myface.subface(twist(i) < 0 ? (7 - j + twist(i)) % 3 : (j + twist(i)) % 3) ;
        }
      }
    case myhface_t::myrule_t::nosplit :
      std::cerr << "**ERROR (FATAL): subface () called on non-refined face. In " << __FILE__ << " " << __LINE__ << std::endl ;
      abort () ;
      return 0 ;
    default:
      std::cerr << "**FEHLER (FATAL): Falsche Verfeinerungsregel [" << myhface(i)->getrule() << "] in " << __FILE__ << " " << __LINE__ << std::endl ;
      abort() ;
    }
    return 0 ;
  }

  template< class A >  const typename TetraTop < A >:: myhface_t * TetraTop < A >::subface (int i, int j) const {
    return ((TetraTop < A > *)this)->subface (i,j) ;
  }

  template< class A >  void TetraTop < A >::split_e01 ()
  {
    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + this->level();

    splitInfo( myrule_t::e01 );

    myhedge_t* subEdge = this->subedge (2, 0);
    myhedge_t* subEdge2 = this->subedge (3, 0);
    myhedge_t* orgEdge = this->myhedge( 5 ) ;

    const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 0 : 1;

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       subEdge,  0, // from face 2 get subedge 0
                       orgEdge,  edgeTwst,
                       subEdge2, 1 // from face 1 get subedge 0
                      ) ;
    alugrid_assert ( newFace );

    facepair_t subFace2 = subFaces( 2, myvertex( 0 ), myvertex( 1 ) ); // get sub face 0 and 1 of face 2
    facepair_t subFace3 = subFaces( 3, myvertex( 0 ), myvertex( 1 ) ); // get sub face 0 and 1 of face 3

    // we divide by 2 means we divide the volume by 2
    const double childVolume = calculateChildVolume( 0.5 * _volume );

    /*

      3               2
         ___________
        |3         2|     new inner face ( 4, 3 , 2 )
        |*\       .*|
        |  \     .  |     child 0 is the child which contains node 0
        | * \   . * |     child 1 is the child which contains node 1
        |    \ .    |
        |  *  \  *  |
        |    . \    |     4 becomes node 1 in child 0
        |   *.  \   |     4 becomes node 0 in child 1
        |  .     \  |
        | .  * *  \ |
        |0   1 0   1|
        ------*------
      0       4       1

    */

    innertetra_t * h0 = new innertetra_t (newLevel,
                                          newFace, 0,
                                          myhface( 1 ),   twist( 1 ),
                                          subFace2.first, twist( 2 ),
                                          subFace3.first, twist( 3 ),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          myhface( 0 ),   twist( 0 ),
                                          newFace, -1,
                                          subFace2.second,twist( 2 ),
                                          subFace3.second,twist( 3 ),
                                          this, 1, childVolume) ;

    alugrid_assert (h0 && h1) ;

    // the new vertices are the ones that are missing
    // i.e. 3 in child 0  and  0 in child 1
    alugrid_assert ( h0->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h0->myvertex( 2 ) == myvertex( 2 ) );
    alugrid_assert ( h0->myvertex( 3 ) == myvertex( 3 ) );

    alugrid_assert ( h1->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h1->myvertex( 2 ) == myvertex( 2 ) );
    alugrid_assert ( h1->myvertex( 3 ) == myvertex( 3 ) );

    // this is always the edge combo, i.e. if we
    // split e30 then 3 is new in child 0 and 0 is new in child 1
    alugrid_assert ( h0->myvertex( 1 ) == h1->myvertex( 0 ) );

    // remap vertices of children
    setNewMapping( h0, h1, newFace, 1, 0);

    // store refinement rule that was used to split this tetra
    _rule = myrule_t::e01;
    return ;
  }

  // --split_e12
  template< class A >  void TetraTop < A >::split_e12 ()
  {
    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + this->level();

    splitInfo( myrule_t::e12 );

    myhedge_t* subEdge = this->subedge (3, 0);
    myhedge_t* subEdge2 = this->subedge (0, 0);
    myhedge_t* orgEdge = this->myhedge( 2 ) ;

    const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 1 : 0;

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       subEdge,  1, // from face 1 get subedge 0
                       subEdge2, 0, // from face 2 get subedge 0
                       orgEdge, edgeTwst
                      ) ;
    alugrid_assert ( newFace );

    facepair_t subFace0 = subFaces( 0, myvertex( 1 ), myvertex( 2 ) ); // get sub face 0 and 1 of face 0
    facepair_t subFace3 = subFaces( 3, myvertex( 1 ), myvertex( 2 ) ); // get sub face 0 and 1 of face 3

    // we divide by 2 means we divide the volume by 2
    const double childVolume = calculateChildVolume( 0.5 * _volume );

    /*

      3               2
         ___________
        |3         2|     new inner face ( 0, 4 , 3 )
        | \ *     . |
        |  \   *  . |     child 0 is the child which contains node 0
        |   \   .* 1|     child 1 is the child which contains node 1
        |    \ .    * 4
        |     \   *2|
        |    . \*   |     4 becomes node 2 in child 0
        |   . * \   |     4 becomes node 1 in child 1
        |  .*    \  |
        | *       \ |
        *0         1|
        -------------
      0               1

    */

    innertetra_t * h0 = new innertetra_t (newLevel,
                                          subFace0.first, twist( 0 ),
                                          newFace, 0,
                                          myhface( 2 ),  twist( 2 ),
                                          subFace3.first, twist( 3 ),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          subFace0.second, twist( 0 ),
                                          myhface( 1 ), twist( 1 ),
                                          newFace, -1,
                                          subFace3.second, twist( 3 ),
                                          this, 1, childVolume) ;

    alugrid_assert (h0 && h1) ;

    // the new vertices are the ones that are missing
    // i.e. 3 in child 0  and  0 in child 1
    alugrid_assert ( h0->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h0->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h0->myvertex( 3 ) == myvertex( 3 ) );

    alugrid_assert ( h1->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h1->myvertex( 2 ) == myvertex( 2 ) );
    alugrid_assert ( h1->myvertex( 3 ) == myvertex( 3 ) );

    // this is always the edge combo, i.e. if we
    // split e30 then 3 is new in child 0 and 0 is new in child 1
    alugrid_assert ( h0->myvertex( 2 ) == h1->myvertex( 1 ) );

    // remap vertices of children
    setNewMapping( h0, h1, newFace, 1, 2 );

    // set refinement rule that was used to refine this tetra
    _rule = myrule_t::e12 ;
  }

  template< class A >  void TetraTop < A >::split_e20 ()
  {
    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + this->level();

    splitInfo( myrule_t::e20 );

    myhedge_t* subEdge2 = this->subedge (1, 0);
    myhedge_t* subEdge = this->subedge (3, 0);
    myhedge_t* orgEdge = this->myhedge( 4 ) ;

    const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 0 : 1;

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       orgEdge, edgeTwst,
                       subEdge2, 1, // from face 2 get subedge 0
                       subEdge,  0 // from face 1 get subedge 0
                      ) ;
    alugrid_assert ( newFace );

    facepair_t subFace1 = subFaces( 1, myvertex( 0 ), myvertex( 2 ) ); // get sub face 0 and 1 of face 0
    facepair_t subFace3 = subFaces( 3, myvertex( 0 ), myvertex( 2 ) ); // get sub face 0 and 1 of face 1

    // we divide by 2 means we divide the volume by 2
    const double childVolume = calculateChildVolume( 0.5 * _volume );

    /*

      3       4       2
         ___________
        |3         2|     new inner face ( 1, 4 , 2 )
        |*\       . |
        |  \     .  |     child 0 is the child which contains node 0
        | * \   .   |     child 1 is the child which contains node 2
        |    \ .    |
        |  *  \     |
        |   0. \    |     4 becomes node 2 in child 0
        |  4*   \   |     4 becomes node 0 in child 1
        |  .2 *  \  |
        | .     * \ |
        |0         1|
        -------------
      0               1

    */

    innertetra_t * h0 = new innertetra_t (newLevel,
                                          newFace, 0,
                                          subFace1.first, twist( 1 ),
                                          myhface( 2 ),  twist( 2 ),
                                          subFace3.first, twist( 3 ),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          myhface( 0 ), twist( 0 ),
                                          subFace1.second, twist( 1 ),
                                          newFace, -2,
                                          subFace3.second, twist( 3 ),
                                          this, 1, childVolume) ;

    alugrid_assert (h0 && h1) ;

    // the new vertices are the ones that are missing
    // i.e. 3 in child 0  and  0 in child 1
    alugrid_assert ( h0->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h0->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h0->myvertex( 3 ) == myvertex( 3 ) );

    alugrid_assert ( h1->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h1->myvertex( 2 ) == myvertex( 2 ) );
    alugrid_assert ( h1->myvertex( 3 ) == myvertex( 3 ) );

    // this is always the edge combo, i.e. if we
    // split e30 then 3 is new in child 0 and 0 is new in child 1
    alugrid_assert ( h0->myvertex( 2 ) == h1->myvertex( 0 ) );

    // remap vertices of children
    setNewMapping( h0, h1, newFace, 2, 0 );

    // set refinement rule that was used to refine this tetra
    _rule = myrule_t::e20;
  }

  template< class A >  void TetraTop < A >::split_e23 ()
  {
    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + this->level();

    splitInfo( myrule_t::e23 );

    myhedge_t* subEdge2 = this->subedge (1, 0);
    myhedge_t* subEdge = this->subedge (0, 0);
    myhedge_t* orgEdge = this->myhedge( 0 ) ;

    const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 0 : 1;

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       subEdge2, 1, // from face 2 get subedge 0
                       subEdge,  0, // from face 1 get subedge 0
                       orgEdge, edgeTwst
                      ) ;
    alugrid_assert ( newFace );

    facepair_t subFace0 = subFaces( 0, myvertex( 2 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 0
    facepair_t subFace1 = subFaces( 1, myvertex( 2 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 1

    // we divide by 2 means we divide the volume by 2
    const double childVolume = calculateChildVolume( 0.5 * _volume );

    /*

      3       4       2
         ___________
        |3   2*3   2|     new inner face ( 1, 4 , 2 )
        | \       . |
        |  \ * * .  |     child 0 is the child which contains node 2
        |   \   .   |     child 1 is the child which contains node 3
        |   *\ .*   |
        |     \     |
        |  * . \ *  |     4 becomes node 3 in child 0
        |3  .   \   |     4 becomes node 0 in child 1
        | *.     \* |
        | .       \ |
        |0         1|
        -------------
      0               1

    */

    innertetra_t * h0 = new innertetra_t (newLevel,
                                          subFace0.first, twist( 0 ),
                                          subFace1.first, twist( 1 ),
                                          newFace, 0,
                                          myhface( 3 ),  twist( 3 ),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          subFace0.second, twist( 0 ),
                                          subFace1.second, twist( 1 ),
                                          myhface( 2 ),  twist( 2 ),
                                          newFace, -1,
                                          this, 1, childVolume) ;

    alugrid_assert (h0 && h1) ;

    // the new vertices are the ones that are missing
    // i.e. 3 in child 0  and  0 in child 1
    alugrid_assert ( h1->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h1->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h1->myvertex( 3 ) == myvertex( 3 ) );

    alugrid_assert ( h0->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h0->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h0->myvertex( 2 ) == myvertex( 2 ) );

    // this is always the edge combo, i.e. if we
    alugrid_assert ( h0->myvertex( 3 ) == h1->myvertex( 2 ) );

    // remap vertices of children
    setNewMapping( h0, h1, newFace, 2, 3 );

    // set refinement rule that was used to refine this tetra
    _rule = myrule_t::e23 ;
  }

  template< class A >  void TetraTop < A >::split_e30 ()
  {
    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + this->level();

    splitInfo( myrule_t::e30 );
    /*

      3               2
         ___________
        |3        *2|     new inner face ( 1, 4 , 2 )
        | \     * . |
        |  \  *  .  |     child 0 is the child which contains node 0
        |   \  .    |     child 1 is the child which contains node 3
        |0*  \.     |
      4 *    .\     |
        | *  . \    |     4 becomes node 3 in child 0
        |3 .*   \   |     4 becomes node 0 in child 1
        | .   *  \  |
        |.      * \ |
        |0         1|
        -------------
      0               1

    */

    myhedge_t* subEdge2 = this->subedge (2, 0);
    myhedge_t* subEdge = this->subedge (1, 0);
    myhedge_t* orgEdge = this->myhedge( 3 ) ;

    const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 0 : 1;

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       subEdge2, 1, // from face 2 get subedge 0
                       subEdge,  0, // from face 1 get subedge 0
                       orgEdge, edgeTwst
                      ) ;

    alugrid_assert ( newFace ) ;

    facepair_t subFace1 = subFaces( 1, myvertex( 0 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 1
    facepair_t subFace2 = subFaces( 2, myvertex( 0 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 2

    // we divide by 2 means we divide the volume by 2
    const double childVolume = calculateChildVolume( 0.5 * _volume );

    innertetra_t * h0 = new innertetra_t (newLevel,
                                          newFace, 0,
                                          subFace1.first, twist (1),
                                          subFace2.first, twist (2),
                                          myhface(3), twist (3),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          myhface( 0 ), twist( 0 ),
                                          subFace1.second, twist (1),
                                          subFace2.second, twist (2),
                                          newFace, -3,
                                          this, 1, childVolume) ;

    alugrid_assert (h0 && h1) ;

    // the new vertices are the ones that are missing
    // i.e. 3 in child 0  and  0 in child 1
    alugrid_assert ( h0->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h0->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h0->myvertex( 2 ) == myvertex( 2 ) );

    alugrid_assert ( h1->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h1->myvertex( 2 ) == myvertex( 2 ) );
    alugrid_assert ( h1->myvertex( 3 ) == myvertex( 3 ) );

    // this is always the edge combo, i.e. if we
    // split e30 then 3 is new in child 0 and 0 is new in child 1
    alugrid_assert ( h0->myvertex( 3 ) == h1->myvertex( 0 ) );

    // remap vertices of children
    setNewMapping( h0, h1, newFace, 3, 0 );

    // set refinement rule that was used to refine this tetra
    _rule = myrule_t::e30;
  }

  template< class A >  void TetraTop < A >::split_e31 ()
  {
    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + this->level () ;

    splitInfo( myrule_t::e31 );

    myhedge_t* subEdge2 = this->subedge (2, 0);
    myhedge_t* subEdge = this->subedge (0, 0);
    myhedge_t* orgEdge = this->myhedge( 1 ) ;

    const int edgeTwst = (orgEdge->myvertex( 0 ) == subEdge->myvertex( 1 )) ? 1 : 0;

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       orgEdge, edgeTwst,
                       subEdge, 1,  // from face 1 get subedge 0
                       subEdge2, 0  // from face 2 get subedge 0
                      ) ;

    alugrid_assert ( newFace ) ;

    /*

      3               2
         ___________
        |3         2|     new inner face ( 0, 4, 2 )
        | \       .*|
        |  \     .  |     child 0 is the child which contains node 1
        |   \   . * |     child 1 is the child which contains node 3
        |    \ .    |
        |     \  *  |
        |    . \    |      4 becomes node 3 in child 0
        |   .  1*  <--- 4  4 becomes node 1 in child 1
        |  .  * 3\  |
        | . *     \ |
        |0*       1\|
        -------------
      0               1

    */
    // we divide by 2 means we divide the volume by 2
    const double childVolume = calculateChildVolume( 0.5 * _volume );

    facepair_t subFace0 = subFaces( 0, myvertex( 1 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 0
    facepair_t subFace2 = subFaces( 2, myvertex( 1 ), myvertex( 3 ) ); // get sub face 0 and 1 of face 1


    innertetra_t * h0 = new innertetra_t (newLevel,
                                          subFace0.first, twist( 0 ),
                                          newFace, 0,
                                          subFace2.first, twist ( 2 ),
                                          myhface( 3 ), twist ( 3 ),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          subFace0.second, twist( 0 ),
                                          myhface( 1 ), twist( 1 ),
                                          subFace2.second, twist( 2 ),
                                          newFace, -1,
                                          this, 1, childVolume) ;

    alugrid_assert (h0 && h1) ;

    // the new vertices are the ones that are missing
    // i.e. 3 in child 0  and  0 in child 1
    alugrid_assert ( h0->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h0->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h0->myvertex( 2 ) == myvertex( 2 ) );

    alugrid_assert ( h1->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h1->myvertex( 2 ) == myvertex( 2 ) );
    alugrid_assert ( h1->myvertex( 3 ) == myvertex( 3 ) );

    // this is always the edge combo, i.e. if we
    // split e30 then 3 is new in child 0 and 0 is new in child 1
    alugrid_assert ( h0->myvertex( 3 ) == h1->myvertex( 1 ) );

    // remap vertices of children
    setNewMapping( h0, h1, newFace, 3, 1 );

    // set refinement rule that was used to refine this tetra
    _rule = myrule_t::e31;
  }

  // --setNewMapping
  template< class A >  void
  TetraTop < A >::setNewMapping( innertetra_t* h0, innertetra_t* h1,
                                 innerface_t* newFace,
                                 const int newVx0, const int newVx1 )
  {
    // vertex 0 is always containd in child 0, and not in child 1
    myvertex_t* vx0 = this->myvertex( _vxMap[ 0 ] );
    //for 2dbisection vx0 stays vx0 (and thus 0) in all children
    // we use the ALBERTA 2d algorithm with the map ALU (or vxMap) 1,2,3 to ALBERTA 0,1,2
    if(this->is2d())
    {
      // vertex 1 is always containd in child 0, and not in child 1
      vx0 = this->myvertex( _vxMap[ 1 ] );
    }

    bool found = false ;
    for( int i=0; i<4; ++i )
    {
      if( h0->myvertex( i ) == vx0 )
      {
        found = true ;
        break ;
      }
    }

    // if vx0 was not found in child 0 we have to swap the children
    innertetra_t* t0 = ( found ) ? h0 : h1;
    innertetra_t* t1 = ( found ) ? h1 : h0;

    // the vxMap for the 2d refinement
    if ( this->is2d() )
    {
      ///////////////////////////////////////////////////
      //  Bisection 2d refinement, always refine edge 1--2
      ///////////////////////////////////////////////////

      // set vertex mapping (child 0)
      t0->_vxMap[ 0 ] = _vxMap[ 0 ];
      t0->_vxMap[ 1 ] = _vxMap[ 3 ];
      t0->_vxMap[ 2 ] = _vxMap[ 1 ];
      t0->_vxMap[ 3 ] = _vxMap[ 2 ];

      // set vertex mapping (child 1)
      t1->_vxMap[ 0 ] = _vxMap[ 0 ];
      t1->_vxMap[ 1 ] = _vxMap[ 2 ];
      t1->_vxMap[ 2 ] = _vxMap[ 3 ];
      t1->_vxMap[ 3 ] = _vxMap[ 1 ];
    }
    else if( stevensonRefinement_ )
    {
      ///////////////////////////////////////////////////
      //  Stevenson refinement, always refine edge 0--3
      ///////////////////////////////////////////////////

      // set vertex mapping (child 0)
      t0->_vxMap[ 0 ] = _vxMap[ 0 ];
      t0->_vxMap[ 1 ] = _vxMap[ 3 ];
      t0->_vxMap[ 2 ] = _vxMap[ 1 ];
      t0->_vxMap[ 3 ] = _vxMap[ 2 ];

      // set vertex mapping (child 1)
      t1->_vxMap[ 0 ] = _vxMap[ 3 ];
      t1->_vxMap[ 1 ] = _vxMap[ 0 ];
      const char fce3 = ( elementType () == 0 ) ? 1 : 0;
      t1->_vxMap[ 2 ] = _vxMap[ 1 + fce3 ]; // for type 0   2 else 1
      t1->_vxMap[ 3 ] = _vxMap[ 2 - fce3 ]; // for type 0   1 else 2
    }
    else
    {
      ///////////////////////////////////////////////////
      //  ALBERTA refinement, always refine edge 0--1
      ///////////////////////////////////////////////////

      // set vertex mapping (child 0)
      t0->_vxMap[ 0 ] = _vxMap[ 0 ];
      t0->_vxMap[ 1 ] = _vxMap[ 2 ];
      t0->_vxMap[ 2 ] = _vxMap[ 3 ];
      t0->_vxMap[ 3 ] = _vxMap[ 1 ];

      // set vertex mapping (child 1)
      t1->_vxMap[ 0 ] = _vxMap[ 1 ];
      t1->_vxMap[ 3 ] = _vxMap[ 0 ];
      const char fce3 = ( elementType () == 0 ) ? 1 : 0;
      t1->_vxMap[ 1 ] = _vxMap[ 2 + fce3 ]; // for type 0   2 else 1
      t1->_vxMap[ 2 ] = _vxMap[ 3 - fce3 ]; // for type 0   1 else 2
    }

#ifdef ALUGRIDDEBUG
    /*
    std::cout << "Map0 = ( " ;
    for( int i=0; i<4 ; ++ i )
    {
      std::cout << int(h0->_vxMap[i]) << " " ;
    }
    std::cout << " ) " << std::endl;
    std::cout << "Map1 = ( " ;
    for( int i=0; i<4 ; ++ i )
    {
      std::cout << int(h1->_vxMap[i]) << " " ;
    }
    std::cout << " ) " << std::endl;
    */

    for(int i=0; i<4; ++i )
    {
      for(int j=i+1; j<4; ++ j)
      {
        if( i != j )
        {
          alugrid_assert ( h0->_vxMap[ i ] != h0->_vxMap[ j ] );
          alugrid_assert ( h1->_vxMap[ i ] != h1->_vxMap[ j ] );
        }
      }
    }
#endif
    // append h1 to h0
    h0->append( h1 );

    // create inner storage with first child and new face
    _inner = new inner_t( h0, newFace );
    alugrid_assert ( _inner );

    // detach myself from being leaf element
    this->detachleafs();
  }


  template< class A >  int
  TetraTop < A >::vertexTwist( const int twst, const int vx ) const
  {
    return twst < 0 ? (7-vx+twst)%3 : (vx+twst)%3;
  }

  template< class A > int
  TetraTop < A >::calculateFace2Twist( const int vxIndex, const myhface_t* subFace ) const
  {
    const int faceIndices[ 3 ] = { subFace->myvertex( 0 )->getIndex(),
                                   subFace->myvertex( 1 )->getIndex(),
                                   subFace->myvertex( 2 )->getIndex() };

    for(int twst = -3; twst<3; ++twst )
    {
      // search for vx 1
      if( vxIndex == faceIndices[ vertexTwist(twst, 1) ] )
      {
        return twst;
      }
    }

    std::cout << "Valid twist not found!!!" << std::endl;
    return 0;
    // we should not get here
    alugrid_assert ( false );
    abort();
    return -5;
  }

  template< class A > int
  TetraTop < A >::calculateFace3Twist( const int (&vx)[2], const myhface_t* subFace, const int secondVx ) const
  {
    //std::cout << "check v0 = " << vx[0] << " v1 = " << vx[1] << std::endl;

    const int faceIndices[ 3 ] = { subFace->myvertex( 0 )->getIndex(),
                                   subFace->myvertex( 1 )->getIndex(),
                                   subFace->myvertex( 2 )->getIndex() };
    //std::cout << faceIndices[0] << " " << faceIndices[1] << " " << faceIndices[2] << " " << std::endl;

    for(int twst = -3; twst<3; ++twst )
    {
      // if vx0 and vx1 match we are done
      if( vx[ 0 ] == faceIndices[ vertexTwist(twst, 0) ] &&
          vx[ 1 ] == faceIndices[ vertexTwist(twst, secondVx ) ] )
      {
        return twst;
      }
    }

    std::cout << "Valid twist not found!!!" << std::endl;
    return 0;

    // we should not get here
    alugrid_assert ( false );
    abort();
    return -5;
  }

  // --checkTetra
  template< class A > bool
  TetraTop < A >::checkTetra( const innertetra_t *tetra, const int nChild ) const
  {
    // make sure face twists are ok
    bool twistOk = true ;

    std::set< int > verticesFound;
    alugrid_assert ( tetra->nChild() == nChild );

    const bool isGhost = tetra->isGhost();
    for(int fce=0; fce<4; ++fce )
    {
      for(int i=0; i<3; ++i )
      {
        verticesFound.insert( tetra->myvertex( fce, i )->getIndex() );
      }

      for(int i=0; i<3; ++i )
      {
        verticesFound.insert( tetra->myvertex( fce, i )->getIndex() );
        // use proto type to check face twists
        if( tetra->myvertex( Gitter::Geometric::Tetra::prototype[ fce ][ i ] )
              != tetra->myvertex( fce, i ) )
        {
          const int vx0 = Gitter::Geometric::Tetra::prototype[ fce ][ 0 ] ;
          const int vx1 = Gitter::Geometric::Tetra::prototype[ fce ][ 1 ] ;

          const int vx[2] = { tetra->myvertex( vx0 )->getIndex(),
                              tetra->myvertex( vx1 )->getIndex()
                            };

          int twst = calculateFace3Twist( vx, tetra->myhface( fce ), 1 );
          std::cout << "Twist of face " << fce << " is wrong, it should be " << twst << std::endl;
          twistOk = false ;
          continue ;
        }
      }


      if( ! isGhost && ! tetra->myneighbour( fce ).first->isRealObject()  )
      {
        std::cout << "Neighbour(type="<<tetra->isInterior() << ") " << fce << " of Tetra " << tetra->getIndex()  << " is wrong " << std::endl;
        std::cout << "Check face " << tetra->myhface( fce )->getIndex() << std::endl;
      }
      // make sure neighbor is something meaningful
      //alugrid_assert ( tetra->myneighbour( fce ).first->isRealObject() );
    }

    // make sure we have only 4 different vertices
    alugrid_assert ( verticesFound.size() == 4 );

    return twistOk;
  }

  template< class A >  void TetraTop < A >::
  splitISO8 ()
  {
    alugrid_assert ( _inner == 0 );
    typedef typename A::myvertex_t  myvertex_t;
    typedef typename A::inneredge_t inneredge_t;
    const int l = 1 + this->level () ;

    myvertex_t * e31 = myhface (0)->myhedge ((twist(0) < 0) ? ((9+twist(0))%3) : (twist(0)%3))->subvertex (0) ;
    myvertex_t * e20 = myhface (1)->myhedge ((twist(1) < 0) ? ((9+twist(1))%3) : (twist(1)%3))->subvertex (0) ;
    alugrid_assert (e31 && e20);
    inneredge_t * e0 = new inneredge_t (l, e31, e20) ;
    alugrid_assert (e0) ;
    innerface_t * f0 = new innerface_t (l, subedge (3, 2), ((twist(3)>=0)?1:0), subedge (1, 2), ((twist(1)>=0)?1:0), subedge (2, 2), ((twist(2)>=0)?1:0)) ;
    innerface_t * f1 = new innerface_t (l, subedge (3, 0), ((twist(3)>=0)?1:0), subedge (2, 1), ((twist(2)>=0)?1:0), subedge (0, 2), ((twist(0)>=0)?1:0)) ;
    innerface_t * f2 = new innerface_t (l, subedge (3, 1), ((twist(3)>=0)?1:0), subedge (0, 1), ((twist(0)>=0)?1:0), subedge (1, 0), ((twist(1)>=0)?1:0)) ;
    innerface_t * f3 = new innerface_t (l, subedge (2, 0), ((twist(2)>=0)?0:1), subedge (0, 0), ((twist(0)>=0)?0:1), subedge (1, 1), ((twist(1)>=0)?0:1)) ;
    innerface_t * f4 = new innerface_t (l, e0, 0, subedge (3, 2), ((twist(3)>=0)?0:1), subedge (2, 1), ((twist(2)>=0)?1:0)) ;
    innerface_t * f5 = new innerface_t (l, e0, 0, subedge (3, 1), ((twist(3)>=0)?1:0), subedge (0, 2), ((twist(0)>=0)?0:1)) ;
    innerface_t * f6 = new innerface_t (l, e0, 0, subedge (1, 0), ((twist(1)>=0)?0:1), subedge (0, 0), ((twist(0)>=0)?1:0)) ;
    innerface_t * f7 = new innerface_t (l, e0, 0, subedge (1, 2), ((twist(1)>=0)?1:0), subedge (2, 0), ((twist(2)>=0)?0:1)) ;
    alugrid_assert (f0 && f1 && f2 && f3 && f4 && f5 && f6 && f7) ;
    f0->append(f1) ;
    f1->append(f2) ;
    f2->append(f3) ;
    f3->append(f4) ;
    f4->append(f5) ;
    f5->append(f6) ;
    f6->append(f7) ;

    // we divide by 8 means we divide the volume by 8
    const double childVolume = calculateChildVolume( 0.125 * _volume );

    // pointer `this' is the pointer to the father element
    innertetra_t * h0 = new innertetra_t (l, f0, -1, subface(1, 0), twist(1), subface(2, 0), twist(2), subface(3, 0), twist(3), this, 0 , childVolume) ;
    innertetra_t * h1 = new innertetra_t (l, subface(0, 0), twist(0), f1, -3, subface(2, 2), twist(2), subface(3, 1), twist(3), this, 1 , childVolume) ;
    innertetra_t * h2 = new innertetra_t (l, subface(0, 2), twist(0), subface(1, 1), twist(1), f2, -1, subface(3, 2), twist(3), this, 2 , childVolume) ;
    innertetra_t * h3 = new innertetra_t (l, subface(0, 1), twist(0), subface(1, 2), twist(1), subface(2, 1), twist(2), f3, 0,  this, 3 , childVolume) ;
    innertetra_t * h4 = new innertetra_t (l, f7, -3, subface(2, 3), ((twist(2)>=0) ? ((twist(2)+2)%3) : twist(2)) , f4, 2, f0, 0, this, 4 , childVolume) ;
    innertetra_t * h5 = new innertetra_t (l, f4, -3, f1, 0, f5, 2, subface(3, 3), ((twist(3)>=0) ? (twist(3)+1)%3 : (twist(3)-1)%3-1), this, 5 , childVolume) ;
    innertetra_t * h6 = new innertetra_t (l, f3, -1, f6, -3, subface(1, 3), ((twist(1)>=0) ? twist(1) : twist(1)%3-1), f7, 1, this, 6 , childVolume) ;
    innertetra_t * h7 = new innertetra_t (l, subface(0, 3), ((twist(0)>=0) ? (twist(0)+1)%3 : (twist(0)-1)%3-1), f5, -3, f2, 0, f6, 1, this, 7 , childVolume) ;
    alugrid_assert (h0 && h1 && h2 && h3 && h4 && h5 && h6 && h7) ;
    h0->append(h1) ;
    h1->append(h2) ;
    h2->append(h3) ;
    h3->append(h4) ;
    h4->append(h5) ;
    h5->append(h6) ;
    h6->append(h7) ;
    _inner = new inner_t( h0, f0, e0 );
    alugrid_assert ( _inner );
    _rule = myrule_t::regular ;

    this->detachleafs();
    return ;
  }

  /* This method is used to split a tetraeder into four smaller tetrahedrons
  *
  * The local vertex 0 is kept vertex 0 on all children and the face 0 opposite
  * of to vertex 0 is divided according to the iso4 rule.
  *
  * We get 3 inner Faces, no inner Edges and no inner vertices.
  *
  * As before the faces and edges already have been refined.
  */
  template< class A >  void TetraTop < A >::
  splitIso4_2d ()
  {
    alugrid_assert ( _inner == 0 );
    const int l = 1 + this->level () ;


#ifdef ALUGRIDDEBUG
   // for(int i = 0; i < 3; ++ i)
  //    std::cout << "( 0, "<< i << ") " << subedge(0,i);
  //  for (int i = 1 ; i <4 ; ++ i)
  //   std::cout << "( " << i << ", 0) " << subedge(i,0);
#endif

    innerface_t * f0 = new innerface_t (l, subedge (2, 0), 1, subedge (0, 2), ((twist(0)>=0)?1:0), subedge (3, 0), 0) ;
    innerface_t * f1 = new innerface_t (l, subedge (1, 0), 1, subedge (0, 0), ((twist(0)>=0)?1:0), subedge (2, 0), 0) ;
    innerface_t * f2 = new innerface_t (l, subedge (3, 0), 1, subedge (0, 1), ((twist(0)>=0)?1:0), subedge (1, 0), 0) ;
  //  std::cout << "f1: " << f1 ;
  //  std::cout << "f0: "<< f0 ;
   // std::cout << "f2: " <<f2 ;

    alugrid_assert (f0 && f1 && f2 ) ;
    f0->append(f1) ;
    f1->append(f2) ;

    // we divide by 4 means we divide the volume by 4
    const double childVolume = calculateChildVolume( 0.25 * _volume );

#ifdef ALUGRIDDEBUG
  //  for(int  i = 0; i<4 ; ++i )
   //   std::cout << "( 0, " << i <<  ")" << subface (0,i);
    //for(int i = 1; i <4 ; ++i)
    //  std::cout << "(" << i <<  ", 0)" << subface (i,0 ) << "( " << i <<  ", 1)" << subface (i,1);
#endif

    // the twist < 0 for subface 3 of face 0 is needed
    // because it is "upside down" in the old face and cannot just be oriented as before

    // pointer `this' is the pointer to the father element
    innertetra_t * h0 = new innertetra_t (l, subface(0, 0), twist(0), f0, -1, subface(2, 1), twist(2), subface(3, 0), twist(3), this, 0 , childVolume) ;
    innertetra_t * h1 = new innertetra_t (l, subface(0, 1), twist(0), subface(1, 1), twist(1), subface(2, 0), twist(2), f1, -1, this, 1 , childVolume) ;
    innertetra_t * h2 = new innertetra_t (l, subface(0, 2), twist(0), subface(1, 0), twist(1), f2, -1, subface(3, 1), twist(3), this, 2 , childVolume) ;
    innertetra_t * h3 = new innertetra_t (l, subface(0, 3), twist(0) < 0 ? twist(0)%3-1 : twist(0), f2, 0, f1, 0, f0, 0, this, 3 , childVolume) ;

#ifdef ALUGRIDDEBUG
    //this check produces output, when loadbalancing and refining the transported elements, because the ghost neighbours do not exist yet
   // std::cout << h0 ;
    //checkTetra(h0,0);
   // std::cout << h1 ;
    //checkTetra(h1,1);
   // std::cout << h2;
    //checkTetra(h2,2);
   // std::cout << h3;
    //alugrid_assert(checkTetra(h3,3));
#endif


    alugrid_assert (h0 && h1 && h2 && h3 ) ;
    h0->append(h1) ;
    h1->append(h2) ;
    h2->append(h3) ;

    _inner = new inner_t( h0, f0 );
    alugrid_assert ( _inner );
    _rule = myrule_t::regular ;

    this->detachleafs();
    return ;
  }

  template< class A > TetraTop < A >::
  BisectionInfo::BisectionInfo ( myrule_t r ) : _caller( 0 )
  {
    switch(r)
    {
      case myrule_t::e01 :
        _faces[ 0 ] = 2 ;
        _faces[ 1 ] = 3 ;
        _vertices[ 0 ] = 0 ;
        _vertices[ 1 ] = 1 ;
        _faceRules[ 0 ] = face3rule_t::e20;
        _faceRules[ 1 ] = face3rule_t::e01;
        _caller = new CallSplitImpl< myrule_t::e01 > ();
        break ;
      case myrule_t::e12 :
        _faces[ 0 ] = 0 ;
        _faces[ 1 ] = 3 ;
        _vertices[ 0 ] = 1 ;
        _vertices[ 1 ] = 2 ;
        _faceRules[ 0 ] = face3rule_t::e20;
        _faceRules[ 1 ] = face3rule_t::e12;
        _caller = new CallSplitImpl< myrule_t::e12 > ();
        break ;
      case myrule_t::e20 :
        _faces[ 0 ] = 1 ;
        _faces[ 1 ] = 3 ;
        _vertices[ 0 ] = 2 ;
        _vertices[ 1 ] = 0 ;
        _faceRules[ 0 ] = face3rule_t::e01;
        _faceRules[ 1 ] = face3rule_t::e20;
        _caller = new CallSplitImpl< myrule_t::e20 > ();
        break ;
      case myrule_t::e23 :
        _faces[ 0 ] = 0 ;
        _faces[ 1 ] = 1 ;
        _vertices[ 0 ] = 2 ;
        _vertices[ 1 ] = 3 ;
        _faceRules[ 0 ] = face3rule_t::e12;
        _faceRules[ 1 ] = face3rule_t::e12;
        _caller = new CallSplitImpl< myrule_t::e23 > ();
        break ;
      case myrule_t::e30 :
        _faces[ 0 ] = 1 ;
        _faces[ 1 ] = 2 ;
        _vertices[ 0 ] = 3 ;
        _vertices[ 1 ] = 0 ;
        _faceRules[ 0 ] = face3rule_t::e20;
        _faceRules[ 1 ] = face3rule_t::e01;
        _caller = new CallSplitImpl< myrule_t::e30 > ();
        break ;
      case myrule_t::e31 :
        _faces[ 0 ] = 0 ;
        _faces[ 1 ] = 2 ;
        _vertices[ 0 ] = 3 ;
        _vertices[ 1 ] = 1 ;
        _faceRules[ 0 ] = face3rule_t::e01;
        _faceRules[ 1 ] = face3rule_t::e12;
        _caller = new CallSplitImpl< myrule_t::e31 > ();
        break ;
      default :
        std::cerr << "**FEHLER (FATAL) beim unbedingten Verfeinern mit unbekannter Regel: " ;
        std::cerr << "[" << r << "]. In " << __FILE__ << __LINE__ << std::endl ;
        abort () ;
        break ;
    }
  }

  // --refineImmediate
  template< class A >  void TetraTop < A >::refineImmediate (myrule_t r)
  {
    alugrid_assert (getrule () == myrule_t::nosplit) ;
    typedef typename myhface_t::myrule_t face3rule_t;

    if( r == myrule_t::regular )
    {
      // Das refineImmediate (..) auf allen Fl"achen wird vom tetra::refine (..)
      // zwar nicht ben"otigt, da schliesslich alle Fl"achen sauber sind, wenn
      // "uberall hface3::refine (..) true geliefert hat, wohl aber z.B. von
      // restore () oder abgeleiteten Funktionen die eine direkte Verfeinerung
      // erzwingen m"ussen und d"urfen.

      //we refine everything with iso4, but first unset is2d on the face 0, so it gets treated as a 3d face
      //we set the is2d() flag correct in the constructor

      {
        for (int i = 0 ; i < 4 ; ++i)
          myhface (i)->refineImmediate (face3rule_t (myhface_t::myrule_t::iso4).rotate (twist (i))) ;
      }

      //in the 2d case we split the tetra differently
      if( this->is2d() )
        splitIso4_2d () ;
      else // 3d case
        splitISO8 () ;
    }
    else if( r == myrule_t::bisect )
    {
      // call refinement with appropriate rule
      // given by suggestRule
      BisectionInfo::splitEdge( this, suggestRule() );
    }
    else
    {
      // it is assured that r is one out of e01 ... e32
      // call refinement directly
      BisectionInfo::splitEdge( this, r );
    }

    // call post refinement procedure
    this->postRefinement () ;
    return ;
  }


  // --refine
  template< class A >  bool TetraTop < A >::refine ()
  {
    myrule_t r = _req ;
    if (r != myrule_t::crs && r != myrule_t::nosplit)
    {
      if (r != getrule ())
      {
        alugrid_assert (getrule () == myrule_t::nosplit) ;
        _req = myrule_t::nosplit ;
        switch (r)
        {
          case myrule_t::crs :
          case myrule_t::nosplit :
            return true ;
          case myrule_t::regular :
            {
              //we refine everything with rule iso4 but in 2d face 0 gets treated as a 3d face and therefore will be refined differently
              //we set the is2d flag correct in the constructor

              for (int i = 0 ; i < 4 ; ++i )
                //rotate should do nothing on iso4
                if (! myhface (i)->refine (face3rule_t (face3rule_t::iso4).rotate (twist (i)), twist (i))) return false ;
            }
            break ;
          case myrule_t::e01 :
          case myrule_t::e12 :
          case myrule_t::e20 :
          case myrule_t::e23 :
          case myrule_t::e30 :
          case myrule_t::e31 :
            if( ! BisectionInfo::refineFaces( this, r ) ) return false ;
            break ;
          default :
            std::cerr << "**WARNING (ERROR IGNORED) wrong refinement rule [" << int(r) ;
            std::cerr << "] (ignored) in " << __FILE__ << " " << __LINE__ << std::endl ;
            alugrid_assert ( false );
            return false ;
        }

        // Vorsicht: Im Fall eines konformen Verfeinerers mu"s hier die entstandene Verfeinerung
        // untersucht werden und dann erst das Element danach verfeinert werden.
        //
        // getrule can have changed because of reucrsive nature of bisection refinement
        // if there is a loop in the mesh
        if(r != getrule ())
        {
          alugrid_assert( getrule() == myrule_t::nosplit);
          refineImmediate (r) ;
        }
        _req = myrule_t::nosplit ;
        return true ;
      }
      _req = myrule_t::nosplit ;
    }
    return true ;
  }

  template< class A >  bool TetraTop < A >::refineBalance (balrule_t r, int fce)
  {
    // if status is still non-refined
    if (getrule () == myrule_t::nosplit)
    {
      if( r == balrule_t::iso4 )
      {
        // if face is a leaf face
        if (! myhface (fce)->leaf ())
        {
          for (int i = 0 ; i < 4 ; ++i)
          {
            if (i != fce)
              if ( ! myhface (i)->refine (balrule_t ( balrule_t::iso4 ).rotate (twist (i)), twist (i)) )
                return false ;
          }
          _req = myrule_t::nosplit ;
          refineImmediate (myrule_t::regular) ;
        }
      }
      else
      {
        // if face is a leaf face
        if (! myhface (fce)->leaf ())
        {
          _req = suggestRule() ;

          //if (! BisectionInfo::refineFaces( this, suggestRule() ) ) return false ;
          refine();
        }
      }
    }
    return true ;
  }

  template< class A >  bool TetraTop < A >::coarse ()
  {
    if (this->leaf ())
    {
      if( _lvl == 0 )
      {
        alugrid_assert ( !up () );
        return false;
      }

      alugrid_assert (_req == myrule_t::nosplit || _req == myrule_t::crs) ;
      myrule_t w = _req ;
      _req = myrule_t::nosplit ;
      // end recursion if rule is not croarsen
      if (w != myrule_t::crs)
      {
        return false ;
      }

      // if I have faces that are not leaf, we cannot coarsen
      for (int i = 0 ; i < 4 ; ++i)
      {
        if ( ! myhface (i)->leaf () ) return false ;
      }
      // else coarsen
      return true ;
    }
    else
    {
      alugrid_assert (_req == myrule_t::nosplit) ;
      bool x = true ;
      {
        for (innertetra_t * h = dwnPtr() ; h ; h = h->next ()) x &= h->coarse () ;
      }

      // if x is true, then all children are marked for coarsening and have
      // not faces that are not leaf
      if (x)
      {
        // test marking on refinement edge
        alugrid_assert ( this->nEdges() == 6 );

        for (int e=0; e<6; ++e)
        {
          myhedge_t *edge = this->myhedge(e);
          myhedge_t *dwn = (myhedge_t * ) edge->down();
          if ( dwn )
          {
            if ( edge->noCoarsen() )
              return false;

            // we need to make sure that none of the children of this
            // refinement edge should not be removed
            // (we could not go up on edges during marking so we go down here)
            if ( ! dwn->canCoarsen() )
            {
              return false;
            }
          }
        }

        this->preCoarsening () ;
        this->attachleafs();

        delete _inner ;
        _inner = 0 ;

        // reset refinement rule
        _rule = myrule_t::nosplit ;
        {
          for (int i = 0 ; i < 4 ; ++i )
          {
            this->myneighbour (i).first->bndNotifyCoarsen () ;
            myhface (i)->coarse () ;
          }
        }
        return false ;
      }
    }
    return false ;
  }

  // buckupIndex of tetra
  template< class A > void TetraTop < A >::backupIndex ( std::ostream &os ) const
  {
    this->doBackupIndex( os );

    // write children
    {
      for (const innertetra_t * c = dwnPtr() ; c ; c = c->next ()) c->backupIndex (os) ;
    }
    return;
  }

  // buckupIndex of tetra
  template< class A > void TetraTop < A >::backupIndex (ObjectStream& os) const
  {
    this->doBackupIndex( os );

    // write children
    {
      for (const innertetra_t * c = dwnPtr() ; c ; c = c->next ()) c->backupIndex (os) ;
    }
    return;
  }

  // buckupTetra
  template< class A > int TetraTop< A >::backup ( std::ostream &os ) const
  {
    return doBackup( os );
  }

  template< class A > int TetraTop < A >::backup (ObjectStream& os) const
  {
    return doBackup( os );
  }

  template< class A > template<class OutStream_t>
  int TetraTop < A >::doBackup (OutStream_t & os) const
  {
    os.put ((char) getrule ()) ;
    for (const inneredge_t * e = innerHedge () ; e ; e = e->next ()) e->backup (os) ;
    for (const innerface_t * f = innerHface () ; f ; f = f->next ()) f->backup (os) ;
    int sons = 1 ;
    for (const innertetra_t * c = dwnPtr() ; c ; c = c->next () )
      sons += c->backup (os) ;
    return sons ;
  }

  // overloaded restoreIndex Method
  template< class A >
  template< class istream_t>
  void TetraTop < A >::
  restoreIndexImpl (istream_t & is, RestoreInfo& restoreInfo )
  {
    // mark this element a non hole
    typedef typename Gitter::Geometric::BuilderIF BuilderIF;

    // restore index
    this->doRestoreIndex( is, restoreInfo, BuilderIF::IM_Elements );

    // TODO
    // restore other indices

    {
      for (innertetra_t * c = dwnPtr() ; c ; c = c->next ()) c->restoreIndex (is, restoreInfo ) ;
    }
    return;
  }

  // overloaded restoreIndex Method
  template< class A >
  void TetraTop< A >::restoreIndex ( std::istream & is, RestoreInfo &restoreInfo )
  {
    restoreIndexImpl( is, restoreInfo );
  }

  // overloaded restoreIndex Method
  template< class A > void TetraTop < A >::
  restoreIndex (ObjectStream& is, RestoreInfo& restoreInfo )
  {
    restoreIndexImpl( is, restoreInfo );
  }

  // restoreTetra
  template< class A >
  void TetraTop < A >::restore ( std::istream &is ) { doRestore( is ); }

  template< class A > void TetraTop < A >::restore (ObjectStream & is)
  {
    doRestore( is );
  }

  template< class A > template<class InStream_t>
  void TetraTop < A >::doRestore (InStream_t & is)
  {
    // restore () stellt den Elementbaum aus der Verfeinerungs-
    // geschichte wieder her. Es ruft refine () auf und testet
    // auf den korrekten Vollzug der Verfeinerung. Danach werden
    // die inneren Gitterteile restore'd.

    myrule_t r ((char) is.get ()) ;
    if (r == myrule_t::nosplit)
    {
      // Vorsicht: beim restore m"ussen sich sowohl Element als auch
      // Randelement um die Korrektheit der Nachbarschaft k"ummern,
      // und zwar dann wenn sie "on the top" sind (= die gelesene
      // Verfeinerungsregel ist nosplit). (s.a. beim Randelement)
      // Die nachfolgende L"osung ist weit davon entfernt, sch"on
      // zu sein - leider. Eventuell wird mit der Verbesserung der
      // Behandlung der nichtkonf. Situationen mal eine "Anderung
      // n"otig.

      for (int i = 0 ; i < 4 ; ++i)
      {
        myhface_t & face (*(myhface (i))) ;
        if (! face.leaf ())
        {
          alugrid_assert( face.getrule() == balrule_t::e01 ||
                          face.getrule() == balrule_t::e12 ||
                          face.getrule() == balrule_t::e20 ||
                          face.getrule() == balrule_t::iso4 );
          const int subFaces = ( face.getrule() == balrule_t::iso4 && ( ! this->is2d() ) ) ? 4 : 2;
          for (int j = 0 ; j < subFaces ; ++j ) face.subface (j)->nb.complete (face.nb) ;
        }
      }
    }
    else
    {
      //alugrid_assert (getrule() == myrule_t::nosplit) ;
      // Auf dem Element gibt es kein refine (myrule_t) deshalb mu"s erst
      // request (myrule_t) und dann refine () durchgef"uhrt werden.

      if(getrule() != r)
      {
        // request read rule
        request (r) ;
        // refine tetra
        refine() ;
      }

      alugrid_assert (getrule() == r) ;

      // call restore on inner items
      for (inneredge_t * e = innerHedge () ; e ; e = e->next ())
      {
        e->restore (is) ;
      }

      for (innerface_t * f = innerHface () ; f ; f = f->next ())
      {
        f->restore (is) ;
      }

      // call restore on children
      for (innertetra_t * c = dwnPtr() ; c ; c = c->next ())
      {
        c->restore (is) ;
      }
    }
    return ;
  }

  // ######                                                           #####  #######
  // #     #  ######  #####      #     ####   #####      #     ####  #     #   #      ####   #####
  // #     #  #       #    #     #    #    #  #    #     #    #    #       #   #     #    #  #    #
  // ######   #####   #    #     #    #    #  #    #     #    #       #####    #     #    #  #    #
  // #        #       #####      #    #    #  #    #     #    #            #   #     #    #  #####
  // #        #       #   #      #    #    #  #    #     #    #    # #     #   #     #    #  #
  // #        ######  #    #     #     ####   #####      #     ####   #####    #      ####   #

  template< class A > typename Periodic3Top < A >::myhedge_t * Periodic3Top < A >::subedge (int i, int j) {
    switch (myhface(i)->getrule()) {
      case myhface_t::myrule_t::e01 :
      case myhface_t::myrule_t::e12 :
      case myhface_t::myrule_t::e20 :
        alugrid_assert ( j == 0 );
        return myhface (i)->subedge (0) ;
      case myhface_t::myrule_t::iso4 :
        if(this->is2d()){
           alugrid_assert ( j == 0 );
          return myhface (i)->subedge (0) ;
        }
        alugrid_assert ( j < 3 );
        return ((twist (i) < 0) ? myhface (i)->subedge ((8 - j + twist (i)) % 3) : myhface (i)->subedge ((j + twist (i)) % 3)) ;
      case myhface_t::myrule_t::nosplit :
        std::cerr << "**FEHLER (FATAL): subedge () auf nicht in verfeinerter Fl\"ache aufgerufen. In " << __FILE__ << " " << __LINE__ << std::endl ;
        abort () ;
        return 0 ;
    }
    return 0 ;
  }

  template< class A > const typename Periodic3Top < A >::myhedge_t * Periodic3Top < A >::subedge (int i, int j) const {
    return ((Periodic3Top < A > *)this)->subedge (i,j) ;
  }

  template< class A > typename Periodic3Top < A >:: myhface_t * Periodic3Top < A >::subface (int i, int j) {
    myhface_t * face = myhface(i);
    switch (face->getrule ())
    {
    case myhface_t::myrule_t::e01 :
      alugrid_assert ( j < 2 );
      if ( twist(i) == 0 ||  twist(i) == 1  ||  twist(i) == -1 )
        return face->subface(j) ;
      if ( twist(i) == 2 ||  twist(i) == -2 ||  twist(i) == -3 )
        return face->subface(!j) ;
      std::cerr << __FILE__ << " " << __LINE__ << "myhface(i)->subface()" << std::endl;
      return 0;
    case myhface_t::myrule_t::e12 :
      alugrid_assert ( j < 2 );
      if ( twist(i) == 0  ||  twist(i) == 2 ||  twist(i) == -3 )
        return face->subface(j) ;
      if ( twist(i) == -1 ||  twist(i) == 1 ||  twist(i) == -2 )
        return face->subface(!j) ;
      std::cerr << __FILE__ << " " << __LINE__ << "myhface(i)->subface()" << std::endl;
      return 0;
    case myhface_t::myrule_t::e20 :
      alugrid_assert ( j < 2 );
      if ( twist(i) == 1 ||  twist(i) == 2 ||  twist(i) == -2 )
        return face->subface(j) ;
      if ( twist(i) == 0 ||  twist(i) == -1 || twist(i) == -3 )
        return myhface(i)->subface(!j) ;
      std::cerr << __FILE__ << " " << __LINE__ << "myhface(i)->subface()" << std::endl;
      return 0;
    case myhface_t::myrule_t::iso4 :
      // in 2d case do the same as e12
      if(face->is2d())
      {
        alugrid_assert ( j < 2 );
        if ( twist(i) == 0  ||  twist(i) == 2 ||  twist(i) == -3 )
          return face->subface(j) ;
        if ( twist(i) == -1 ||  twist(i) == 1 ||  twist(i) == -2 )
          return face->subface(!j) ;
        std::cerr << __FILE__ << " " << __LINE__ << "myhface(i)->subface()" << std::endl;
        return 0;
      }

      alugrid_assert ( j < 4 );
      if ( j == 3 )
      {
        return face->subface (3) ;
      }
      else // ( j < 3 )
      {
        alugrid_assert( j < 3 );
        return face->subface (twist(i) < 0 ? (7 - j + twist(i)) % 3 : (j + twist(i)) % 3) ;
      }
    case myhface_t::myrule_t::nosplit :
      std::cerr << "**FEHLER (FATAL): subface () auf nicht verfeinerter Fl\"ache aufgerufen. In " << __FILE__ << " " << __LINE__ << std::endl ;
      std::abort () ;
      return 0 ;
    default:
      std::cerr << "**FEHLER (FATAL): Falsche Verfeinerungsregel [" << myhface(i)->getrule() << "] in " << __FILE__ << " " << __LINE__ << std::endl ;
      std::abort() ;
    }
    return 0 ;
  }

  template< class A > const typename Periodic3Top < A >:: myhface_t * Periodic3Top < A >::subface (int i, int j) const {
    return ((Periodic3Top < A > *)this)->subface (i,j) ;
  }

  template< class A > void Periodic3Top < A >::split_iso4 ()
  {
    alugrid_assert( ! myhface(0)->is2d() );
    const int l = 1 + this->level () ;
    innerperiodic3_t * p0 = new innerperiodic3_t (l, subface (0,0), twist (0), subface (1,0), twist (1), this , 0) ;
    innerperiodic3_t * p1 = new innerperiodic3_t (l, subface (0,1), twist (0), subface (1,2), twist (1), this , 1) ;
    innerperiodic3_t * p2 = new innerperiodic3_t (l, subface (0,2), twist (0), subface (1,1), twist (1), this , 2) ;

    // Mir ist nicht ganz klar, warum der Twist auf diese seltsame Art umzurechnen ist,
    // die Zeile (bzw. die Formel) habe ich aus Mario's Tetradeder Split Iso-8 "uber-
    // nommen, ohne im einzelnen nachzupr"ufen, ob die Regel richtig ist. (BS)

    innerperiodic3_t * p3 = new innerperiodic3_t (l, subface (0,3), (twist(0) >= 0 ? (twist(0)+1)%3 : (twist(0)-1)%3-1), subface (1,3), (twist(1)>=0 ? (twist(1)+1)%3 : (twist(1)-1)%3-1) , this , 3) ;
    alugrid_assert (p0 && p1 && p2 && p3) ;
    p0->append(p1) ;
    p1->append(p2) ;
    p2->append(p3) ;
    _dwn = p0 ;
    _rule = myrule_t::iso4 ;
    p0->_up = p1->_up = p2->_up = p3->_up = this;
    return ;
  }

  template< class A > void Periodic3Top < A >::split_iso2 ()
  {
    alugrid_assert( myhface(0)->is2d() );
    const int l = 1 + this->level () ;
    innerperiodic3_t * p0 = new innerperiodic3_t (l, subface (0,1), twist (0), subface (1,0), twist (1), this , 0) ;
    innerperiodic3_t * p1 = new innerperiodic3_t (l, subface (0,0), twist (0), subface (1,1), twist (1), this , 1) ;

    alugrid_assert (p0 && p1) ;
    p0->append(p1) ;
    _dwn = p0 ;
    // nevertheless, set iso4 rule
    _rule = myrule_t::iso4 ;
    p0->_up = p1->_up = this;
    return ;
  }

  template< class A > void Periodic3Top < A >::refineImmediate (myrule_t r) {

    // Die Methode wird nur vom restore () und vom refineBalance () auf-
    // gerufen und geht davon aus, dass das betroffene Element noch nicht
    // verfeinert ist -> ist ein Blatt der Hierarchie.

    alugrid_assert (this->leaf()) ;
    switch(r)
    {
      case myrule_t::iso4 :
        if(myhface (0)->is2d()) // 2d refinement
        {
          //std::cerr << "**ERROR (FATAL) refinement of Periodic3Top in 2d not implemented yet! " ;
          //std::cerr << "[" << r << "]. In " << __FILE__ << __LINE__ << std::endl ;
          //std::abort () ;

          typedef typename myhface_t::myrule_t face3rule_t;
          myhface (0)->refineImmediate (face3rule_t (r).rotate (twist (0))) ;
          myhface (1)->refineImmediate (face3rule_t (r).rotate (twist (1))) ;
          split_iso2 () ;
          break ;
        }
        else // 3d case
        {
          // Das refineImmediate (..) auf allen Fl"achen wird vom periodic3::refine (..)
          // zwar nicht ben"otigt, da schliesslich alle Fl"achen sauber sind, wenn
          // "uberall hface3::refine (..) true geliefert hat, wohl aber z.B. von
          // restore () oder abgeleiteten Funktionen die eine direkte Verfeinerung
          // erzwingen m"ussen und d"urfen.

          typedef typename myhface_t::myrule_t face3rule_t;
          myhface (0)->refineImmediate (face3rule_t (r).rotate (twist (0))) ;
          myhface (1)->refineImmediate (face3rule_t (r).rotate (twist (1))) ;
          split_iso4 () ;
          break ;
        }

        std::cerr << "**ERROR (FATAL) refinement of Periodic3Top didd not work: " ;
        std::cerr << "[" << r << "]. In " << __FILE__ << __LINE__ << std::endl ;
        std::abort () ;
        break ;

      case myrule_t::e01 :
      case myrule_t::e12 :
      case myrule_t::e20 :
        split_iso2 () ;
        break;

        // Mit den drei anisotropen Regeln k"onnen wir leider noch nichts anfangen.

        std::abort () ;
      default :
        std::cerr << "**FEHLER (FATAL) beim unbedingten Verfeinern mit unbekannter Regel: " ;
        std::cerr << "[" << r << "]. In " << __FILE__ << __LINE__ << std::endl ;
        std::abort () ;
        break ;
    }
    this->postRefinement () ;
    return ;
  }

  template< class A > bool Periodic3Top < A >::refine () {

    // Das refine () reagiert nicht auf die Elementaktivierung zur Verfeinerung
    // in der globalen Schleife, weil das perioodische Randelement sich nur auf
    // Anforderung zur Balancierung aus einem anliegenden Element direkt verfeinert.

    return true ;
  }

  template< class A > bool Periodic3Top < A >::refineBalance (balrule_t r, int fce)
  {
    if (r != balrule_t::iso4)
    {
      std::cerr << "**WARNING (IGNORED) in Periodic3Top < A >::refineBalance (..) , file "
         << __FILE__ << " line " << __LINE__ << " periodic refinement is only implemented for isometric refinement!" << std::endl ;

    // Bisher kann die Balancierung nur die isotrope Achtelung handhaben,
    // falls mehr gew"unscht wird, muss es hier eingebaut werden. Im Moment wird
    // die Balancierung einfach verweigert, d.h. die Verfeinerung des anfordernden
    // Elements f"allt flach.

      return false ;
    }
    else
    {
      // Der nachfolgende Aufruf nutzt aus, dass die Regel der periodischen R"ander
      // sich direkt auf die Balancierungsregel des entsprechenden Polygonverbinders
      // projezieren l"asst (n"amlich 1:1). Deshalb unterscheidet der Aufruf nicht nach
      // der angeforderten Regel in einer 'case' Anweisung.

      // take opposite face
      const int opp = 1 - fce ;
      if (myhface (opp)->refine (typename myhface_t::myrule_t (r).rotate (twist (opp)), twist (opp)))
      {
        refineImmediate (r) ;
        return true ;
      }
      else
      {
        return false ;
      }
    }
  }

  template< class A > bool Periodic3Top < A >::coarse () {

    // Das Vergr"obern geschieht auch passiv, sobald ein anliegendes Element
    // vergr"obert wird durch den Aufruf von "bndNotifyCoarsen ()" s.u.

    bndNotifyCoarsen () ;
    return false ;
  }

  template< class A > bool Periodic3Top < A >::bndNotifyCoarsen () {

    // Wie beim Randelement auch: Die Vergr"oberung eines anliegenden Elements
    // l"ost einen Vorgang aus, der feststellt ob das periodische RE ebenfalls
    // vergr"obert werden soll.

    innerperiodic3_t * p = down () ;
    if (!p) return false ;
    bool x = true ;
    do {

    // Falls p kein Blatt der Hierarchie ist,
    // die Vergr"oberungsm"oglichkeit weitergeben.

      if (!p->leaf ()) p->coarse () ;

    // F"ur die hintere und vordere Fl"ache feststellen, ob
    // der Referenzenz"ahler mehr als einen Eintrag ergibt.

      if (p->myhface (0)->ref > 1) (x = false) ;
      if (p->myhface (1)->ref > 1) (x = false) ;

    } while ( (p = p->next ()) ) ;
    if (x) {

    // Falls keine Fl"achen anliegen, die auf Kinder oder Kindes-
    // mit mehr als einer Referenz f"uhren, ist sicher, dass das
    // Bezugsrandelement zwischen zwei 'relativ groben' Elementen
    // liegt. Somit kann es vergr"obert werden.

      this->preCoarsening () ;

      delete _dwn ;
      _dwn = 0 ;
      _rule = myrule_t::nosplit ;
      myhface (0)->coarse () ;
      myhface (1)->coarse () ;
    }
    return x ;
  }

  template< class A > int Periodic3Top< A >::backup ( std::ostream &os ) const
  {
    return doBackup( os );
  }

  template< class A > int Periodic3Top < A >::backup (ObjectStream& os) const
  {
    return doBackup( os );
  }

  template< class A > template<class OutStream_t>
  int Periodic3Top < A >::doBackup (OutStream_t& os) const
  {
    os.put ((char) getrule ()) ;
    { for (const innerperiodic3_t * c = down () ; c ; c = c->next ()) c->backup (os) ; }
    return 0;
  }

  template< class A > void Periodic3Top< A >::restore ( std::istream &is )
  {
    doRestore( is );
  }
  template< class A > void Periodic3Top < A >::restore (ObjectStream& is)
  {
    doRestore( is );
  }

  template< class A > template<class InStream_t>
  void Periodic3Top < A >::doRestore (InStream_t & is)
  {
    myrule_t r ((char) is.get ()) ;
    alugrid_assert (getrule () == myrule_t::nosplit) ; // Testen auf unverfeinerten Zustand
    if (r == myrule_t::nosplit) {
      for (int i = 0 ; i < 2 ; i ++) {
        myhface_t & f (*(myhface (i))) ;
        if (!f.leaf ()) {
          switch (f.getrule ()) {
      case balrule_t::iso4 :
              {for (int j = 0 ; j < 4 ; j ++) f.subface (j)->nb.complete (f.nb) ;}
        break ;
      default :
        std::cerr << "**FEHLER (FATAL) beim restore mit unbekannter Balancierungsregel: "
                   << "[" << r << "]. In " << __FILE__ << __LINE__ << std::endl ;
        abort () ;
        break ;
    }
        }
      }
    }
    else {
      refineImmediate (r) ;
      alugrid_assert (getrule() == r) ;
      {for (innerperiodic3_t * c = down () ; c ; c = c->next ()) c->restore (is) ; }
    }
    return ;
  }



  // Template Instantiation
  // ----------------------

#ifndef GITTER_TETRA_TOP_PLL_H_INCLUDED
  template class Hface3Top< GitterBasis::Objects::Hface3Empty >;
  template class Hbnd3Top< GitterBasis::Objects::Hbnd3Default >;
  template class TetraTop < GitterBasis::Objects::TetraEmpty >;
  template class Periodic3Top < GitterBasis::Objects::Periodic3Empty >;
#endif // #ifndef GITTER_TETRA_TOP_PLL_H_INCLUDED

} // namespace ALUGrid
