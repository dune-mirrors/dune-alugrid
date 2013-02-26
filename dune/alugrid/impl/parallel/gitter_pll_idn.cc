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
        std::vector< int > estimate = mi.item ().accessPllX ().estimateLinkage ();
        if (estimate.size ()) 
        {
          LinkedObject::Identifier id = mi.item ().accessPllX ().getIdentifier ();
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
        assert ( look.find (id) != look.end () );
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

  std::set< int > GitterPll::MacroGitterPll::secondScan () 
  {
    std::set< int > s;
    {
      AccessIterator < vertex_STI >::Handle w (*this);
      for ( w.first (); ! w.done (); w.next ()) 
      {
        vertex_STI& vertex = w.item();
        // only border vertices can have linkage 
        if( vertex.isBorder() ) 
        {
          const std::vector< int > l = w.item ().accessPllX ().estimateLinkage ();
          const std::vector< int >::const_iterator iEnd = l.end ();
          for (std::vector< int >::const_iterator i = l.begin (); i != iEnd; ++i ) 
          {
            s.insert ( *i );
          }
        }
      }
    }
    return s;
  }

  void GitterPll::MacroGitterPll::vertexLinkageEstimateGCollect (MpAccessLocal & mpAccess) 
  {
    typedef std::map< int, AccessIterator < vertex_STI >::Handle > map_t;
    map_t vxmap;
    const int np = mpAccess.psize (), me = mpAccess.myrank ();

    ObjectStream os;
    // choose negative endmarker, since all ids should be positive 
    const int endMarker = -127 ;
    {
      AccessIterator < vertex_STI >::Handle w (*this);

      const int estimate = 0.25 * w.size() + 1;
      // reserve memory 
      os.reserve( estimate * sizeof(int) );
      for (w.first (); ! w.done (); w.next ()) 
      {
        vertex_STI& vertex = w.item();

        // only insert border vertices 
        if( vertex.isBorder() )
        {
          int id = vertex.ident ();
          os.writeObject( id );
          vxmap[ id ] = w;
        }
      }
      os.writeObject( endMarker );
    }

    // exchange data 
    std::vector< ObjectStream > osv = mpAccess.gcollect( os );

    // free memory 
    os.reset();

    {
      map_t::const_iterator vxmapEnd = vxmap.end();
      for (int i = 0; i < np; ++i ) 
      {
        if (i != me) 
        {
          ObjectStream& osv_i = osv[ i ]; 

          int id ;
          osv_i.readObject ( id );
          while( id != endMarker )
          {
            map_t::const_iterator hit = vxmap.find (id);
            if( hit != vxmapEnd ) 
            {
              std::vector< int > s = (*hit).second.item ().accessPllX ().estimateLinkage ();
                    if (find (s.begin (), s.end (), i) == s.end ()) 
              {
                s.push_back( i );
                (*hit).second.item ().accessPllX ().setLinkage (s);
              }
            }
            // read next id 
            osv_i.readObject( id );
          }
          // free memory 
          osv_i.reset();
        }
      }
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
            std::vector< int > s = (*hit).second->accessPllX ().estimateLinkage ();
            if (find (s.begin (), s.end (), rank) == s.end ()) 
            {
              s.push_back( rank );
              (*hit).second->accessPllX ().setLinkage (s);
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

  void GitterPll::MacroGitterPll::identification (MpAccessLocal & mpa) 
  {
    // clear all entries and also clear memory be reassigning 
    vertexTT_t().swap( _vertexTT );
    hedgeTT_t ().swap( _hedgeTT );
    hfaceTT_t ().swap( _hfaceTT );

    // make sure the memory was deallocated 
    assert( _vertexTT.capacity() == 0 );
    assert( _hedgeTT.capacity()  == 0 );
    assert( _hfaceTT.capacity()  == 0 );

    mpa.removeLinkage ();
    
    int lap1 = clock ();
    vertexLinkageEstimate ( mpa );

    int lap2 = clock ();
    mpa.insertRequestSymetric (secondScan ());
    if (debugOption (2)) mpa.printLinkage (std::cout);

    int lap3 = clock ();
    identify< vertex_STI, hedge_STI, hface_STI >( AccessIterator < vertex_STI >::Handle (*this), _vertexTT, 
              AccessIterator < hedge_STI >::Handle (*this), _hedgeTT,
              AccessIterator < hface_STI >::Handle (*this), _hfaceTT,
              mpa);

    int lap4 = clock ();

    if (debugOption (2)) 
    {
      float u2 = (float)(lap2 - lap1)/(float)(CLOCKS_PER_SEC);
      float u3 = (float)(lap3 - lap2)/(float)(CLOCKS_PER_SEC);
      float u4 = (float)(lap4 - lap3)/(float)(CLOCKS_PER_SEC);
      std::cout.precision (3);
      std::cout << "**INFO GitterPll::MacroGitterPll::identification () [lnk|vtx|idn] ";
      std::cout << u2 << " " << u3 << " " << u4 << " sec." << std::endl;
    }
  }

} // namespace ALUGrid
