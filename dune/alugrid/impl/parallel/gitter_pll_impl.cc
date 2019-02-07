// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface
// (c) Robert Kloefkorn 2004 - 2005
#include <config.h>

#include <fstream>
#include <sstream>

#include "../serial/gatherscatter.hh"
#include "../serial/mapp_cube_3d.h"
#include "../serial/mapp_tetra_3d.h"

#include "gitter_pll_ldb.h"
#include "gitter_pll_impl.h"
#include "gitter_hexa_top_pll.h"
#include "gitter_tetra_top_pll.h"

namespace ALUGrid
{

  template < class A >
  const linkagePattern_t VertexPllBaseX< A >::nullPattern;

  template < class A >
  VertexPllBaseX< A >::VertexPllBaseX (double x, double y, double z, int i, IndexManagerStorageType& ims )
    : A( x, y, z, i, ims ),
      _lpn (),
      _moveTo ( 0 )
  {
    doClearLinkage();

    // ident == 0 only appears for the fake vertex in a triangle grid
    if( (this->indexManagerStorage().dimension() == 2) && (this->ident() == 0) )
    {
      this->setFlagFakeVertex();
    }
  }

  template < class A >
  VertexPllBaseX< A >::~VertexPllBaseX ()
  {
    // make sure _moveTo was already deleted
    alugrid_assert ( _moveTo == 0 );
  }

  template < class A >
  LinkedObject::Identifier VertexPllBaseX< A >::getIdentifier () const
  {
    return LinkedObject::Identifier (myvertex().ident ());
  }

  template < class A >
  void VertexPllBaseX< A >::checkAndAddLinkage( const int rank )
  {
    const std::vector< int >& lp = ((*_lpn).first);
    const std::vector< int >::const_iterator lpEnd = lp.end();
    // if rank is not contained in the linkage, add it
    if( std::find( lp.begin(), lpEnd, rank ) == lpEnd )
    {
      const size_t lpSize = lp.size() ;
      std::vector< int > newLinkage( lpSize+1 );
      // copy content
      std::copy( lp.begin(), lpEnd, newLinkage.begin() );
      newLinkage[ lpSize ] = rank ;
      setLinkage( newLinkage );
    }
  }

  template < class A >
  std::vector< int > VertexPllBaseX< A >::estimateLinkage () const
  {
    return std::vector< int > ((*_lpn).first);
  }

  template < class A >
  void VertexPllBaseX< A >::doClearLinkage()
  {
    // set iterator to empty linkage
    linkagePatternMap_t& _map = linkagePatterns();
    typename linkagePatternMap_t::iterator pos = _map.find ( nullPattern );
    _lpn = (pos != _map.end ()) ? pos : _map.insert (make_pair( nullPattern, int(0) )).first;
    ++ (*_lpn).second;
  }

  template < class A >
  void VertexPllBaseX< A >::clearLinkage()
  {
    doClearLinkage();
  }

  template < class A >
  bool VertexPllBaseX< A >::setLinkage ( const std::vector< int >& newLinkage )
  {
    if( this->isFakeVertex() ) return true;

    std::vector< int > lp ( newLinkage );
    std::sort( lp.begin(), lp.end() );
    return doSetLinkageSorted( lp );
  }

  template < class A >
  bool VertexPllBaseX< A >::setLinkageSorted ( const std::vector< int >& slp )
  {
    if( this->isFakeVertex() ) return true;
    return doSetLinkageSorted( slp );
  }

  template < class A >
  bool VertexPllBaseX< A >::doSetLinkageSorted ( const std::vector< int >& slp )
  {
    alugrid_assert( ! this->isFakeVertex() );

    -- (*_lpn).second;
    linkagePatternMap_t& _map = linkagePatterns();
    typename linkagePatternMap_t::iterator pos = _map.find ( slp );
    _lpn = (pos != _map.end ()) ? pos : _map.insert (make_pair( slp, int(0) )).first;
    ++ (*_lpn).second;
    return true;
  }

  template < class A >
  void VertexPllBaseX< A >::unattach2 (int i)
  {
    alugrid_assert ( _moveTo );
    typename moveto_t::iterator pos = _moveTo->find( i );
    alugrid_assert ( pos != _moveTo->end ());
    if ( (-- (*pos).second ) == 0 )
    {
      _moveTo->erase ( pos );
      if( _moveTo->empty() )
      {
        delete _moveTo;
        _moveTo = 0;
      }
    }
    return;
  }

  template < class A >
  void VertexPllBaseX< A >::attach2 (int i)
  {
    // create moveTo if not already existent
    if( ! _moveTo ) _moveTo = new moveto_t ();

    typename moveto_t::iterator pos = _moveTo->find( i );
    if( pos == _moveTo->end() )
      _moveTo->insert (std::pair< const int, int > (i,1));
    else
      ++ (*pos).second;
  }

  template < class A >
  bool VertexPllBaseX< A >::packAll (std::vector< ObjectStream > & osv)
  {
    bool action (false);
    if( _moveTo )
    {
      typedef typename moveto_t::const_iterator  const_iterator;
      const const_iterator iEnd =  _moveTo->end ();
      for (const_iterator i = _moveTo->begin (); i != iEnd; ++i)
      {
        const int link = (*i).first;
        action = doPackLink( link, osv[ link ] );
      }
    }
    return action ;
  }

  template < class A >
  bool VertexPllBaseX< A >::doPackLink ( const int link, ObjectStream& os )
  {
    os.writeObject (VERTEX);
    os.writeObject (myvertex ().ident ());
    const alucoord_t (&point)[ 3 ] = myvertex ().Point ();
    os.writeObject ( point[0] );
    os.writeObject ( point[1] );
    os.writeObject ( point[2] );

    const int elSize = _elements.inactive() ? 0 : _elements.size();
    os.writeObject( elSize );
    for( int i=0; i<elSize; ++ i )
    {
      os.writeObject( _elements[ i ] );
    }
    return true ;
  }

  template < class A >
  void VertexPllBaseX< A >::unpackSelf (ObjectStream & os, bool isNew)
  {
    int elSize;
    os.readObject( elSize );
    if( elSize > 0 )
    {
      if( _elements.inactive() )
      {
        std::vector< int > elements( elSize ) ;
        for( int el=0; el<elSize; ++el )
        {
          os.readObject( elements[ el ] );
        }
        _elements.insertElementLinkage( elements );
      }
      else
      {
        alugrid_assert( elSize == _elements.size() );
        // advance buffer by elsize * sizeof( int ) bytes
        os.removeObject( elSize * sizeof( int ) );
      }
    } // end elSize > 0

    return;
  }

  /////////////////////////////////////////////////////////////////
  // --EdgePllBaseXMacro
  /////////////////////////////////////////////////////////////////
  template < class A >
  std::vector< int > EdgePllBaseXMacro< A >::estimateLinkage () const
  {
    const typename Gitter::vertex_STI* v0 = myhedge().myvertex(0);
    const typename Gitter::vertex_STI* v1 = myhedge().myvertex(1);

    // check for fake vertex to make sure we get the right linkage estimate
    if( v0->isFakeVertex() )
      return v1->estimateLinkage ();
    if( v1->isFakeVertex() )
      return v0->estimateLinkage ();

    // otherwise compute linkage estimate as usual
    std::vector< int > est;
    const std::vector< int > l0 ( v0->estimateLinkage () );
    const std::vector< int > l1 ( v1->estimateLinkage () );

    // reserve memory for est elements
    est.reserve( std::min( l0.size(), l1.size() ) );

    set_intersection( l0.begin(), l0.end(), l1.begin(), l1.end(), std::back_inserter( est ) );
    return est;
  }

  template < class A >
  LinkedObject::Identifier EdgePllBaseXMacro< A >::getIdentifier () const
  {
    return LinkedObject::Identifier (myhedge ().myvertex (0)->ident (), myhedge ().myvertex (1)->ident ());
  }

  template < class A >
  void EdgePllBaseXMacro< A >::unattach2 (int i)
  {
    alugrid_assert ( _moveTo );
    typename moveto_t::iterator pos = _moveTo->find( i );
    alugrid_assert ( pos != _moveTo->end ());
    if ( (-- (*pos).second ) == 0 )
    {
      _moveTo->erase ( pos );
      if( _moveTo->empty() )
      {
        delete _moveTo;
        _moveTo = 0;
      }
    }
    myhedge ().myvertex (0)->unattach2 (i);
    myhedge ().myvertex (1)->unattach2 (i);
    return;
  }

  template < class A >
  void EdgePllBaseXMacro< A >::attach2 (int i)
  {
    // create moveTo if not already existent
    if( ! _moveTo ) _moveTo = new moveto_t ();

    typename moveto_t::iterator pos = _moveTo->find( i );
    if( pos == _moveTo->end() )
      _moveTo->insert (std::pair< const int, int > (i,1));
    else
      ++ (*pos).second;

    myhedge ().myvertex (0)->attach2 (i);
    myhedge ().myvertex (1)->attach2 (i);
    return;
  }

  template < class A >
  bool EdgePllBaseXMacro< A >::packAll (std::vector< ObjectStream > & osv)
  {
    bool action (false);
    if( _moveTo )
    {
      typedef typename moveto_t::const_iterator const_iterator;
      const const_iterator iEnd =  _moveTo->end ();
      for (const_iterator i = _moveTo->begin (); i != iEnd; ++i)
      {
        const int link = (*i).first;
        alugrid_assert ((osv.begin () + link) < osv.end ());
        action = doPackLink( link, osv[ link ] );
      }
    }
    return action;
  }

  template < class A >
  bool EdgePllBaseXMacro< A >::doPackLink ( const int link, ObjectStream& os )
  {
    os.writeObject (EDGE1);
    os.writeObject (myhedge ().myvertex (0)->ident ());
    os.writeObject (myhedge ().myvertex (1)->ident ());

    // make sure ENDOFSTREAM is not a valid refinement rule
    alugrid_assert ( ! myhedge_t::myrule_t::isValid (ObjectStream::ENDOFSTREAM) );

    // pack refinement information
    myhedge ().backup ( os );
    os.put( ObjectStream::ENDOFSTREAM );
    return true ;
  }

  template < class A >
  void EdgePllBaseXMacro< A >::unpackSelf (ObjectStream & os, bool i)
  {
    if (i)
    {
      myhedge ().restore ( os );

      // stream should be at position ENDOFSTREAM now
      char c = os.get();
      if( c != ObjectStream::ENDOFSTREAM )
      {
        std::cerr << "ERROR (fatal): c != ENDOFSTREAM." << std::endl;
        alugrid_assert(0);
        abort();
      }
    }
    else
    {
      try
      {
        char c = os.get();
        // read stream until ENDOFSTREAM
        while ( c != ObjectStream::ENDOFSTREAM )
        {
          os.read(c);
        }
      }
      catch( ObjectStream::EOFException& )
      {
        std::cerr << "ERROR (fatal): EdgePllBaseXMacro< A >::unpackSelf EOF encountered." << std::endl;
        alugrid_assert(0);
        abort();
      }
    }
    return;
  }

  // Template Instantiation
  template class EdgePllBaseXMacro< GitterBasisPll::ObjectsPll::hedge1_IMPL >;

  // #######
  // #         #####   #####  ######
  // #        #     # #       #
  // #####    ####### #       ######
  // #        #     # #       #
  // #        #     # #       #
  // #        #     #  #####  ######

  // constructor for hface3
  template <> FacePllBaseXMacro<GitterBasisPll::ObjectsPll::hface3_IMPL>::
  FacePllBaseXMacro(int l, myhedge_t * e0, int s0, myhedge_t * e1, int s1,
                    myhedge_t * e2, int s2)
   : GitterBasisPll::ObjectsPll::hface3_IMPL(l, e0, s0, e1, s1, e2, s2), _moveTo( 0 )
  {
  }

