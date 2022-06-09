// (c) bernhard schupp 1997 - 1998
// modifications for dune interface
// (c) Robert Kloefkorn 2004 - 2005
#include <config.h>

#include <fstream>
#include <iterator>

#include "gitter_pll_sti.h"
#include "gitter_pll_mgb.h"
#include "../serial/gatherscatter.hh"
#include "../serial/walk.h"

namespace ALUGrid
{

  std::pair< IteratorSTI < GitterPll::vertex_STI > *, IteratorSTI < GitterPll::vertex_STI > *> GitterPll::
    iteratorTT (const GitterPll::vertex_STI *, int l) {

    std::vector< IteratorSTI < vertex_STI > * > _iterators_inner, _iterators_outer;

    _iterators_inner.push_back (new AccessIteratorTT < vertex_STI >::InnerHandle (containerPll (), l));
    _iterators_outer.push_back (new AccessIteratorTT < vertex_STI >::OuterHandle (containerPll (), l));
    {
      AccessIteratorTT < hedge_STI >::InnerHandle mie (containerPll (), l);
      AccessIteratorTT < hedge_STI >::OuterHandle moe (containerPll (), l);

      Insert < AccessIteratorTT < hedge_STI >::InnerHandle,
               TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > lie (mie);
      Insert < AccessIteratorTT < hedge_STI >::OuterHandle,
               TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > loe (moe);
      _iterators_inner.push_back (new Wrapper < Insert < AccessIteratorTT < hedge_STI >::InnerHandle,
                                                TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (lie));
      _iterators_outer.push_back (new Wrapper < Insert < AccessIteratorTT < hedge_STI >::OuterHandle,
                                                TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (loe));
    }
    {
      AccessIteratorTT < hface_STI >::InnerHandle mfi (containerPll (), l);
      AccessIteratorTT < hface_STI >::OuterHandle mfo (containerPll (), l);
      {
        Insert < AccessIteratorTT < hface_STI >::InnerHandle,
                 TreeIterator < hface_STI, has_int_vertex < hface_STI > > > lfi (mfi);
        Insert < AccessIteratorTT < hface_STI >::OuterHandle,
                 TreeIterator < hface_STI, has_int_vertex < hface_STI > > > lfo (mfo);

        _iterators_inner.push_back (new Wrapper < Insert < AccessIteratorTT < hface_STI >::InnerHandle,
                        TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (lfi));
        _iterators_outer.push_back (new Wrapper < Insert < AccessIteratorTT < hface_STI >::OuterHandle,
                        TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (lfo));
      }
      {
        Insert < AccessIteratorTT < hface_STI >::InnerHandle,
                 TreeIterator < hface_STI, has_int_edge < hface_STI > > > lfi (mfi);
        Insert < AccessIteratorTT < hface_STI >::OuterHandle,
                 TreeIterator < hface_STI, has_int_edge < hface_STI > > > lfo (mfo);
        Wrapper < Insert < AccessIteratorTT < hface_STI >::InnerHandle,
                  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > dlfi (lfi);
        Wrapper < Insert < AccessIteratorTT < hface_STI >::OuterHandle,
                  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > dlfo (lfo);
        Insert < Wrapper < Insert < AccessIteratorTT < hface_STI >::InnerHandle,
                 TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
        TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > vdlfi (dlfi);
        Insert < Wrapper < Insert < AccessIteratorTT < hface_STI >::OuterHandle,
                 TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
                 TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > vdlfo (dlfo);

        _iterators_inner.push_back (new Wrapper < Insert < Wrapper <
                  Insert < AccessIteratorTT < hface_STI >::InnerHandle,
                  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
                  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (vdlfi));

        _iterators_outer.push_back (new Wrapper <
          Insert < Wrapper < Insert < AccessIteratorTT < hface_STI >::OuterHandle,
          TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
          TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (vdlfo));
      }
    }
    return std::pair< IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > * >
    (new VectorAlign < vertex_STI > (_iterators_inner), new VectorAlign < vertex_STI > (_iterators_outer));
  }

  std::pair< IteratorSTI < GitterPll::hedge_STI > *, IteratorSTI < GitterPll::hedge_STI > * > GitterPll ::
    iteratorTT (const GitterPll::hedge_STI * fakep, int l)
  {
    // fakerule is only for type determination
    is_leaf < hedge_STI > * rule = 0;
    // see gitter_pll_sti.h
    return createEdgeIteratorTT(rule,l);
  }

  std::pair< IteratorSTI < GitterPll::hface_STI > *, IteratorSTI < GitterPll::hface_STI > *>
    GitterPll::iteratorTT (const GitterPll::hface_STI *, int l)
  {
    is_leaf< hface_STI > rule;
    return this->createFaceIteratorTT(rule, l);
  }

  void GitterPll::printSizeTT () {
    std::cout << std::endl << "GitterPll::printSizeTT ()" << std::endl << std::endl;
    mpAccess ().printLinkage( std::cout );
    std::cout << std::endl;
    for (int l = 0; l < mpAccess ().nlinks (); l ++ )
    {
      LeafIteratorTT < vertex_STI > w (*this, l);
      std::cout << "me: " << mpAccess ().myrank () << " link: " << l << " vertices: [inner|outer] " << w.inner ().size () << " " << w.outer ().size () << std::endl;
    }
    for (int l = 0; l < mpAccess ().nlinks (); l ++ )
    {
      LeafIteratorTT < hedge_STI > w (*this, l);
      std::cout << "me: " << mpAccess ().myrank () << " link: " << l << " edges:   [inner|outer] " << w.inner ().size () << " " << w.outer ().size () << std::endl;
    }
    for (int l = 0; l < mpAccess ().nlinks (); l ++ )
    {
      LeafIteratorTT < hface_STI > w (*this, l);
      std::cout << "me: " << mpAccess ().myrank () << " link: " << l << " faces: [inner|outer] " << w.inner ().size () << " " << w.outer ().size () << std::endl;
    }
  }

  void GitterPll::printsize ()
  {
    const int me = mpAccess ().myrank (), np = mpAccess ().psize (), nl = mpAccess ().nlinks ();

    if (debugOption (10)) Gitter::printsize ();
    std::vector< int > n;
    {
      int sum = 0;
      for (int i = 0; i < nl; ++i)
        sum += LeafIteratorTT < vertex_STI > (*this, i).outer ().size ();
      n.push_back (LeafIterator < vertex_STI > (*this)->size() - sum);
    }
    {
      int sum = 0;
      for (int i = 0; i < nl; ++i)
        sum += LeafIteratorTT < hedge_STI > (*this, i).outer ().size ();
      n.push_back (LeafIterator < hedge_STI > (*this)->size() - sum);
    }
    int sumCutFaces = 0;
    {
      int sum = 0;
      for (int i = 0; i < nl; ++i) {
        LeafIteratorTT < hface_STI > w (*this, i);
        sum += w.outer ().size ();
        sumCutFaces += w.outer ().size ();
        sumCutFaces += w.inner ().size ();
      }
      n.push_back (LeafIterator < hface_STI > (*this)->size() - sum);
    }
    n.push_back (LeafIterator < helement_STI > (*this)->size());
    n.push_back (LeafIterator < hbndseg_STI > (*this)->size() - sumCutFaces);

    {
      std::cout << "\nP[" << me << "] GitterPll::printSize () : \n\n";
      std::cout << " - Elements ......... "  << n[3] << "\n";
      std::cout << " - Boundaries ....... "  << n[4] << "\n";
      std::cout << " - Faces ............ "  << n[2] << "\n";
      std::cout << " - Edges ............ "  << n[1] << "\n";
      std::cout << " - Vertices ......... "  << n[0] << "\n";
      std::cout << std::endl;
    }

    // could better use MPI_gather here
    std::vector< std::vector< int > > in = mpAccess ().gcollect (n);
    alugrid_assert (static_cast<int> (in.size ()) == np);

    if (me == 0)
    {
      int nv = 0, nd = 0, nf = 0, ne = 0, nb = 0;
      for (int i = 0; i < np; ++i) {
        nv += (in [i])[0];
        nd += (in [i])[1];
        nf += (in [i])[2];
        ne += (in [i])[3];
        nb += (in [i])[4];
      }
      std::cout << "\nSummary -- GitterPll::printSize () : \n\n";
      std::cout << " - Elements ......... " << ne << "\n";
      std::cout << " - Boundaries ....... " << nb << "\n";
      std::cout << " - Faces ............ " << nf << "\n";
      std::cout << " - Edges ............ " << nd << "\n";
      std::cout << " - Vertices ......... " << nv << "\n";
      std::cout << std::endl;
    }
    return;
  }

