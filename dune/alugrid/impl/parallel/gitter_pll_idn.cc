// (c) bernhard schupp 1997 - 1998
// (c) Robert Kloefkorn 2012 - 2013
#include <config.h>

#include <list>
#include <map>
#include <set>
#include <vector>

#include "gitter_pll_sti.h"

namespace ALUGrid
{
  template < class A, class B, class C >
  class UnpackIdentification 
    : public MpAccessLocal::NonBlockingExchange::DataHandleIF
  {
    typedef std::set< std::vector< int > > lp_map_t;

    typedef std::map< typename LinkedObject::Identifier, 
                      std::pair< typename AccessIterator < A >::Handle, 
                      typename lp_map_t::const_iterator > > vx_lmap_t;

    typedef std::map< typename LinkedObject::Identifier, 
                      std::pair< typename AccessIterator < B >::Handle, 
                      typename lp_map_t::const_iterator > > edg_lmap_t;

    typedef std::map< typename LinkedObject::Identifier, 
                      std::pair< typename AccessIterator < C >::Handle, 
                      typename lp_map_t::const_iterator > > fce_lmap_t;

    typedef std::vector< std::pair< std::list< typename AccessIterator < A >::Handle >, 
                         std::list< typename AccessIterator < A >::Handle > > > vx_tt_t;
    typedef std::vector< std::pair< std::list< typename AccessIterator < B >::Handle >, 
                         std::list< typename AccessIterator < B >::Handle > > > edg_tt_t;
    typedef std::vector< std::pair< std::list< typename AccessIterator < C >::Handle >, 
                         std::list< typename AccessIterator < C >::Handle > > > fce_tt_t;

    lp_map_t&   _linkagePatternMapVx;
    vx_lmap_t&  _lookVx;
    vx_tt_t& _vx;

    lp_map_t&   _linkagePatternMapEdg;
    edg_lmap_t& _lookEdg;
    edg_tt_t& _edg;

    lp_map_t&   _linkagePatternMapFce;
    fce_lmap_t& _lookFce;
    fce_tt_t& _fce;

    const std::vector< int >& _dest;
    const bool _firstLoop ;

    UnpackIdentification( const UnpackIdentification& );
  public:
    UnpackIdentification( lp_map_t& linkagePatternMapVx, 
                          vx_lmap_t&  lookVx, vx_tt_t& vx,
                          lp_map_t& linkagePatternMapEdg, 
                          edg_lmap_t& lookEdg, edg_tt_t& edg,
                          lp_map_t& linkagePatternMapFce, 
                          fce_lmap_t& lookFce, fce_tt_t& fce,
                          const std::vector< int >& dest,
                          const bool firstLoop )
      : _linkagePatternMapVx( linkagePatternMapVx ),
        _lookVx( lookVx ),
        _vx( vx ),
        _linkagePatternMapEdg( linkagePatternMapEdg ),
        _lookEdg( lookEdg ),
        _edg( edg ),
        _linkagePatternMapFce( linkagePatternMapFce ),
        _lookFce( lookFce ),
        _fce( fce ),
        _dest( dest ),
        _firstLoop( firstLoop )
    {}

    void pack( const int link, ObjectStream& os ) 
    {
      std::cerr << "ERROR: UnpackIdentification::pack should not be called!" << std::endl;
      abort();
    }

    void packAll( typename AccessIterator < A >::Handle& vxMi,
                  typename AccessIterator < B >::Handle& edgMi,
                  typename AccessIterator < C >::Handle& fceMi,
                  std::vector< ObjectStream >& inout, const MpAccessLocal & mpa ) 
    {
      // clear streams 
      const int nl = mpa.nlinks();
      for( int l=0; l < nl; ++ l ) 
        inout[ l ].clear();
      
      if( _firstLoop ) 
      {
        // vertices 
        packFirstLoop< A >( inout, mpa, vxMi , _linkagePatternMapVx , _lookVx );
        // edges 
        packFirstLoop< B >( inout, mpa, edgMi, _linkagePatternMapEdg, _lookEdg );
        // faces 
        packFirstLoop< C >( inout, mpa, fceMi, _linkagePatternMapFce, _lookFce );
      }
      else 
      {
        // vertices 
        packSecondLoop( inout, mpa, _lookVx , _vx  );
        // edges 
        packSecondLoop( inout, mpa, _lookEdg, _edg );
        // faces 
        packSecondLoop( inout, mpa, _lookFce, _fce );
      }
    }

    template < class T, class look_t > 
    void packFirstLoop( std::vector< ObjectStream> &inout,
                        const MpAccessLocal & mpa,
                        typename AccessIterator < T >::Handle& mi,
                        lp_map_t& linkagePatternMap,  
                        look_t& look )
    {
      const int me = mpa.myrank ();
      lp_map_t::const_iterator meIt = linkagePatternMap.insert (std::vector< int >  (1L, me)).first;

      for (mi.first (); ! mi.done (); mi.next ()) 
      {
        T& item = mi.item ();
        std::vector< int > estimate = item.estimateLinkage ();
        if (estimate.size ()) 
        {
          LinkedObject::Identifier id = item.getIdentifier ();
          look [id].first  = mi;
          look [id].second = meIt;
          {
            std::vector< int >::const_iterator iEnd = estimate.end ();
            for (std::vector< int >::const_iterator i = estimate.begin (); 
                 i != iEnd; ++i )
            {
              id.write ( inout [ mpa.link (*i) ] );
            }
          }
        }
      }

      // write end marker to stream 
      const int nl = mpa.nlinks();
      for( int link = 0; link < nl ; ++ link ) 
      {
        LinkedObject::Identifier::endOfStream( inout[ link ] );
      }
    }
      
    template < class look_t, class ttt > 
    void packSecondLoop( std::vector< ObjectStream> &inout,
                         const MpAccessLocal & mpa,
                         look_t& look, ttt& tt ) 
    {
      const int me = mpa.myrank ();

      const typename look_t::const_iterator lookEnd = look.end ();
      for (typename look_t::const_iterator pos = look.begin (); 
           pos != lookEnd; ++pos) 
      {
        const std::vector< int > & lk (*(*pos).second.second);
        if (* lk.begin () == me) 
        {
          typename LinkedObject::Identifier id = (*pos).second.first.item ().accessPllX ().getIdentifier ();
          { 
            typename std::vector< int >::const_iterator iEnd = lk.end ();
            for (typename std::vector< int >::const_iterator i = lk.begin (); 
                 i != iEnd; ++i) 
            {
              if (*i != me) 
              {
                const int link = mpa.link (*i);
                tt [ link ].first.push_back ((*pos).second.first);
                id.write ( inout[ link ] );
              }
            } 
          }
        }
      }

      // write end marker to stream 
      const int nl = mpa.nlinks();
      for( int link = 0; link < nl ; ++ link ) 
      {
        LinkedObject::Identifier::endOfStream( inout[ link ] );
      }
    }

    void unpack( const int link, ObjectStream& os ) 
    {
      if( _firstLoop ) 
      {
        // vertices 
        unpackFirstLoop( link, os, _linkagePatternMapVx , _lookVx );
        // edges 
        unpackFirstLoop( link, os, _linkagePatternMapEdg, _lookEdg );
        // faces 
        unpackFirstLoop( link, os, _linkagePatternMapFce, _lookFce );
      }
      else
      {
        // vertices 
        unpackSecondLoop( link, os, _lookVx , _vx );
        // edges 
        unpackSecondLoop( link, os, _lookEdg, _edg );
        // faces
        unpackSecondLoop( link, os, _lookFce, _fce );
      }
    }

    template < class look_t > 
    void unpackFirstLoop( const int link, ObjectStream& os,
                          lp_map_t& linkagePatternMap,  
                          look_t& look )
    {
      typename LinkedObject::Identifier id;
      bool good = id.read( os );
      while ( good ) 
      {
        typename look_t::iterator hit = look.find (id);
        if (hit != look.end ()) 
        {
          std::vector< int > lpn (*(*hit).second.second);
          if (find (lpn.begin (), lpn.end (), _dest[ link ]) == lpn.end () ) 
          {
            lpn.push_back ( _dest[ link ] );
            std::sort (lpn.begin(), lpn.end() );
            (*hit).second.second = linkagePatternMap.insert (lpn).first;
          }
        }

        // read next id and check whether it was successful 
        good = id.read( os );
      }
    }

    template < class look_t, class ttt > 
    void unpackSecondLoop( const int link, ObjectStream& os, 
                           look_t& look, ttt& tt ) 
    {
      typename LinkedObject::Identifier id;
      bool good = id.read( os );
      while ( good ) 
      {
        alugrid_assert ( look.find (id) != look.end () );
        tt[ link ].second.push_back ((*look.find (id)).second.first);
      
        // is end marker was read break while loop
        good = id.read( os );
      } 
    }
  };