  // constructor for hface4
  template <> FacePllBaseXMacro<GitterBasisPll::ObjectsPll::hface4_IMPL>::
  FacePllBaseXMacro(int l, myhedge_t * e0, int s0, myhedge_t * e1, int s1,
                    myhedge_t * e2, int s2, myhedge_t * e3, int s3)
   : GitterBasisPll::ObjectsPll::hface4_IMPL(l, e0, s0, e1, s1, e2, s2, e3, s3), _moveTo( 0 )
  {
  }

  // destructor
  template < class A > FacePllBaseXMacro < A >::~FacePllBaseXMacro()
  {
    alugrid_assert ( _moveTo == 0 );
  }

  template < class A > std::vector< int > FacePllBaseXMacro < A >::estimateLinkage () const
  {
    // Diese Methode sch"atzt den Verbindungsstern der Grobgitterfl"ache,
    // indem sie die Schnittmenge der Verbindungssterne der anliegenden
    // Grobgitterknoten bildet. Je besser die Sch"atzung, desto schneller
    // arbeitet das Identifikationsmodul. Falls keine Sch"atzung m"oglich
    // ist, kann man auch einfach aller log. Teilgiternummern in einem
    // Vektor zur"uckgeben. Dann geht die Identifikation eben langsam.

    std::vector< int > t1, t2temp, est;
    const std::vector< int >& l0 ( myhface ().myhedge (0)->estimateLinkage () );
    const std::vector< int >& l1 ( myhface ().myhedge (1)->estimateLinkage () );
    const std::vector< int >& l2 ( myhface ().myhedge (2)->estimateLinkage () );

    const size_t t1size = std::min( l0.size(), l1.size() );
    t1.reserve( t1size );
    set_intersection( l0.begin(), l0.end(), l1.begin(), l1.end(), std::back_inserter( t1 ) );
    // for tetras we don't need the forth linkage
    if( A::polygonlength == 4 )
    {
      const std::vector< int >& l3 (myhface ().myhedge (3)->estimateLinkage ());
      t2temp.reserve( std::min( l3.size(), l2.size() ) );
      set_intersection( l2.begin(), l2.end(), l3.begin(), l3.end(), std::back_inserter( t2temp ) );
    }
    const std::vector<int>& t2 = (A::polygonlength == 4) ? t2temp : l2 ;

    est.reserve( std::min( t1size, t2.size() ) );
    set_intersection( t1.begin(), t1.end(), t2.begin(), t2.end(), std::back_inserter( est ) );
    return est;
  }

  template < class A > LinkedObject::Identifier FacePllBaseXMacro < A >::getIdentifier () const
  {
    // Diese Methode erzeugt den Schl"ussel einer Grobgitterfl"ache im Auftrag
    // des Identifikationsmoduls 'gitter_pll_idn.cc'. Der Schl"ussel besteht
    // aus drei Integer Werten 'key.h', die aus der globelen Ausrichtung der
    // Fl"ache folgen und auch f"ur Fl"achen mit mehr als drei Knoten korrekt
    // (d.h. gleiche Fl"ache in versch. Teilgittern -> gleicher Schl"ussel) sind.

    return LinkedObject::Identifier (this->myhface ().myvertex (0)->ident (),
        this->myhface ().myvertex (1)->ident (), this->myhface ().myvertex (2)->ident ());
  }

  template < class A >
  bool FacePllBaseXMacro < A >::ldbUpdateGraphEdge (LoadBalancer::DataBase & db, const bool serialPartitioner )
  {
    // Diese Methode erzeugt eine Kante im Graphen f"ur die Berechnung
    // der Neupartitionierung, der sie das Gewicht der Anzahl aller feinsten
    // Fl"achen "uber der verwalteten Grobgitterfl"ache gibt.
    typedef typename myhface_t::myconnect_t myconnect_t;

    const myconnect_t * mycon1 = this->myhface().nb.front().first;
    const myconnect_t * mycon2 = this->myhface().nb.rear ().first;

    // default is no periodic boundary
    bool periodicBnd = false;

    if(mycon1 && mycon2)
    {
      // get graph vertex number of the adjacent elements
      int ldbVx1 = mycon1->accessPllX ().ldbVertexIndex ();
      int ldbVx2 = mycon2->accessPllX ().ldbVertexIndex ();
      int master1 = mycon1->accessPllX ().master ();
      int master2 = mycon2->accessPllX ().master ();

      // only insert graph edge on the rank where the smaller vertex number is interior
      // and this we only need to do for serial partitioners
      if( serialPartitioner )
      {
        if( ldbVx1 < ldbVx2 )
        {
          if ( mycon1->isboundary() ) return periodicBnd;
        }
        else
        {
          if ( mycon2->isboundary() ) return periodicBnd;
        }
      }

      if( mycon1->isperiodic() )
      {
        alugrid_assert ( ! mycon2->isperiodic() );
        ldbVx1 = mycon1->otherLdbVertexIndex( myhface().getIndex() );
        ldbVx2 = mycon2->accessPllX ().ldbVertexIndex ();
        periodicBnd = true;
      }

      if( mycon2->isperiodic() )
      {
        alugrid_assert ( ! mycon1->isperiodic() );
        ldbVx1 = mycon1->accessPllX ().ldbVertexIndex ();
        ldbVx2 = mycon2->otherLdbVertexIndex( myhface().getIndex() );
        periodicBnd = true;
      }

      // count leaf faces for this macro face
      const int weight =  TreeIterator < typename Gitter::hface_STI,
                                         is_leaf < Gitter::hface_STI > > ( myhface () ).size ();
      alugrid_assert ( weight>=0 );

      // if we have a periodic situation
      if( periodicBnd )
      {
        // if one of them is periodic, increase factor
        // this should reduce cutting of edges between
        // periodic and normal elements

        alugrid_assert ( mycon1->isperiodic() || mycon2->isperiodic() );
        alugrid_assert ( ldbVx1 >= 0 && ldbVx2 >= 0 );
        // increase the edge weight for periodic connections
        // TODO: make weight factor (here 4) dynamically adjustable
        db.edgeUpdate ( LoadBalancer::GraphEdge ( ldbVx1, ldbVx2, weight*4,
              master1, master2 ) );
      }
      else
      {
        // the default graph edge
        db.edgeUpdate ( LoadBalancer::GraphEdge ( ldbVx1, ldbVx2, weight,
              master1, master2 ) );
      }
    }
    return periodicBnd;
  }

  template < class A > void FacePllBaseXMacro < A >::unattach2 (int i)
  {
    if( _moveTo )
    {
      // Diese Methode bindet die Fl"ache von einer Zuweisung zu einem neuen
      // Teilgitter ab. D.h. der Eintrag in der Zuweisungsliste wird gel"oscht,
      // und dann wird die M"oglichkeit an die anliegenden Kanten weitervermittelt.
      typename moveto_t::iterator pos = _moveTo->find( i );

      if( pos == _moveTo->end () ) return;

      if ( (--(*pos).second) == 0)
      {
        _moveTo->erase ( pos );
        if( _moveTo->empty() )
        {
          delete _moveTo;
          _moveTo = 0;
        }
      }

      for (int j = 0; j < A::polygonlength; ++j )
      {
        this->myhface ().myhedge (j)->unattach2 (i);
      }
    }
  }

  template < class A > void FacePllBaseXMacro < A >::attach2 (int i)
  {
    // create moveTo if not already existent
    if ( ! _moveTo ) _moveTo = new moveto_t ();

    typename moveto_t::iterator pos = _moveTo->find( i );
    if( pos == _moveTo->end() )
      _moveTo->insert (std::pair< const int, int > (i,1));
    else
      ++ (*pos).second;

    {
      for (int j = 0; j < A::polygonlength; ++j )
        this->myhface ().myhedge (j)->attach2 (i);
    }
    return;
  }

  template < class A > bool FacePllBaseXMacro < A >::packAll (std::vector< ObjectStream > & osv)
  {

    // Die Methode packAll () verpackt die Fl"ache auf alle Datenstr"ome,
    // die zu Teilgittern f"uhren, an die sie zugewiesen wurde mit attach2 ().
    // Ausserdem geht die Methode noch an die anliegenden Elemente (Randelemente)
    // "uber.

    bool action = false;
    if( _moveTo )
    {
      typedef typename moveto_t::const_iterator const_iterator;
      const const_iterator iEnd =  _moveTo->end ();
      for (const_iterator i = _moveTo->begin (); i != iEnd; ++i)
      {
        const int link = (*i).first;
        alugrid_assert ((osv.begin () + link) < osv.end ());
        action = doPackLink( link, osv[ link ] );
      }
    }
    return action;
  }

  template < class A > bool
  FacePllBaseXMacro < A >::doPackLink ( const int link, ObjectStream & os )
  {
    const bool ghostCellsEnabled = myhface().myvertex( 0 )->myGrid()->ghostCellsEnabled();

    if (A::polygonlength == 4)
    {
      os.writeObject (MacroGridMoverIF::FACE4);
    }
    else if (A::polygonlength == 3)
    {
      os.writeObject (MacroGridMoverIF::FACE3);
    }
    else
    {
      // something wrong
      alugrid_assert (false);
      alugrid_assert(0);
      abort ();
    }

    {
      // write vertex idents
      for (int k = 0; k < A::polygonlength; ++ k)
      {
        os.writeObject (this->myhface ().myvertex (k)->ident ());
      }
    }

    try {

      // Sicherheitshalber testen, ob das ENDOFSTREAM Tag nicht auch
      // mit einer Verfeinerungsregel identisch ist - sonst gibt's
      // nachher beim Auspacken nur garbage.

      alugrid_assert (! myhface_t::myrule_t::isValid (ObjectStream::ENDOFSTREAM) );

      this->myhface ().backup ( os );
      os.put( ObjectStream::ENDOFSTREAM );
    }
    catch( ObjectStream::OutOfMemoryException& )
    {
      std::cerr << "ERROR (fatal): Out of memory." << std::endl;
      alugrid_assert(0);
      abort();
    }

    try
    {
      // Wenn die Fl"ache auf den link-ten Strom des Lastverschiebers
      // geschrieben wurde, dann mu"ussen auch die anliegenden
      // Elemente daraufhin untersucht werden, ob sie sich nicht
      // als Randelemente dorthin schreiben sollen - das tun sie
      // aber selbst.

      this->myhface ().nb.front ().first->accessPllX ().packAsBnd (this->myhface ().nb.front ().second, link, os, ghostCellsEnabled );
      this->myhface ().nb.rear  ().first->accessPllX ().packAsBnd (this->myhface ().nb.rear  ().second, link, os, ghostCellsEnabled );
    }
    catch( Parallel::AccessPllException& )
    {
      std::cerr << "ERROR (fatal): AccessPllException caught." << std::endl;
      alugrid_assert(0);
      abort();
    }
    return true;
  }

  //- --unpackSelf
  template < class A > void FacePllBaseXMacro < A >::unpackSelf (ObjectStream & os, bool i)
  {

    // Die Methode wird eine Fl"ache aus dem Datenstrom rekonstruieren,
    // die der MacrogridBuilder angelegt hat, oder sie wird nur die
    // Verfeinerungsregeln aus dem Strom nehmen, falls schon vorher
    // eine identische Fl"ache bestand. Dies wird vom MacrogridBuilder
    // durch die 'bool' Variable mitgeteilt. Trotzdem k"onnen auch auf
    // einen bestehenden Fl"achenbaum durch die Lastverschiebung neue
    // Daten aufgebracht werden - dies ist dann hier zu realisieren.

    if (i)
    {
      // Sobald der Stringstream mit den 'byte' Verfeinerungsregeln
      // voll ist, kann mit dem normalen restore der ganze Fl"achen-
      // baum wieder hochgezogen werden. Analog zur Wiederherstellung
      // aus einer Datei.

      this->myhface ().restore ( os );

      char c = os.get();
      if( c != ObjectStream::ENDOFSTREAM )
      {
        std::cerr << "ERROR (fatal): c != ENDOFSTREAM." << std::endl;
        alugrid_assert(0);
        abort();
      }
    }
    else
    {
      try
      {
        char c = os.get();
        // read stream until ENDOFSTREAM
        while( c != ObjectStream::ENDOFSTREAM )
          os.read( c );
      }
      catch( ObjectStream::EOFException& )
      {
        std::cerr << "ERROR (fatal): FacePllBaseXMacro < A >::unpackSelf EOF encountered." << std::endl;
        alugrid_assert(0);
        abort();
      }
    }
  }