  void GitterPll::fullIntegrityCheck () {
    int start = clock ();
    Gitter::fullIntegrityCheck ();
    containerPll().fullIntegrityCheck (mpAccess ());
    if (debugOption (0)) {
      std::cout << "**INFO GitterPll::fullIntegrityCheck () used: " << (float)((float)(clock() - start)/(float)(CLOCKS_PER_SEC)) << " sec." << std::endl;
    }
    return;
  }

  std::pair< IteratorSTI < Gitter::vertex_STI > *, IteratorSTI < Gitter::vertex_STI > * >
    GitterPll::MacroGitterPll::iteratorTT (const vertex_STI *, int i) {
    alugrid_assert (i < static_cast<int> (_vertexTT.size ()) );
    return std::pair< IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > * >
    (new listSmartpointer__to__iteratorSTI < vertex_STI > (_vertexTT [i].first),
           new listSmartpointer__to__iteratorSTI < vertex_STI > (_vertexTT [i].second));
  }

  std::pair< IteratorSTI < Gitter::vertex_STI > *, IteratorSTI < Gitter::vertex_STI > * >
    GitterPll::MacroGitterPll::iteratorTT (const std::pair< IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > * > & p, int) {
    return std::pair< IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > * >
    (new listSmartpointer__to__iteratorSTI < vertex_STI > (*(const listSmartpointer__to__iteratorSTI < vertex_STI > *)p.first),
           new listSmartpointer__to__iteratorSTI < vertex_STI > (*(const listSmartpointer__to__iteratorSTI < vertex_STI > *)p.second));
  }

  std::pair< IteratorSTI < Gitter::hedge_STI > *, IteratorSTI < Gitter::hedge_STI > * >
    GitterPll::MacroGitterPll::iteratorTT (const hedge_STI *, int i) {
    alugrid_assert (i < static_cast<int> (_hedgeTT.size ()));
    return std::pair< IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > * >
    (new listSmartpointer__to__iteratorSTI < hedge_STI > (_hedgeTT [i].first),
           new listSmartpointer__to__iteratorSTI < hedge_STI > (_hedgeTT [i].second));
  }

  std::pair< IteratorSTI < Gitter::hedge_STI > *, IteratorSTI < Gitter::hedge_STI > * >
    GitterPll::MacroGitterPll::iteratorTT (const std::pair< IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > * > & p, int) {
    return std::pair< IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > * >
    (new listSmartpointer__to__iteratorSTI < hedge_STI > (*(const listSmartpointer__to__iteratorSTI < hedge_STI > *)p.first),
           new listSmartpointer__to__iteratorSTI < hedge_STI > (*(const listSmartpointer__to__iteratorSTI < hedge_STI > *)p.second));
  }

  std::pair< IteratorSTI < Gitter::hface_STI > *, IteratorSTI < Gitter::hface_STI > * >
    GitterPll::MacroGitterPll::iteratorTT (const hface_STI *, int i) {
    alugrid_assert (i < static_cast<int> (_hfaceTT.size ()));
    return std::pair< IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > * >
    (new listSmartpointer__to__iteratorSTI < hface_STI > (_hfaceTT [i].first),
     new listSmartpointer__to__iteratorSTI < hface_STI > (_hfaceTT [i].second));
  }

  std::pair< IteratorSTI < Gitter::hface_STI > *, IteratorSTI < Gitter::hface_STI > * >
    GitterPll::MacroGitterPll::iteratorTT (const std::pair< IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > * > & p, int) {
    return std::pair< IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > * >
      (new listSmartpointer__to__iteratorSTI < hface_STI > (*(const listSmartpointer__to__iteratorSTI < hface_STI > *)p.first),
       new listSmartpointer__to__iteratorSTI < hface_STI > (*(const listSmartpointer__to__iteratorSTI < hface_STI > *)p.second));
  }

  class PackUnpackRefineLoop : public MpAccessLocal::NonBlockingExchange::DataHandleIF
  {
    typedef Gitter::hface_STI hface_STI ;
    typedef std::vector< hface_STI * > facevec_t ;
    std::vector< facevec_t >& _innerFaces ;
    std::vector< facevec_t >& _outerFaces ;

    typedef facevec_t::const_iterator hface_iterator;

    const size_t  _factor ;
    bool          _repeat ;
    const bool    _bisectionRefinement;

    PackUnpackRefineLoop( const PackUnpackRefineLoop& );
  public:
    PackUnpackRefineLoop( std::vector< facevec_t >& innerFaces,
                          std::vector< facevec_t >& outerFaces,
                          const bool bisectionRefinement )
      : _innerFaces( innerFaces ),
        _outerFaces( outerFaces ),
        _factor( bisectionRefinement ? 7 : 1 ),
        _repeat( false ),
        _bisectionRefinement( bisectionRefinement )
    {}

    bool repeat () const { return _repeat; }

    void pack( const int link, ObjectStream& os )
    {
      try
      {
        // clear stream
        os.clear();

        // reserve memory for object stream
        os.reserve( (_outerFaces[ link ].size() + _innerFaces[ link ].size() ) * _factor * sizeof(char) );
        {
          const hface_iterator iEnd = _outerFaces[ link ].end ();
          for (hface_iterator i = _outerFaces[ link ].begin (); i != iEnd; ++i )
          {
            packFace( (*i), os );
          }
        }
        {
          const hface_iterator iEnd = _innerFaces[ link ].end ();
          for (hface_iterator i = _innerFaces[ link ].begin (); i != iEnd; ++i )
          {
            packFace( (*i), os );
          }
        }
      }
      catch( Parallel::AccessPllException& )
      {
        std::cerr << "ERROR (fatal): AccessPllException caught." << std::endl;
        abort();
      }
    }

    void unpack( const int link, ObjectStream& os )
    {
      try
      {
#ifdef ALUGRIDDEBUG
        const size_t expecetedSize = (_innerFaces[ link ].size() + _outerFaces[ link ].size() ) * _factor * sizeof( char );
        alugrid_assert ( os.size() == (int)expecetedSize );
#endif
        {
          const hface_iterator iEnd = _innerFaces[ link ].end ();
          for (hface_iterator i = _innerFaces [ link ].begin (); i != iEnd; ++i )
          {
            unpackFace( (*i), os );
          }
        }
        {
          const hface_iterator iEnd = _outerFaces[ link ].end ();
          for (hface_iterator i = _outerFaces [ link ].begin (); i != iEnd; ++i )
          {
            unpackFace( (*i), os );
          }
        }
      }
      catch (Parallel::AccessPllException&)
      {
        std::cerr << "ERROR (fatal): AccessPllException caught." << std::endl;
        abort();
      }
    }
  protected:
    void packFace( hface_STI* face, ObjectStream& os ) const
    {
      face->accessOuterPllX ().first->getRefinementRequest ( os );
      if( _bisectionRefinement )
      {
        hface_STI* child = face->down();
        if( child )
        {
          for(; child; child = child->next() )
          {
            child->accessOuterPllX ().first->getRefinementRequest ( os );
            hface_STI* grandChild = child->down();
            if( grandChild )
            {
              for(; grandChild; grandChild = grandChild->next() )
              {
                grandChild->accessOuterPllX ().first->getRefinementRequest ( os );
                if(grandChild->down())
                  alugrid_assert(!grandChild->down()->down());
              }
            }
            else
            {
              typedef RefinementRules::Hface3Rule rule_t;
              os.put( char(rule_t::nosplit) );
              os.put( char(rule_t::nosplit) );
            }
          }
        }
        else
        {
          typedef RefinementRules::Hface3Rule rule_t;
          os.put( char(rule_t::nosplit) );
          os.put( char(rule_t::nosplit) );
          os.put( char(rule_t::nosplit) );
          os.put( char(rule_t::nosplit) );
          os.put( char(rule_t::nosplit) );
          os.put( char(rule_t::nosplit) );
        }
      }
    }