  template < class A, class B, class C > 
  void identify (typename AccessIterator < A >::Handle vxMi, 
                 std::vector< std::pair< std::list< typename AccessIterator < A >::Handle >, 
                              std::list< typename AccessIterator < A >::Handle > > > & vertexTT, 
                 typename AccessIterator < B >::Handle edgMi, 
                 std::vector< std::pair< std::list< typename AccessIterator < B >::Handle >, 
                              std::list< typename AccessIterator < B >::Handle > > > & edgeTT, 
                 typename AccessIterator < C >::Handle fceMi, 
                 std::vector< std::pair< std::list< typename AccessIterator < C >::Handle >, 
                              std::list< typename AccessIterator < C >::Handle > > > & faceTT, 
                 const MpAccessLocal & mpa) 
  {
    typedef std::set< std::vector< int > > lp_map_t;

    typedef std::map< typename LinkedObject::Identifier, 
                      std::pair< typename AccessIterator < A >::Handle, 
                      typename lp_map_t::const_iterator > > vx_lmap_t;

    typedef std::map< typename LinkedObject::Identifier, 
                      std::pair< typename AccessIterator < B >::Handle, 
                      typename lp_map_t::const_iterator > > edg_lmap_t;

    typedef std::map< typename LinkedObject::Identifier, 
                      std::pair< typename AccessIterator < C >::Handle, 
                      typename lp_map_t::const_iterator > > fce_lmap_t;

    const int nl = mpa.nlinks ();
    
    lp_map_t linkagePatternMapVx;
    lp_map_t linkagePatternMapEdg;
    lp_map_t linkagePatternMapFce;

    vx_lmap_t  lookVx;
    edg_lmap_t lookEdg;
    fce_lmap_t lookFce;

    // resize vectors 
    vertexTT.resize( nl );
    edgeTT.resize( nl );
    faceTT.resize( nl );

    std::vector< ObjectStream > inout (nl);

    {
      // data, first loop 
      UnpackIdentification< A, B, C > data( linkagePatternMapVx,  lookVx,  vertexTT, 
                                            linkagePatternMapEdg, lookEdg, edgeTT, 
                                            linkagePatternMapFce, lookFce, faceTT,
                                            mpa.dest(), true );

      // pack all data 
      data.packAll( vxMi, edgMi, fceMi, inout, mpa );
      
      // exchange data 
      mpa.exchange (inout, data );
    }

    {
      // data, second loop 
      UnpackIdentification< A, B, C > data( linkagePatternMapVx,  lookVx,  vertexTT, 
                                            linkagePatternMapEdg, lookEdg, edgeTT, 
                                            linkagePatternMapFce, lookFce, faceTT,
                                            mpa.dest(), false );

      // pack all data 
      data.packAll( vxMi, edgMi, fceMi, inout, mpa );
      
      // exchange data 
      mpa.exchange (inout, data );
    }
  }