  // Template Instantiation
  template class FacePllBaseXMacro < GitterBasisPll::ObjectsPll::hface3_IMPL >;
  template class FacePllBaseXMacro < GitterBasisPll::ObjectsPll::hface4_IMPL >;

  //////////////////////////////////////////////////////////////////////
  //
  //  --BndsegPllBaseXMacroClosure
  //
  //////////////////////////////////////////////////////////////////////
  template < class A > void BndsegPllBaseXClosure < A >::getRefinementRequest (ObjectStream & os)
  {
    typename balrule_t::rule_t rule = _rul;
    os.put( char(rule) );
    _rul = balrule_t::nosplit;
    return;
  }

  template < class A > bool BndsegPllBaseXClosure < A >::setRefinementRequest (ObjectStream & os) {

    // Die Methode schickt die Regel aus dem Nachbargitter in das
    // eigene Teilgitter hinein. Sie liefert "true" immer dann, wenn
    // sich das eigene Gitter g"andert hat -> f"uhrt zu einer weiteren
    // Iteration des parallelen refine ().

    signed char ru;
    try
    {
      ru = os.get();
    }
    catch( ObjectStream::EOFException& )
    {
      std::cerr << "ERROR (fatal): EOF encountered in BndsegPllBaseXClosure::setRefinementRequest( ... )" << std::endl;
      alugrid_assert(0);
      abort();
    }
    balrule_t rule ( ru ) ;
    if (rule == balrule_t::nosplit)
    {
      return false;
    }
    else
    {
      if (myhbnd ().getrule () == rule )
      {
        return false;
      }
      else
      {
        if (myhbnd ().refineLikeElement ( rule ))
        {
          // Verfeinerung erfolgreich
          return true;
        }
        else
        {
          // Verfeinerung verhindert irgendwo im Gitter. Dies ist ein Vorbehalt
          // f"ur den parallelen anisotropen Verfeinerungsalgorithmus. Daher
          // sollte die Situation im isotropen Fall nicht auftreten.

          if(rule.bisection())
            return myhbnd ().refineLikeElement ( rule );

          std::cerr << "ERROR (fatal): Refinement inhibited on inner boundary." << std::endl;
          alugrid_assert(0);
          abort();
        }
      }
    }
    return (alugrid_assert(0), abort (), false);
  }

  template < class A > void BndsegPllBaseXClosure < A >::
  writeDynamicState (ObjectStream & os, GatherScatterType & gs ) const
  {
    gs.sendData( os, myhbnd() );
    return;
  }

  template < class A > void BndsegPllBaseXClosure < A >::
  readDynamicState (ObjectStream & os, GatherScatterType & gs )
  {
    gs.recvData( os , myhbnd () );
    return;
  }

  template < class A > void BndsegPllBaseXClosure < A >::
  readDynamicState (ObjectStream & os, int)
  {
    try
    {
      // read the real level of ghost
      alugrid_assert (myhbnd().leafRefCount()==0 || myhbnd().leafRefCount()==1);
      const bool wasLeaf = this->ghostLeaf();

      // read level and leaf of interior element on other side
      // this changes the state of this leaf or not leaf
      unsigned char ghLvl = 0;
      os.read( ghLvl );
      _ghostLevel = ghLvl ;

      _ghostLeaf  = bool( os.get() );

      const bool nowLeaf = this->ghostLeaf();

      // if leaf state has changed the attach or detach
      if (! wasLeaf && nowLeaf)
      {
        myhbnd().attachleafs();
      }
      else if (wasLeaf && !nowLeaf)
      {
        myhbnd().detachleafs();
      }

      alugrid_assert ( myhbnd().leafRefCount()==0 || myhbnd().leafRefCount()==1 );
      alugrid_assert ( (!nowLeaf) ? (! myhbnd().isLeafEntity()) : 1);
      alugrid_assert ( ( nowLeaf) ? (  myhbnd().isLeafEntity()) : 1);

      // only transmitted vertices if numVerticesProjected > 0
      // number is number of all vertices communicated
      const int numVerticesProjected = os.get();

      if( numVerticesProjected > 0)
      {
        const int fce = os.get();

        std::array< std::array<alucoord_t,3>, 8 > projectedVertices;
        for(int i = 0; i < numVerticesProjected; ++i)
        {
          for(int j = 0 ; j < 3 ; ++j)
          {
            os.read( projectedVertices[ i ][ j ] );
          }
        }

        double vol = 0;
        os.read( vol );

        myhbnd().projectGhostElement( fce, projectedVertices, vol );
      }
    }
    catch (ObjectStream::EOFException&)
    {
      return;
      std::cerr << "ERROR (fatal): BndsegPllBaseXClosure < A >::readDynamicState EOF encountered." << std::endl;
      alugrid_assert(0);
      abort();
    }
  }

  template < class A >
  void BndsegPllBaseXMacroClosure < A > :: readStaticState( ObjectStream & os, int)
  {
    try
    {
      os.readObject ( _ldbVertexIndex ) ;
      os.readObject ( _master );
      alugrid_assert ( _master != this->myhbnd().myvertex(0,0)->indexManagerStorage ().myrank() );
    }
    catch (ObjectStream :: EOFException&)
    {
      std::cerr << "**ERROR (fatal): BndsegPllBaseXMacroClosure < A >::readStaticState EOF encountered." << std::endl ;
      alugrid_assert(0);
      abort () ;
    }
    alugrid_assert (_ldbVertexIndex >= 0) ;
    alugrid_assert (_master >= 0) ;
  }

  template < class A > void BndsegPllBaseXMacroClosure < A >::
  packAsBnd (int fce, int who, ObjectStream & os, const bool ghostCellsEnabled) const
  {
    alugrid_assert (!fce); // fce should be 0, because we only have 1 face
    alugrid_assert (this->myhbnd ().bndtype () == Gitter::hbndseg::closure);

    if (myhface_t::polygonlength == 3) os.writeObject (MacroGridMoverIF::HBND3INT);
    else if (myhface_t::polygonlength == 4) os.writeObject (MacroGridMoverIF::HBND4INT);
    else
    {
      std::cerr << "ERROR (fatal): Wrong face type in BndsegPllBaseXMacroClosure::packAsBnd." << std::endl;
      alugrid_assert(0);
      abort();
    }

    os.writeObject ( this->myhbnd ().bndtype () );

    // write unique graph vertex index
    os.writeObject ( _ldbVertexIndex );
    // os.writeObject ( _master );
    os.writeObject ( this->myhbnd().myvertex(0,0)->indexManagerStorage().myrank() );

    {
      for (int i = 0; i < myhface_t::polygonlength; ++i)
        os.writeObject (this->myhbnd ().myvertex (fce,i)->ident ());
    }

    if(_ghInfo) // is stored ghost point exists
    {
      os.put( MacroGridMoverIF::POINTTRANSMITTED );

      // see ghost_info.h for implementation of this functions
      _ghInfo->inlineGhostElement(os);
    }
    else
    {
      os.put( MacroGridMoverIF::NO_POINT ); // no point transmitted
    }

    return;
  }

  template < class A > inline void BndsegPllBaseXMacroClosure < A >::
  insertGhostCell(ObjectStream & os, int fce)
  {
    alugrid_assert ( _ghInfo == 0 );
    _ghInfo = this->myhbnd().buildGhostCell(os , fce);
    alugrid_assert ( _ghInfo );
  }

  // template Instantiation
  template class BndsegPllBaseXMacroClosure< GitterBasis::Objects::Hbnd4Default >;
  template class BndsegPllBaseXMacroClosure< GitterBasis::Objects::Hbnd3Default >;

  // #######
  //    #     ######   #####  #####     ##
  //    #     #          #    #    #   #  #
  //    #     #####      #    #    #  #    #
  //    #     #          #    #####   ######
  //    #     #          #    #   #   #    #
  //    #     ######     #    #    #  #    #
  template < class A >
  void TetraPllXBase< A >::writeDynamicState (ObjectStream & os, GatherScatterType & gs) const
  {
    gs.sendData( os , mytetra () );
    return;
  }

  template < class A >
  void TetraPllXBase< A >::writeDynamicState (ObjectStream & os, int face) const
  {
    // write level to know the level of ghost on the other side
    // write level and leaf for the ghost element
    // to determine leafEntity or not

    const unsigned char lvl = mytetra().level();
    os.write( lvl );
    bool leaf = mytetra().leaf();
    os.put( char( leaf ) );

    // Put numVertices of the element
    // >0 means, that at least one vertex has been projected
    const int numVerticesProjected = mytetra().vertexWasProjected() ? 4 : 0;
    os.put( numVerticesProjected );

    if( numVerticesProjected > 0 )
    {
      // store local face number
      signed char fce = face ;
      os.put( fce );

      const auto& f = *(mytetra().myhface( face ));
      // write coordinates of the face
      for(int vx=0; vx<3; ++vx)
      {
        //const alucoord_t (&p)[3] = mytetra().myvertex(face, vx)->Point();
        const alucoord_t (&p)[3] = f.myvertex(vx)->Point();
        os.write( p[0] );
        os.write( p[1] );
        os.write( p[2] );
      }

      {
        // write point opposite to face
        const alucoord_t (&p)[3] = mytetra().myvertex(face)->Point();
        os.write( p[0] );
        os.write( p[1] );
        os.write( p[2] );
      }

      // also store volume since the projection of vertices will change this
      const double vol = mytetra().volume();
      os.write( vol );
    }
    return;
  }

  template < class A >
  TetraPllXBaseMacro< A >::
  TetraPllXBaseMacro (int l, myhface3_t *f0, int s0, myhface3_t *f1, int s1,
                             myhface3_t *f2, int s2, myhface3_t *f3, int s3,
                      SimplexTypeFlag elementType )
    : A(l, f0, s0, f1, s1, f2, s2, f3, s3, elementType )
    , _moveTo ( -1 )
    , _ldbVertexIndex (-1)
  {
    // don't allow erase
    set( flagLock );
  }

  template < class A >
  TetraPllXBaseMacro< A >::~TetraPllXBaseMacro ()
  {
    if( _moveTo >= 0 )
    {
      unattach2 ( _moveTo );
    }
  }

  template < class A >
  void TetraPllXBaseMacro< A >::computeBaryCenter( alucoord_t (&center)[3] ) const
  {
    LinearMapping::barycenter(
        mytetra ().myvertex (0)->Point (),
        mytetra ().myvertex (1)->Point (),
        mytetra ().myvertex (2)->Point (),
        mytetra ().myvertex (3)->Point (),
        center);
  }

  template < class A >
  int TetraPllXBaseMacro< A >::ldbVertexIndex () const
  {
    alugrid_assert ( _ldbVertexIndex >= 0 );
    return _ldbVertexIndex;
  }

  template < class A >
  void TetraPllXBaseMacro< A >::setLoadBalanceVertexIndex ( const int ldbVx )
  {
    //std::cout << "Set ldbVertex " << ldbVx << std::endl;
    _ldbVertexIndex = ldbVx;
  }