    void unpackFace( hface_STI* face, ObjectStream& os )
    {
      _repeat |= face->accessOuterPllX ().first->setRefinementRequest ( os );
      if( _bisectionRefinement )
      {
        hface_STI* child = face->down();
        if( child )
        {
          for( ; child ; child = child->next() )
          {
            _repeat |= child->accessOuterPllX ().first->setRefinementRequest ( os );
            hface_STI* grandChild = child->down();
            if( grandChild )
            {
              for( ; grandChild ; grandChild = grandChild->next() )
              {
                _repeat |= grandChild->accessOuterPllX ().first->setRefinementRequest ( os );
              }
            }
            else
            {
              // remove two chars from the stream
              os.get();
              os.get();
            }
          }
        }
        else
        {
          // remove two chars from the stream
          os.get();
          os.get();
          os.get();
          os.get();
          os.get();
          os.get();
        }
      }
    }

  };

  class PackUnpackEdgeCleanup : public MpAccessLocal::NonBlockingExchange::DataHandleIF
  {
    typedef Gitter::hedge_STI hedge_STI ;
    typedef std::vector< hedge_STI * > edgevec_t ;
    std::vector< edgevec_t >& _innerEdges ;
    std::vector< edgevec_t >& _outerEdges ;
    const bool _firstLoop ;

    typedef edgevec_t::const_iterator hedge_iterator;

    PackUnpackEdgeCleanup( const PackUnpackEdgeCleanup& );
  public:
    PackUnpackEdgeCleanup( std::vector< edgevec_t >& innerEdges,
                           std::vector< edgevec_t >& outerEdges,
                           const bool firstLoop )
      : _innerEdges( innerEdges ),
        _outerEdges( outerEdges ),
        _firstLoop( firstLoop )
    {}

    void pack( const int link, ObjectStream& os )
    {
      // the first loop needs outerEdges the second loop inner
      edgevec_t& edges = ( _firstLoop ) ? _outerEdges[ link ] : _innerEdges[ link ];

      os.clear();
      // reserve memory
      os.reserve( edges.size() * sizeof(char) );

      // write refinement request
      const hedge_iterator iEnd = edges.end ();
      for (hedge_iterator i = edges.begin (); i != iEnd; ++i )
      {
        (*i)->getRefinementRequest ( os );
      }
    }

    void unpack( const int link, ObjectStream& os )
    {
      // the first loop needs innerEdges the second loop outer
      edgevec_t& edges = ( _firstLoop ) ? _innerEdges[ link ] : _outerEdges[ link ];

      // the edge sizes should match on both sides
      alugrid_assert ( os.size() == int( edges.size() * sizeof(char)) );

      const hedge_iterator iEnd = edges.end ();
      for (hedge_iterator i = edges.begin (); i != iEnd; ++i )
      {
        (*i)->setRefinementRequest ( os );
      }
    }
  };

  bool GitterPll::checkRepeat( const bool repeat, const unsigned int refineLoops ) const
  {
    // perform a minimus of refine loops before doing an all_reduce
    if( refineLoops < _minRefineLoops ) return true ;

    // all_reduce to check if every process is finished
    return mpAccess().gmax( repeat );
  }

  bool GitterPll::refine ()
  {
    alugrid_assert (debugOption (5) ? (std::cout << "**INFO GitterPll::refine () " << std::endl, 1) : 1);
    const int nl = mpAccess ().nlinks ();
    bool state = false;

    typedef std::vector< hedge_STI * > edgevec_t ;
    typedef std::vector< hface_STI * > facevec_t ;
    std::vector< edgevec_t > innerEdges (nl), outerEdges (nl);
    std::vector< facevec_t > innerFaces (nl), outerFaces (nl);

    {
      // Erst die Zeiger auf alle Fl"achen und Kanten mit paralleler
      // Mehrdeutigkeit sichern, da die LeafIteratorTT < . > nach dem
      // Verfeinern auf gitter nicht mehr stimmen werden. Die Technik
      // ist zul"assig, da keine mehrfache Verfeinerung entstehen kann.

      {
        for (int l = 0; l < nl; ++l)
        {
          LeafIteratorTT < hface_STI > fw (*this,l);
          LeafIteratorTT < hedge_STI > dw (*this,l);

          facevec_t& outerFace = outerFaces[ l ];
          facevec_t& innerFace = innerFaces[ l ];

          // reserve memory first
          outerFace.reserve( fw.outer().size() );
          innerFace.reserve( fw.inner().size() );

          for (fw.outer ().first (); ! fw.outer().done (); fw.outer ().next ())
            outerFace.push_back (& fw.outer ().item ());
          for (fw.inner ().first (); ! fw.inner ().done (); fw.inner ().next ())
            innerFace.push_back (& fw.inner ().item ());

          edgevec_t& outerEdge = outerEdges[ l ];
          edgevec_t& innerEdge = innerEdges[ l ];

          // reserve memory first
          outerEdge.reserve( dw.outer().size() );
          innerEdge.reserve( dw.inner().size() );

          for (dw.outer ().first (); ! dw.outer().done (); dw.outer ().next ())
            outerEdge.push_back (& dw.outer ().item ());
          for (dw.inner ().first (); ! dw.inner ().done (); dw.inner ().next ())
            innerEdge.push_back (& dw.inner ().item ());
        }
      }
      // jetzt normal verfeinern und den Status der Verfeinerung
      // [unvollst"andige / vollst"andige Verfeinerung] sichern.

      state = Gitter::refine ();

      // Phase des Fl"achenausgleichs an den Schnittfl"achen des
      // verteilten Gitters. Weil dort im sequentiellen Fall pseudorekursive
      // Methodenaufrufe vorliegen k"onnen, muss solange iteriert werden,
      // bis die Situation global station"ar ist.

      bool repeat (false);
      int refineLoops = 0;
      do
      {
        // unpack handle to unpack the data once their received
        PackUnpackRefineLoop dataHandle ( innerFaces, outerFaces, conformingClosureNeeded() );

        // exchange data and unpack when received
        mpAccess ().exchange ( dataHandle );

        // get repeat flag
        repeat = dataHandle.repeat();

        // count loops
        ++refineLoops;
      }
      while ( checkRepeat( repeat, refineLoops ) );

      _averageRefineLoops += refineLoops;

      // increase refines counter
      ++_refinesDone;
      if( _refinesDone == 50 )
      {
        const unsigned int newAverage = _averageRefineLoops / _refinesDone ;
        _minRefineLoops = std::min( (unsigned int )5, newAverage );
        // reset values
        _refinesDone = 0;
        _averageRefineLoops = 0;
        // make sure every process has the same value
        alugrid_assert( static_cast<int>( _minRefineLoops ) == mpAccess().gmax( static_cast<int>( _minRefineLoops ) ) );
      }

      //if( mpAccess().myrank() == 0)
      //  std::cout << refineLoops << " refLoops | minRefLoops " << _minRefineLoops << std::endl;

      // Jetzt noch die Kantensituation richtigstellen, es gen"ugt ein Durchlauf,
      // weil die Verfeinerung einer Kante keine Fernwirkungen hat. Vorsicht: Die
      // Kanten sind bez"uglich ihrer Identifikation sternf"ormig organisiert, d.h.
      // es muss die Verfeinerungsinformation einmal am Eigent"umer gesammelt und
      // dann wieder zur"ucktransportiert werden, eine einfache L"osung, wie bei
      // den Fl"achen (1/1 Beziehung) scheidet aus.

      {
        PackUnpackEdgeCleanup edgeData( innerEdges, outerEdges, true );
        mpAccess().exchange( edgeData );
      }

      {
        PackUnpackEdgeCleanup edgeData( innerEdges, outerEdges, false );
        mpAccess().exchange( edgeData );
      }
    }

    return state;
  }

  class EdgeFlagExchange
  : public GatherScatterType
  {
  public:
    EdgeFlagExchange () {}
    virtual ~EdgeFlagExchange () {}

    // type of used object stream
    typedef GatherScatterType::ObjectStreamType   ObjectStreamType;

    using GatherScatterType::containsItem;
    using GatherScatterType::sendData;
    using GatherScatterType::recvData;
    using GatherScatterType::setData;

    // only contains edge information
    virtual bool contains ( int dim, int codim ) const { return codim == 2; }
    // every element is contained
    virtual bool containsItem ( const Gitter::hedge_STI &elem ) const { return true; }
    // send does pack the no edge coarsening flag
    virtual void sendData ( ObjectStreamType & str , Gitter::hedge_STI & edge )
    {
      str.put( char(edge.noCoarsen()) );
    }

    // receive gets flag and disabled coarsen if flag is set
    virtual void recvData ( ObjectStreamType & str , Gitter::hedge_STI & edge )
    {
      const bool noCoarsen = bool( str.get() );
      if( noCoarsen )
        edge.disableEdgeCoarsen();
    }

    // this method is only needed for ghost cells
    virtual void setData  ( ObjectStreamType & str , Gitter::hedge_STI & elem )
    {
      std::cout << "ERROR: EdgeFlagExchange::setData was called in " << __FILE__ << " " << __LINE__ << std::endl;
      abort();
    }
  };