  class UnpackVertexLinkage 
    : public MpAccessLocal::NonBlockingExchange::DataHandleIF
  {
    // choose negative endmarker, since all ids should be positive 
    static const int endMarker = -32767 ;
    typedef Gitter :: vertex_STI vertex_STI ;
    typedef std::map< int, vertex_STI* > map_t;
    map_t _vxmap;
    GitterPll::MacroGitterPll& _containerPll ;
    const int _me ;
    int _size ;
  public: 
    UnpackVertexLinkage( GitterPll::MacroGitterPll& containerPll, const int me ) 
      : _containerPll( containerPll ),
        _me( me ),
        _size( 0 )
    {
      // compute vertex linkage locally 
      if( Gitter :: storeLinkageInVertices ) 
      {
        AccessIterator < Gitter::helement_STI >::Handle w ( _containerPll );
        for (w.first (); ! w.done (); w.next ()) 
        {
          w.item().computeVertexLinkage();
        }
      }
    }

    void pack( const int rank, ObjectStream& os ) 
    {
      alugrid_assert ( rank == _me );
      AccessIterator < vertex_STI >::Handle w ( _containerPll );

      int _size = w.size() ;
      const int estimate = 0.25 * _size + 1;
      // reserve memory 
      os.reserve( estimate * sizeof(int) );
      os.writeObject( _size );
      for (w.first (); ! w.done (); w.next ()) 
      {
        vertex_STI& vertex = w.item();

        // clear all linkage of this vertex 
        vertex.clearLinkage();

        // only insert border vertices 
        if( vertex.isBorder() )
        {
          int id = vertex.ident ();
          os.writeObject( id );
          _vxmap[ id ] = &vertex;
          if( Gitter :: storeLinkageInVertices ) 
          {
            const std::set<int>& linkedElements = vertex.linkedElements();
            typedef std::set<int>::const_iterator set_iterator;
            const int linkedSize = linkedElements.size();
            os.writeObject( int(-linkedSize-1) );
            const set_iterator endElem = linkedElements.end();
            for( set_iterator it = linkedElements.begin(); it != endElem; ++it ) 
            {
              os.writeObject( *it );
            }
          }
        }
      }
      os.writeObject( endMarker );
    }

    void unpack( const int rank, ObjectStream& os )
    {
      alugrid_assert ( rank != _me );

      const map_t::const_iterator vxmapEnd = _vxmap.end();

      int wSize; 
      os.readObject( wSize );
      _size += wSize ;

      const bool storeLinkage = Gitter::storeLinkageInVertices ;

      int id ;
      os.readObject ( id );
      std::vector< int > linkedElements ;
      while( id != endMarker )
      {
        // search vertex 
        map_t::const_iterator hit = _vxmap.find (id);
        // read next id 
        os.readObject( id );
        if( storeLinkage && id < 0 && id != endMarker ) 
        {
          const int linkedSize = -id-1 ;
          linkedElements.resize( linkedSize );
          for( int el=0; el<linkedSize; ++el ) 
          {
            os.readObject( linkedElements[ el ] );
          }
          // read next vertex id 
          os.readObject( id );
        }

        // check vertex linkages 
        if( hit != vxmapEnd ) 
        {
          vertex_STI* vertex = (*hit).second;
          if( storeLinkage ) 
          {
            const int linkedSize = linkedElements.size();
            for( int el=0; el< linkedSize; ++el ) 
            {
              vertex->addGraphVertexIndex( linkedElements[ el ] );
            }
          }
          // check whether rank already is contained in the linkage and add otherwise 
          vertex->checkAndAddLinkage( rank );
        }
      }
    }

    void printVertexLinkage()
    {
      std::cout << "Global Vertex Size = " << _size << std::endl;

      AccessIterator < vertex_STI >::Handle w ( _containerPll );
      for (w.first (); ! w.done (); w.next ()) 
      {
        vertex_STI& vertex = w.item();
        std::vector< int > s = vertex.estimateLinkage() ;
        const size_t size = s.size() ;
        if( size > 0 ) 
        {
          std::cout << "Vx[ " << vertex.ident() << " ] = ";
          for( size_t i=0; i<size ; ++i ) 
            std::cout << s[ i ] << ",";
          std::cout << std::endl;
        }
      }
    }
  };

