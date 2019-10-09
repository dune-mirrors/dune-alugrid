// (c) Robert Kloefkorn 2010
#include <config.h>

#include "mappings.hh"
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
             1  /           \ 2
               /1    2*1    2\
              /               \
             /        *        \
            /                   \
           /          *          \
          /0    0    1 0    0    1\
          ------------*------------
        0            0               1
      */

    alugrid_assert ( !(this->is2d()) );
    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + level () ;
    myhedge_t* splitEdge = myhedge(0);

    myvertex_t * ev0 = splitEdge->subvertex (0) ;

    myvertex_t * vx2 = myvertex( 2 );
    alugrid_assert (ev0) ;
    inneredge_t * e0 = new inneredge_t (newLevel, ev0, vx2 ) ;
    alugrid_assert ( e0 ) ;
    innerface_t * f0 = new innerface_t (newLevel,
                                        splitEdge->subedge(0),
                                        myhedge(1),
                                        e0,
                                        0) ; // child number

    innerface_t * f1 = new innerface_t (newLevel,
                                        splitEdge->subedge(1),
                                        e0,
                                        myhedge(2),
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
                    /     2 \
                   /         \
                  /           \
               1 / 1         1 \ 2
                /             * \
               /           *   2 \
              /      0  *         \
             /       *           2 \
            /     *   1             \
           /0  *                     \
          / *0                      1 \
          -----------------------------
        0              0                 1
      */
    alugrid_assert ( _inner == 0 );
    const int newLevel= 1 + level () ;
    myhedge_t* splitEdge = myhedge(2);

    myvertex_t * ev0 = splitEdge->subvertex (0) ;
    alugrid_assert (ev0) ;

    myvertex_t * vxOld = myvertex(0);

    // create new inner edge
    inneredge_t * e0 = new inneredge_t (newLevel, vxOld, ev0 ) ;
    alugrid_assert ( e0 ) ;

    //std::cout << "split_e12 " << ev0 << std::endl;
    //std::cout << "new inner edge " << e0 << std::endl;

    innerface_t * f0 = new innerface_t (newLevel, // level
                                        myhedge(0),         // edge 0
                                        e0,                         // edge 1
                                        splitEdge->subedge(0),
                                        0 ) ; // child number

    innerface_t * f1 = new innerface_t (newLevel, // level
                                        e0,                         // edge 0
                                        myhedge(1),  // edge 1
                                        splitEdge->subedge(1),       // edge 2
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

  template< class A >  void Hface3Top < A >::split_e02 ()
  {
    // NOTE: edge numbering is not opposite vertex !!!
    // see gitter_geo.cc
    // the new edge needs to be in the middle (meaning edge 1 out of {0,1,2})
    //

    /*                  2

                       / \
                      / 2 \
                     /     \
                    /2      \
                   /         \
                  /           \
               1 /1            \  2
                / *            1\
               / 2   *           \
              /         * 0       \
             /1          2 *       \
            /                 *     \
           /                     * 0 \
          / 0          0          1 * \
          -----------------------------
        0              0                 1
      */

    alugrid_assert( !(this->is2d()) );
    alugrid_assert ( _inner == 0 );
    const int newLevel= 1 + level () ;

    myhedge_t* splitEdge = myhedge( 1 );

    // get vertices of new edge
    myvertex_t * ev0 = splitEdge->subvertex (0) ;

    myvertex_t * vxOld = myvertex( 1 );

    alugrid_assert (ev0) ;
    inneredge_t * e0 = new inneredge_t (newLevel, vxOld, ev0 ) ;
    alugrid_assert ( e0 ) ;

    innerface_t * f0 = new innerface_t (newLevel, // level
                                        myhedge(0),         // edge 0
                                        splitEdge->subedge(0),         // edge 1
                                        e0, // edge 2
                                        0 ) ; // child number

    innerface_t * f1 = new innerface_t (newLevel, // level
                                        e0,         // edge 0
                                        myhedge(2),                         // edge 1
                                        splitEdge->subedge(1), // edge 2
                                        1 ) ; // child number

    alugrid_assert (f0 && f1 ) ;

    //std::cout << "split_e02 " << ev0 << std::endl;
    //std::cout << "Split face " << this << " into " << std::endl;
    //std::cout << "New subface 0" << f0 << std::endl;
    //std::cout << "New subface 1" << f1 << std::endl;

    f0->append(f1) ;
    _inner = new inner_t( f0 , e0 );
    _rule = myrule_t::e02 ;

    return ;
  }

  template< class A >  void Hface3Top < A >::split_iso4 ()
  {
    alugrid_assert ( _inner == 0 );
    alugrid_assert ( !(this->is2d()) );
    int l = 1 + level () ;
    myvertex_t * ev0 = myhedge(0)->subvertex (0) ;
    myvertex_t * ev1 = myhedge(1)->subvertex (0) ;
    myvertex_t * ev2 = myhedge(2)->subvertex (0) ;
    alugrid_assert (ev0 && ev1 && ev2 ) ;
    inneredge_t * e0 = new inneredge_t (l, ev0, ev1) ;
    inneredge_t * e1 = new inneredge_t (l, ev0, ev2) ;
    inneredge_t * e2 = new inneredge_t (l, ev1, ev2) ;
    alugrid_assert ( e0 && e1 && e2 ) ;
    e0->append(e1) ;
    e1->append(e2) ;
    innerface_t * f0 = new innerface_t (l, this->subedge(0,0), this->subedge(1,0), e0, 0) ;
    innerface_t * f1 = new innerface_t (l, this->subedge(0,1), e1, this->subedge(2,0), 1) ;
    innerface_t * f2 = new innerface_t (l, e2, this->subedge(1,1), this->subedge(2,1), 2) ;
    innerface_t * f3 = new innerface_t (l, e0, e1, e2, 3 ) ;
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

        case myrule_t::e01 :
          myhedge (0)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2)) ;
          split_e01 () ;
          break ;
        case myrule_t::e12 :
          myhedge (2)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2)) ;
          split_e12 () ;
          break ;
        case myrule_t::e02 :
          myhedge (1)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2)) ;
          split_e02 () ;
          break ;
        case myrule_t::iso4 :
          //iso4 on a 2d face is just e12 bisection + setting the rule to iso4
          if(this->is2d()) {
            myhedge (2)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2)) ;
            split_e12 () ;
            _rule = myrule_t::iso4 ;
            break ;
          }
          myhedge (0)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2)) ;
          myhedge (1)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2)) ;
          myhedge (2)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2)) ;
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

  template< class A > bool Hface3Top < A >::refine (myrule_t r, bool isRear)
  {
    if (r != getrule ())
    {
      alugrid_assert (getrule () == myrule_t::nosplit ? 1 :
        (std::cerr << "**FEHLER beim Verfeinern mit Regel " << r << " auf " << getrule () << std::endl, 0)) ;
      switch(r)
      {
        case myrule_t::e01 :
        case myrule_t::e12 :
        case myrule_t::e02 :
        case myrule_t::iso4 :
        {
          typedef typename A::face3Neighbour::neighbour_t neighbour_t ;
          // get face neighbour
          neighbour_t neigh = isRear ? this->nb.front () : this->nb.rear()  ;

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
                neighbour_t neigh = isRear ? this->nb.front () : this->nb.rear()  ;
                //refine neighbour as long as it is leaf.
                //We are at the refinement face, so the neighbour of the refined element
                //has to be refined too
                while(neigh.first->nbLeaf())
                {
                  neigh.first->refineBalance (r, neigh.second);
                  neigh = isRear ? this->nb.front () : this->nb.rear()  ;
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

    // In the case of facerule == e02 the second child flips
    // rear and front are exchanged


    innerbndseg_t * b0 = new innerbndseg_t (l, subface (0,0), isRear (0), this , _bt, ghostInfo.child(0), ghostInfo.face(0) ) ;
    innerbndseg_t * b1 = new innerbndseg_t (l, subface (0,1), isRear (0), this , _bt, ghostInfo.child(1), ghostInfo.face(1) ) ;
    //innerbndseg_t * b0 = new innerbndseg_t (l, subface (0,0), isRear (0), this , _bt, 0, gFace ) ;
    //innerbndseg_t * b1 = new innerbndseg_t (l, subface (0,1), isRear (0), this , _bt, 0, gFace ) ;

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

    innerbndseg_t * b0 = new innerbndseg_t (l, subface (0,0), isRear (0), this , _bt, ghostInfo.child(0), ghostInfo.face(0)) ;
    innerbndseg_t * b1 = new innerbndseg_t (l, subface (0,1), isRear (0), this , _bt, ghostInfo.child(1), ghostInfo.face(1)) ;
    innerbndseg_t * b2 = new innerbndseg_t (l, subface (0,2), isRear (0), this , _bt, ghostInfo.child(2), ghostInfo.face(2)) ;
    innerbndseg_t * b3 = new innerbndseg_t (l, subface (0,3), isRear (0), this , _bt, ghostInfo.child(3), ghostInfo.face(3)) ;
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
        case balrule_t::e02 :
          //std::cout << "refLikeEl: e01 " << std::endl;
          // if (!myhface (0)->refine (balrule_t (balrule_t::e01), isRear(0))) return false ;
          if (! face.refine (r, isRear (0))) return false ;

          split_bisection() ;
          break;
        case balrule_t::iso4 :
          //in the 2d case we just to the same as e12
          if( face.is2d()){
            if (! face.refine (r, isRear (0))) return false ;
            split_bisection() ;
            break;
          }
          //if (!myhface (0)->refine (balrule_t (balrule_t::iso4), (isRear (0)))) return false ;
          if (! face.refine (r, isRear (0))) return false ;
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
        case myrule_t::e02 :
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
  :: TetraTop (int l, myhface_t * f0, bool  t0,
               myhface_t * f1, bool t1, myhface_t * f2, bool t2,
               myhface_t * f3, bool t3, innertetra_t *up, int nChild, double vol)
    : A (f0, t0, f1, t1, f2, t2, f3, t3),
      _bbb (0), _up(up)
    , _inner( 0 )
    , _volume( vol < 0.0 ? computeVolume() : vol )
    , _lvl (l)
    , _nChild(nChild)
    , _rule (myrule_t::nosplit)
  {

    // set level
    alugrid_assert ( this->level() == l );

    // set element's index and 2d flag if necessary
    this->setIndexAnd2dFlag( indexManager() );

    // we need boundary id now, for elements is the same as fathers
    this->_bndid = _up->bndId();

#ifdef ALUGRIDDEBUG
    //TODO  check that _volume has the correct value
    //if( std::abs( calculatedVolume - _volume ) >1e-10 )
    //  std::cout << "Determinant of Tetra[" << this->getIndex() << "] is wrong" << std::endl;
  //  alugrid_assert ( std::abs( calculatedVolume - _volume ) / _volume  < 1e-10 );
#endif
  }

  // this is the macro element constructor
  template< class A > TetraTop < A >::
  TetraTop (int l,  // level
            myhface_t * f0, bool t0, // face, isRear
            myhface_t * f1, bool t1, // face, isRear
            myhface_t * f2, bool t2, // face, isRear
            myhface_t * f3, bool t3, // face, isRear
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
        // get face and isRear
        myhface_t* face = myhface( i );
        const bool twst = isRear( i );

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
        myhface( i )->detachElement ( isRear( i ) ) ;
    }
  }

  //- --subedge
  template< class A >  typename TetraTop < A >::myhedge_t * TetraTop < A >::subedge (int face, int edge)
  {
    switch ( myhface( face )->getrule() )
    {
    case myhface_t::myrule_t::e01 :
    case myhface_t::myrule_t::e12 :
    case myhface_t::myrule_t::e02 :
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
        return  myface.subedge ( edge);
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



  // --subface
  template< class A >  typename TetraTop < A >:: myhface_t * TetraTop < A >::subface (int i, int j)
  {
    switch ( myhface( i )->getrule() )
    {
    case myhface_t::myrule_t::e01 :
    case myhface_t::myrule_t::e12 :
    case myhface_t::myrule_t::e02 :
      alugrid_assert ( j < 2 );
      return myhface(i)->subface(j) ;
    case myhface_t::myrule_t::iso4 :
      {
        myhface_t& myface (*(myhface(i)));
        //for the 2d faces we do the same as for e12
        if(myface.is2d()){
           alugrid_assert ( j < 2 );
            return myface.subface(j) ;
        }
        alugrid_assert ( j < 4 );
        return myface.subface(j) ;
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

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       this->subedge (0, 0), // from face 0 get subedge 0
                       this->subedge (1, 0), // from face 1 get subedge 0
                       this->myhedge( 5 )
                      ) ;
    alugrid_assert ( newFace );


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
                                          subface(0,0), isRear(0),
                                          subface(1,0), isRear(1),
                                          myhface(2), isRear(2),
                                          newFace, isRear(3),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          subface(0,1), isRear(0),
                                          subface(1,1), isRear(1),
                                          newFace, !isRear(3),
                                          myhface(3), isRear(3),
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
    // split e03 then 3 is new in child 0 and 0 is new in child 1
    alugrid_assert ( h0->myvertex( 1 ) == h1->myvertex( 0 ) );

    // remap vertices of children
    setNewMapping( h0, h1, newFace );

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

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       this->subedge(0,0), // from face 1 get subedge 0
                       myhedge(3), // from face 2 get subedge 0
                       this->subedge(3,0)
                      ) ;
    alugrid_assert ( newFace );


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
                                          subface(0,0), isRear(0),
                                          myhface(1), isRear(1),
                                          newFace, isRear(2),
                                          subface(3,0), isRear(3),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          subface(0,1), isRear(0),
                                          newFace, !isRear(2),
                                          myhface(2), isRear(2),
                                          subface(3,1), isRear(3),
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
    // split e03 then 3 is new in child 0 and 0 is new in child 1
    alugrid_assert ( h0->myvertex( 2 ) == h1->myvertex( 1 ) );

    // remap vertices of children
    setNewMapping( h0, h1, newFace );

    // set refinement rule that was used to refine this tetra
    _rule = myrule_t::e12 ;
  }

  template< class A >  void TetraTop < A >::split_e02 ()
  {
    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + this->level();

    splitInfo( myrule_t::e02 );

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       this->subedge(0,0),
                       myhedge(4), // from face 1 get subedge 0
                       this->subedge(2,0) // from face 2 get subedge 0
                      ) ;
    alugrid_assert ( newFace );


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
                                          subface(0,0), isRear(0),
                                          myhface(1), isRear(1),
                                          subface(2,0), isRear(2),
                                          newFace, isRear(3),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          subface(0,1), isRear(0),
                                          newFace, !(isRear(3)),
                                          myhface(3), isRear(3),
                                          subface(2,1), isRear(2),
                                          this, 1, childVolume) ;

    alugrid_assert (h0 && h1) ;

    // the new vertices are the ones that are missing
    // i.e. 3 in child 0  and  0 in child 1
    alugrid_assert ( h0->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h0->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h0->myvertex( 3 ) == myvertex( 3 ) );

    alugrid_assert ( h1->myvertex( 0 ) == myvertex( 1 ) );
    alugrid_assert ( h1->myvertex( 2 ) == myvertex( 2 ) );
    alugrid_assert ( h1->myvertex( 3 ) == myvertex( 3 ) );

    // this is always the edge combo, i.e. if we
    // split e03 then 3 is new in child 0 and 0 is new in child 1
    alugrid_assert ( h0->myvertex( 2 ) == h1->myvertex( 1 ) );

    // remap vertices of children
    setNewMapping( h0, h1, newFace );

    // set refinement rule that was used to refine this tetra
    _rule = myrule_t::e02;
  }

  template< class A >  void TetraTop < A >::split_e23 ()
  {
    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + this->level();

    splitInfo( myrule_t::e23 );

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       myhedge(0), // from face 2 get subedge 0
                       this->subedge(2,0), // from face 1 get subedge 0
                       this->subedge(3,0)
                      ) ;
    alugrid_assert ( newFace );


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
                                          myhface(0), isRear(0),
                                          newFace, isRear(1),
                                          subface(2,0), isRear(2),
                                          subface(3,0), isRear(3),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          newFace, !isRear(1),
                                          myhface(1), isRear(1),
                                          subface(2,1), isRear(2),
                                          subface(3,1), isRear(3),
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
    setNewMapping( h0, h1, newFace );

    // set refinement rule that was used to refine this tetra
    _rule = myrule_t::e23 ;
  }

  template< class A >  void TetraTop < A >::split_e03 ()
  {
    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + this->level();

    splitInfo( myrule_t::e03 );
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

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       myhedge(2), // from face 2 get subedge 0
                       this->subedge(1,0), // from face 1 get subedge 0
                       this->subedge(2,0)
                      ) ;

    alugrid_assert ( newFace ) ;


    // we divide by 2 means we divide the volume by 2
    const double childVolume = calculateChildVolume( 0.5 * _volume );

    innertetra_t * h0 = new innertetra_t (newLevel,
                                          myhface(0), isRear(0),
                                          subface(1,0), isRear(1),
                                          subface(2,0), isRear(2),
                                          newFace, isRear(3),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          newFace, !isRear(3),
                                          myhface(3), isRear(3),
                                          subface(1,1), isRear(1),
                                          subface(2,1), isRear(2),
                                          this, 1, childVolume) ;

    alugrid_assert (h0 && h1) ;

    // the new vertices are the ones that are missing
    // i.e. 3 in child 0  and  0 in child 1
    alugrid_assert ( h0->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h0->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h0->myvertex( 2 ) == myvertex( 2 ) );

    alugrid_assert ( h1->myvertex( 0 ) == myvertex( 1 ) );
    alugrid_assert ( h1->myvertex( 1 ) == myvertex( 2 ) );
    alugrid_assert ( h1->myvertex( 3 ) == myvertex( 3 ) );

    // this is always the edge combo, i.e. if we
    // split e03 then 3 is new in child 0 and 0 is new in child 1
    alugrid_assert ( h0->myvertex( 3 ) == h1->myvertex( 2 ) );

    // remap vertices of children
    setNewMapping( h0, h1, newFace );

    // set refinement rule that was used to refine this tetra
    _rule = myrule_t::e03;
  }

  template< class A >  void TetraTop < A >::split_e13 ()
  {
    alugrid_assert ( _inner == 0 );
    const int newLevel = 1 + this->level () ;

    splitInfo( myrule_t::e13 );

    // new inner face
    innerface_t * newFace =
      new innerface_t (newLevel,
                       myhedge(1),
                       this->subedge(1,0),  // from face 1 get subedge 0
                       this->subedge(3,0)  // from face 2 get subedge 0
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



    innertetra_t * h0 = new innertetra_t (newLevel,
                                          myhface(0), isRear(0),
                                          subface(1,0), isRear(1),
                                          newFace, isRear(2),
                                          subface(3,0), isRear(3),
                                          this, 0, childVolume) ;

    innertetra_t * h1 = new innertetra_t (newLevel,
                                          newFace, !(isRear(2)),
                                          myhface(2), isRear(2),
                                          subface(1,1), isRear(1),
                                          subface(3,1), isRear(3),
                                          this, 1, childVolume) ;

    alugrid_assert (h0 && h1) ;

    // the new vertices are the ones that are missing
    // i.e. 3 in child 0  and  0 in child 1
    alugrid_assert ( h0->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h0->myvertex( 1 ) == myvertex( 1 ) );
    alugrid_assert ( h0->myvertex( 2 ) == myvertex( 2 ) );

    //the internal numbering changes
    alugrid_assert ( h1->myvertex( 0 ) == myvertex( 0 ) );
    alugrid_assert ( h1->myvertex( 1 ) == myvertex( 2 ) );
    alugrid_assert ( h1->myvertex( 3 ) == myvertex( 3 ) );

    // this is always the edge combo, i.e. if we
    // split e03 then 3 is new in child 0 and 0 is new in child 1
    alugrid_assert ( h0->myvertex( 3 ) == h1->myvertex( 2 ) );

    // remap vertices of children
    setNewMapping( h0, h1, newFace );

    // set refinement rule that was used to refine this tetra
    _rule = myrule_t::e13;
  }

  // --setNewMapping
  template< class A >  void
  TetraTop < A >::setNewMapping( innertetra_t* h0, innertetra_t* h1,
                                 innerface_t* newFace)
  {
    // append h1 to h0
    h0->append( h1 );

    // create inner storage with first child and new face
    _inner = new inner_t( h0, newFace );
    alugrid_assert ( _inner );

    // detach myself from being leaf element
    this->detachleafs();
  }



  // --checkTetra
  template< class A > bool
  TetraTop < A >::checkTetra( const innertetra_t *tetra, const int nChild ) const
  {
    // make sure faces are ok
    bool facesOk = true ;

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
        // use proto type to check face
        if( tetra->myvertex( Gitter::Geometric::Tetra::prototype[ fce ][ i ] )
              != tetra->myvertex( fce, i ) )
        {
          const int vx0 = Gitter::Geometric::Tetra::prototype[ fce ][ 0 ] ;
          const int vx1 = Gitter::Geometric::Tetra::prototype[ fce ][ 1 ] ;

          const int vx[2] = { tetra->myvertex( vx0 )->getIndex(),
                              tetra->myvertex( vx1 )->getIndex()
                            };

          std::cout << "face " << fce << " is wrong" << std::endl;
          facesOk = false ;
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

    return facesOk;
  }

  template< class A >  void TetraTop < A >::
  splitISO8 ()
  {
    alugrid_assert ( _inner == 0 );
    typedef typename A::myvertex_t  myvertex_t;
    typedef typename A::inneredge_t inneredge_t;
    const int l = 1 + this->level () ;

    //Choice of diagonal is important for shape regularity
    //This algorithm is described in Bey,2000 : Simplicial Grid Refinement ...
    myvertex_t * e13 = myhface (1)->myhedge (2)->subvertex (0) ;
    myvertex_t * e02 = myhface (0)->myhedge (1)->subvertex (0) ;
    alugrid_assert (e13 && e02);
    inneredge_t * e0 = new inneredge_t (l, e02, e13) ;
    alugrid_assert (e0) ;
    //first the 4 inner faces of tetrahedra at each corner of the father
    //In DUNE numbering
    // 0,1,3 : 0,2,4 : 1,2,5 : 3,4,5
    //then the four faces containing the diagonal
    //In DUNE numbering of the edges they are
    // 0,1,4 : 1,2,4 :  1,3,4 : 1,4,5
    innerface_t * f0 = new innerface_t (l, subedge (0, 0), subedge (1, 0), subedge (2, 0)) ;
    innerface_t * f1 = new innerface_t (l, subedge (0, 1), subedge (1, 1), subedge (3, 0)) ;
    innerface_t * f2 = new innerface_t (l, subedge (0, 2), subedge (2, 1), subedge (3, 1)) ;
    innerface_t * f3 = new innerface_t (l, subedge (1, 2), subedge (2, 2), subedge (3, 2)) ;
    innerface_t * f4 = new innerface_t (l, subedge (0, 0), subedge (1, 1), e0) ;
    innerface_t * f5 = new innerface_t (l, subedge (0, 2), e0, subedge (3, 0)) ;
    innerface_t * f6 = new innerface_t (l, subedge (2, 0), e0, subedge (1, 2)) ;
    innerface_t * f7 = new innerface_t (l, e0, subedge (2, 1), subedge (3, 2)) ;
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
    // First the tetrahedra at the corners - numbered by corner number
    innertetra_t * h0 = new innertetra_t (l, subface(0, 0), isRear(0), subface(1, 0), isRear(1), subface(2, 0), isRear(2), f0, isRear(3), this, 0 , childVolume) ;
    innertetra_t * h1 = new innertetra_t (l, subface(0, 1), isRear(0), subface(1, 1), isRear(1), f1, isRear(2), subface(3, 0), isRear(3), this, 1 , childVolume) ;
    innertetra_t * h2 = new innertetra_t (l, subface(0, 2), isRear(0), f2, isRear(1), subface(2, 1), isRear(2), subface(3, 1), isRear(3), this, 2 , childVolume) ;
    innertetra_t * h3 = new innertetra_t (l, f3, isRear(0), subface(1, 2), isRear(1), subface(2, 2), isRear(2), subface(3, 2), isRear(3), this, 3 , childVolume) ;
    innertetra_t * h4 = new innertetra_t (l, f0, !(isRear(3)), f4, true, subface(1, 3), isRear(1), f6, false, this, 4 , childVolume) ;
    innertetra_t * h5 = new innertetra_t (l, subface(0, 3), isRear(0), f4, false, f1, !(isRear(2)), f5, true, this, 5 , childVolume) ;
    innertetra_t * h6 = new innertetra_t (l, f6, true, subface(2, 3), isRear(2), f7, false, f3, !(isRear(0)), this, 6 , childVolume) ;
    innertetra_t * h7 = new innertetra_t (l, f5, false, f2, !(isRear(1)), f7, true, subface(3, 3), isRear(3), this, 7 , childVolume) ;

  //std::cout << h0 ;
 //checkTetra(h0,0);
 //std::cout << h1 ;
 //checkTetra(h1,1);
 //std::cout << h2;
 //checkTetra(h2,2);
 //std::cout << h3 ;
 //checkTetra(h3,3);
 //std::cout << h4 ;
 //checkTetra(h4,4);
 //std::cout << h5;
 //checkTetra(h5,5);
 //std::cout << h6 ;
 //checkTetra(h6,6);
 //std::cout << h7;
 //checkTetra(h7,7);
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

    innerface_t * f0 = new innerface_t (l, subedge (0, 0), subedge (1, 0), subedge (3, 0) );
    innerface_t * f1 = new innerface_t (l, subedge (0, 0), subedge (2, 0), subedge (3, 1) );
    innerface_t * f2 = new innerface_t (l, subedge (1, 0), subedge (2, 0), subedge (3, 2) );
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

    // pointer `this' is the pointer to the father element
    innertetra_t * h0 = new innertetra_t (l, subface(0,0), isRear(0), subface(1,0), isRear(1), f0, isRear(2), subface(3,0), isRear(3), this, 0 , childVolume) ;
    innertetra_t * h1 = new innertetra_t (l, subface(0,1), isRear(0), f1, isRear(1), subface(2,0), isRear(2), subface(3,1), isRear(3), this, 1 , childVolume) ;
    innertetra_t * h2 = new innertetra_t (l, f2, isRear(0), subface(1,1), isRear(1), subface(2,1), isRear(2), subface(3,2), isRear(3), this, 2 , childVolume) ;
    innertetra_t * h3 = new innertetra_t (l, f0, !(isRear(2)), f1, !(isRear(1)), f2, !(isRear(0)), subface(3,3), isRear(3), this, 3 , childVolume) ;

#ifdef ALUGRIDDEBUG
    //this check produces output, when loadbalancing and refining the transported elements, because the ghost neighbours do not exist yet
  // std::cout << h0 ;
  // checkTetra(h0,0);
  // std::cout << h1 ;
  // checkTetra(h1,1);
  // std::cout << h2;
  // checkTetra(h2,2);
  // std::cout << h3;
  // alugrid_assert(checkTetra(h3,3));
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
        _faces[ 0 ] = 0 ;
        _faces[ 1 ] = 1 ;
        _faceRules[ 0 ] = face3rule_t::e01;
        _faceRules[ 1 ] = face3rule_t::e01;
        _caller = new CallSplitImpl< myrule_t::e01 > ();
        break ;
      case myrule_t::e12 :
        _faces[ 0 ] = 0 ;
        _faces[ 1 ] = 3 ;
        _faceRules[ 0 ] = face3rule_t::e12;
        _faceRules[ 1 ] = face3rule_t::e01;
        _caller = new CallSplitImpl< myrule_t::e12 > ();
        break ;
      case myrule_t::e02 :
        _faces[ 0 ] = 0 ;
        _faces[ 1 ] = 2 ;
        _faceRules[ 0 ] = face3rule_t::e02;
        _faceRules[ 1 ] = face3rule_t::e01;
        _caller = new CallSplitImpl< myrule_t::e02 > ();
        break ;
      case myrule_t::e23 :
        _faces[ 0 ] = 2 ;
        _faces[ 1 ] = 3 ;
        _faceRules[ 0 ] = face3rule_t::e12;
        _faceRules[ 1 ] = face3rule_t::e12;
        _caller = new CallSplitImpl< myrule_t::e23 > ();
        break ;
      case myrule_t::e03 :
        _faces[ 0 ] = 1 ;
        _faces[ 1 ] = 2 ;
        _faceRules[ 0 ] = face3rule_t::e02;
        _faceRules[ 1 ] = face3rule_t::e02;
        _caller = new CallSplitImpl< myrule_t::e03 > ();
        break ;
      case myrule_t::e13 :
        _faces[ 0 ] = 1 ;
        _faces[ 1 ] = 3 ;
        _faceRules[ 0 ] = face3rule_t::e12;
        _faceRules[ 1 ] = face3rule_t::e02;
        _caller = new CallSplitImpl< myrule_t::e13 > ();
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
          myhface (i)->refineImmediate (face3rule_t (myhface_t::myrule_t::iso4)) ;
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
                if (! myhface (i)->refine (face3rule_t (face3rule_t::iso4), isRear (i))) return false ;
            }
            break ;
          case myrule_t::e01 :
          case myrule_t::e12 :
          case myrule_t::e02 :
          case myrule_t::e23 :
          case myrule_t::e03 :
          case myrule_t::e13 :
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
              if ( ! myhface (i)->refine (balrule_t ( balrule_t::iso4 ), isRear(i)) )
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
                          face.getrule() == balrule_t::e02 ||
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
      case myhface_t::myrule_t::e02 :
        alugrid_assert ( j == 0 );
        return myhface (i)->subedge (0) ;
      case myhface_t::myrule_t::iso4 :
        if(this->is2d()){
           alugrid_assert ( j == 0 );
          return myhface (i)->subedge (0) ;
        }
        alugrid_assert ( j < 3 );
        return myhface(i)->subedge(j);
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
    switch (myhface (i)->getrule ()) {
    case myhface_t::myrule_t::e01 :
    case myhface_t::myrule_t::e12 :
    case myhface_t::myrule_t::e02 :
      alugrid_assert ( j < 2 );
        return myhface(i)->subface(j) ;
      std::cerr << __FILE__ << " " << __LINE__ << "myhface(i)->subface()" << std::endl;
      return 0;
    case myhface_t::myrule_t::iso4 :
      // in 2d case do the same as e12
      if(this->is2d()){
        alugrid_assert ( j < 2 );
          return myhface(i)->subface(j) ;
        std::cerr << __FILE__ << " " << __LINE__ << "myhface(i)->subface()" << std::endl;
        return 0;
      }
      alugrid_assert ( j < 4 );
      return myhface(i)->subface (j) ;
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
    const int l = 1 + this->level () ;
    //TODO: This may be wrong unless we can guarantee twist-free periodic faces
    //which we should...
    innerperiodic3_t * p0 = new innerperiodic3_t (l, subface (0,0), isRear (0), subface (1,0), isRear (1), this , 0) ;
    innerperiodic3_t * p1 = new innerperiodic3_t (l, subface (0,1), isRear (0), subface (1,1), isRear (1), this , 1) ;
    innerperiodic3_t * p2 = new innerperiodic3_t (l, subface (0,2), isRear (0), subface (1,2), isRear (1), this , 2) ;
    innerperiodic3_t * p3 = new innerperiodic3_t (l, subface (0,3), isRear (0), subface (1,3), isRear (1), this , 3) ;
    alugrid_assert (p0 && p1 && p2 && p3) ;
    p0->append(p1) ;
    p1->append(p2) ;
    p2->append(p3) ;
    _dwn = p0 ;
    _rule = myrule_t::iso4 ;
    p0->_up = p1->_up = p2->_up = p3->_up = this; //us
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
        if(!this->is2d())
        {
          // Das refineImmediate (..) auf allen Fl"achen wird vom periodic3::refine (..)
          // zwar nicht ben"otigt, da schliesslich alle Fl"achen sauber sind, wenn
          // "uberall hface3::refine (..) true geliefert hat, wohl aber z.B. von
          // restore () oder abgeleiteten Funktionen die eine direkte Verfeinerung
          // erzwingen m"ussen und d"urfen.

          typedef typename myhface_t::myrule_t face3rule_t;
          myhface (0)->refineImmediate (face3rule_t (r)) ;
          myhface (1)->refineImmediate (face3rule_t (r)) ;
          split_iso4 () ;
          break ;
        }

        std::cerr << "**ERROR (FATAL) refinement of Periodic3Top didd not work: " ;
        std::cerr << "[" << r << "]. In " << __FILE__ << __LINE__ << std::endl ;
        std::abort () ;
        break ;

      case myrule_t::e01 :
      case myrule_t::e12 :
      case myrule_t::e02 :

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
      if (myhface (opp)->refine (typename myhface_t::myrule_t (r), isRear (opp)))
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
