// (c) Robert Kloefkorn 2010
#include <config.h>

#include "mapp_cube_3d.h"
#include "gitter_hexa_top.h"
#include "gitter_impl.h"

namespace ALUGrid
{

  // #     #                                    #    #######
  // #     #  ######  #####    ####   ######   ##       #      ####   #####
  // #     #  #       #    #  #    #  #       # #       #     #    #  #    #
  // #######  #####   #    #  #       #####     #       #     #    #  #    #
  // #     #  #       #    #  #  ###  #         #       #     #    #  #####
  // #     #  #       #    #  #    #  #         #       #     #    #  #
  // #     #  ######  #####    ####   ######  #####     #      ####   #


  template< class A > void Hedge1Top < A >::refineImmediate (myrule_t r)
  {
    if (r != getrule ()) {
      alugrid_assert (getrule () == myrule_t::nosplit);
      switch (r) {
        case myrule_t::iso2 :
          {
            int l = 1 + level ();
            alugrid_assert ( _inner == 0 );

            innervertex_t* v0 = static_cast<innervertex_t *> (myvertex(0));
            innervertex_t* v1 = static_cast<innervertex_t *> (myvertex(1));
            // get the vertex coordinates
            const alucoord_t (&p0)[3] = v0->Point();
            const alucoord_t (&p1)[3] = v1->Point();

            // the last myvertex(0) is submitted for the indexmanager reference, rk
            _inner = new inner_t (l,
                                  0.5 * (p0[0] + p1[0]),
                                  0.5 * (p0[1] + p1[1]),
                                  0.5 * (p0[2] + p1[2]),
                                  *v0 );
            alugrid_assert (_inner);

            inneredge_t * e0 = new inneredge_t (l, v0 ,    inVx(), 0 );
            inneredge_t * e1 = new inneredge_t (l, inVx(), v1,     1 );

            alugrid_assert (e0 && e1);
            e0->append (e1);
            _inner->store( e0 );
            _rule = myrule_t::iso2;
            break;
          }
        default :
          std::cerr << "**ERROR (fatal): Invalid refinement rule Verfeinerungsregel [" << r << "]" << std::endl;
          abort();
          break;
      }
      this->postRefinement ();
    }
    return;
  }

  template< class A > bool Hedge1Top < A >::coarse ()
  {
    if ( this->leaf () ) return false;
    bool x = true;

    // Der Wert von x bleibt 'true' falls alle Kinder der Kante
    // Bl"atter sind und zudem keine Referenzen auf diese Kanten
    // gesetzt sind. Andernfalls liegt kein vergr"oberungsf"ahiger
    // Knoten vor.
    // Vorsicht: Im parallelen Gitter bleiben auch Kanten ohne
    // Refcount stehen, um konsistente "Uberg"ange zu erhalten.

    for (inneredge_t * edge = dwnPtr(); edge; edge = edge->next ())
    {
      if ( edge->leaf () )
      {
        x &= ! edge->ref;
      }
      else
      {
        x = false;
        edge->coarse ();
      }
    }

    if (x)
    {
      // Falls lockedAgainstCoarsening () aufgerufen 'true' liefert
      // soll die Operation des Vergr"oberns nicht sofort ausgef"uhrt
      // sondern (pending) zur"uckgestellt werden.

      if ( ! this->lockedAgainstCoarsening () )
      {
        delete _inner;
        _inner = 0;
        _rule = myrule_t::nosplit;
      }
    }
    return x;
  }

  // #     #                                 #       #######
  // #     #  ######    ##     ####   ###### #    #     #      ####   #####
  // #     #  #        #  #   #    #  #      #    #     #     #    #  #    #
  // #######  #####   #    #  #       #####  #    #     #     #    #  #    #
  // #     #  #       ######  #       #      #######    #     #    #  #####
  // #     #  #       #    #  #    #  #           #     #     #    #  #
  // #     #  #       #    #   ####   ######      #     #      ####   #


  template< class A >  void Hface4Top < A >::splitISO4 () {
    int l = 1 + level ();
    alugrid_assert ( _inner == 0 );

    {
      // calculate barycenter of face
      innervertex_t* v0 = static_cast<innervertex_t *> (myvertex (0));
      alucoord_t p [3];
      BilinearSurfaceMapping::barycenter(
            v0->Point(),
            myvertex (1)->Point(),
            myvertex (2)->Point(),
            myvertex (3)->Point(),
            p );

      // myvertex(0) is submitted for the indexmanager reference
      _inner = new inner_t (l, p[0], p[1], p[2], *v0 );
      alugrid_assert (_inner);
    }

    myvertex_t * ev0 = myhedge(0)->subvertex (0);
    myvertex_t * ev1 = myhedge(1)->subvertex (0);
    myvertex_t * ev2 = myhedge(2)->subvertex (0);
    myvertex_t * ev3 = myhedge(3)->subvertex (0);
    alugrid_assert (ev0 && ev1 && ev2 && ev3);

    inneredge_t * e0 = new inneredge_t (l, ev0, inVx());
    inneredge_t * e1 = new inneredge_t (l, ev1, inVx());
    inneredge_t * e2 = new inneredge_t (l, ev2, inVx());
    inneredge_t * e3 = new inneredge_t (l, ev3, inVx());

    alugrid_assert ( e0 && e1 && e2 && e3);
    e0->append(e1);
    e1->append(e2);
    e2->append(e3);

    innerface_t * f0 = new innerface_t (l, this->subedge(0,0), twist(0), e0, 0, e3, 1, this->subedge(3,1), twist(3), 0);
    innerface_t * f1 = new innerface_t (l, this->subedge(0,1), twist(0), this->subedge(1,0), twist(1), e1, 0, e0, 1, 1);
    innerface_t * f2 = new innerface_t (l, e1, 1, this->subedge(1,1), twist(1), this->subedge(2,0), twist(2), e2, 0, 2);
    innerface_t * f3 = new innerface_t (l, e3, 0, e2, 1, this->subedge(2,1), twist(2), this->subedge(3,0), twist(3), 3);

    alugrid_assert (f0 && f1 && f2 && f3);
    f0->append(f1);
    f1->append(f2);
    f2->append(f3);
    // inner edge
    _inner->store( e0 );
    // down pointer
    _inner->store( f0 );
    _rule = myrule_t::iso4;
    return;
  }

  //splitISO2 splits edges 03 and 12
  //            1
  //   1|-------|-------|2
  //    |       |       |
  //   0|   0   |   1   |2
  //    |       |       |
  //   0|_______|_______|3
  //            3
  // so we just need to add two inner faces - one inner edges - no inner vertices

  template< class A >  void Hface4Top < A >::splitISO2 () {
    int l = 1 + level ();
    alugrid_assert ( _inner == 0 );


    myvertex_t * ev1 = myhedge(1)->subvertex (0);
    myvertex_t * ev3 = myhedge(3)->subvertex (0);
    alugrid_assert (ev1 && ev3 );

    inneredge_t * e0 = new inneredge_t (l, ev3, ev1);
    alugrid_assert ( e0 );

    //create new InnerStorage with 0 pointer as down pointer
    _inner = new inner_t( 0 , e0);
    alugrid_assert(_inner);

    // level, edge,twist x 4, nChild
    innerface_t * f0 = new innerface_t (l, this->myhedge(0), twist(0), this->subedge(1,0), twist(0), e0, 1, this->subedge(3,1), twist(3), 0);
    innerface_t * f1 = new innerface_t (l, e0, 0, this->subedge(1,1), twist(1), this->myhedge(2), twist(2), this->subedge(3,0), twist(3), 1);

    //std::cout << this << f0 << f1 << std::endl ;

    alugrid_assert (f0 && f1);
    f0->append(f1);

    // down pointer
    _inner->store( f0 );
    _rule = myrule_t::iso4;

    return;
  }



