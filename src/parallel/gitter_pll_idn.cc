// (c) bernhard schupp 1997 - 1998
#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <assert.h>
#include <time.h>
#include <stdio.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
  #include <functional>
  #include <algorithm>
#else
  #include <iostream.h>
  #include <function.h>
  #include <algo.h>
#endif

#include "gitter_pll_sti.h"

	// vorsicht mit optimierenden compilern bei dem untenstehnden template !

template < class A > void identify (typename AccessIterator < A > :: Handle mi, 
  vector < pair < list < typename AccessIterator < A > :: Handle >, 
  list < typename AccessIterator < A > :: Handle > > > & tt, const MpAccessLocal & c) {
  
  typedef set < vector < int >, less < vector < int > > > lp_map_t ;

  typedef map < typename LinkedObject :: Identifier, 
                pair < typename AccessIterator < A > :: Handle, 
                typename lp_map_t :: const_iterator >, 
                less < typename LinkedObject :: Identifier > > lmap_t ;

  const int me = c.myrank (), np = c.psize (), nl = c.nlinks () ;
  
  lp_map_t linkagePatternMap ;
  lmap_t look ;
  
  {
    vector < vector < int > > inout (nl) ;
    lp_map_t :: const_iterator meIt = linkagePatternMap.insert (vector < int >  (1L, me)).first ;
    {for (mi.first () ; ! mi.done () ; mi.next ()) {
      vector < int > estimate = mi.item ().accessPllX ().estimateLinkage () ;
      if (estimate.size ()) {
        LinkedObject :: Identifier id = mi.item ().accessPllX ().getIdentifier () ;
        look [id].first = mi ;
        look [id].second = meIt ;
	{for (vector < int > :: const_iterator i = estimate.begin (); i != estimate.end (); i ++ )
          id.write (inout [c.link (*i)]) ;
        }
      }
    }}
    inout = c.exchange (inout) ;
    vector < int > d = c.dest () ;
    {for (int l = 0 ; l < nl ; l ++ ) {
      vector < int > :: const_iterator pos = inout [l].begin (), end = inout [l].end () ;
      while (pos != end) {
        typename LinkedObject :: Identifier id ;
        id.read (pos,end) ;
        typename lmap_t :: iterator hit = look.find (id) ;
        if (hit != look.end ()) {
          vector < int > lpn (*(*hit).second.second) ;
          if (find (lpn.begin (), lpn.end (), d [l]) == lpn.end ()) {
	    lpn.push_back (d [l]) ;
	    sort (lpn.begin (), lpn.end (), less < int > ()) ;
	    (*hit).second.second = linkagePatternMap.insert (lpn).first ;
	  }
        }
      }
    }}
  }
  tt = vector < pair < list < typename AccessIterator < A > :: Handle >, 
                       list < typename AccessIterator < A > :: Handle > > > (nl) ;
  {
    vector < vector < int > > inout (nl) ;
    {for (typename lmap_t :: const_iterator pos = look.begin () ; 
        pos != look.end () ; pos ++) {
      const vector < int > & lk (*(*pos).second.second) ;
      if (* lk.begin () == me) {
        typename LinkedObject :: Identifier id = (*pos).second.first.item ().accessPllX ().getIdentifier () ;
        {for (typename vector < int > :: const_iterator i = lk.begin () ; i != lk.end () ; i ++) {
          if (*i != me) {
            int l = c.link (*i) ;
            tt [l].first.push_back ((*pos).second.first) ;
            id.write (inout [l]) ;
          }
        }}
      }
    }}
    inout = c.exchange (inout) ;
    {for (int i = 0 ; i < nl ; i ++ ) {
      typename vector < int > :: const_iterator pos = inout [i].begin (), end = inout [i].end () ;
      while (pos != inout [i].end ()) {
        typename LinkedObject :: Identifier id ;
        id.read (pos,end) ;
        assert (look.find (id) != look.end ()) ;
        tt [i].second.push_back ((*look.find (id)).second.first) ;
      } 
    }}
  }
  return ;
}

set < int, less < int > > GitterPll :: MacroGitterPll :: secondScan () {
  set < int, less < int > > s ;
  {
    AccessIterator < vertex_STI > :: Handle w (*this) ;
    for ( w.first () ; ! w.done () ; w.next ()) {
      vector < int > l = w.item ().accessPllX ().estimateLinkage () ;
      for (vector < int > :: const_iterator i = l.begin () ; i != l.end () ; s.insert (* i ++)) ;
    }
  }
  return s ;
}