  class PackUnpackCoarsenLoop : public MpAccessLocal::NonBlockingExchange::DataHandleIF
  {
    typedef Gitter::hface_STI hface_STI ;
    typedef std::vector< hface_STI * > facevec_t ;

    typedef std::vector< int > cleanvector_t;
    std::vector< cleanvector_t >& _clean ;

    std::vector< facevec_t >& _innerFaces ;
    std::vector< facevec_t >& _outerFaces ;

    const bool _firstLoop ;

    typedef facevec_t::const_iterator hface_iterator;

    PackUnpackCoarsenLoop( const PackUnpackCoarsenLoop& );
  public:
    PackUnpackCoarsenLoop( std::vector< cleanvector_t >& clean,
                           std::vector< facevec_t >& innerFaces,
                           std::vector< facevec_t >& outerFaces,
                           const bool firstLoop )
      : _clean( clean ),
        _innerFaces( innerFaces ),
        _outerFaces( outerFaces ),
        _firstLoop( firstLoop )
    {}

    void pack( const int link, ObjectStream& os )
    {
      // clear stream
      os.clear();

      if( _firstLoop )
      {
        // reserve memory
        os.reserve( _outerFaces[ link ].size() * sizeof(char) );

        // get end iterator
        const hface_iterator iEnd = _outerFaces[ link ].end ();
        for (hface_iterator i = _outerFaces[ link ].begin (); i != iEnd; ++i)
        {
          char lockAndTry = (*i)->accessOuterPllX ().first->lockAndTry ();
          os.putNoChk( lockAndTry );
        }
      }
      else
      {
        // reserve memory
        os.reserve( _innerFaces[ link ].size() * sizeof(char) );

        cleanvector_t::iterator j = _clean[ link ].begin ();
        const hface_iterator iEnd = _innerFaces[ link ].end ();
        for (hface_iterator i = _innerFaces[ link ].begin (); i != iEnd; ++i, ++j)
        {
          const bool unlock = *j;
          os.putNoChk( char(unlock) );
          (*i)->accessOuterPllX ().first->unlockAndResume ( unlock );
        }
      }
    }

    void unpack( const int link, ObjectStream& os )
    {
      if( _firstLoop )
      {
#ifdef ALUGRIDDEBUG
        const size_t expecetedSize = (_innerFaces[ link ].size() ) * sizeof( char );
        // the size of the received ObjectStream should be the faces
        alugrid_assert ( os.size() == (int)expecetedSize );
#endif
        cleanvector_t& cl = _clean[ link ];

        // reset clean vector
        cl = cleanvector_t( _innerFaces[ link ].size (), int(true) );

        cleanvector_t::iterator j = cl.begin ();

        const hface_iterator iEnd = _innerFaces[ link ].end ();
        for (hface_iterator i = _innerFaces[ link ].begin (); i != iEnd; ++i, ++j )
        {
          // get lockAndTry info
          const bool locked = bool( os.get() );

          alugrid_assert (j != cl.end ());
          (*j) &= locked && (*i)->accessOuterPllX ().first->lockAndTry ();
        }
      }
      else
      {
#ifdef ALUGRIDDEBUG
        const size_t expecetedSize = (_outerFaces[ link ].size() ) * sizeof( char );
        // the size of the received ObjectStream should be the faces
        alugrid_assert ( os.size() == (int)expecetedSize );
#endif
        const hface_iterator iEnd = _outerFaces[ link ].end ();
        for (hface_iterator i = _outerFaces[ link ].begin (); i != iEnd; ++i )
        {
          const bool unlock = bool( os.get() );
          (*i)->accessOuterPllX ().first->unlockAndResume ( unlock );
        }
      }
    }
  };

  class PackUnpackDynamicState : public MpAccessLocal::NonBlockingExchange::DataHandleIF
  {

    typedef Gitter::hface_STI hface_STI ;
    typedef Insert < AccessIteratorTT < hface_STI >::InnerHandle,
       TreeIterator < hface_STI, is_def_true < hface_STI > > > InnerIteratorType;
    typedef Insert < AccessIteratorTT < hface_STI >::OuterHandle,
      TreeIterator < hface_STI, is_def_true < hface_STI > > > OuterIteratorType;

    GitterPll::MacroGitterPll& _containerPll;

    PackUnpackDynamicState( const PackUnpackDynamicState& );
  public:
    PackUnpackDynamicState( GitterPll::MacroGitterPll& containerPll )
      : _containerPll( containerPll )
    {}

    void pack( const int link, ObjectStream& os )
    {
      // clear stream
      os.clear();

      // pack data
      packNoClear( link, os );
    }

    // pack version without clearing ObjectStream
    void packNoClear( const int link, ObjectStream& os )
    {
      AccessIteratorTT < hface_STI >::InnerHandle mif ( _containerPll, link );
      AccessIteratorTT < hface_STI >::OuterHandle mof ( _containerPll, link );

      InnerIteratorType wi (mif);
      for (wi.first (); ! wi.done (); wi.next ())
      {
        std::pair< ElementPllXIF_t *, int > p = wi.item ().accessInnerPllX ();
        p.first->writeDynamicState (os, p.second);
      }

      OuterIteratorType wo (mof);
      for (wo.first (); ! wo.done (); wo.next ())
      {
        std::pair< ElementPllXIF_t *, int > p = wo.item ().accessInnerPllX ();
        p.first->writeDynamicState (os, p.second);
      }
    }

    void unpack( const int link, ObjectStream& os )
    {
      AccessIteratorTT < hface_STI >::OuterHandle mof ( _containerPll, link );
      AccessIteratorTT < hface_STI >::InnerHandle mif ( _containerPll, link );

      OuterIteratorType wo (mof);
      for (wo.first (); ! wo.done (); wo.next ())
      {
        std::pair< ElementPllXIF_t *, int > p = wo.item ().accessOuterPllX ();
        p.first->readDynamicState (os, p.second);
      }

      InnerIteratorType wi (mif);
      for (wi.first (); ! wi.done (); wi.next ())
      {
        std::pair< ElementPllXIF_t *, int > p = wi.item ().accessOuterPllX ();
        p.first->readDynamicState (os, p.second);
      }
    }
  };


  class PackUnpackEdgeCoarsen : public MpAccessLocal::NonBlockingExchange::DataHandleIF
  {
    typedef Gitter::hedge_STI hedge_STI ;
    typedef std::vector< hedge_STI * > edgevec_t ;

    typedef std::pair< bool, bool >  clean_t;
    typedef std::map< hedge_STI *, clean_t > cleanmap_t;
    typedef cleanmap_t::iterator cleanmapiterator_t;

    PackUnpackDynamicState _dynamicState ;

    cleanmap_t& _clean;
    std::vector< edgevec_t >& _innerEdges ;
    std::vector< edgevec_t >& _outerEdges ;
    const bool _firstLoop ;

    typedef edgevec_t::const_iterator hedge_iterator;

    PackUnpackEdgeCoarsen( const PackUnpackEdgeCoarsen& );
  public:
    PackUnpackEdgeCoarsen( GitterPll::MacroGitterPll& containerPll,
                           cleanmap_t& clean,
                           std::vector< edgevec_t >& innerEdges,
                           std::vector< edgevec_t >& outerEdges,
                           const int nLinks,
                           const bool firstLoop )
      : _dynamicState( containerPll ),
        _clean( clean ),
        _innerEdges( innerEdges ),
        _outerEdges( outerEdges ),
        _firstLoop( firstLoop )
    {}