  void GitterPll::MacroGitterPll::vertexLinkageEstimateGCollect (MpAccessLocal & mpAccess) 
  {
    const int np = mpAccess.psize (), me = mpAccess.myrank ();

    static bool useAllgather = true ;
    if( useAllgather ) 
    {
      try 
      {
        ObjectStream os;
        // data handle 
        UnpackVertexLinkage data( *this, me );

        // pack data 
        data.pack( me, os );

        // exchange data 
        std::vector< ObjectStream > osv = mpAccess.gcollect( os );

        // free memory 
        os.reset();

        for (int link = 0; link < np; ++link ) 
        {
          // skip my rank 
          if( link == me ) continue ;

          // unpack data for link 
          data.unpack( link, osv[ link ] );
          // free memory 
          osv[ link ].reset(); 
        }
      }
      catch( MyAlloc :: OutOfMemoryException ) 
      {
        useAllgather = false ;
        // make sure every process caught an exception 
        // this needs to be revised since the exception 
        // might only be caught on a few processes 
        mpAccess.barrier();
      }
    }

    if( ! useAllgather ) 
    {
      // data handle 
      UnpackVertexLinkage data( *this, me );

      //std::cout <<"Print first linkage" << std::endl;
      //data.printVertexLinkage();
      //std::cout <<"Exchange data " << std::endl;
      // exchange all-to-all 
      mpAccess.allToAll( data );
    }
  }