  template < class A >
  void TetraPllXBaseMacro< A >::
  computeVertexLinkage( vertexelementlinkage_t& vxElemLinkge )
  {
    for( int i=0; i<4; ++i )
    {
      // add my ldb vertex index to vertex's list of elements
      vxElemLinkge[ mytetra().myvertex( i ) ].insert( _ldbVertexIndex );
    }
  }

  template < class A >
  bool TetraPllXBaseMacro< A >::
  ldbUpdateGraphVertex (LoadBalancer::DataBase & db, GatherScatterType* gs )
  {
    // parameter for GraphVertex are:
    // - macro vertex index
    // - number of elementes below macro element
    // - pointer to this element in case bary center is needed (only if GRAPHVERTEX_WITH_CENTER defined)
    typedef TreeIterator < Gitter::helement_STI, is_leaf < Gitter::helement_STI > >  TreeIteratorType;

    // get macro element weight
    // if gs is not null, then use the weight provided by gs
    // otherwise count number of leaf elements
    const int weight = ( gs ) ? gs->loadWeight( mytetra() ) :
                                TreeIteratorType( mytetra () ).size ();

    db.vertexUpdate ( LoadBalancer::GraphVertex (ldbVertexIndex (), weight, *this ) );
    return true;
  }

  template < class A >
  void TetraPllXBaseMacro< A >::unattach2 (int i)
  {
    alugrid_assert ( i >= 0 );
    mytetra ().myhface3 (0)->unattach2 (i);
    mytetra ().myhface3 (1)->unattach2 (i);
    mytetra ().myhface3 (2)->unattach2 (i);
    mytetra ().myhface3 (3)->unattach2 (i);

    // reset move to
    _moveTo = -1;

    // unset erasable flag
    set( flagLock );
  }

  template < class A >
  void TetraPllXBaseMacro< A >::attachElement2 (const int destination, const int face)
  {
    alugrid_assert ( destination >= 0 );

    // attach the element
    attach2( destination );

    // make sure we to correct destination
    alugrid_assert ( _moveTo == destination );

    // check all neighbours
    // face is the face this method was called from
    for( int f=0; f<4; ++f )
    {
      if( face == f ) continue;

      // attach also periodic neighbours
      // this method only affects periodic neighbours
      myneighbour( f ).first->attachPeriodic( destination );
    }
  }

  template < class A >
  void TetraPllXBaseMacro< A >::attach2 (int i)
  {
    // don't attach elements twice
    if( _moveTo == -1 )
    {
      // set my destination
      _moveTo = i;

      // also move all faces to the same process
      mytetra ().myhface3 (0)->attach2 (i);
      mytetra ().myhface3 (1)->attach2 (i);
      mytetra ().myhface3 (2)->attach2 (i);
      mytetra ().myhface3 (3)->attach2 (i);
    }
  }

  template < class A >
  void TetraPllXBaseMacro< A > :: writeStaticState (ObjectStream & os, int face ) const
  {
    alugrid_assert ( ldbVertexIndex () >= 0 );
    os.writeObject (ldbVertexIndex ()) ;
    os.writeObject (master ());
  }

  template < class A >
  bool TetraPllXBaseMacro< A >::packAll (std::vector< ObjectStream > & osv)
  {
    return doPackAll( osv, (GatherScatterType * ) 0 );
  }

  template < class A >
  bool TetraPllXBaseMacro< A >::doPackAll (std::vector< ObjectStream > & osv,
                                           GatherScatterType* gs)
  {
    if( _moveTo >= 0 )
    {
      alugrid_assert ((osv.begin () + _moveTo) < osv.end ());
      return doPackLink( _moveTo, osv[ _moveTo ], gs );
    }
    return false;
  }

  template < class A >
  bool TetraPllXBaseMacro< A >::doPackLink( const int link, ObjectStream& os,
                                          GatherScatterType* gs)
  {
    os.writeObject (TETRA);
    os.writeObject (_ldbVertexIndex);
    os.writeObject (mytetra ().myvertex (0)->ident ());
    os.writeObject (mytetra ().myvertex (1)->ident ());
    os.writeObject (mytetra ().myvertex (2)->ident ());
    os.writeObject (mytetra ().myvertex (3)->ident ());

    // write bisection simplex type and orientation
    mytetra ().simplexTypeFlag().write( os );

    // make sure ENDOFSTREAM is not a valid refinement rule
    alugrid_assert ( ! mytetra_t::myrule_t::isValid (ObjectStream::ENDOFSTREAM) );

    // pack refinement information
    const int estimatedElements = mytetra ().backup ( os );
    os.put( ObjectStream::ENDOFSTREAM );

    // if gather scatter was passed
    if( gs )
    {
      // pack Dune data
      gs->inlineData( os , mytetra(), estimatedElements );
    }

    // unset erasable flag
    unset( flagLock );
    return true;
  }

  template < class A >
  bool TetraPllXBaseMacro< A >::dunePackAll (std::vector< ObjectStream > & osv,
                                             GatherScatterType & gs)
  {
    return doPackAll( osv, &gs );
  }

  template < class A >
  void TetraPllXBaseMacro< A >::packAsBndNow (int fce, ObjectStream & os, const bool packGhost ) const
  {
    os.writeObject (HBND3INT);
    os.writeObject ( Gitter::hbndseg::closure );
    alugrid_assert ( _ldbVertexIndex >= 0 );
    os.writeObject ( _ldbVertexIndex ); // write unique graph vertex index
    os.writeObject ( master() );
    os.writeObject ( mytetra ().myvertex (fce,0)->ident () );
    os.writeObject ( mytetra ().myvertex (fce,1)->ident () );
    os.writeObject ( mytetra ().myvertex (fce,2)->ident () );

    // see method unpackHbnd3Int
    if( packGhost )
    {
      alugrid_assert ( this->myGrid()->ghostCellsEnabled() );

      os.put( MacroGridMoverIF::POINTTRANSMITTED ); // write point info, point is transmitted

      // write ghost information
      MacroGhostInfoTetra::writeGhostInfo( os, fce, mytetra() );
    }
    else
    {
      os.put( MacroGridMoverIF::NO_POINT ); // no point transmitted
    }
  }

  // packs macro element as internal bnd for other proc
  template < class A >
  void TetraPllXBaseMacro< A >::packAsBnd (int fce, int who, ObjectStream & os, const bool ghostCellsEnabled) const
  {
    if( _moveTo != who )
    {
      // write data to stream
      packAsBndNow(fce, os, ghostCellsEnabled);
    }
  }

  // packs macro element as internal bnd for other proc
  template < class A >
  void TetraPllXBaseMacro< A >::packAsGhost(ObjectStream & os, int fce) const
  {
    alugrid_assert ( this->myGrid()->ghostCellsEnabled() );
    packAsBndNow(fce,os, true);
  }

  template < class A >
  void TetraPllXBaseMacro< A >::
  unpackSelf (ObjectStream & os, bool i )
  {
    duneUnpackSelf( os, i , ( GatherScatterType * ) 0 );
  }

  template < class A >
  void TetraPllXBaseMacro< A >::
  duneUnpackSelf (ObjectStream & os, const bool i, GatherScatterType* gatherScatter )
  {
    doUnpackSelf( os, i, gatherScatter );
  }

  template < class A >
  void TetraPllXBaseMacro< A >::
  doUnpackSelf (ObjectStream & os, const bool i, GatherScatterType* gatherScatter )
  {
    alugrid_assert (i);
    if (i)
    {
      // restore refinement information
      mytetra ().restore (os);

      char c = os.get();
      if( c != ObjectStream::ENDOFSTREAM )
      {
        std::cerr << "ERROR (fatal): c != ENDOFSTREAM." << std::endl;
        alugrid_assert(0);
        abort();
      }

      // unpack dune data if present, pointer can be zero
      if( gatherScatter )
      {
        // unpack Dune data
        gatherScatter->xtractData( os , mytetra() );
      }
    }
    else
    {
      std::cerr << "ERROR (fatal): i == false, should be true." << std::endl;
      alugrid_assert(0);
      abort();
    }
  }

  template < class A >
  bool TetraPllXBaseMacro< A >::erasable () const
  {
    // return true if tetra is not locked
    return ! isSet( flagLock );
  }

  // template instantiation
  template class TetraPllXBase< GitterBasisPll::ObjectsPll::TetraEmpty >;
  template class TetraPllXBaseMacro< GitterBasisPll::ObjectsPll::tetra_IMPL >;

  // ######                                                           #####
  // #     #  ######  #####      #     ####   #####      #     ####  #     #
  // #     #  #       #    #     #    #    #  #    #     #    #    #       #
  // ######   #####   #    #     #    #    #  #    #     #    #       #####
  // #        #       #####      #    #    #  #    #     #    #            #
  // #        #       #   #      #    #    #  #    #     #    #    # #     #
  // #        ######  #    #     #     ####   #####      #     ####   #####

  template < class A >
  void Periodic3PllXBase< A >::writeDynamicState (ObjectStream & os, int) const
  {
  }

  template < class A >
  Periodic3PllXBaseMacro< A >::
  Periodic3PllXBaseMacro ( int level, myhface3_t* f0,int s0, myhface3_t *f1,int s1, const Gitter::hbndseg_STI::bnd_t (&bt)[2] )
  : A(level, f0, s0, f1, s1, bt )
  , _moveTo ( -1 )
  {
    // don't allow erase
    set( flagLock );
  }

  template < class A >
  Periodic3PllXBaseMacro< A >::~Periodic3PllXBaseMacro ()
  {
    if( _moveTo >= 0 )
    {
      unattach2 ( _moveTo );
    }
  }

  template < class A >
  void Periodic3PllXBaseMacro< A >::unattach2 (int i)
  {
    alugrid_assert ( i>= 0 );
    myperiodic ().myhface3 (0)->unattach2 (i);
    myperiodic ().myhface3 (1)->unattach2 (i);
    _moveTo = -1;
    // unset erasable flag
    set( flagLock );
  }

  // return the first element's ldbVertexIndex (used in Periodic3PllXBaseMacro)
  template < class A >
  std::pair<int,int> Periodic3PllXBaseMacro< A >::insideLdbVertexIndex() const
  {
    return std::pair<int,int>  ( myneighbour( 0 ).first->firstLdbVertexIndex(),
                            myneighbour( 1 ).first->firstLdbVertexIndex() );
  }

  template < class A >
  int Periodic3PllXBaseMacro< A >::otherLdbVertexIndex( const int faceIndex ) const
  {
    if( myhface3( 0 )->getIndex() == faceIndex )
      return myneighbour( 1 ).first->firstLdbVertexIndex();
    else
    {
      alugrid_assert (  myhface3( 1 )->getIndex() == faceIndex  );
      return myneighbour( 0 ).first->firstLdbVertexIndex();
    }
  }

  template < class A >
  void Periodic3PllXBaseMacro< A >::attachPeriodic(const int destination)
  {
    //std::cout << "Attach periodic element" << std::endl;
    attach2( destination );
    alugrid_assert ( _moveTo == destination );
  }

  template < class A >
  void Periodic3PllXBaseMacro< A >::attach2 (int i)
  {
    if( _moveTo == -1 )
    {
      // store new destination
      _moveTo = i;

      myperiodic ().myhface3 (0)->attach2 (i);
      myperiodic ().myhface3 (1)->attach2 (i);

      // attach both neighbours to the same process
      for(int n=0; n<2; ++n )
      {
        typename A::myneighbour_t nb = this->myneighbour( n );
        nb.first->attachElement2( i, nb.second );
      }
    }
  }

  template < class A >
  bool Periodic3PllXBaseMacro< A >::packAll (std::vector< ObjectStream > & osv)
  {
    if( _moveTo >= 0 )
    {
      alugrid_assert ( myneighbour( 0 ).first->moveTo() == _moveTo );
      alugrid_assert ( myneighbour( 1 ).first->moveTo() == _moveTo );

      return doPackLink( _moveTo, osv[ _moveTo ] );
    }
    return false;
  }