  template< class A > void Hface4Top < A >::refineImmediate (myrule_t r) {
    if (r != getrule ()) {
      alugrid_assert (getrule () == myrule_t::nosplit);
      switch(r) {
        typedef typename myhedge_t::myrule_t myhedgerule_t;
        case myrule_t::iso4 :
        if(this->is2d() )
        {
           // refine such that global indices of new vertices are sequential
          myhedge (3)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (3)));
          myhedge (1)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (1)));
          //  Assert that global index of new vertices are sequential -
          splitISO2 ();
          break;
        }
        myhedge (0)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (0)));
        myhedge (1)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (1)));
        myhedge (2)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (2)));
        myhedge (3)->refineImmediate (myhedgerule_t (myhedge_t::myrule_t::iso2).rotate (twist (3)));
        splitISO4 ();
        break;
      default :
        std::cerr << "ERROR (fatal): Invalid refinement rule [" << r << "]" << std::endl;
        abort();
        break;
      }

      // * higher order, this is a hack
      for (innerface_t* f = down(); f; f = f->next())
      {
        f->nb._parRule = getrule();
      }

      this->postRefinement ();
    }
    return;
  }

  template< class A >
  bool Hface4Top< A >::refine ( myrule_t r, int twist )
  {
    if( r != getrule() )
    {
      if( getrule() != myrule_t::nosplit )
      {
        std::cerr << "ERROR: Trying to apply refinement rule " << r << " on top of rule " << getrule() << std::endl;
        alugrid_assert ( false );
      }

      switch( r )
      {
        case myrule_t::iso4:
        {
          const bool a = (twist < 0)
                  ? this->nb.front ().first->refineBalance (r,this->nb.front ().second)
                  : this->nb.rear  ().first->refineBalance (r,this->nb.rear  ().second);
          if( a )
          {
            if( getrule() == myrule_t::nosplit )
            {
              refineImmediate( r );
              // assign my neighbor info to child faces for initialization
              for( innerface_t *f = dwnPtr(); f; f = f->next() )
                f->nb.assign( this->nb );
            }
            else
              alugrid_assert ( getrule() == myrule_t::iso4 );
          }
          return a;
        }
        default :
          std::cerr << "WARNUNG (ignored): Invalid refinement rule [" << r << "]" << std::endl;
          return false;
      }
    }
    return true;
  }

  template< class A > bool Hface4Top < A >::coarse ()
  {
    innerface_t * f = down();
    if (!f) return false;
    bool x = true;
    do {

    // Falls eine Kind-Fl"ache noch referenziert wird, kann
    // nicht auf diesem Level vergr"obert werden.
    // Daher wird nur die nichtkonforme Nachbarschaft ver-
    // vollst"andigt, die eventuell durch Elementvergr"oberung
    // durcheinander gekommen war. Die Vergr"oberung geht dann
    // auf das n"achste Level "uber.

      if (f->ref) {
        if (f->ref == 1) f->nb.complete (this->nb);
        f->coarse ();
        x = false;
      }
    } while ( (f = f->next()) );

    if (x)
    {
      // Hier wird tats"achlich vergr"obert, d.h. alle Kinder
      // werden beseitigt, und das Bezugsobjekt wird zum neuen
      // Blatt im Baum.

      delete _inner;
      _inner = 0;

      _rule = myrule_t::nosplit;
      {for (int i = 0; i < 4; ++i ) myhedge (i)->coarse (); }
    }
    return x;
  }

  // #     #                         #       #######
  // #     #  #####   #    #  #####  #    #     #      ####   #####
  // #     #  #    #  ##   #  #    # #    #     #     #    #  #    #
  // #######  #####   # #  #  #    # #    #     #     #    #  #    #
  // #     #  #    #  #  # #  #    # #######    #     #    #  #####
  // #     #  #    #  #   ##  #    #      #     #     #    #  #
  // #     #  #####   #    #  #####       #     #      ####   #

  template< class A > bool Hbnd4Top < A >::coarse () {
    innerbndseg_t * b = down ();
    if (!b) return false;
    bool x = true;
    do {
      if(b->myhface4(0)->ref > 1) (b->coarse (), x = false);
    } while ( (b = b->next()) );
    if (x) {
      if (! this->lockedAgainstCoarsening ()) {

        this->preCoarsening ();
        this->coarseGhost();

        delete _dwn; _dwn = 0;
        myhface4 (0)->coarse ();
      }
    }
    return x;
  }

  template< class A >  bool Hbnd4Top < A >::bndNotifyCoarsen () {
    return coarse ();
  }

  template< class A >  void Hbnd4Top < A >::splitISO4 () {
    int l = 1 + level ();
    alugrid_assert (_dwn == 0);

    // refine ghost element and fill ghost info
    typedef typename Gitter::GhostChildrenInfo GhostChildrenInfo;
    GhostChildrenInfo ghostInfo;
    // ghostInfo is filled by splitGhost, see gitter_hexa_top_pll.h
    this->splitGhost( ghostInfo );

    innerbndseg_t * b0 = new innerbndseg_t (l, subface (0,0), twist (0), this, ghostInfo.child(0), ghostInfo.face(0));
    innerbndseg_t * b1 = new innerbndseg_t (l, subface (0,1), twist (0), this, ghostInfo.child(1), ghostInfo.face(1));
    innerbndseg_t * b2 = new innerbndseg_t (l, subface (0,2), twist (0), this, ghostInfo.child(2), ghostInfo.face(2));
    innerbndseg_t * b3 = new innerbndseg_t (l, subface (0,3), twist (0), this, ghostInfo.child(3), ghostInfo.face(3));


    alugrid_assert (b0 && b1 && b2 && b3);
    b0->append(b1);
    b1->append(b2);
    b2->append(b3);
    _dwn = b0;
    return;
  }

  //split ISO2 is only necessary for the 2d fake grid
  template< class A >  void Hbnd4Top < A >::splitISO2 () {
    int l = 1 + level ();
    alugrid_assert (_dwn == 0);

    // refine ghost element and fill ghost info
    typedef typename Gitter::GhostChildrenInfo GhostChildrenInfo;
    GhostChildrenInfo ghostInfo;
    // ghostInfo is filled by splitGhost, see gitter_hexa_top_pll.h
    // ghostInfo should do the right thing if invoked with ISO4 of hexaTop
    // TODO: Check that
    this->splitGhost( ghostInfo );

    //TODO: subface is maybe wrong....
    innerbndseg_t * b0 = new innerbndseg_t (l, subface (0,0), twist (0), this, ghostInfo.child(0), ghostInfo.face(0));
    innerbndseg_t * b1 = new innerbndseg_t (l, subface (0,1), twist (0), this, ghostInfo.child(1), ghostInfo.face(1));

    alugrid_assert (b0 && b1 );
    b0->append(b1);
    _dwn = b0;
    return;
  }


  template< class A >  bool Hbnd4Top < A >::refineBalance (balrule_t r, int b)
  {
    // Die Methode refineBalance () f"uhrt auf dem Randabschluss entweder
    // unbedingt die Verfeinerung durch, da im Verlauf der Verfeinerung keine
    // weiteren Anforerungen mehr an den Randabschluss  gerichtet werden
    // ODER gibt die Verfeinerung als nicht erf"ullt zur"uck: Dann liegt
    // es am Aufrufer die Verfeinerung nochmals anzuforern.

    alugrid_assert (b == 0);
    alugrid_assert (this->leaf ());
    if (! bndNotifyBalance (r,b))
    {
      // Hier kann der innere Rand [parallel] die Verfeinerung
      // verhindern, damit z.B. das Durchverfeinern im anisotropen
      // Fall erstmal nicht stattfindet, wenn nicht klar ist, wie die
      // weitere Rekursion aussieht. Dazu muss auf dem Niveau der Klasse
      // des Template-Arguments die Methode bndNotifyBalance () "uber-
      // schrieben werden. Die Defaultmethode liefert immer 'true'.

      return false;
    }
    else
    {
      if(r == myrule_t::iso4)
      {
        // Der Rand verfeinert unbedingt die anliegende Fl"ache und dann
        // sich selbst, weil die Anforderung durch die Fl"ache kam, und
        // dahinter keine Balancierung stattfinden muss.

        myhface4_t& face (*(myhface4(0)));
        // refine face
        face.refineImmediate (r);
        // refine myself
        if( face.is2d() )
          splitISO2();
        else
          splitISO4();
      }
      else
      {
        std::cerr << "ERROR (fatal): Cannot apply refinement rule " << r << " on boundary segment." << std::endl;
        abort();
      }

      // postRefinement () gibt die M"oglichkeit auf dem Niveau des
      // Template-Arguments eine Methode aufzurufen, um eventuelle
      // Operationen auf dem verfeinerten Randst"uck aufzurufen.

      this->postRefinement();
      return true;
    }
  }

  template< class A >  bool Hbnd4Top < A >::refineLikeElement (balrule_t r)
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

    if( r == myrule_t::nosplit )
    {
      std::cerr << "WARNING (ignored): Cannot apply refinement rule 'nosplit'." << std::endl;

      // Eine Anforderung mit nosplit zu Verfeinern nur erf"ullt,
      // falls die zugeh"orige Fl"achenregel auch nosplit ist, sonst
      // wird die Anforderung als nicht erf"ullt zur"uckgegeben.

      return (this->getrule() == balrule_t::nosplit);
    }
    else
    {
      if( this->getrule() == r )
      {
        // Alles schon wie es sein soll -> true.
        return true;
      }
      else
      {
        // Der nachfolgende Test bezieht sich auf die Verfeinerungssituation
        // der Fl"ache, da getrule () auf myhface4 (0)->getrule () umgeleitet
        // ist.

        alugrid_assert (this->getrule () == myrule_t::nosplit);
        switch (r)
        {
        case balrule_t::iso4 :
          {
            myhface4_t &face (*(myhface4 (0)));
            if (! face.refine(balrule_t (balrule_t::iso4).rotate (twist (0)), twist (0))) return false;

            // call refinement method
            if( face.is2d())
              splitISO2 ();
            else
              splitISO4 ();

            // postRefinement () gibt die M"oglichkeit auf dem Niveau des
            // Template-Arguments eine Methode aufzurufen, um eventuelle
            // Operationen auf dem verfeinerten Randst"uck aufzurufen.
            this->postRefinement ();

            return true;
          }

        default:
          std::cerr << "WARNING (ignored): Invalid refinement rule [" << r << "]." << std::endl;
          return false;
        }
      }
    }
  }

  template< class A >
  void Hbnd4Top< A >::restoreFollowFace ()
  {
    // retoreFollowFace () veranlasst das Randelement sich am
    // bestehenden Fl"achenbaum wiederherzustellen durch die
    // entsprechende Verfeinerung.

    myhface4_t & face (*(myhface4 (0)));
    if (! face.leaf ())
    {
      balrule_t r = face.getrule ();
      switch (r)
      {
      case myrule_t::iso4:
        if( face.is2d())
          splitISO2 ();
        else
          splitISO4 ();
        break;

      default:
        std::cerr << "ERROR (fatal): Cannot apply refinement rule " << r << " on boundary segment." << std::endl;
        abort();
        break;
      }

      // do post refinement
      this->postRefinement();
      for( innerbndseg_t *b = down(); b; b = b->next() )
        b->restoreFollowFace();
    }
  }

  // #     #                         #######
  // #     #  ######  #    #    ##      #      ####   #####
  // #     #  #        #  #    #  #     #     #    #  #    #
  // #######  #####     ##    #    #    #     #    #  #    #
  // #     #  #         ##    ######    #     #    #  #####
  // #     #  #        #  #   #    #    #     #    #  #
  // #     #  ######  #    #  #    #    #      ####   #

  // constructor for macro elements
  template< class A > HexaTop < A >
  :: HexaTop (int l, myhface4_t * f0, int t0, myhface4_t * f1, int t1,
              myhface4_t * f2, int t2, myhface4_t * f3, int t3, myhface4_t * f4,
              int t4, myhface4_t * f5, int t5)
    : A (f0, t0, f1, t1, f2, t2, f3, t3, f4, t4, f5, t5 )
    , _bbb (0), _up(0)
    , _inner( 0 )
    , _volume (0.0)
    , _lvl (l)
    , _nChild(0)
    , _rule (myrule_t::nosplit), _req (myrule_t::nosplit)
  {
    TrilinearMapping trMap (myvertex(0)->Point(), myvertex(1)->Point(),
                            myvertex(2)->Point(), myvertex(3)->Point(),
                            myvertex(4)->Point(), myvertex(5)->Point(),
                            myvertex(6)->Point(), myvertex(7)->Point());
    // calculate volume
    _volume = QuadraturCube3D < VolumeCalc > (trMap).integrate2 (0.0);

    // check whether mapping is affine
    if( ! trMap.affine() )
      this->setNonAffineGeometry();

#ifdef ALUGRIDDEBUG
      // make sure determinant is ok
      alucoord_t point[3] = { 0.0, 0.0, 0.0 };
      alugrid_assert ( trMap.det( point ) > 0 );
#endif

    alugrid_assert ( this->level() == l );

    this->setIndexAnd2dFlag( indexManager() );

    return;
  }

  // constructor for refinement
  template< class A > HexaTop < A >
  :: HexaTop (int l, myhface4_t * f0, int t0, myhface4_t * f1, int t1,
              myhface4_t * f2, int t2, myhface4_t * f3, int t3, myhface4_t * f4,
              int t4, myhface4_t * f5, int t5, innerhexa_t * up , int nChild , double vol )
    : A (f0, t0, f1, t1, f2, t2, f3, t3, f4, t4, f5, t5 )
    , _bbb (0), _up(up)
    , _inner( 0 )
    , _volume ( vol )
    , _lvl (l)
    , _nChild(nChild)
    , _rule (myrule_t::nosplit), _req (myrule_t::nosplit)
  {
    alugrid_assert ( this->level() == l );

    this->setIndexAnd2dFlag( indexManager() );

    // set bndid to fathers bndid now
    this->_bndid = _up->bndId();

    // if mapping is not affine recalculate volume
    if( ! _up->affineGeometry() )
    {
      TrilinearMapping triMap (myvertex(0)->Point(),
                               myvertex(1)->Point(),
                               myvertex(2)->Point(),
                               myvertex(3)->Point(),
                               myvertex(4)->Point(),
                               myvertex(5)->Point(),
                               myvertex(6)->Point(),
                               myvertex(7)->Point() );

#ifdef ALUGRIDDEBUG
      // make sure determinant is ok
      alucoord_t point[3] = { 0.0, 0.0, 0.0 };
      alugrid_assert ( triMap.det( point ) > 0 );
#endif

      // calculate volume
      _volume = QuadraturCube3D < VolumeCalc > (triMap).integrate2 (0.0);
      // make as non-affine geometry
      this->setNonAffineGeometry();
    }

    // make sure that given volume is the same as calulated
#ifdef ALUGRIDDEBUG
    const double calculatedVolume =
        QuadraturCube3D < VolumeCalc >
         (TrilinearMapping (myvertex(0)->Point(), myvertex(1)->Point(),
                            myvertex(2)->Point(), myvertex(3)->Point(),
                            myvertex(4)->Point(), myvertex(5)->Point(),
                            myvertex(6)->Point(), myvertex(7)->Point())).integrate2 (0.0);
     alugrid_assert ( std::abs( calculatedVolume - _volume ) / _volume  < 1e-10 );
#endif

    return;
  }

  template< class A > HexaTop < A >::~HexaTop ()
  {
    this->freeIndex( indexManager() );

    if (! _inner ) this->detachleafs();
    else alugrid_assert (!this->isLeafEntity());
    if (_bbb) delete _bbb;
    if (_inner) delete _inner;
    return;
  }

  //for regular 2d we use the subedge routine for the faces 0,1
  template< class A >  typename HexaTop < A >::myhedge_t * HexaTop < A >::subedge (int i, int j) {
    return (j < 4) ? ((twist (i) < 0) ? myhface4 (i)->myhedge ((8 - j + twist (i)) % 4) :
      myhface4 (i)->myhedge ((j + twist (i)) % 4)) :
      ((twist (i) < 0) ? myhface4 (i)->subedge ((12 - j + twist (i)) % 4) :
      myhface4 (i)->subedge ((j + twist (i)) % 4));
  }

  template< class A >  const typename HexaTop < A >::myhedge_t * HexaTop < A >::subedge (int i, int j) const {
    return (j < 4) ? ((twist (i) < 0) ? myhface4 (i)->myhedge ((8 - j + twist (i)) % 4) :
        myhface4 (i)->myhedge ((j + twist (i)) % 4)) :
    ((twist (i) < 0) ? myhface4 (i)->subedge ((12 - j + twist (i)) % 4) :
    myhface4 (i)->subedge ((j + twist (i)) % 4));
  }

  // subface routine for regular 2d has to use twist
  template< class A >  typename HexaTop < A >::myhface4_t * HexaTop < A >::subface (int i, int j)
  {
    typedef typename myhface4_t::myrule_t  facerule_t ;
    myhface4_t * face = myhface4(i);
    const facerule_t facerule = face->getrule();
    return ( facerule == facerule_t::iso4 ) ?
            ( face->is2d() ) ?
              ((twist(i) < 0) ? face->subface((j+1)%2): face->subface(j) )
              : face->subface(twist(i) < 0 ? (9 - j + twist(i)) % 4 : (j + twist(i)) % 4) :
              (abort (), (myhface4_t *)0);
  }

  //check subface routine  for regular 2d has to use twist
  template< class A >  const typename HexaTop < A >::myhface4_t * HexaTop < A >::subface (int i, int j) const {
    typedef typename myhface4_t::myrule_t  facerule_t ;
    const myhface4_t * face = myhface4(i);
    const facerule_t facerule = face->getrule();
    return ( facerule == facerule_t::iso4 ) ?
            ( face->is2d() ) ?
              ((twist(i) < 0) ? face->subface((j+1)%2): face->subface(j) )
              : face->subface(twist(i) < 0 ? (9 - j + twist(i)) % 4 : (j + twist(i)) % 4) :
              (abort (), (myhface4_t *)0);
  }

  template< class A > void HexaTop < A >::splitISO8 ()
  {
    int l = 1 + this->level ();

    alugrid_assert (_inner == 0 );
    {
      alucoord_t p[3];
      // calculate barycenter
      TrilinearMapping::barycenter(
          myvertex(0)->Point(), myvertex(1)->Point(),
          myvertex(2)->Point(), myvertex(3)->Point(), myvertex(4)->Point(),
          myvertex(5)->Point(), myvertex(6)->Point(), myvertex(7)->Point(),
          p);

      innervertex_t* v0 = static_cast<innervertex_t *> (myvertex (0));
      _inner = new inner_t (l, p[0], p[1], p[2], *v0 );
      alugrid_assert (_inner);
    }

    myvertex_t * fv0 = myhface4 (0)->subvertex (0);
    myvertex_t * fv1 = myhface4 (1)->subvertex (0);
    myvertex_t * fv2 = myhface4 (2)->subvertex (0);
    myvertex_t * fv3 = myhface4 (3)->subvertex (0);
    myvertex_t * fv4 = myhface4 (4)->subvertex (0);
    myvertex_t * fv5 = myhface4 (5)->subvertex (0);
    alugrid_assert (fv0 && fv1 && fv2 && fv3 && fv4 && fv5);

    inneredge_t * e0 = new inneredge_t (l, fv0, inVx());
    inneredge_t * e1 = new inneredge_t (l, fv1, inVx());
    inneredge_t * e2 = new inneredge_t (l, fv2, inVx());
    inneredge_t * e3 = new inneredge_t (l, fv3, inVx());
    inneredge_t * e4 = new inneredge_t (l, fv4, inVx());
    inneredge_t * e5 = new inneredge_t (l, fv5, inVx());

    alugrid_assert (e0 && e1 && e2 && e3 && e4 && e5);
    e0->append(e1);
    e1->append(e2);
    e2->append(e3);
    e3->append(e4);
    e4->append(e5);

    innerface_t * f0 = new innerface_t (l, this->subedge (2, 7), 0, e2, 0, e5, 1, this->subedge (5, 4), 1);
    innerface_t * f1 = new innerface_t (l, this->subedge(2, 5), 1, this->subedge (3, 7), 0, e3, 0, e2, 1);
    innerface_t * f2 = new innerface_t (l, e3, 1, this->subedge (3, 5), 1, this->subedge (4, 7), 0, e4, 0 );
    innerface_t * f3 = new innerface_t (l, e5, 0, e4, 1, this->subedge (4, 5), 1, this->subedge (5, 6), 0 );
    innerface_t * f4 = new innerface_t (l, this->subedge (0, 7), 0, e0, 0, e2, 1, this->subedge (2, 4), 1 );
    innerface_t * f5 = new innerface_t (l, this->subedge (0, 5), 1, this->subedge (4, 4), 0, e4, 0, e0, 1 );
    innerface_t * f6 = new innerface_t (l, e4, 1, this->subedge (4, 6), 1, this->subedge (1, 6), 0, e1, 0 );
    innerface_t * f7 = new innerface_t (l, e2, 0, e1, 1, this->subedge (1, 4), 1, this->subedge (2, 6), 0 );
    innerface_t * f8 = new innerface_t (l, this->subedge (0, 4), 0, e0, 0, e5, 1, this->subedge (5, 7), 1 );
    innerface_t * f9 = new innerface_t (l, this->subedge (0, 6), 1, this->subedge (3, 4), 0, e3, 0, e0, 1 );
    innerface_t * f10 = new innerface_t (l, e3, 1, this->subedge (3, 6), 1, this->subedge (1, 5), 0, e1, 0 );
    innerface_t * f11 = new innerface_t (l, e5, 0, e1, 1, this->subedge (1, 7), 1, this->subedge (5, 5), 0 );

    alugrid_assert (f0 && f1 && f2 && f3 && f4 && f5 && f6 && f7 && f8 && f9 && f10 && f11);
    f0->append(f1);
    f1->append(f2);
    f2->append(f3);
    f3->append(f4);
    f4->append(f5);
    f5->append(f6);
    f6->append(f7);
    f7->append(f8);
    f8->append(f9);
    f9->append(f10);
    f10->append(f11);

    // calculate child volume which is volume divided by 8
    double childVolume = 0.125 * _volume;

    // only check for affine faces
    // for other it does not matter
    if( this->affineGeometry() )
    {
      // if vertex projection is available
      // then set affine to false to invoke volume calculation
      for( int i=0; i<6; ++i)
      {
        if( this->myneighbour( i ).first->hasVertexProjection() )
        {
          this->setNonAffineGeometry();
          break;
        }
      }
    }

    innerhexa_t * h0 = new innerhexa_t (l, subface (0, 0), twist (0), f0, 0, subface (2, 0), twist (2), f4, 0, f8, -4, subface (5, 0), twist (5) , this, 0, childVolume);
    innerhexa_t * h1 = new innerhexa_t (l, subface (0, 3), twist (0), f1, 0, subface (2, 1), twist (2), subface (3, 0), twist (3), f9, -4, f4, -1, this, 1, childVolume);
    innerhexa_t * h2 = new innerhexa_t (l, subface (0, 2), twist (0), f2, 0,f9, 0, subface (3, 1), twist (3), subface (4, 0), twist (4), f5, -1        , this, 2, childVolume);
    innerhexa_t * h3 = new innerhexa_t (l, subface (0, 1), twist (0), f3, 0, f8, 0, f5, 0, subface(4, 1), twist (4), subface(5, 3), twist (5)    , this, 3, childVolume);
    innerhexa_t * h4 = new innerhexa_t (l, f0, -1, subface(1, 0), twist (1), subface(2, 3), twist (2), f7, 0, f11, -4, subface(5, 1), twist (5)  , this, 4, childVolume);
    innerhexa_t * h5 = new innerhexa_t (l, f1, -1, subface(1, 1), twist (1), subface(2, 2), twist (2), subface(3, 3), twist (3), f10, -4, f7, -1 , this, 5, childVolume);
    innerhexa_t * h6 = new innerhexa_t (l, f2, -1, subface(1, 2), twist (1), f10, 0, subface(3, 2), twist (3), subface(4, 3), twist (4), f6, -1  , this, 6, childVolume);
    innerhexa_t * h7 = new innerhexa_t (l, f3, -1, subface(1, 3), twist (1), f11, 0, f6, 0, subface(4, 2), twist (4), subface(5, 2), twist (5)   , this, 7, childVolume);

    alugrid_assert (h0 && h1 && h2 && h3 && h4 && h5 && h6 && h7);
    h0->append(h1);
    h1->append(h2);
    h2->append(h3);
    h3->append(h4);
    h4->append(h5);
    h5->append(h6);
    h6->append(h7);


    // inner edge
    _inner->store( e0 );
    // inne face
    _inner->store( f0 );
    // down ptr
    _inner->store( h0 );
    _rule = myrule_t::regular ;
    this->detachleafs();
    return;
  }

  //splitISO4 for the fake 2d grid
  template< class A > void HexaTop < A >::splitISO4 ()
  {
    int l = 1 + this->level ();

    alugrid_assert (_inner == 0 );

    //the only subvertices we need are from the top and bottom face
    myvertex_t * fv4 = myhface4 (0)->subvertex (0);
    myvertex_t * fv5 = myhface4 (1)->subvertex (0);
    alugrid_assert ( fv4 && fv5);

    inneredge_t * e0 = new inneredge_t (l, fv5, fv4);

    alugrid_assert ( e0 );

    //create new InnerStorage with 0 pointer as down pointer and 0 as face pointer
    _inner = new inner_t( 0 , 0, e0);
    alugrid_assert(_inner);

    // we just need four inner Faces
    // always the inner edge + the subedge of top and bottom  + one subedge of a side face
    // the top face is rotated in the other direction as the bottom face, so we need the subedges in the descending order.
    // inner face 0 at face 2
    innerface_t * f0 = new innerface_t (l, this->myhface4(2)->subedge(0), 0, this->subedge(1,4), 0, e0, 0, this->subedge(0,7), 1);
    // inner face 1 at face 3
    innerface_t * f1 = new innerface_t (l, this->myhface4(3)->subedge(0), 0, this->subedge(1,5), 0, e0, 0, this->subedge(0,6), 1);
    //inner face 2 at face 4
    innerface_t * f2 = new innerface_t (l, this->myhface4(4)->subedge(0), 0, this->subedge(1,6), 0, e0, 0, this->subedge(0,5), 1);
    // inner face 3 at face 5
    innerface_t * f3 = new innerface_t (l, this->myhface4(5)->subedge(0), 0, this->subedge(1,7), 0, e0, 0, this->subedge(0,4), 1);

    alugrid_assert (f0 && f1 && f2 && f3 );
    f0->append(f1);
    f1->append(f2);
    f2->append(f3);


    // calculate child volume which is volume divided by 4
    double childVolume = 0.25 * _volume;

    // only check for affine faces
    // for other it does not matter
    if( this->affineGeometry() )
    {
      // if vertex projection is available
      // then set affine to false to invoke volume calculation
      for( int i=0; i<6; ++i)
      {
        if( this->myneighbour( i ).first->hasVertexProjection() )
        {
          this->setNonAffineGeometry();
          break;
        }
      }
    }

    //4 inner hexas

    innerhexa_t * h0 = new innerhexa_t (l, subface (0, 0), twist(0), subface(1,0), twist(1), subface (2, 1), twist(2), f0, -1, f3, 3,  subface (5, 0), twist(5),  this, 0, childVolume);
    innerhexa_t * h1 = new innerhexa_t (l, subface (0, 3), twist(0), subface(1,1), twist(1), subface (2, 0), twist(2), subface (3, 1), twist(3),  f1, -1, f0, 0, this, 1, childVolume);
    innerhexa_t * h2 = new innerhexa_t (l, subface (0, 2), twist (0), subface(1,2), twist(1), f1, 3, subface (3, 0), twist (3), subface (4, 1), twist (4), f2, -2 , this, 2, childVolume);
    innerhexa_t * h3 = new innerhexa_t (l, subface (0, 1), twist (0), subface(1,3), twist(1), f3, -1, f2, 3, subface (4, 0), twist(4), subface(5,1), twist(5),   this, 3, childVolume);

    alugrid_assert (h0 && h1 && h2 && h3 );

    //this check produces output, when loadbalancing and refining the transported elements, because the ghost neighbours do not exist yet
    //alugrid_assert( checkHexa( h0, 0 ) );
    //alugrid_assert( checkHexa( h1, 1 ) );
    //alugrid_assert( checkHexa( h2, 2 ) );
    //alugrid_assert( checkHexa( h3, 3 ) );

    h0->append(h1);
    h1->append(h2);
    h2->append(h3);

    // inner face
    _inner->store( f0 );
    // down ptr
    _inner->store( h0 );
    _rule = myrule_t::regular;
    this->detachleafs();
    return;
  }



  template< class A > void HexaTop < A >::refineImmediate (myrule_t r)
  {
    // Das refineImmediate (..) auf allen Fl"achen wird vom Hexa::refine (..)
    // zwar nicht ben"otigt, da schliesslich alle Fl"achen sauber sind wenn
    // "uberall hface4::refine (..) true geliefert hat, wohl aber z.B. von
    // restore () oder abgeleiteten Funktionen die eine direkte Verfeinerung
    // erzwingen m"ussen und d"urfen.

    alugrid_assert ( getrule() == myrule_t::nosplit );
    alugrid_assert ( r == myrule_t::regular );

      typedef typename myhface4_t::myrule_t myhface4rule_t;
      for( int i = 0; i < 6; ++i )
        myhface4 (i)->refineImmediate (myhface4rule_t (myhface4_t::myrule_t::iso4).rotate (twist (i)));

      if(this->is2d())
        splitISO4 ();
      else
        splitISO8();

    // call post refinement method (default is empty)
    this->postRefinement();
  }

  template< class A > bool HexaTop < A >::refine ()
  {
    myrule_t r = _req;
    if( (r != myrule_t::crs) && (r != myrule_t::nosplit) )
    {
      if( r != getrule() )
      {
        alugrid_assert ( getrule() == myrule_t::nosplit );
        _req = myrule_t::nosplit;
        switch( r )
        {
        case myrule_t::crs:
        case myrule_t::nosplit:
          return true;

        case myrule_t::regular :

         typedef typename myhface4_t::myrule_t myhface4rule_t;
         for( int i = 0; i < 6; ++i )
           if( !myhface4( i )->refine( myhface4rule_t( myhface4_t::myrule_t::iso4 ).rotate( twist( i ) ), twist( i ) ) )
             return false;
         refineImmediate( r );
         return true;

        default:
          std::cerr << "WARNING (ignored): Invalid refinement rule [" << getrule() << "]." << std::endl;
          return false;
        }
      }
    }
    return true;
  }

  template< class A > bool HexaTop < A >::refineBalance (balrule_t r, int fce)
  {
    alugrid_assert (r == balrule_t::iso4 );
    if (getrule () == myrule_t::nosplit)
    {
      if (! myhface4 (fce)->leaf ())
      {
        switch( r )
        {
        case balrule_t::iso4:
          for (int i = 0; i < 6; ++i)
          {
            if (i != fce)
              if (!myhface4 (i)->refine (balrule_t (balrule_t::iso4).rotate (twist (i)), twist (i)))
                return false;
          }
          _req = myrule_t::nosplit;
          refineImmediate ( myrule_t::regular );
          break;
        default:
          std::cerr << "WARNING (ignored): Invalid refinement rule [" << getrule() << "]." << std::endl;
          return false;
        }
      }
    }
    return true;
  }

  template< class A > bool HexaTop < A >::coarse () {
    if (this->leaf ())
    {
      alugrid_assert (_req == myrule_t::nosplit || _req == myrule_t::crs);
      myrule_t w = _req;
      _req = myrule_t::nosplit;
      if (w != myrule_t::crs)
      {
        return false;
      }
      for (int i = 0; i < 6; ++i)
      {
        if (!myhface4 (i)->leaf ()) return false;
      }
      return true;
    }
    else
    {
      alugrid_assert (_req == myrule_t::nosplit);
      bool x = true;
      {
        for (innerhexa_t * h = dwnPtr(); h; h = h->next ()) x &= h->coarse ();
      }
      if (x)
      {
        this->preCoarsening ();
        this->attachleafs();
        delete _inner;
        _inner = 0;

        _rule = myrule_t::nosplit;
        {
          for (int i = 0; i < 6; ++i)
          {
            this->myneighbour (i).first->bndNotifyCoarsen ();
            myhface4 (i)->coarse ();
          }
        }
        return false;
      }
    }
    return false;
  }

  template< class A >
  bool HexaTop< A >::bndNotifyCoarsen () { return true; }

  template< class A >
  void HexaTop< A >::backupIndex ( std::ostream &os ) const
  {
    this->doBackupIndex( os );
    for (const innerhexa_t* c = down(); c; c = c->next())
      c->backupIndex( os );
  }

  template< class A >
  void HexaTop < A >::backupIndex (ObjectStream& os) const
  {
    this->doBackupIndex( os );
    for (const innerhexa_t* c = down(); c; c = c->next())
      c->backupIndex(os);
  }

  template< class A >
  int HexaTop< A >::backup ( std::ostream &os ) const
  {
    return doBackup( os );
  }

  template< class A > int HexaTop < A >::backup (ObjectStream& os) const
  {
    return doBackup( os );
  }

  template< class A > template<class OutStream_t>
  int HexaTop < A >::doBackup (OutStream_t& os) const
  {
    os.put ((char) getrule ());
    for (const inneredge_t * e = innerHedge (); e; e = e->next ()) e->backup (os);
    for (const innerface_t * f = innerHface (); f; f = f->next ()) f->backup (os);
    int sons = 1 ;
    for (const innerhexa_t * c = dwnPtr(); c; c = c->next () )
    {
      sons += c->backup (os);
    }
    return sons;
  }

  template< class A >
  template< class istream_t >
  void HexaTop < A >::
  restoreIndexImpl (istream_t & is, RestoreInfo& restoreInfo)
  {
    // free index from constructor
    // indexManager is cleared from outside
    // mark this element a non hole
    typedef typename Gitter::Geometric::BuilderIF BuilderIF;

    this->doRestoreIndex( is, restoreInfo, BuilderIF::IM_Elements );

    for (innerhexa_t * c = dwnPtr(); c; c = c->next ())
    {
      c->restoreIndex (is, restoreInfo );
    }
  }

  template< class A >
  void HexaTop< A >::restoreIndex ( std::istream &is, RestoreInfo &restoreInfo )
  {
    restoreIndexImpl( is, restoreInfo );
  }

  template< class A > void HexaTop < A >::
  restoreIndex (ObjectStream& is, RestoreInfo& restoreInfo)
  {
    restoreIndexImpl( is, restoreInfo );
  }

  template< class A >
  void HexaTop< A >::restore ( std::istream &is )
  {
    doRestore( is );
  }

  template< class A > void HexaTop < A >::restore (ObjectStream& is)
  {
    doRestore( is );
  }

  template< class A > template<class InStream_t>
  void HexaTop < A >::doRestore (InStream_t & is)
  {
    // restore () stellt den Elmentbaum aus der Verfeinerungs
    // geschichte wieder her. Es ruft refine () auf und testet
    // auf den korrekten Vollzug der Verfeinerung. Danach werden
    // die inneren Gitterteile restore'd.

    myrule_t r ((char) is.get ());
    alugrid_assert (getrule() == myrule_t::nosplit);
    if (r == myrule_t::nosplit)
    {

    // Vorsicht: beim restore m"ussen sich sowohl Element als auch
    // Randelement um die Korrektheit der Nachbarschaft k"ummern,
    // und zwar dann wenn sie "on the top" sind (= die gelesene
    // Verfeinerungsregel ist nosplit). (s.a. beim Randelement)



      for (int i = 0; i < 6; i ++) {
        myhface4_t & f (*(myhface4 (i)));
        if (!f.leaf ()) {
          //For 2d we have to take into account that the number of subfaces is just 2
          if(f.is2d())
            for (int j = 0; j < 2; j ++) f.subface (j)->nb.complete (f.nb);
          else
            for (int j = 0; j < 4; j ++) f.subface (j)->nb.complete (f.nb);
        }
      }
    }
    else
    {
      request (r);
      refine ();
      alugrid_assert (getrule() == r);
      {for (inneredge_t * e = innerHedge (); e; e = e->next ()) e->restore (is); }
      {for (innerface_t * f = innerHface (); f; f = f->next ()) f->restore (is); }
      {for (innerhexa_t * c = dwnPtr (); c; c = c->next ()) c->restore (is); }
    }
    return;
  }

  //Methods for checkhexa

  template< class A >  int
  HexaTop < A >::vertexTwist( const int twst, const int vx ) const
  {
    return (twst< 0) ?   (9 - vx + twst) % 4 : (vx + twst) % 4;
  }

  template< class A > int
  HexaTop < A >::calculateFace2Twist( const int vxIndex, const myhface4_t* subFace ) const
  {
    const int faceIndices[ 3 ] = { subFace->myvertex( 0 )->getIndex(),
                                   subFace->myvertex( 1 )->getIndex(),
                                   subFace->myvertex( 2 )->getIndex() };

    for(int twst = -4; twst<4; ++twst )
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
  HexaTop < A >::calculateFace3Twist( const int (&vx)[4], const myhface4_t* subFace, const int thirdVx ) const
  {
    //std::cout << "check v0 = " << vx[0] << " v1 = " << vx[1] << " v2 = " << vx[2] << " v3 = " << vx[3] << std::endl;

    const int faceIndices[ 4 ] = { subFace->myvertex( 0 )->getIndex(),
                                   subFace->myvertex( 1 )->getIndex(),
                                   subFace->myvertex( 2 )->getIndex(),
                                   subFace->myvertex( 3 )->getIndex() };
    //std::cout << faceIndices[0] << " " << faceIndices[1] << " " << faceIndices[2] << " " << faceIndices[3] << std::endl;

    for(int twst = -4; twst<4; ++twst )
    {
      // if vx0 and vx1 match we are done
      if( vx[ 0 ] == faceIndices[ vertexTwist(twst, 0 ) ] &&
          vx[ 1 ] == faceIndices[ vertexTwist(twst, 1 ) ] &&
          vx[ 2 ] == faceIndices[ vertexTwist(twst, 2 ) ] )
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
  HexaTop < A >::checkHexa( const innerhexa_t *hexa, const int nChild ) const
  {
    // make sure face twists are ok
    bool twistOk = true ;

    std::set< int > verticesFound;
    alugrid_assert ( hexa->nChild() == nChild );

    const bool isGhost = hexa->isGhost();
    for(int fce=0; fce<6; ++fce )
    {
      for(int i=0; i<4; ++i )
      {
        verticesFound.insert( hexa->myvertex( fce, i )->getIndex() );
      }

      for(int i=0; i<4; ++i )
      {
        verticesFound.insert( hexa->myvertex( fce, i )->getIndex() );
        // use proto type to check face twists
        if( hexa->myvertex( Gitter::Geometric::Hexa::prototype[ fce ][ i ] )
              != hexa->myvertex( fce, i ) )
        {
          const int vx0 = Gitter::Geometric::Hexa::prototype[ fce ][ 0 ] ;
          const int vx1 = Gitter::Geometric::Hexa::prototype[ fce ][ 1 ] ;
          const int vx2 = Gitter::Geometric::Hexa::prototype[ fce ][ 2 ] ;
          const int vx3 = Gitter::Geometric::Hexa::prototype[ fce ][ 3 ] ;

          const int vx[4] = { hexa->myvertex( vx0 )->getIndex(),
                              hexa->myvertex( vx1 )->getIndex(),
                              hexa->myvertex( vx2 )->getIndex(),
                              hexa->myvertex( vx3 )->getIndex()
                            };

          int twst = calculateFace3Twist( vx, hexa->myhface4( fce ), 2 );
          std::cout << "Twist of face" << fce <<" is wrong, it should be " << twst << std::endl;
          twistOk = false ;
          continue ;
        }
      }

      if( ! isGhost && ! hexa->myneighbour( fce ).first->isRealObject()  )
      {
        std::cout << "Neighbour(type="<<hexa->isInterior() << ") " << fce << " of Hexa " << hexa->getIndex()  << " is wrong " << std::endl;
        std::cout << "Check face " << hexa->myhface4( fce ) ;
      }
      // make sure neighbor is something meaningful
      //alugrid_assert ( hexa->myneighbour( fce ).first->isRealObject() );

    }

    // make sure we have only 8 different vertices
    alugrid_assert ( verticesFound.size() == 8 );

    return twistOk;
  }


  // ######                                                          #       #######
  // #     #  ######  #####      #     ####   #####      #     ####  #    #     #
  // #     #  #       #    #     #    #    #  #    #     #    #    # #    #     #
  // ######   #####   #    #     #    #    #  #    #     #    #      #    #     #
  // #        #       #####      #    #    #  #    #     #    #      #######    #
  // #        #       #   #      #    #    #  #    #     #    #    #      #     #
  // #        ######  #    #     #     ####   #####      #     ####       #     #


  template< class A >  Periodic4Top < A >::
  Periodic4Top (int l, myhface4_t * f0, int t0, myhface4_t * f1, int t1, const bnd_t (&bt)[2] )
    : A (f0, t0, f1, t1)
    , _dwn (0), _bbb (0), _up(0)
    , _lvl (l)
    , _nChild (0)
    , _rule (myrule_t::nosplit)
  {
    IndexManagerType& im = indexManager();
    // get index
    this->setIndex( im.getIndex() );

    // take macro index as segment index
    _segmentId[ 0 ] = this->getIndex();
    // get additional segment index
    _segmentId[ 1 ] = im.getIndex();

    // store bnd id
    _bt[ 0 ] = bt[ 0 ];
    _bt[ 1 ] = bt[ 1 ];
  }

  template< class A >  Periodic4Top < A >::Periodic4Top (int l, myhface4_t * f0,
      int t0, myhface4_t * f1, int t1, innerperiodic4_t * up, int nChild )
  : A (f0, t0, f1, t1)
    , _dwn (0), _bbb (0), _up(up)
    , _lvl (l)
    , _nChild (nChild)
  {
    // get index
    this->setIndex( indexManager().getIndex() );

    alugrid_assert ( _up );
    // get segment index from father
    _segmentId[ 0 ] = _up->_segmentId[ 0 ];
    _segmentId[ 1 ] = _up->_segmentId[ 1 ];

    // copy bnd ids from father
    _bt[ 0 ] = _up->_bt[ 0 ];
    _bt[ 1 ] = _up->_bt[ 1 ];
  }

  template< class A >  Periodic4Top < A >::~Periodic4Top ()
  {
    IndexManagerType& im = indexManager();

    // free index
    im.freeIndex( this->getIndex() );
    if( level() == 0 ) im.freeIndex( _segmentId[ 1 ] );

    // delete down and next
    if (_bbb) delete _bbb;
    if (_dwn) delete _dwn;
  }

  template< class A > typename Periodic4Top < A >::myhedge_t * Periodic4Top < A >::subedge (int i, int j) {
    alugrid_assert (getrule () == myrule_t::iso4);
    return (j < 4) ? ((twist (i) < 0) ? myhface4 (i)->myhedge ((8 - j + twist (i)) % 4) :  // aus dem Hexaeder
      myhface4 (i)->myhedge ((j + twist (i)) % 4)) :
      ((twist (i) < 0) ? myhface4 (i)->subedge ((12 - j + twist (i)) % 4) :
      myhface4 (i)->subedge ((j + twist (i)) % 4));
  }

  template< class A > const typename Periodic4Top < A >::myhedge_t * Periodic4Top < A >::subedge (int i, int j) const {
    return ((Periodic4Top < A > *)this)->subedge (i,j);
  }

  template< class A > typename Periodic4Top < A >:: myhface4_t * Periodic4Top < A >::subface (int i, int j) {
    typedef typename myhface4_t::myrule_t  facerule_t ;
    myhface4_t * face = myhface4(i);
    const facerule_t facerule = face->getrule();
    return ( facerule == facerule_t::iso4 ) ?
            ( face->is2d() ) ?
              ((twist(i) < 0) ? face->subface((j+1)%2): face->subface(j) )
              : face->subface(twist(i) < 0 ? (9 - j + twist(i)) % 4 : (j + twist(i)) % 4) :
              (abort (), (myhface4_t *)0);
  }

  template< class A > const typename Periodic4Top < A >:: myhface4_t * Periodic4Top < A >::subface (int i, int j) const {
    return ((Periodic4Top < A > *)this)->subface (i,j);
  }

  template< class A > void Periodic4Top < A >::splitISO4 ()
  {
    alugrid_assert (_dwn == 0);

    const int l = 1 + this->level ();
    innerperiodic4_t * p0 = new innerperiodic4_t (l, subface (0,0), twist (0), subface (1,0), twist (1), this, 0);
    innerperiodic4_t * p1 = new innerperiodic4_t (l, subface (0,1), twist (0), subface (1,3), twist (1), this, 1);
    innerperiodic4_t * p2 = new innerperiodic4_t (l, subface (0,2), twist (0), subface (1,2), twist (1), this, 2);
    innerperiodic4_t * p3 = new innerperiodic4_t (l, subface (0,3), twist (0), subface (1,1), twist (1), this, 3);
    alugrid_assert (p0 && p1 && p2 && p3);
    p0->append(p1);
    p1->append(p2);
    p2->append(p3);
    _dwn = p0;
    _rule = myrule_t::iso4;
    return;
  }


  template< class A > void Periodic4Top < A >::splitISO2 ()
  {
    alugrid_assert (_dwn == 0);

    const int l = 1 + this->level ();
    innerperiodic4_t * p0 = new innerperiodic4_t (l, subface (0,1), twist (0), subface (1,0), twist (1), this, 0);
    innerperiodic4_t * p1 = new innerperiodic4_t (l, subface (0,0), twist (0), subface (1,1), twist (1), this, 1);

    alugrid_assert (p0 && p1 );
    p0->append(p1);

    _dwn = p0;
    _rule = myrule_t::iso4;
    return;
  }

  template< class A > void Periodic4Top < A >::refineImmediate (myrule_t r)
  {
    // Die Methode wird nur vom restore () und vom refineBalance () auf-
    // gerufen und geht davon aus, dass das betroffene Element noch nicht
    // verfeinert ist -> ist ein Blatt der Hierarchie.

    alugrid_assert (this->leaf());
    switch (r) {
      case myrule_t::iso4 :

    // Das refineImmediate (..) auf allen Fl"achen wird vom periodic4::refine (..)
    // zwar nicht ben"otigt, da schliesslich alle Fl"achen sauber sind, wenn
    // "uberall hface4::refine (..) true geliefert hat, wohl aber z.B. von
    // restore () oder abgeleiteten Funktionen, die eine direkte Verfeinerung
    // erzwingen m"ussen und d"urfen.

        typedef typename myhface4_t::myrule_t myhface4rule_t;
        myhface4 (0)->refineImmediate (myhface4rule_t (r).rotate (twist (0)));
        myhface4 (1)->refineImmediate (myhface4rule_t (r).rotate (twist (1)));
        if(myhface4(0)->is2d())
          splitISO2 ();
        else
          splitISO4 ();
        break;

      default:
        std::cerr << "ERROR (fatal): Forced refinement using rule " << r << " not possible." << std::endl;
        abort();
        break;
    }
    this->postRefinement ();
    return;
  }

  template< class A > bool Periodic4Top < A >::refineBalance ( balrule_t r, int fce )
  {
    if( r != balrule_t::iso4  )
    {
      std::cerr << "WARNING (ignored): Something is wrong in Periodic4Top < A >::refineBalance." << std::endl;

      // Bisher kann die Balancierung nur die isotrope Achtelung handhaben,
      // falls mehr gew"unscht wird, muss es hier eingebaut werden. Im Moment wird
      // die Balancierung einfach verweigert, d.h. die Verfeinerung des anfordernden
      // Elements f"allt flach.

      return false;
    }
    else
    {
      // Der nachfolgende Aufruf nutzt aus, dass die Regel der periodischen R"ander
      // sich direkt auf die Balancierungsregel des entsprechenden Polygonverbinders
      // projezieren l"asst (n"amlich 1:1). Deshalb unterscheidet der Aufruf nicht nach
      // der angeforderten Regel in einer 'case' Anweisung.
      alugrid_assert( fce == 0 || fce == 1 );

      typedef typename myhface4_t::myrule_t myhface4rule_t;
      int opp = 1 - fce; // (fce == 0) ? 1 : 0;
      if (myhface4 (opp)->refine (myhface4rule_t (r).rotate (twist (opp)), twist (opp)))
      {
        refineImmediate( r );
        return true;
      }
      else
        return false;
    }
  }

  template< class A > bool Periodic4Top < A >::coarse () {

    // Das Vergr"obern geschieht auch passiv, sobald ein anliegendes Element
    // vergr"obert wird durch den Aufruf von "bndNotifyCoarsen ()" s.u.

    bndNotifyCoarsen ();
    return false;
  }

  template< class A > bool Periodic4Top < A >::bndNotifyCoarsen () {

    // Wie beim Randelement auch: Die Vergr"oberung eines anliegenden Elements
    // l"ost einen Vorgang aus, der feststellt ob das periodische RE ebenfalls
    // vergr"obert werden soll.

    innerperiodic4_t * p = down ();
    if (!p) return false;
    bool x = true;
    do {

    // Falls p kein Blatt der Hierarchie ist,
    // die Vergr"oberungsm"oglichkeit weitergeben.

      if (!p->leaf ()) p->coarse ();

    // F"ur die hintere und vordere Fl"ache feststellen, ob
    // der Referenzenz"ahler mehr als einen Eintrag ergibt.

      if (p->myhface4 (0)->ref > 1) (x = false);
      if (p->myhface4 (1)->ref > 1) (x = false);

    } while ( (p = p->next ()) );
    if (x)
    {

      // Falls keine Fl"achen anliegen, die auf Kinder oder Kindes-
      // mit mehr als einer Referenz f"uhren, ist sicher, dass das
      // Bezugsrandelement zwischen zwei 'relativ groben' Elementen
      // liegt. Somit kann es vergr"obert werden.

      this->preCoarsening ();

      delete _dwn;
      _dwn = 0;
      _rule = myrule_t::nosplit;
      myhface4 (0)->coarse ();
      myhface4 (1)->coarse ();
    }
    return x;
  }

  template< class A >
  int Periodic4Top< A >::backup ( std::ostream &os ) const
  {
    return doBackup( os );
  }

  template< class A >
  int Periodic4Top< A >::backup (ObjectStream& os) const
  {
    return doBackup( os );
  }

  template< class A > template<class OutStream_t>
  int Periodic4Top < A >::doBackup (OutStream_t& os) const
  {
    os.put ((char) getrule ());
    {for (const innerperiodic4_t * c = down (); c; c = c->next ()) c->backup (os); }
    return 0;
  }

  template< class A >
  void Periodic4Top< A >::restore ( std::istream &is )
  {
    doRestore( is );
  }

  template< class A > void Periodic4Top < A >::restore (ObjectStream& is)
  {
    doRestore( is );
  }

  template< class A >
  template< class InStream_t >
  void Periodic4Top< A >::doRestore ( InStream_t &is )
  {
    myrule_t r( (char)is.get() );
    alugrid_assert ( getrule() == myrule_t::nosplit ); // Testen auf unverfeinerten Zustand
    if( r == myrule_t::nosplit )
    {
      for( int i = 0; i < 2; ++i )
      {
        myhface4_t &f = *(myhface4( i ) );
        if( !f.leaf() )
        {
          switch( f.getrule() )
          {
          case balrule_t::iso4:
            {
              const int nSubFaces = f.is2d() ? 2 : 4;
              for( int j = 0; j < nSubFaces; ++j )
                f.subface( j )->nb.complete( f.nb );
              break;
            }

          default:
            std::cerr << "ERROR (fatal): Trying to restore using unknown refinement rule [" << r << "]." << std::endl;
            abort ();
            break;
          }
        }
      }
    }
    else
    {
      refineImmediate( r );
      alugrid_assert ( getrule() == r );
      for( innerperiodic4_t *c = down(); c; c = c->next () )
        c->restore( is );
    }
  }



  // Template Instantiation
  // ----------------------
#ifndef GITTER_HEXA_TOP_PLL_H_INCLUDED
  template class Hedge1Top< GitterBasis::Objects::Hedge1Empty >;
  template class Hface4Top< GitterBasis::Objects::Hface4Empty >;
  template class Hbnd4Top< GitterBasis::Objects::Hbnd4Default >;
  template class HexaTop < GitterBasis::Objects::HexaEmpty >;
  template class Periodic4Top < GitterBasis::Objects::Periodic4Empty >;
#endif

} // namespace ALUGrid