    void pack( const int link, ObjectStream& os )
    {
      // clear stream
      os.clear();

      if( _firstLoop )
      {
        // reserve memory first
        os.reserve( _outerEdges[ link ].size() * sizeof(char) );

        // get end iterator
        const hedge_iterator iEnd = _outerEdges[ link ].end ();
        for (hedge_iterator i = _outerEdges[ link ].begin (); i != iEnd; ++i)
        {
          char lockAndTry = (*i)->lockAndTry ();
          os.putNoChk( lockAndTry );
        }
      }
      else
      {
        // reserve memory first
        os.reserve( _innerEdges[ link ].size() * sizeof(char) );

        // get end iterator
        const hedge_iterator iEnd = _innerEdges[ link ].end ();
        for (hedge_iterator i = _innerEdges[ link ].begin (); i != iEnd; ++i)
        {
          hedge_STI* edge = (*i);
          alugrid_assert ( _clean.find ( edge ) != _clean.end ());

          clean_t& a = _clean[ edge ];
          os.putNoChk( char( a.first) );

          if (a.second)
          {
            // Wenn wir hier sind, kann die Kante tats"achlich vergr"obert werden, genauer gesagt,
            // sie wird es auch und der R"uckgabewert testet den Vollzug der Aktion. Weil aber nur
            // einmal vergr"obert werden kann, und die Iteratoren 'innerEdges [l]' aber eventuell
            // mehrfach "uber eine Kante hinweglaufen, muss diese Vergr"oberung im map 'clean'
            // vermerkt werden. Dann wird kein zweiter Versuch unternommen.

            a.second = false;
#ifdef ALUGRIDDEBUG
            bool b =
#endif
              edge->unlockAndResume (a.first);
            alugrid_assert (b == a.first);
          }
        }

        // pack dynamic state, don't clear object stream
        _dynamicState.packNoClear( link, os );
      }
    }

    void unpack( const int link, ObjectStream& os )
    {
      if( _firstLoop )
      {
        // get end iterators
        const cleanmapiterator_t cleanEnd = _clean.end();
        const hedge_iterator iEnd = _innerEdges[ link ].end ();
        // fill cleanmap first
        for (hedge_iterator i = _innerEdges[ link ].begin (); i != iEnd; ++i)
        {
          hedge_STI* edge = (*i);
          cleanmapiterator_t cit = _clean.find ( edge );
          if (cit == cleanEnd )
          {
            clean_t& cp = _clean[ edge ];
            cp.first  = edge->lockAndTry ();
            cp.second = true;
          }
        }

        for (hedge_iterator i = _innerEdges[ link ].begin (); i != iEnd; ++i)
        {
          const bool locked = bool( os.get() );
          if( locked == false )
          {
            alugrid_assert ( _clean.find (*i) != cleanEnd );
            _clean[ *i ].first = false;
          }
        }
      }
      else
      {
        // get end iterator
        const hedge_iterator iEnd = _outerEdges[ link ].end ();
        for (hedge_iterator i = _outerEdges[ link ].begin (); i != iEnd; ++i )
        {
          // Selbe Situation wie oben, aber der Eigent"umer der Kante hat mitgeteilt, dass sie
          // vergr"obert werden darf und auch wird auf allen Teilgebieten also auch hier. Der
          // Vollzug der Vergr"oberung wird durch den R"uckgabewert getestet.

          const bool unlock = bool( os.get() );
#ifdef ALUGRIDDEBUG
          bool b =
#endif
            (*i)->unlockAndResume ( unlock );
          alugrid_assert (b == unlock);
        }

        // unpack dynamic state
        _dynamicState.unpack( link, os );
      }
    }
  };