  template < class A >
  bool Periodic3PllXBaseMacro< A >::doPackLink( const int link, ObjectStream& os )
  {
    if( _moveTo != link ) return false;

    alugrid_assert ( myneighbour( 0 ).first->moveTo() == link );
    alugrid_assert ( myneighbour( 1 ).first->moveTo() == link );

    os.writeObject (PERIODIC3);

    // write boundary id
    const int bnd[ 2 ] = { int( this->bndtype( 0 ) ), int( this->bndtype( 1 ) ) };
    os.writeObject ( bnd[ 0 ] );
    os.writeObject ( bnd[ 1 ] );

    os.writeObject (myperiodic ().myvertex (0)->ident ());
    os.writeObject (myperiodic ().myvertex (1)->ident ());
    os.writeObject (myperiodic ().myvertex (2)->ident ());
    os.writeObject (myperiodic ().myvertex (3)->ident ());
    os.writeObject (myperiodic ().myvertex (4)->ident ());
    os.writeObject (myperiodic ().myvertex (5)->ident ());

    // make sure ENDOFSTREAM is not a valid refinement rule
    alugrid_assert ( ! myperiodic_t::myrule_t::isValid (ObjectStream::ENDOFSTREAM) );

    // pack refinement information
    myperiodic ().backup ( os );
    os.put( ObjectStream::ENDOFSTREAM );

    // allow erasure
    unset( flagLock );
    return true;
  }

  template < class A >
  void Periodic3PllXBaseMacro< A >::
  packAsBnd (int fce, int who, ObjectStream & os, const bool ghostCellsEnabled) const
  {
    // we require that periodic element are never packed as boundary
    // since they are on the same process as their faces
    alugrid_assert ( _moveTo == who );
  }

  template < class A >
  void Periodic3PllXBaseMacro< A >::unpackSelf (ObjectStream & os, bool i)
  {
    alugrid_assert (i);

    if (i)
    {
      myperiodic ().restore ( os );

      char c = os.get();
      if( c != ObjectStream::ENDOFSTREAM )
      {
        std::cerr << "ERROR (fatal): c != ENDOFSTREAM." << std::endl;
        alugrid_assert(0);
        abort();
      }
    }
    else
    {
      std::cerr << "ERROR (fatal): i == false, should be true." << std::endl;
      alugrid_assert(0);
      abort();
    }
  }


  // ######                                                          #
  // #     #  ######  #####      #     ####   #####      #     ####  #    #
  // #     #  #       #    #     #    #    #  #    #     #    #    # #    #
  // ######   #####   #    #     #    #    #  #    #     #    #      #    #
  // #        #       #####      #    #    #  #    #     #    #      #######
  // #        #       #   #      #    #    #  #    #     #    #    #      #
  // #        ######  #    #     #     ####   #####      #     ####       #

  template < class A >
  void Periodic4PllXBase< A >::writeDynamicState (ObjectStream & os, int) const
  {
  }

  template < class A >
  Periodic4PllXBaseMacro< A >::
  Periodic4PllXBaseMacro ( int level, myhface4_t* f0,int s0, myhface4_t *f1,int s1, const Gitter::hbndseg_STI::bnd_t (&bt)[2] )
    : A(level, f0, s0, f1, s1, bt )
    , _moveTo ( -1 )
  {
    // don't allow erase
    set( flagLock );
  }

  template < class A >
  Periodic4PllXBaseMacro< A >::~Periodic4PllXBaseMacro ()
  {
    if ( _moveTo >= 0 )
    {
      unattach2( _moveTo );
    }
  }

  template < class A >
  void Periodic4PllXBaseMacro< A >::unattach2 (int i)
  {
    alugrid_assert (i >= 0 );
    _moveTo = -1;

    myperiodic ().myhface4 (0)->unattach2 (i);
    myperiodic ().myhface4 (1)->unattach2 (i);
  }

  // return the first element's ldbVertexIndex (used in Periodic3PllXBaseMacro)
  template < class A >
  std::pair<int,int> Periodic4PllXBaseMacro< A >::insideLdbVertexIndex() const
  {
    return std::pair<int,int>  ( myneighbour( 0 ).first->firstLdbVertexIndex(),
                            myneighbour( 1 ).first->firstLdbVertexIndex() );
  }

  template < class A >
  int Periodic4PllXBaseMacro< A >::otherLdbVertexIndex( const int faceIndex ) const
  {
    if( myhface4( 0 )->getIndex() == faceIndex )
      return myneighbour( 1 ).first->firstLdbVertexIndex();
    else
    {
      alugrid_assert (  myhface4( 1 )->getIndex() == faceIndex );
      return myneighbour( 0 ).first->firstLdbVertexIndex();
    }
  }

  template < class A >
  void Periodic4PllXBaseMacro< A >::attachPeriodic(const int destination)
  {
    attach2( destination );
  }

  template < class A >
  void Periodic4PllXBaseMacro< A >::attach2 (int i)
  {
    if( _moveTo == -1 )
    {
      //std::cout << "Attach periodic element to " << i << std::endl;
      // store my destination
      _moveTo = i;

      myperiodic ().myhface4 (0)->attach2 (i);
      myperiodic ().myhface4 (1)->attach2 (i);

      // attach both neighbours to the same process
      for(int n=0; n<2; ++n )
      {
        typename A::myneighbour_t nb = this->myneighbour( n );
        nb.first->attachElement2( i, nb.second );
      }
    }

    //std::cout << "Don't attach periodic element to " << i << " " << _moveTo <<std::endl;
  }

  template < class A >
  bool Periodic4PllXBaseMacro< A >::packAll (std::vector< ObjectStream > & osv)
  {
    if( _moveTo >= 0 )
    {
      // make sure the connected elements are moved to the same proc
      alugrid_assert ( myneighbour( 0 ).first->moveTo() == _moveTo );
      alugrid_assert ( myneighbour( 1 ).first->moveTo() == _moveTo );

      alugrid_assert ((osv.begin () + _moveTo) < osv.end ());
      return doPackLink( _moveTo, osv[ _moveTo ] );
    }
    return false ;
  }

  template < class A >
  bool Periodic4PllXBaseMacro< A >::doPackLink( const int link, ObjectStream& os )
  {
    if( _moveTo != link ) return false;

    alugrid_assert ( myneighbour( 0 ).first->moveTo() == _moveTo );
    alugrid_assert ( myneighbour( 1 ).first->moveTo() == _moveTo );

    os.writeObject (PERIODIC4);

    // write boundary id
    const int bnd[ 2 ] = { int( this->bndtype( 0 ) ), int( this->bndtype( 1 ) ) };
    os.writeObject ( bnd[ 0 ] );
    os.writeObject ( bnd[ 1 ] );

    os.writeObject (myperiodic ().myvertex (0)->ident ());
    os.writeObject (myperiodic ().myvertex (1)->ident ());
    os.writeObject (myperiodic ().myvertex (2)->ident ());
    os.writeObject (myperiodic ().myvertex (3)->ident ());
    os.writeObject (myperiodic ().myvertex (4)->ident ());
    os.writeObject (myperiodic ().myvertex (5)->ident ());
    os.writeObject (myperiodic ().myvertex (6)->ident ());
    os.writeObject (myperiodic ().myvertex (7)->ident ());

    // make sure ENDOFSTREAM is not a valid refinement rule
    alugrid_assert ( ! myperiodic_t::myrule_t::isValid (ObjectStream::ENDOFSTREAM) );

    // pack refinement information
    myperiodic ().backup ( os );
    os.put( ObjectStream::ENDOFSTREAM );

    // allow erase
    unset( flagLock );
    return true;
  }

  template < class A >
  void Periodic4PllXBaseMacro< A >::
  packAsBnd (int fce, int who, ObjectStream & os, const bool ghostCellsEnabled) const
  {
    // we require that periodic element are never packed as boundary
    // since they are on the same process as their faces
    // alugrid_assert ( _moveTo[ 0 ].first == who );
  }

  template < class A >
  void Periodic4PllXBaseMacro< A >::unpackSelf (ObjectStream & os, bool i)
  {
    alugrid_assert (i);
    if (i)
    {
      myperiodic ().restore (os);

      char c = os.get();
      if( c != ObjectStream::ENDOFSTREAM )
      {
        std::cerr << "ERROR (fatal): c != ENDOFSTREAM." << std::endl;
        alugrid_assert(0);
        abort();
      }
    }
    else
    {
      std::cerr << "ERROR (fatal): i == false, should be true." << std::endl;
      alugrid_assert(0);
      abort();
    }
  }

  template < class A >
  bool Periodic4PllXBaseMacro< A >::erasable () const {
    // return true if object can be erased
    return ! isSet( flagLock );
  }

    // #     #
    // #     #  ######  #    #    ##
    // #     #  #        #  #    #  #
    // #######  #####     ##    #    #
    // #     #  #         ##    ######
    // #     #  #        #  #   #    #
    // #     #  ######  #    #  #    #

  template < class A >
  void HexaPllBaseX< A > ::writeDynamicState (ObjectStream & os, GatherScatterType & gs) const
  {
    gs.sendData( os , myhexa () );
  }

  template < class A >
  void HexaPllBaseX< A >::writeDynamicState (ObjectStream & os, int face) const
  {
    // write level and leaf for the ghost element
    // to determine leafEntity or not
    const unsigned char lvl = myhexa().level();
    os.write( lvl );
    const bool leaf = myhexa().leaf();

    os.put( char( leaf ) );

    // Put numVertices of the element
    // >0 means, that at least one vertex has been projected
    const int numVerticesProjected = myhexa().vertexWasProjected() ? 8 : 0;
    os.put( numVerticesProjected );

    if( numVerticesProjected > 0 )
    {
      signed char fce = face ;
      os.put( fce );

      const auto& f1 = *(myhexa().myhface( face ));
      // store inside face
      for(int vx=0; vx<4; ++vx)
      {
        const alucoord_t (&p)[3] = f1.myvertex(vx)->Point();
        os.writeObject ( p[0] );
        os.writeObject ( p[1] );
        os.writeObject ( p[2] );
      }

      // store opposite face
      const int oppFace = Gitter::Geometric::Hexa::oppositeFace[face];
      const auto& f2 = *(myhexa().myhface( oppFace ));
      for(int vx=0; vx<4; ++vx)
      {
        const alucoord_t (&p)[3] = f2.myvertex(vx)->Point();
        os.writeObject ( p[0] );
        os.writeObject ( p[1] );
        os.writeObject ( p[2] );
      }

      // also store volume since the projection of vertices will change this
      const double vol = myhexa().volume();
      os.write( vol );
    }
  }

  template < class A >
  HexaPllBaseXMacro< A >::
  HexaPllBaseXMacro(int l, myhface4_t *f0, int s0, myhface4_t *f1, int s1,
                           myhface4_t *f2, int s2, myhface4_t *f3, int s3,
                           myhface4_t *f4, int s4, myhface4_t *f5, int s5)
  : A(l, f0, s0, f1, s1, f2, s2, f3, s3, f4, s4, f5, s5)
  , _moveTo ( -1 )
  , _ldbVertexIndex (-1)
  {
    // don't allow erase
    set( flagLock );
  }

  template < class A >
  HexaPllBaseXMacro< A >::~HexaPllBaseXMacro ()
  {
    if( _moveTo >= 0 )
    {
      unattach2( _moveTo );
    }
  }

  template < class A >
  void HexaPllBaseXMacro< A >::computeBaryCenter( alucoord_t (&center)[3] ) const
  {
    // calculate bary center
    TrilinearMapping::barycenter (
        myhexa ().myvertex (0)->Point (),
        myhexa ().myvertex (1)->Point (),
        myhexa ().myvertex (2)->Point (),
        myhexa ().myvertex (3)->Point (),
        myhexa ().myvertex (4)->Point (),
        myhexa ().myvertex (5)->Point (),
        myhexa ().myvertex (6)->Point (),
        myhexa ().myvertex (7)->Point (),
        center );
  }