  void GitterPll::MacroGitterPll::vertexLinkageEstimateBcast (MpAccessLocal & mpAccess) 
  {
    typedef std::map< int, vertex_STI* > map_t;
    map_t vxmap;
    const int np = mpAccess.psize ();
    const int me = mpAccess.myrank ();

    // exchange data 
    {
      AccessIterator < vertex_STI >::Handle w (*this);
      for (w.first (); ! w.done (); w.next ()) 
      {
        vertex_STI& vertex = w.item();

        // only insert border vertices 
        if( vertex.isBorder() )
        {
          vxmap[ vertex.ident() ] = &vertex;
        }

        // clear all linkage of this vertex 
        vertex.clearLinkage();
      }
    }

    // get max size for all ranks needed in later bcast cycle 
    const int mysize  = vxmap.size();
    const int maxSize = mpAccess.gmax( mysize ) + 1; // +1 for the size 

    // create buffer 
    std::vector< int > borderIds( maxSize, 0 ); 

    // loop over all ranks 
    for (int rank = 0; rank < np; ++rank ) 
    {
      const map_t::const_iterator vxmapEnd = vxmap.end();
      // fill buffer  
      if( rank == me ) 
      {
        // store the size on first position 
        borderIds[ 0 ] = mysize;
        int count = 1;
        for(map_t::const_iterator vx = vxmap.begin(); vx != vxmapEnd; ++vx, ++count )
        {
          borderIds[ count ] = (*vx).first;
        }
      }

      // send border vertex list to all others 
      mpAccess.bcast( &borderIds[ 0 ], maxSize, rank );

      // check connectivy for receives vertex ids 
      if( rank != me ) 
      {
        // get size which is the first entry 
        const int idSize = borderIds[ 0 ];
        // loop over all sended vertex ids 
        for (int j = 1; j <= idSize; ++j ) 
        {
          const int id = borderIds[ j ];
          map_t::const_iterator hit = vxmap.find (id);
          if( hit != vxmapEnd ) 
          {
            std::vector< int > s = (*hit).second->estimateLinkage ();
            if (find (s.begin (), s.end (), rank) == s.end ()) 
            {
              s.push_back( rank );
              (*hit).second->setLinkage (s);
            }
          }
        }
      }
    }
  }