  void GitterPll::coarse ()
  {
    alugrid_assert (debugOption (20) ? (std::cout << "**INFO GitterDunePll::coarse () " << std::endl, 1) : 1);
    const int nl = mpAccess ().nlinks ();

    typedef std::vector< hedge_STI * >::iterator hedge_iterator;
    typedef std::vector< hface_STI * >::iterator hface_iterator;

    {
      std::vector< std::vector< hedge_STI * > > innerEdges (nl), outerEdges (nl);
      std::vector< std::vector< hface_STI * > > innerFaces (nl), outerFaces (nl);

      for (int l = 0; l < nl; ++l)
      {

        // Zun"achst werden f"ur alle Links die Zeiger auf Gitterojekte mit
        // Mehrdeutigkeit gesichert, die an der Wurzel einer potentiellen
        // Vergr"oberungsoperation sitzen -> es sind die Knoten in der Hierarchie,
        // deren Kinder alle Bl"atter sind. Genau diese Knoten sollen gegen"uber
        // der Vergr"oberung blockiert werden und dann die Vergr"oberung falls
        // sie zul"assig ist, sp"ater durchgef"uhrt werden (pending);

        AccessIteratorTT < hface_STI >::InnerHandle mfwi (containerPll (),l);
        AccessIteratorTT < hface_STI >::OuterHandle mfwo (containerPll (),l);
        AccessIteratorTT < hedge_STI >::InnerHandle mdwi (containerPll (),l);
        AccessIteratorTT < hedge_STI >::OuterHandle mdwo (containerPll (),l);

        // Die inneren und a"usseren Iteratoren der potentiell vergr"oberungsf"ahigen
        // Fl"achen "uber den Grobgitterfl"achen. In den Elementen passiert erstmal
        // nichts, solange nicht mit mehrfachen Grobgitterelementen gearbeitet wird.

        Insert < AccessIteratorTT < hface_STI >::InnerHandle,
          TreeIterator < hface_STI, childs_are_leafs < hface_STI > > > fwi (mfwi);
        Insert < AccessIteratorTT < hface_STI >::OuterHandle,
          TreeIterator < hface_STI, childs_are_leafs < hface_STI > > > fwo (mfwo);

        // Die inneren und a"usseren Iteratoren der potentiell vergr"oberungsf"ahigen
        // Kanten "uber den Grobgitterkanten.

        Insert < AccessIteratorTT < hedge_STI >::InnerHandle,
          TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dwi (mdwi);
        Insert < AccessIteratorTT < hedge_STI >::OuterHandle,
          TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dwo (mdwo);

        // Die inneren und a"usseren Iteratoren der potentiell vergr"oberungsf"ahigen
        // Kanten "uber den Grobgitterfl"achen. Diese Konstruktion wird beim Tetraeder-
        // gitter notwendig, weil dort keine Aussage der Form:
        //

        Insert < AccessIteratorTT < hface_STI >::InnerHandle,
          TreeIterator < hface_STI, has_int_edge < hface_STI > > > efi (mfwi);
        Insert < AccessIteratorTT < hface_STI >::OuterHandle,
          TreeIterator < hface_STI, has_int_edge < hface_STI > > > efo (mfwo);
        Wrapper < Insert < AccessIteratorTT < hface_STI >::InnerHandle,
          TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > eifi (efi);
        Wrapper < Insert < AccessIteratorTT < hface_STI >::OuterHandle,
          TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > eifo (efo);
        Insert < Wrapper < Insert < AccessIteratorTT < hface_STI >::InnerHandle,
          TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
        TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dfi (eifi);
          Insert < Wrapper < Insert < AccessIteratorTT < hface_STI >::OuterHandle,
            TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
        TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dfo (eifo);

        // Die 'item ()' Resultatwerte (Zeiger) werden in Vektoren gesichert, weil die
        // Kriterien die zur Erzeugung der Iteratoren angewendet wurden (Filter) nach
        // einer teilweisen Vergr"oberung nicht mehr g"ultig sein werden, d.h. die
        // Iterationsobjekte "andern w"ahrend der Vergr"oberung ihre Eigenschaften.
        // Deshalb werden sie auch am Ende des Blocks aufgegeben. Der Vektor 'cache'
        // ist zul"assig, weil kein Objekt auf das eine Referenz im 'cache' vorliegt
        // beseitigt werden kann. Sie sind alle ein Niveau darunter.

        std::vector< hedge_STI * >& innerEdgesLink = innerEdges[ l ];
        std::vector< hedge_STI * >& outerEdgesLink = outerEdges[ l ];

        std::vector< hface_STI * >& innerFacesLink = innerFaces[ l ];
        std::vector< hface_STI * >& outerFacesLink = outerFaces[ l ];

        // reserve memory first
        innerFacesLink.reserve( fwi.size() );
        outerFacesLink.reserve( fwo.size() );

        for (fwi.first (); ! fwi.done (); fwi.next ()) innerFacesLink.push_back (& fwi.item ());
        for (fwo.first (); ! fwo.done (); fwo.next ()) outerFacesLink.push_back (& fwo.item ());

        // reserve memory first
        innerEdgesLink.reserve( dwi.size() + dfi.size() );
        outerEdgesLink.reserve( dwo.size() + dfo.size() );

        for (dwo.first (); ! dwo.done (); dwo.next ()) outerEdgesLink.push_back (& dwo.item ());
        for (dfo.first (); ! dfo.done (); dfo.next ()) outerEdgesLink.push_back (& dfo.item ());
        for (dwi.first (); ! dwi.done (); dwi.next ()) innerEdgesLink.push_back (& dwi.item ());
        for (dfi.first (); ! dfi.done (); dfi.next ()) innerEdgesLink.push_back (& dfi.item ());
      }

      // first check edges that cannot be coarsened
      // due to bisection rule (only enabled for bisection)
      // communicate edge flags if bisection is enabled
      if( Gitter::markEdgeCoarsening() )
      {
        // see class implementation in this file above
        EdgeFlagExchange dataHandle;
        // this communicates the edge no coarsen flags
        borderBorderCommunication( dataHandle, dataHandle, dataHandle, dataHandle );
      }

      try
      {
        // Erstmal alles was mehrdeutig ist, gegen die drohende Vergr"oberung sichern.
        // Danach werden sukzessive die Fl"achenlocks aufgehoben, getestet und
        // eventuell vergr"obert, dann das gleiche Spiel mit den Kanten.

        for (int l = 0; l < nl; ++l)
        {
          {
            const hedge_iterator iEnd = outerEdges [l].end ();
            for (hedge_iterator i = outerEdges [l].begin (); i != iEnd; ++i )
            {
              (*i)->lockAndTry ();
            }
          }
          {
            const hedge_iterator iEnd = innerEdges [l].end ();
            for (hedge_iterator i = innerEdges [l].begin (); i != iEnd; ++i )
            {
              (*i)->lockAndTry ();
            }
          }
          {
            const hface_iterator iEnd = outerFaces [l].end ();
            for (hface_iterator i = outerFaces [l].begin (); i != iEnd; ++i )
              (*i)->accessOuterPllX ().first->lockAndTry ();
          }
          {
            const hface_iterator iEnd = innerFaces [l].end ();
            for (hface_iterator i = innerFaces [l].begin (); i != iEnd; ++i )
              (*i)->accessOuterPllX ().first->lockAndTry ();
          }
        }

        // Gitter::coarse () ist elementorientiert, d.h. die Vergr"oberung auf Fl"achen und
        // Kanten wird nur durch Vermittlung eines sich vergr"obernden Knotens in der Element-
        // hierarchie angestossen. In allen gegen Vergr"oberung 'gelockten' Fl"achen und Kanten
        // wird die angeforderte Operation zur"uckgewiesen, um erst sp"ater von aussen nochmals
        // angestossen zu werden.

        // do real coarsening of elements
        Gitter::doCoarse ();

        // reset edge coarsen flags to avoid problems with coarsening
        // Gitter::resetEdgeCoarsenFlags ();
      }
      catch( Parallel::AccessPllException& )
      {
        std::cerr << "ERROR( fatal ): AccessPllException caught during coarsening of element hierarchy." << std::endl;
        abort ();
      }

      try {

        // Phase des Fl"achenausgleichs des verteilten Vergr"oberungsalgorithmus
        // alle Schnittfl"achenpaare werden daraufhin untersucht, ob eine
        // Vergr"oberung in beiden Teilgittern durchgef"uhrt werden darf,
        // wenn ja, wird in beiden Teilgittern vergr"obert und der Vollzug
        // getestet.

        typedef std::vector< int > cleanvector_t;
        std::vector< cleanvector_t > clean (nl);

        {
          // face data, first loop
          PackUnpackCoarsenLoop faceData( clean, innerFaces, outerFaces, true );
          mpAccess().exchange( faceData );
        }

        {
          // face data, second loop
          PackUnpackCoarsenLoop faceData( clean, innerFaces, outerFaces, false );
          mpAccess().exchange( faceData );
        }

      }
      catch( Parallel::AccessPllException& )
      {
        std::cerr << "ERROR (fatal): AccessPllException caught when coarsening area trees." << std::endl;
        abort();
      }

      try
      {
        // Phase des Kantenausgleichs im parallelen Vergr"oberungsalgorithmus:

        // Weil hier jede Kante nur eindeutig auftreten darf, muss sie in einem
        // map als Adresse hinterlegt werden, dann k"onnen die verschiedenen
        // Refcounts aus den verschiedenen Links tats"achlich global miteinander
        // abgemischt werden. Dazu werden zun"achst alle eigenen Kanten auf ihre
        // Vergr"oberbarkeit hin untersucht und dieser Zustand (true = vergr"oberbar
        // false = darf nicht vergr"obert werden) im map 'clean' hinterlegt. Dazu
        // kommt noch ein zweiter 'bool' Wert, der anzeigt ob die Kante schon ab-
        // schliessend vergr"obert wurde oder nicht.

        typedef std::pair< bool, bool >  clean_t;
        typedef std::map< hedge_STI *, clean_t > cleanmap_t;
        cleanmap_t clean;

        {
          // edge data, first loop
          PackUnpackEdgeCoarsen edgeData( this->containerPll(),
                                          clean, innerEdges, outerEdges, nl, true );
          mpAccess().exchange( edgeData );
        }

        {
          // edge data, second loop, also exchanges dynamic state
          PackUnpackEdgeCoarsen edgeData( this->containerPll(),
                                          clean, innerEdges, outerEdges, nl, false );
          mpAccess().exchange( edgeData );
        }
      }
      catch( Parallel::AccessPllException& )
      {
        std::cerr << "ERROR (fatal): AccessPllException caught when coarsening edge trees." << std::endl;
        abort();
      }
    }

  }

#ifdef ENABLE_ALUGRID_VTK_OUTPUT
  extern int adaptstep;
  extern int stepnumber;
#endif

  bool GitterPll::adapt ()
  {
#ifdef ENABLE_ALUGRID_VTK_OUTPUT
    stepnumber = 0;
#endif

    // refine the grid, should also lead to conforming grid
    // when bisection refinement is used
    const bool refined = refine();

#ifdef ALUGRIDDEBUG
    bool needConformingClosure = false;
    const bool bisectionEnabled = conformingClosureNeeded();
    alugrid_assert ( bisectionEnabled == mpAccess().gmax( bisectionEnabled ) );
#endif

    /*
    bool refined = false;

    // loop until refinement leads to a conforming situation (conforming refinement only)
    do
    {
      alugrid_assert (debugOption (20) ? (std::cout << "**INFO GitterPll::adapt ()" << std::endl, 1) : 1);
      alugrid_assert (! iterators_attached ());

      // call refine
      refined |= refine ();

      // for bisection refinement repeat loop if non-confoming edges are still present
      // markForConformingClosure returns true if all elements have conforming closure
      needConformingClosure =
        bisectionEnabled ? mpAccess().gmax( markForConformingClosure() ) : false;

    }
    while ( needConformingClosure );
    */

    // now do one coarsening step
    coarse ();

#ifdef ALUGRIDDEBUG
    needConformingClosure =
      bisectionEnabled ? mpAccess().gmax( markForConformingClosure() ) : false;
    alugrid_assert ( ! needConformingClosure );
#endif

#ifdef ENABLE_ALUGRID_VTK_OUTPUT
    ++adaptstep;
#endif

    return refined;
  }