  template < class A >
  int HexaPllBaseXMacro< A >::ldbVertexIndex () const {
    alugrid_assert ( _ldbVertexIndex >= 0 );
    return _ldbVertexIndex;
  }

  template < class A >
  void HexaPllBaseXMacro< A >::setLoadBalanceVertexIndex ( const int ldbVx )
  {
    _ldbVertexIndex = ldbVx ;
  }

  template < class A >
  void HexaPllBaseXMacro< A >::
  computeVertexLinkage( vertexelementlinkage_t& vxElemLinkge )
  {
    for( int i=0; i<8; ++i )
    {
      // add my ldb vertex index to vertex's list of elements
      vxElemLinkge[ myhexa().myvertex( i ) ].insert( _ldbVertexIndex );
    }
  }

  template < class A >
  bool HexaPllBaseXMacro< A >::ldbUpdateGraphVertex (LoadBalancer::DataBase & db, GatherScatter* gs )
  {
    // parameter for GraphVertex are:
    // - macro vertex index
    // - number of elementes below macro element
    // - pointer to this element in case bary center is needed (only if GRAPHVERTEX_WITH_CENTER defined)
    typedef TreeIterator < Gitter::helement_STI, is_leaf < Gitter::helement_STI > >  TreeIteratorType;

    // get macro element weight
    // if gs is not null, then use the weight provided by gs
    // otherwise count number of leaf elements
    const int weight = ( gs ) ? gs->loadWeight( myhexa() ) :
                                TreeIteratorType( myhexa() ).size ();

    db.vertexUpdate ( LoadBalancer::GraphVertex (ldbVertexIndex (), weight, *this ) );
    return true;
  }

  template < class A >
  void HexaPllBaseXMacro< A >::unattach2 (int i)
  {
    alugrid_assert ( i >= 0 );
    myhexa ().myhface4 (0)->unattach2 (i);
    myhexa ().myhface4 (1)->unattach2 (i);
    myhexa ().myhface4 (2)->unattach2 (i);
    myhexa ().myhface4 (3)->unattach2 (i);
    myhexa ().myhface4 (4)->unattach2 (i);
    myhexa ().myhface4 (5)->unattach2 (i);

    // reset moveTo
    _moveTo = -1;
  }

  template < class A >
  void HexaPllBaseXMacro< A >::attachElement2 (const int destination, const int face)
  {
    attach2( destination );
    // check all neighbours
    // face is the face this method was called from
    for( int f=0; f<6; ++f )
    {
      if( face == f ) continue;

      //std::cout << "Attach hexa neighbor to " << destination << "  " << _moveTo << std::endl;
      // attach also periodic neighbours
      // this method only affects periodic neighbours
      myneighbour( f ).first->attachPeriodic( destination );
    }
  }

  template < class A >
  void HexaPllBaseXMacro< A > :: writeStaticState (ObjectStream & os, int face ) const
  {
    alugrid_assert ( ldbVertexIndex () >= 0 );
    os.writeObject (ldbVertexIndex ()) ;
    os.writeObject (master ());
  }

  template < class A >
  void HexaPllBaseXMacro< A >::attach2 (int i)
  {
    // don't attach elements twice
    if( _moveTo == -1 )
    {
      //std::cout << "Attach hexa to " << i << std::endl;
      // store new destination
      _moveTo = i;

      // also attach all my faces
      myhexa ().myhface4 (0)->attach2 (i);
      myhexa ().myhface4 (1)->attach2 (i);
      myhexa ().myhface4 (2)->attach2 (i);
      myhexa ().myhface4 (3)->attach2 (i);
      myhexa ().myhface4 (4)->attach2 (i);
      myhexa ().myhface4 (5)->attach2 (i);
    }

    //std::cout << "Don't attach hexa to " << i << " " << _moveTo << std::endl;
  }

  // pack all function for dune
  template < class A >
  bool HexaPllBaseXMacro< A >::doPackAll (std::vector< ObjectStream > & osv,
                                          GatherScatterType* gs)
  {
    if( _moveTo >= 0 )
    {
      alugrid_assert ((osv.begin () + _moveTo) < osv.end ());
      return doPackLink( _moveTo, osv[ _moveTo ], gs );
    }
    return false;
  }

  // pack all function for dune
  template < class A >
  bool HexaPllBaseXMacro< A >::doPackLink ( const int link, ObjectStream& os,
                                            GatherScatterType* gs)
  {
    os.writeObject (HEXA);
    alugrid_assert ( _ldbVertexIndex >= 0 );
    os.writeObject (_ldbVertexIndex );
    os.writeObject (myhexa ().myvertex (0)->ident ());
    os.writeObject (myhexa ().myvertex (1)->ident ());
    os.writeObject (myhexa ().myvertex (2)->ident ());
    os.writeObject (myhexa ().myvertex (3)->ident ());
    os.writeObject (myhexa ().myvertex (4)->ident ());
    os.writeObject (myhexa ().myvertex (5)->ident ());
    os.writeObject (myhexa ().myvertex (6)->ident ());
    os.writeObject (myhexa ().myvertex (7)->ident ());

    // make sure ENDOFSTREAM is not a valid refinement rule
    alugrid_assert ( ! myhexa_t::myrule_t::isValid (ObjectStream::ENDOFSTREAM) );

    // backup refinement information (1 char per element)
    const int estimatedElements = myhexa(). backup ( os );
    os.put( ObjectStream::ENDOFSTREAM );

    if( gs )
    {
      // pack Dune data
      gs->inlineData( os , myhexa(), estimatedElements );
    }

    //allow erase
    unset( flagLock );
    return true;
  }


  // packall without gather scatter
  template < class A >
  bool HexaPllBaseXMacro< A >::packAll (std::vector< ObjectStream > & osv)
  {
    return doPackAll( osv, ( GatherScatterType* ) 0 );
  }

  // pack all function for dune
  template < class A >
  bool HexaPllBaseXMacro< A >::dunePackAll (std::vector< ObjectStream > & osv,
                                            GatherScatterType & gs)
  {
    return doPackAll( osv, &gs );
  }

  template < class A >
  void HexaPllBaseXMacro< A >::packAsBndNow(int fce, ObjectStream & os, const bool packGhost ) const
  {
    os.writeObject (HBND4INT);
    os.writeObject (Gitter::hbndseg::closure);
    alugrid_assert ( _ldbVertexIndex >= 0 );
    os.writeObject (_ldbVertexIndex ); // write unique graph vertex index
    os.writeObject ( master() ); // write unique graph vertex index

    // write the four identifiers of the hexa
    os.writeObject (myhexa ().myvertex (fce,0)->ident ());
    os.writeObject (myhexa ().myvertex (fce,1)->ident ());
    os.writeObject (myhexa ().myvertex (fce,2)->ident ());
    os.writeObject (myhexa ().myvertex (fce,3)->ident ());

    // see method unpackHbnd4Int
    if( packGhost )
    {
      alugrid_assert ( this->myGrid()->ghostCellsEnabled() );

      os.put( MacroGridMoverIF::POINTTRANSMITTED ); // 1 == points are transmitted

      // write ghost information
      MacroGhostInfoHexa::writeGhostInfo( os, fce, myhexa() );
    }
    else
    {
      os.put( MacroGridMoverIF::NO_POINT ); // no point transmitted
    }
  }

  template < class A >
  void HexaPllBaseXMacro< A >::packAsGhost(ObjectStream & os, int fce) const
  {
    alugrid_assert ( this->myGrid()->ghostCellsEnabled() );
    packAsBndNow(fce, os, true );
  }

  // packs macro element as internal bnd for other proc
  template < class A >
  void HexaPllBaseXMacro< A >::
  packAsBnd (int fce, int who, ObjectStream & os, const bool ghostCellsEnabled) const
  {
    if ( _moveTo != who )
    {
      packAsBndNow( fce, os, ghostCellsEnabled );
    }
    return;
  }

  template < class A >
  void HexaPllBaseXMacro< A >::
  unpackSelf (ObjectStream & os,
              const bool i )
  {
    doUnpackSelf( os, i , ( GatherScatterType * ) 0 );
  }

  template < class A >
  void HexaPllBaseXMacro< A >::
  duneUnpackSelf (ObjectStream & os,
                  const bool i,
                  GatherScatterType* gatherScatter )
  {
    doUnpackSelf( os, i, gatherScatter );
  }

  template < class A >
  void HexaPllBaseXMacro< A >::
  doUnpackSelf (ObjectStream & os,
                const bool i,
                GatherScatterType* gatherScatter )
  {
    alugrid_assert (i);
    if (i)
    {
      // unpack refinement data and restore
      myhexa ().restore ( os );

      // stream should now be at position ENDOFSTREAM
      char c = os.get();
      if( c != ObjectStream::ENDOFSTREAM )
      {
        std::cerr << "ERROR (fatal): c != ENDOFSTREAM." << std::endl;
        alugrid_assert(0);
        abort();
      }

      // unpack dune data if present, pointer can be zero
      if( gatherScatter )
        gatherScatter->xtractData( os , myhexa() );
    }
    else
    {
      std::cerr << "ERROR (fatal): i == false, should be true." << std::endl;
      alugrid_assert(0);
      abort();
    }
  }

  template < class A >
  bool HexaPllBaseXMacro< A >::erasable () const
  {
    // return true if object can be deleted
    return ! isSet( flagLock );
  }

  // template instatiation
  template class HexaPllBaseX< GitterBasisPll::ObjectsPll::HexaEmpty >;
  template class HexaPllBaseXMacro< GitterBasisPll::ObjectsPll::hexa_IMPL >;

  ///////////////////////////////////////////////////////////
  //
  //  --BndsegPllBaseX
  //
  ///////////////////////////////////////////////////////////

  std::pair< ElementPllXIF_t *, int > BndsegPllBaseX::accessOuterPllX (const std::pair< ElementPllXIF_t *, int > &, int f) {
    alugrid_assert (!f);
    return std::pair< ElementPllXIF_t *, int > (this,0);
  }

  std::pair< ElementPllXIF_t *, int > BndsegPllBaseX::accessInnerPllX (const std::pair< ElementPllXIF_t *, int > & x, int) {
    return x;
  }

  std::pair< const ElementPllXIF_t *, int > BndsegPllBaseX::accessOuterPllX (const std::pair< const ElementPllXIF_t *, int > &, int f) const {
    alugrid_assert (!f);
    return std::pair< const ElementPllXIF_t *, int > (this,0);
  }

  std::pair< const ElementPllXIF_t *, int > BndsegPllBaseX::accessInnerPllX (const std::pair< const ElementPllXIF_t *, int > & x, int) const {
    return x;
  }

  GitterBasisPll::ObjectsPll::Hface3EmptyPllMacro::
  Hface3EmptyPllMacro (myhedge_t * e0, int s0, myhedge_t *e1,int s1, myhedge_t *e2, int s2)
    : Base_t(0, e0, s0, e1, s1, e2, s2) // 0 == level 0
  {
  }

  GitterBasisPll::ObjectsPll::Hface4EmptyPllMacro::
  Hface4EmptyPllMacro (myhedge_t *e0, int s0, myhedge_t *e1, int s1,
                       myhedge_t *e2, int s2, myhedge_t *e3, int s3)
    : Base_t(0, e0, s0, e1, s1, e2, s2, e3, s3) // 0 == level 0
  {
  }