void GitterPll :: MacroGitterPll :: vertexLinkageEstimate (MpAccessLocal & c) {
  typedef map < int, AccessIterator < vertex_STI > :: Handle, less < int > > map_t ;
  map_t map ;
  const int np = c.psize (), me = c.myrank () ;
  ObjectStream os ;
  {
    AccessIterator < vertex_STI > :: Handle w (*this) ;
    os.writeObject (w.size ()) ;
    for (w.first () ; ! w.done () ; w.next ()) {
      int id = w.item ().ident () ;
      os.writeObject (id) ;
      map [id] = w ;
    }
  }
  vector < ObjectStream > osv = c.gcollect (os) ;
  {
    for (int i = 0 ; i < np ; i ++ ) {
      if (i != me) {
        int num ;
        osv [i].readObject (num) ;
        for (int j = 0 ; j < num ; j ++ ) {
	  int id ;
	  osv [i].readObject (id) ;
          map_t :: const_iterator hit = map.find (id) ;
	  if (hit != map.end ()) {
	    vector < int > s = (*hit).second.item ().accessPllX ().estimateLinkage () ;
	    if (find (s.begin (), s.end (), i) == s.end ()) {
	      s.push_back (i) ;
	      (*hit).second.item ().accessPllX ().setLinkage (s) ;
	    }
	  }
	}
      }
    }
  }
  return ;
}

void GitterPll :: MacroGitterPll :: identification (MpAccessLocal & c) {
  _vertexTT.erase (_vertexTT.begin (), _vertexTT.end ()) ;
  _hedgeTT.erase (_hedgeTT.begin (), _hedgeTT.end ()) ;
  _hfaceTT.erase (_hfaceTT.begin (), _hfaceTT.end ()) ;
  c.removeLinkage () ;
  
  int lap1 = clock () ;
  vertexLinkageEstimate (c) ;
  c.insertRequestSymetric (secondScan ()) ;
  if (debugOption (2)) c.printLinkage (cout) ;
  int lap2 = clock () ;
#ifdef IBM_XLC
  //  identify (AccessIterator < vertex_STI > :: Handle (*this), _vertexTT, c) ;
  // ### Goettingen ###
  identify < vertex_STI > (AccessIterator < vertex_STI > :: Handle (*this), _vertexTT, c) ;
#else
  identify < vertex_STI > (AccessIterator < vertex_STI > :: Handle (*this), _vertexTT, c) ;
#endif
  int lap3 = clock () ;
#ifdef IBM_XLC
  //  identify (AccessIterator < hedge_STI > :: Handle (*this), _hedgeTT, c) ;
  // ### Goettingen ###
  identify < hedge_STI > (AccessIterator < hedge_STI > :: Handle (*this), _hedgeTT, c) ;
#else
  identify < hedge_STI > (AccessIterator < hedge_STI > :: Handle (*this), _hedgeTT, c) ;
#endif
  int lap4 = clock () ;
#ifdef IBM_XLC
  //  identify (AccessIterator < hface_STI > :: Handle (*this), _hfaceTT, c) ;
  // ### Goettingen ###
  identify < hface_STI > (AccessIterator < hface_STI > :: Handle (*this), _hfaceTT, c) ;
#else
  identify < hface_STI > (AccessIterator < hface_STI > :: Handle (*this), _hfaceTT, c) ;
#endif
  int lap5 = clock () ;
  if (debugOption (2)) {
    float u2 = (float)(lap2 - lap1)/(float)(CLOCKS_PER_SEC) ;
    float u3 = (float)(lap3 - lap2)/(float)(CLOCKS_PER_SEC) ;
    float u4 = (float)(lap4 - lap3)/(float)(CLOCKS_PER_SEC) ;
    float u5 = (float)(lap5 - lap4)/(float)(CLOCKS_PER_SEC) ;
    float u6 = (float)(lap5 - lap1)/(float)(CLOCKS_PER_SEC) ;
    cout.precision (3) ;
    cout << "**INFO GitterPll :: MacroGitterPll :: identification () [lnk|vtx|edg|fce|all] " ;
    cout << u2 << " " << u3 << " " << u4 << " " << u5 << " " << u6 << " sec." << endl ;
  }
  return ;
}