  void GitterPll::MacroGitterPll::fullIntegrityCheck (MpAccessLocal & mpa)
  {
    std::cerr << "ERROR: GitterPll::MacroGitterPll::fullIntegrityCheck needs a reimplementation to avoid the old exchange methods" << std::endl;
    abort();
    /*
    const int nl = mpa.nlinks (), me = mpa.myrank ();

    try {
      std::vector< std::vector< int > > inout (nl);

      {for (int l = 0; l < nl; l ++) {
        AccessIteratorTT < hface_STI >::InnerHandle w (*this,l);
        for ( w.first (); ! w.done (); w.next ()) {
          std::vector< int > i = w.item ().checkParallelConnectivity ();
          copy (i.begin (), i.end (), back_inserter (inout [l]));
        }
      }}
      inout = mpa.exchange (inout);
      {for (int l = 0; l < nl; l ++) {
        std::vector< int >::const_iterator pos = inout [l].begin ();
        AccessIteratorTT < hface_STI >::OuterHandle w (*this,l);
        for (w.first (); ! w.done (); w.next ()) {
          std::vector< int > t1 = w.item ().checkParallelConnectivity ();
          std::vector< int > t2 (t1.size (), 0);
          copy (pos, pos + t1.size (), t2.begin ());
          pos += t1.size ();
          if (t1 != t2)
          {
            std::cerr << "fehler an gebiet " << me << " : ";
            std::copy( t1.begin(), t1.end(), std::ostream_iterator< int >( std::cerr, "-" ) );
            std::cerr << "\t";
            std::copy( t2.begin(), t2.end(), std::ostream_iterator< int >( std::cerr, "-" ) );
            std::cerr << std::endl;
          }
        }
      }}
    }
    catch( Parallel::AccessPllException& )
    {
      std::cerr << "ERROR (fatal): Parallel::AccessPllException caught." << std::endl;
      abort();
    }
    */
  }

  class PackUnpackStaticState : public MpAccessLocal::NonBlockingExchange::DataHandleIF
  {
    typedef Gitter::hface_STI hface_STI ;
    typedef Insert < AccessIteratorTT < hface_STI >::InnerHandle,
       TreeIterator < hface_STI, is_def_true < hface_STI > > > InnerIteratorType;
    typedef Insert < AccessIteratorTT < hface_STI >::OuterHandle,
      TreeIterator < hface_STI, is_def_true < hface_STI > > > OuterIteratorType;

    GitterPll::MacroGitterPll& _containerPll;

    PackUnpackStaticState( const PackUnpackDynamicState& );
  public:
    PackUnpackStaticState( GitterPll::MacroGitterPll& containerPll )
      : _containerPll( containerPll )
    {}

    // pack version without clearing ObjectStream
    void pack( const int link, ObjectStream& os )
    {
      // clear stream
      os.clear();

      // pack data
      AccessIteratorTT < hface_STI > :: InnerHandle wi ( _containerPll, link ) ;
      AccessIteratorTT < hface_STI > :: OuterHandle wo ( _containerPll, link ) ;
      for (wi.first () ; ! wi.done () ; wi.next ())
      {
        std::pair < ElementPllXIF_t *, int > p = wi.item ().accessInnerPllX () ;
        p.first->writeStaticState (os, p.second) ;
      }
      for (wo.first () ; ! wo.done () ; wo.next ())
      {
        std::pair < ElementPllXIF_t *, int > p = wo.item ().accessInnerPllX () ;
        p.first->writeStaticState (os, p.second) ;
      }

      // mark end of stream
      os.writeObject( MacroGridMoverIF :: ENDSTREAM );
    }

    void unpack( const int link, ObjectStream& os )
    {
      AccessIteratorTT < hface_STI > :: InnerHandle wi ( _containerPll, link ) ;
      AccessIteratorTT < hface_STI > :: OuterHandle wo ( _containerPll, link ) ;
      for (wo.first () ; ! wo.done () ; wo.next ())
      {
        std::pair < ElementPllXIF_t *, int > p = wo.item ().accessOuterPllX () ;
        p.first->readStaticState (os, p.second) ;
      }
      for (wi.first () ; ! wi.done () ; wi.next ())
      {
        std::pair < ElementPllXIF_t *, int > p = wi.item ().accessOuterPllX () ;
        p.first->readStaticState (os, p.second) ;
      }

      // check consistency of stream
      int endStream ;
      os.readObject( endStream );
      if( endStream != MacroGridMoverIF :: ENDSTREAM )
      {
        os.readObject( endStream );
        if( endStream != MacroGridMoverIF :: ENDSTREAM )
        {
          std::cerr << "**ERROR: writeStaticState: inconsistent stream, got " << endStream << std::endl;
          alugrid_assert ( false );
          abort();
        }
      }
    }
  };