  ////////////////////////////////////////////////////////////////
  //  --MacroGitterBasisPll
  ////////////////////////////////////////////////////////////////
  GitterBasisPll::MacroGitterBasisPll::
  MacroGitterBasisPll ( const int dim, GitterBasisPll * mygrid, const ProjectVertexPtrPair& ppv, std::istream &in )
    : GitterPll::MacroGitterPll (),
      GitterBasis:: MacroGitterBasis (dim, mygrid, ppv),
      _linkagePatterns( indexManagerStorage().linkagePatterns() )
  {
    macrogridBuilder (in );
    indexManagerStorage().setRank( mygrid->mpAccess().myrank() );
  }

  GitterBasisPll::MacroGitterBasisPll::
  MacroGitterBasisPll (const int dim, GitterBasisPll * mygrid, const ProjectVertexPtrPair& ppv)
   : GitterPll::MacroGitterPll () ,
     GitterBasis::MacroGitterBasis (dim, mygrid, ppv),
     _linkagePatterns( indexManagerStorage().linkagePatterns() )
  {
    indexManagerStorage().setRank( mygrid->mpAccess().myrank() );
  }

  GitterBasisPll::MacroGitterBasisPll::~MacroGitterBasisPll ()
  {
    try
    {
      {
        AccessIterator < helement_STI >::Handle w (*this);
        for (w.first (); ! w.done (); w.next ()) w.item ().detachPllXFromMacro ();
      }
    }
    catch( Parallel::AccessPllException& )
    {
      std::cerr << "WARNING (ignored): Ignoring AccessPllException." << std::endl;
    }
  }

  void GitterBasisPll::MacroGitterBasisPll::clearLinkagePattern()
  {
    // clear linkage pattern map
    _linkagePatterns.clear();
  }

  void GitterBasisPll::MacroGitterBasisPll::secondScan ( std::set< int >& s )
  {
    // clear set
    s.clear();

    int n = 0;
    const linkagePatternMap_t::iterator pEnd = _linkagePatterns.end ();
    for (linkagePatternMap_t::iterator p = _linkagePatterns.begin (); p != pEnd; )
    {
      // if linkage exists
      if ((*p).second)
      {
        typedef std::vector< int >::const_iterator  const_iterator ;
        const const_iterator iEnd = (*p).first.end ();
        // insert all links into set
        for ( const_iterator i = (*p).first.begin (); i != iEnd; ++i )
        {
          s.insert( *i );
        }
        // next pattern
        ++p;
      }
      // else remove vertex from pattern list
      else
      {
        _linkagePatterns.erase ( p++ );
        ++n;
      }
    }
    alugrid_assert (debugOption (20) ? (std::cout << "  GitterBasisPll::MacroGitterBasisPll::secondScan () deleted " << n << " patterns" << std::endl, 1) : 1);
  }

  Gitter::Geometric::VertexGeo * GitterBasisPll::MacroGitterBasisPll::
  insert_vertex (double x,double y,double z,int i) {
    return new ObjectsPll::VertexPllImplMacro (x, y, z, i, indexManagerStorage(), _linkagePatterns);
  }

  Gitter::Geometric::VertexGeo * GitterBasisPll::MacroGitterBasisPll::
  insert_ghostvx (double x, double y, double z, int i)
  {
    return GitterBasis::MacroGitterBasis::insert_vertex (x,y,z,i);
  }

  Gitter::Geometric::hedge1_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_hedge1 (VertexGeo *a, VertexGeo *b)
  {
    return new ObjectsPll::Hedge1EmptyPllMacro (a,b);
  }

  Gitter::Geometric::hedge1_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_hedge1_twist (VertexGeo *a, int aid,  VertexGeo *b , int bid )
  {
    if(aid < bid)
      return GitterBasis::MacroGitterBasis::insert_hedge1 (a,b);
    else
      return GitterBasis::MacroGitterBasis::insert_hedge1 (b,a);
  }

  Gitter::Geometric::hface4_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_hface4 (hedge1_GEO *(&e)[4], int (&s)[4])
  {
    return new ObjectsPll::Hface4EmptyPllMacro (e [0], s [0], e [1], s [1], e [2], s [2], e [3], s [3]);
  }

  Gitter::Geometric::hface3_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_hface3 (hedge1_GEO *(&e)[3], int (&s)[3])
  {
    return new ObjectsPll::Hface3EmptyPllMacro (e [0], s [0], e [1], s [1], e [2], s [2] );
  }

  Gitter::Geometric::hexa_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_hexa (hface4_GEO *(&f)[6], int (&t)[6])
  {
    return new ObjectsPll::HexaEmptyPllMacro (f [0], t[0], f [1], t[1], f [2], t[2], f[3], t[3], f[4], t[4], f[5], t[5]);
  }

  Gitter::Geometric::tetra_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_tetra (hface3_GEO *(&f)[4], int (&t)[4], SimplexTypeFlag simplexType )
  {
    return new ObjectsPll::TetraEmptyPllMacro (f [0], t[0], f [1], t[1], f [2], t[2], f[3], t[3], simplexType );
  }

  Gitter::Geometric::periodic3_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_periodic3 (hface3_GEO *(&f)[2], int (&t)[2],
                    const Gitter::hbndseg_STI::bnd_t (&bt)[2] )
  {
    return new ObjectsPll::Periodic3EmptyPllMacro (f [0], t[0], f [1], t[1], bt);
  }

  Gitter::Geometric::periodic4_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_periodic4 (hface4_GEO *(&f)[2], int (&t)[2],
                    const Gitter::hbndseg_STI::bnd_t (&bt)[2] )
  {
    return new ObjectsPll::Periodic4EmptyPllMacro (f [0], t[0], f [1], t[1], bt );
  }

  Gitter::Geometric::hbndseg4_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_hbnd4 (hface4_GEO * f, int t, Gitter::hbndseg_STI::bnd_t b)
  {
    typedef GitterBasis::Objects::Hbnd4Default Hbnd4DefaultType;
    if (b == Gitter::hbndseg_STI::closure)
    {
      // internal face always get dummy index manager
      return new Hbnd4PllInternal < Hbnd4DefaultType , BndsegPllBaseXClosure < Hbnd4DefaultType > ,
            BndsegPllBaseXMacroClosure < Hbnd4DefaultType > >::macro_t (f,t, b, *this );
    }
    else
    {
      return new Hbnd4PllExternal < Hbnd4DefaultType, BndsegPllBaseXMacro < hbndseg4_GEO > > (f,t, b );
    }
  }


  Gitter::Geometric::hbndseg4_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_hbnd4 (hface4_GEO * f, int t,
                Gitter::hbndseg_STI::bnd_t b,
                MacroGhostInfoHexa* ghInfo)
  {
    typedef GitterBasis::Objects::Hbnd4Default Hbnd4DefaultType;
    // if internal boundary create ghost
    if (b == Gitter::hbndseg_STI::closure )
    {
      if( ! indexManagerStorage().myGrid()->ghostCellsEnabled() )
        return insert_hbnd4( f, t, b );

      alugrid_assert ( ghInfo );
      return new Hbnd4PllInternal < Hbnd4DefaultType , BndsegPllBaseXClosure < Hbnd4DefaultType > ,
            BndsegPllBaseXMacroClosure < Hbnd4DefaultType > >::
            macro_t (f,t, b, *this, ghInfo );
    }
    else
    {
      return new Hbnd4PllExternal < Hbnd4DefaultType ,
          BndsegPllBaseXMacro < hbndseg4_GEO > > (f,t, b );
    }
  }

  // version with point
  Gitter::Geometric::hbndseg3_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_hbnd3 (hface3_GEO * f, int t,
                Gitter::hbndseg_STI::bnd_t b,
                MacroGhostInfoTetra * ghInfo)
  {
    typedef GitterBasis::Objects::Hbnd3Default Hbnd3DefaultType;
    if (b == Gitter::hbndseg_STI::closure)
    {
      if( ! indexManagerStorage().myGrid()->ghostCellsEnabled() )
        return insert_hbnd3( f, t, b );

      alugrid_assert ( ghInfo );
      // this HbnPll has a ghost element so is dosent get and index ==> dummyindex == 5 (see gitter_sti.h)
      return new Hbnd3PllInternal < Hbnd3DefaultType , BndsegPllBaseXClosure < Hbnd3DefaultType > ,
            BndsegPllBaseXMacroClosure < Hbnd3DefaultType > >::
                macro_t (f,t, b, *this, ghInfo );
    }
    else
    {
      return new Hbnd3PllExternal < Hbnd3DefaultType ,
          BndsegPllBaseXMacro < hbndseg3_GEO > > (f,t, b );
    }
  }

  // version without point
  Gitter::Geometric::hbndseg3_GEO * GitterBasisPll::MacroGitterBasisPll::
  insert_hbnd3 (hface3_GEO * f, int t,
                Gitter::hbndseg_STI::bnd_t b )
  {
    typedef GitterBasis::Objects::Hbnd3Default Hbnd3DefaultType;
    if (b == Gitter::hbndseg_STI::closure)
    {
      // here we have a ghost of the ghost, therefor we need the element index manager
      return new Hbnd3PllInternal < Hbnd3DefaultType , BndsegPllBaseXClosure < Hbnd3DefaultType > ,
            BndsegPllBaseXMacroClosure < Hbnd3DefaultType > >::macro_t (f,t, b, *this );
    }
    else
    {
      return new Hbnd3PllExternal < Hbnd3DefaultType ,
             BndsegPllBaseXMacro < hbndseg3_GEO > > (f,t, b );
    }
  }

  GitterBasisPll::GitterBasisPll (const int dim, MpAccessLocal & mpa)
    : GitterPll(mpa),
      _mpaccess(mpa), _macrogitter (0)
  {
    ProjectVertexPtrPair ppv;
    _macrogitter = new MacroGitterBasisPll (dim, this, ppv);

    dumpInfo();

    alugrid_assert (_macrogitter);
    notifyMacroGridChanges ();
    return;
  }

  GitterBasisPll::GitterBasisPll ( const int dim, const std::string &filename, MpAccessLocal & mpa, const ProjectVertexPtrPair& ppv )
  : GitterPll( mpa ),
    _mpaccess( mpa ),
    _macrogitter( 0 )
  {
    alugrid_assert (debugOption (20) ? (std::cout << "GitterBasisPll::GitterBasisPll (const char * = \"" << filename << "\" ...)" << std::endl, 1) : 1);

    const int myrank = mpa.myrank();
    std::stringstream rank;
    rank << "." << myrank;

    // if still no macrogitter, try old method
    if(!_macrogitter)
    {
      std::string extendedName ( filename );
      extendedName += rank.str();

      std::ifstream in( extendedName.c_str() );
      if( in )
        _macrogitter = new MacroGitterBasisPll (dim, this, ppv, in);
      else
      {
        alugrid_assert (debugOption (5) ?
          ( std::cerr << "  GitterBasisPll::GitterBasisPll () file: " << extendedName
             << " cannot be read. Try " << filename << " instead. In " << __FILE__ << " line " << __LINE__ << std::endl, 1) : 1);
      }
    }

    // only check this for higher ranks
    // we assume that filename already contains rank info
    // if not empty grid is created
    bool validFilename = true;
    if( ! _macrogitter && myrank > 0 )
    {
      // search rank info in filename
      const int result = filename.rfind( rank.str() );
      // if not found then filename is not valid
      // and empty grid should be created
      if( result == -1 )
      {
        validFilename = false;
      }
    }

    // read normal macro gitter if not created yet and
    // filename is valid for this rank
    if( ! _macrogitter && validFilename )
    {
      std::ifstream in( filename.c_str() );
      if( in )
        _macrogitter = new MacroGitterBasisPll (dim, this, ppv, in);
    }

    // create empty macro gitter
    if(!_macrogitter) _macrogitter = new MacroGitterBasisPll (dim, this, ppv);

    dumpInfo();

    alugrid_assert (_macrogitter);
    notifyMacroGridChanges ();
    return;
  }