  void GitterPll::MacroGitterPll::vertexLinkageEstimate (MpAccessLocal & mpAccess) 
  {
    // for small processor numbers use gcollect( MPI_Allgather ) version 
    // this method should be faster (log p), 
    // but is more memory consuming O( p ) 
    if( ALUGridExternalParameters::useAllGather( mpAccess ) ) 
    {
      vertexLinkageEstimateGCollect ( mpAccess );
    }
    else 
    {
      // for larger processor numbers use bcast ( MPI_Bcast ) version 
      // this method is more time consuming (p log p)
      // but is the memory consumption is only O( 1 )
      vertexLinkageEstimateBcast ( mpAccess );
    }
  }

  double identU2 = 0.0 ;
  double identU3 = 0.0 ;
  double identU4 = 0.0 ;

  void GitterPll::MacroGitterPll::identification (MpAccessLocal & mpa, bool computeVertexLinkage ) 
  {
    // clear all entries and also clear memory be reassigning 
    vertexTT_t().swap( _vertexTT );
    hedgeTT_t ().swap( _hedgeTT  );
    hfaceTT_t ().swap( _hfaceTT  );

    // make sure the memory was deallocated 
    alugrid_assert ( _vertexTT.capacity() == 0 );
    alugrid_assert ( _hedgeTT.capacity()  == 0 );
    alugrid_assert ( _hfaceTT.capacity()  == 0 );

    // clear linkage of mpAccess 
    mpa.removeLinkage ();
    
    clock_t lap1 = clock ();
    // this does not have to be computed every time (depending on partitioning method)
    if( computeVertexLinkage ) 
    {
      // clear linkage pattern map since it is newly build here
      clearLinkagePattern();

      // compute new vertex linkage 
      vertexLinkageEstimate ( mpa );
    }

    clock_t lap2 = clock ();
    // compute linkage due to vertex linkage 
    std::set< int > linkage; 
    secondScan( linkage );

    // insert linkage without communication into mpAccess 
    mpa.insertRequestSymmetric( linkage );

    if (debugOption (2)) 
      mpa.printLinkage (std::cout);

    clock_t lap3 = clock ();
    identify< vertex_STI, hedge_STI, hface_STI >( 
              AccessIterator < vertex_STI >::Handle (*this), _vertexTT, 
              AccessIterator < hedge_STI  >::Handle (*this), _hedgeTT,
              AccessIterator < hface_STI  >::Handle (*this), _hfaceTT,
              mpa);

    clock_t lap4 = clock ();

    float u2 = (float)(lap2 - lap1)/(float)(CLOCKS_PER_SEC);
    float u3 = (float)(lap3 - lap2)/(float)(CLOCKS_PER_SEC);
    float u4 = (float)(lap4 - lap3)/(float)(CLOCKS_PER_SEC);

    // accumulate times 
    identU2 += u2 ;
    identU3 += u3 ;
    identU4 += u4 ;

    if (debugOption (5)) 
    {
      std::cout.precision (6);
      std::cout << "**INFO MacroGitterPll::identification () [lnk|vtx|idn] ";
      std::cout << u2 << " " << u3 << " " << u4 << " sec." << std::endl;
    }

    if (debugOption (1)) 
    {
      const double nlinks = mpa.nlinks();
      double  u[ 4 ] = { u2, u3, u4, nlinks };
      double  uMax[ 4 ];
      mpa.gmax( &u[ 0 ], 4, &uMax[ 0 ] );
      if( mpa.myrank() == 0 ) 
      {
        std::cout.precision (6);
        std::cout << "**INFO MacroGitterPll::identification (): max links = "<< int(uMax[ 3 ]) << " [lnk|vtx|idn] ";
        std::cout << uMax[ 0 ] << " " << uMax[ 1 ] << " " << uMax[ 2 ] << " sec." << std::endl;
      }
    }
  }

} // namespace ALUGrid