  void GitterPll :: exchangeStaticState ()
  {
    // Die Methode wird jedesmal aufgerufen, wenn sich der statische
    // Zustand (d.h. der Zustand, der mit dem Makrogitter verbunden ist)
    // ge"andert hat: Makrogitteraufbau und Lastvertielung. Der statische
    // Zustand darf durch Verfeinerung und h"ohere Methoden nicht beeinflusst
    // sein.

#ifdef ALUGRIDDEBUG
    const int start = clock () ;
#endif
    try
    {
      PackUnpackStaticState data( containerPll () );
      mpAccess ().exchange ( data );
    }
    catch (Parallel ::  AccessPllException&)
    {
      std::cerr << "  FEHLER Parallel :: AccessPllException entstanden" << std::endl ;
    }
    alugrid_assert (debugOption (20) ? (std::cout << "**INFO GitterPll :: exchangeStaticState () used "
      << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec. " << std::endl, 1) : 1 ) ;
    return ;
  }

  void GitterPll::exchangeDynamicState ()
  {
    // Zustand des Gitters ge"andert hat: Verfeinerung und alle Situationen
    // die einer "Anderung des statischen Zustands entsprechen. Sie wird in
    // diesem Fall NACH dem Update des statischen Zustands aufgerufen (nach loadBalance),
    // und kann demnach von einem korrekten statischen Zustand ausgehen. F"ur
    // Methoden die noch h"aufigere Updates erfordern m"ussen diese in der
    // Regel hier eingeschleift werden.
    {
#ifdef ALUGRIDDEBUG
      // if debug mode, then count time
      const int start = clock ();
#endif
      try
      {
        PackUnpackDynamicState data( this->containerPll() );
        mpAccess().exchange( data );
      }
      catch( Parallel:: AccessPllException& )
      {
        std::cerr << "ERROR: Parallel::AccessPllException caught." << std::endl;
      }
      alugrid_assert (debugOption (20) ? (std::cout << "**INFO GitterDunePll::packUnpackDynamicState () used " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec. " << std::endl, 1) : 1 );
    }
  }

  bool GitterPll::
  checkPartitioning( LoadBalancer::DataBase& db,
                     GatherScatterType* gs )
  {
    // build macro graph, either using user defined weights or default weighs
    alugrid_assert (debugOption (20) ? (std::cout << "**GitterPll::checkPartitioning ( db, gs ) " << std::endl, 1) : 1);
    // only for the SFC approach without edges we don't need to setup these connections
    const bool insertGraphEdges = LoadBalancer::DataBase:: graphEdgesNeeded( _ldbMethod ) ;
    if( insertGraphEdges )
    {
      // insert edges to graph and check for periodic bnds
      bool foundPeriodicBnd = false;
      const bool serialPart = serialPartitioner();
      AccessIterator < hface_STI >::Handle w (containerPll ());
      for (w.first (); ! w.done (); w.next ())
      {
        foundPeriodicBnd |= w.item ().ldbUpdateGraphEdge (db, serialPart );
      }

      // the foundPeriodic information is exchange upon graphCollect
      if( foundPeriodicBnd )
      {
        // this should not be set then
        alugrid_assert ( _graphSizes.size() == 0 );

        // clear graph sizes since the
        // precomputed sizes don't work with periodic bnd
        // since we change the partitioning after the call of repartition
        // to make sure periodic elements stay on the same partition
        db.clearGraphSizesVector();
      }
    }

    {
      // if gs is given and user defined weight is enabled, use these to setup graph
      GatherScatter* gatherScatter = gs && gs->userDefinedLoadWeights() ? gs : 0;
      AccessIterator < helement_STI >::Handle w (containerPll ());
      for (w.first (); ! w.done (); w.next ()) w.item ().ldbUpdateGraphVertex (db, gatherScatter);
    }

    // check in case repartition is initially false
    const int np = mpAccess ().psize ();
    // Criterion, if a repartition has to be done
    //
    // load    - own load
    // mean    - mean load of elements
    // minload - minmal load
    // maxload - maximal load

    // number of leaf elements
    const double myload = db.accVertexLoad ();

    // get:  min(myload), max(myload), sum(myload)
    MpAccessLocal::minmaxsum_t load = mpAccess ().minmaxsum( myload );

    // get mean value of leaf elements
    const double mean = load.sum / double( np );

    const bool repartition = ((load.max > (_ldbOver * mean)) || (load.min < (_ldbUnder * mean) )) ? true : false ;

#ifdef ALUGRIDDEBUG
    // make sure every process has the same value of repartition
    const bool checkNeu = mpAccess().gmax( repartition );
    alugrid_assert ( repartition == checkNeu );
#endif

    return repartition;
  }

  double ldbTimerU2 = 0.0;
  double ldbTimerU3 = 0.0;
  double ldbTimerU4 = 0.0;

  // --loadBalance ( gs can be a pointer to NULL)
  bool GitterPll::loadBalance( GatherScatterType* gs )
  {
    int lap1 = clock ();

    // create load balancer data base, _elementCuts will be empty afterwards
    LoadBalancer::DataBase db( _graphSizes, _elementCuts );

    // check if partioner is provided by used
    const bool userDefinedPartitioning = gs && gs->userDefinedPartitioning();
    // check whether we have to repartition
    const bool repartition = (userDefinedPartitioning) ? gs->repartition() :
                                                         checkPartitioning( db, gs );

    int lap2 = clock ();
    int lap3 = lap2 ;
    int lap4 = lap2 ;

    // if repartioning necessary, do it
    if ( repartition )
    {
      // clear graph sizes since they will change
      _graphSizes.clear();

      const int ldbMth = int( _ldbMethod );
#ifdef ALUGRIDDEBUG
      // make sure every process has the same ldb method
      int checkMth = mpAccess ().gmax( ldbMth );
      alugrid_assert ( checkMth == ldbMth );
#endif

      // if a method was given, perform load balancing
      if (userDefinedPartitioning || ldbMth)
      {
        // storage of vertex linkage is only done when no user defined partioning is given
        const bool storeLinkage = storeLinkageInVertices() && ! userDefinedPartitioning ;

        // call repartition macro grid
        repartitionMacroGrid (db, gs);

        lap3 = clock();

        // calls identification and exchangeDynamicState
        if( storeLinkage )
          doNotifyMacroGridChanges( &db );
        else
          doNotifyMacroGridChanges();

        lap4 = clock();
      }
    }

    float u2 = (float)(lap2 - lap1)/(float)(CLOCKS_PER_SEC); // check partitioning
    float u3 = (float)(lap3 - lap2)/(float)(CLOCKS_PER_SEC); // repartition macro grid
    float u4 = (float)(lap4 - lap3)/(float)(CLOCKS_PER_SEC); // identification

    ldbTimerU2 += u2 ;
    ldbTimerU3 += u3 ;
    ldbTimerU4 += u4 ;

    // store element cut information for later use (parallel sfc only)
    db.storeElementCuts( _elementCuts );

    return repartition;
  }

  void GitterPll::loadBalancerMacroGridChangesNotify ()
  {
    // exchanges the ldbVertexIndex for the internal boundaries
    // to obtain a consistent numbering
    exchangeStaticState ();
  }

  void GitterPll::computeGraphVertexIndices ()
  {
    // this method computes the globally unique element indices
    // that are needed for the graph partitioning methods
    alugrid_assert (debugOption (20) ? (std::cout << "**INFO GitterPll::computeGraphVertexIndices () " << std::endl, 1) : 1);
    AccessIterator < helement_STI >::Handle w ( containerPll () );

    // get number of macro elements
    const int macroElements = w.size ();

    // sum up for each process and and substract macroElements again
    int cnt = mpAccess ().scan( macroElements ) - macroElements;

#ifdef ALUGRIDDEBUG
    // make sure that we get the same value as before
    //std::cout << "P[ " << mpAccess().myrank() << " ] cnt = " << cnt << std::endl;
    {
      int oldcnt = 0;
      // get sizes from all processes
      std::vector< int > sizes = mpAccess ().gcollect ( macroElements );

      // count sizes for all processors with a rank lower than mine
      for (int i = 0; i < mpAccess ().myrank (); oldcnt += sizes [ i++ ]);
      alugrid_assert ( oldcnt == cnt );
    }
#endif // #ifdef ALUGRIDDEBUG

    // set ldb vertex indices to all elements
    for (w.first (); ! w.done (); w.next (), ++ cnt )
    {
      w.item ().setLoadBalanceVertexIndex ( cnt );
    }

    // mark unique element indices as computed if there are macro elements
    _ldbVerticesComputed = mpAccess().gmax( bool( macroElements > 0 ) );

    // clear graphSize vector since the new numbering leads to different sizes
    std::vector< int >().swap( _graphSizes );
#ifdef ALUGRIDDEBUG
    {
      alugrid_assert (debugOption (20) ? (std::cout << "**INFO GitterPll::loadBalancerMacroGridChangesNotify () " << std::endl, 1) : 1);
      AccessIterator < helement_STI >::Handle w ( containerPll () );

      int lastIndex = -1;
      // set ldb vertex indices to all elements
      for (w.first (); ! w.done (); w.next () )
      {
        const int ldbVx = w.item ().ldbVertexIndex();
        alugrid_assert ( lastIndex < ldbVx );
        lastIndex = ldbVx;
      }
    }
#endif // #ifdef ALUGRIDDEBUG
  }

  void GitterPll::notifyMacroGridChanges ()
  {
    // make sure graph indices are computed before identification is done
    if( ! _ldbVerticesComputed )
      computeGraphVertexIndices ();

    doNotifyMacroGridChanges();
  }

  void GitterPll::doNotifyMacroGridChanges ( LoadBalancer::DataBase* db )
  {
    alugrid_assert (debugOption (20) ? (std::cout << "**INFO GitterPll::notifyMacroGridChanges () " << std::endl, 1) : 1 );
    Gitter::notifyMacroGridChanges ();

    containerPll ().identification (mpAccess (), db, storeLinkageInVertices() );

    loadBalancerMacroGridChangesNotify ();
    exchangeDynamicState ();
    return;
  }

  GitterPll::GitterPll ( MpAccessLocal & mpa )
    : _graphSizes(),
      _ldbOver (0.0),
      _ldbUnder (0.0),
      _ldbMethod (LoadBalancer::DataBase::NONE),
      _minRefineLoops( 2 ),
      _refinesDone( 0 ),
      _averageRefineLoops( 0 ),
      _ldbVerticesComputed( false )
  {
    // check if load balance parameters were set internally
    auto& ldbParam = ALUGridExternalParameters::loadBalanceParameters();
    if( ldbParam.second != -1 )
    {
      _ldbUnder  = ldbParam.first.first;
      _ldbOver   = ldbParam.first.second;
      _ldbMethod = (LoadBalancer::DataBase::method) ldbParam.second ;
      // reset parameters for next grid generation
      ALUGridExternalParameters::resetLoadBalanceParameters();
    }
    else
    {
      if( mpa.myrank() == 0 )
      {
        // set default values
        _ldbOver = 1.2;
        // default partitioning which does not require external packages
        _ldbMethod = LoadBalancer::DataBase::ALUGRID_SpaceFillingCurve;

        std::ifstream in( "alugrid.cfg" );
        if( in )
        {
          int i;
          in >> _ldbUnder;
          in >> _ldbOver;
          in >> i;
          _ldbMethod = (LoadBalancer::DataBase::method) i;
        }
        else if (ALUGridExternalParameters::verbosityLevel() > 1)
        {
          std::cerr << "WARNING (ignored): Could not open file 'alugrid.cfg', using default values!" << std::endl;
        }
      } // got values on rank 0

      // now communicate them
      double buff[ 3 ]  = { _ldbOver, _ldbUnder, double(_ldbMethod) };

      // broadcast values from rank 0 to all others
      // (much better then to read file on all procs)
      const int root = 0;
      mpa.bcast( &buff[ 0 ], 3, root);

      // store values
      _ldbOver   = buff[ 0 ];
      _ldbUnder  = buff[ 1 ];
      _ldbMethod = (LoadBalancer::DataBase::method ) buff[ 2 ];
    }

    if( mpa.myrank() == 0 && ALUGridExternalParameters::verbosityLevel() > 1)
    {
      std::cerr << "ALUGrid-LB values: " << _ldbUnder << " < [balance] < " << _ldbOver << ", partitioning method '" << LoadBalancer::DataBase::methodToString( _ldbMethod ) << "'." << std::endl;
    }

    // we possibly need to initialize zoltan at some point - we will do it here...
    LoadBalancer::DataBase::initializeZoltan( _ldbMethod );

    // wait for all to finish
#ifdef ALUGRIDDEBUG
    mpa.barrier();
#endif

  }

} // namespace ALUGrid