  GitterBasisPll::GitterBasisPll ( const int dim, std::istream &in, MpAccessLocal &mpa, const ProjectVertexPtrPair& ppv )
  : GitterPll( mpa ),
    _mpaccess( mpa ),
    _macrogitter( 0 )
  {
    alugrid_assert (debugOption (20) ? (std::cout << "GitterBasisPll::GitterBasisPll ( istream& = \"" << &in << "\" ...)" << std::endl, 1) : 1);

    _macrogitter = new MacroGitterBasisPll (dim, this, ppv, in);

    dumpInfo();

    alugrid_assert ( _macrogitter );
    notifyMacroGridChanges ();
  }

  GitterBasisPll::~GitterBasisPll ()
  {
    delete _macrogitter;
  }

  void GitterBasisPll::printMemUsage ()
  {
    const int psize  = _mpaccess.psize();
    const int myrank = _mpaccess.myrank();

    for( int rank = 0; rank < psize; ++ rank )
    {
      _mpaccess.barrier();
      if( rank != myrank ) continue;

      typedef GitterBasisPll::ObjectsPll::TetraEmptyPllMacro tetra_MACRO;
      typedef GitterBasisPll::ObjectsPll::HexaEmptyPllMacro  hexa_MACRO;
      typedef GitterBasisPll::ObjectsPll::hbndseg3_IMPL hbndseg3_IMPL;
      typedef GitterBasisPll::ObjectsPll::hbndseg4_IMPL hbndseg4_IMPL;
      //typedef GitterBasisPll::ObjectsPll::Hface3EmptyPllMacro hface3_IMPL;
      //typedef GitterBasisPll::ObjectsPll::Hface4EmptyPllMacro hface4_IMPL;
      //typedef GitterBasisPll::ObjectsPll::Hedge1EmptyPllMacro hedge1_IMPL;
      //typedef GitterBasisPll::ObjectsPll::VertexPllImplMacro VertexMacro;
      typedef GitterBasisPll::ObjectsPll::tetra_IMPL tetra_IMPL;
      typedef GitterBasisPll::ObjectsPll::hexa_IMPL  hexa_IMPL;
      typedef GitterBasisPll::ObjectsPll::hbndseg3_IMPL hbndseg3_IMPL;
      typedef GitterBasisPll::ObjectsPll::hbndseg4_IMPL hbndseg4_IMPL;
      typedef GitterBasisPll::ObjectsPll::hface3_IMPL hface3_IMPL;
      typedef GitterBasisPll::ObjectsPll::Hface3EmptyPllMacro  hface3_MACRO;
      typedef GitterBasisPll::ObjectsPll::hface4_IMPL hface4_IMPL;
      typedef GitterBasisPll::ObjectsPll::Hface4EmptyPllMacro  hface4_MACRO;
      typedef GitterBasisPll::ObjectsPll::hedge1_IMPL hedge1_IMPL;
      typedef GitterBasisPll::ObjectsPll::Hedge1EmptyPllMacro  hedge1_MACRO;
      typedef GitterBasisPll::ObjectsPll::VertexPllImplMacro VertexMacro;
      typedef GitterBasis::DuneIndexProvider DuneIndexProvider;
      typedef GitterBasis::Objects::VertexEmptyMacro VertexEmptyMacro;
      typedef GitterBasis::Objects::VertexEmpty VertexEmpty;
      typedef Gitter::Geometric::VertexGeo VertexGeo;
      typedef TreeIterator< Gitter::helement_STI, is_leaf< Gitter::helement_STI > > TreeIter;

      if( rank == 0 )
      {
        std::cout << "bool   = " << sizeof(bool) << std::endl;
        std::cout << "char   = " << sizeof(unsigned char) << std::endl;
        std::cout << "signed char   = " << sizeof(signed char) << std::endl;
        std::cout << "moveto = " << sizeof( MacroGridMoverIF::moveto_t ) << std::endl;
        std::cout << "MyAlloc = " << sizeof(MyAlloc) << "\n";
        std::cout << "Refcount = " << sizeof(Refcount) << "\n";
        std::cout << "HedgeRule  = " << sizeof(Gitter::Geometric::Hedge1Rule) <<"\n";
        std::cout << "Hface3Rule = " << sizeof(Gitter::Geometric::Hface3Rule) <<"\n";
        std::cout << "Hface4Rule = " << sizeof(Gitter::Geometric::Hface4Rule) <<"\n";
        std::cout << "DuneIndexProvider = "<< sizeof(DuneIndexProvider) << "\n";
        std::cout << "GraphVertex = " << sizeof(LoadBalancer::GraphVertex) << std::endl;
        std::cout << "GraphEdge   = " << sizeof(LoadBalancer::GraphEdge) << std::endl;
        std::cout << "Identifier  = " << sizeof(LinkedObject::Identifier) << std::endl << std::endl;

        std::cout << "******** TETRA *************************8\n";
        std::cout << "Tetrasize  = " << sizeof(tetra_IMPL) << std::endl;
        std::cout << "TetraMacro = " << sizeof(tetra_MACRO) << std::endl;
        std::cout << "MacroGhostTetra = " << sizeof(MacroGhostTetra) << std::endl;
        std::cout << "Hface3_MACRO = " << sizeof(hface3_MACRO) << std::endl;
        std::cout << "Hface3_IMPL = " << sizeof(hface3_IMPL) << std::endl;
        std::cout << "Hface3_GEO = " << sizeof( Gitter::Geometric::hface3_GEO ) << std::endl;
        std::cout << "Hface3::nb = " << sizeof( Gitter::Geometric::hface3::face3Neighbour ) << std::endl;
        std::cout << "HEdge1_MACRO = " << sizeof(hedge1_MACRO) << std::endl;
        std::cout << "HEdge1_IMPL = " << sizeof(hedge1_IMPL) << std::endl;
        std::cout << "HEdge1_GEO = " << sizeof(Gitter::Geometric ::hedge1_GEO) << std::endl;
        std::cout << "VertexMacroPll = " << sizeof(VertexMacro) << std::endl;
        std::cout << "VertexMacro = " << sizeof(VertexEmptyMacro) << std::endl;
        std::cout << "VertexGeo   = " << sizeof(VertexGeo) << std::endl;
        std::cout << "Vertex = " << sizeof(VertexEmpty) << std::endl;
        std::cout << "Hbnd3_IMPL  = " << sizeof(hbndseg3_IMPL) << std::endl;
        std::cout << "MacroGhostInfoTetra = " << sizeof(MacroGhostInfoTetra) << std::endl << std::endl;

        std::cout << "******** HEXA *************************8\n";
        std::cout << "Hexasize  = " << sizeof(hexa_IMPL) << std::endl;
        std::cout << "HexaMacro = " << sizeof(hexa_MACRO) << std::endl;
        std::cout << "MacroGhostHexa = " << sizeof(MacroGhostHexa) << std::endl;
        std::cout << "Hface4_MACRO = " << sizeof(hface4_MACRO) << std::endl;
        std::cout << "Hface4_IMPL = " << sizeof(hface4_IMPL) << std::endl;
        std::cout << "Hface4_GEO = " << sizeof( Gitter::Geometric::hface4_GEO ) << std::endl;
        std::cout << "Hface4::nb = " << sizeof( Gitter::Geometric::hface4::face4Neighbour ) << std::endl;
        std::cout << "Hbnd4_IMPL  = " << sizeof(hbndseg4_IMPL) << std::endl;
        std::cout << "MacroGhostInfoHexa = " << sizeof(MacroGhostInfoHexa) << std::endl << std::endl;

        std::cout << "TreeIterator< A, B > = " << sizeof( TreeIter) << std::endl;

      }

      std::cout << "******** Number of Elements ************************8\n";
      {
        size_t totalSize = 0;
        bool simplex = false;
        {
          AccessIterator < helement_STI >::Handle iter (container ());
          int size = iter.size();
          iter.first();
          if( !iter.done() )
          {
            if( iter.item().type() == tetra )
            {
              simplex = true;
              size *= sizeof(tetra_IMPL);
            }
            else
            {
              size *= sizeof(hexa_IMPL);
            }
          }
          totalSize += size;
          std::cout << "Macro elements: size = " << size/1024/1024 << " MB \n";
        }

        {
          int size = AccessIterator < hbndseg_STI >::Handle (container ()).size();
          size *= (simplex) ?  sizeof(hbndseg3_IMPL) : sizeof(hbndseg4_IMPL);
          std::cout << "Macro boundary : size = " << size/1024/1024 << " MB \n";
          totalSize += size;
        }

        {
          int size = AccessIterator < hface_STI >::Handle (container ()).size();
          size *= (simplex) ?  sizeof(hface3_IMPL) : sizeof(hface4_IMPL);
          std::cout << "Macro faces : size = " << size/1024/1024 << " MB \n";
          totalSize += size;
        }

        {
          int size = AccessIterator < hedge_STI >::Handle (container ()).size();
          size *= sizeof(hedge1_IMPL);
          std::cout << "Macro edges : size = " << size/1024/1024 << " MB \n";
          totalSize += size;
        }

        {
          int size = AccessIterator < vertex_STI >::Handle (container ()).size();
          size *= sizeof(VertexEmptyMacro);
          std::cout << "Macro vertices : size = " << size/1024/1024 << " MB \n";
          totalSize += size;
        }

        size_t allSize = 0;
        size_t numElements = 0;
        {
          LeafIterator< helement_STI > it( *this );
          int size = it->size();
          numElements = size;
          size *= (simplex ? sizeof( tetra_IMPL ) : sizeof( hexa_IMPL ));
          std::cout << "Elements : size = " << size/1024/1024 << " MB" << std::endl;
          allSize += size;
        }

        {
          LeafIterator< hbndseg_STI > it( *this );
          const int size = it->size() * (simplex ? sizeof( hbndseg3_IMPL ) : sizeof( hbndseg4_IMPL ));
          std::cout << "Boundaries : size = " << size/1024/1024 << " MB" << std::endl;
          allSize += size;
        }

        {
          LeafIterator< hface_STI > it( *this );
          const int size = it->size() * (simplex ? sizeof( hface3_IMPL ) : sizeof( hface4_IMPL ));
          std::cout << "Faces : size = " << size/1024/1024 << " MB" << std::endl;
          allSize += size;
        }

        {
          LeafIterator< hedge_STI > it( *this );
          const int size = it->size() * sizeof( hedge1_IMPL );
          std::cout << "Edges : size = " << size/1024/1024 << " MB" << std::endl;
          allSize += size;
        }

        {
          LeafIterator< vertex_STI > it( *this );
          const int size = it->size() * sizeof( VertexEmpty );
          std::cout << "Vertices : size = " << size/1024/1024 << " MB" << std::endl;
          allSize += size;
        }

        {
          size_t indexMem = 0;
          for(int i=0; i<4; ++i)
          {
            std::cout << "P[ " << _mpaccess.myrank() << " ] max index codim " << i << " = " << indexManager( i ).getMaxIndex() << std::endl;
            indexMem += indexManager( i ).getMaxIndex() * sizeof( int );
          }
          std::cout << "Indices : size = " << indexMem/1024/1024 << " MB" << std::endl;
          allSize += indexMem;
        }

        size_t perElement = ( numElements > 0 ) ? allSize/numElements : 0;
        std::cout << "All leaf size : " << allSize/1024/1024 << " MB" << std::endl;
        std::cout << "bytes per Element: " << perElement << std::endl;
        std::cout << "Estimated all size : " << (9*long(allSize) / 8 / 1024/1024) << " MB" << std::endl;

        size_t build = container().memUsage();
        std::cout << "BuilderIF size = " << build/1024/1024 << " MB \n";
        totalSize += build;
        std::cout << "Overall size = " << totalSize/1024/1024 << " MB \n";
        std::cout << "\n";
      }
    }
  }

} // namespace ALUGrid
