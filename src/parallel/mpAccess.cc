// (c) bernhard schupp 1997 - 1998
#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#ifdef _ANSI_HEADER
  using namespace std;
  #include <functional>
  #include <algorithm>
#else
  #include <function.h>
  #include <algo.h>
#endif

#include "mpAccess.h"

vector < int > MpAccessLocal :: dest () const {
  vector < int > d (_linkage.size ()) ;
  for (map < int, int, less < int > > :: const_iterator i = _linkage.begin () ; 
  	i != _linkage.end () ; i ++) d [(*i).second] = (*i).first ;
  return d ;
}

void MpAccessLocal :: printLinkage (ostream & out) const {
  out << "  MpAccessLocal :: printLinkage () " << myrank () << " -> " ;
  { for (map < int, int, less < int > > :: const_iterator i = _linkage.begin () ; 
  	i != _linkage.end () ; i ++) out << (*i).first << " " ; }
  out << endl ;
  return ;
}

int MpAccessLocal :: insertRequestSymetric (set < int, less < int > > req) {
  const int me = myrank () ;
  req.erase (me) ;
  vector < int > out ;
  {for (set < int, less < int > > :: const_iterator i = req.begin () ; i != req.end () ; i ++ )
    if (_linkage.find (*i) == _linkage.end ()) out.push_back (*i) ; }
  vector < vector < int > > in = gcollect (out) ;
  { for (vector < int > :: const_iterator i = out.begin () ; i != out.end () ; i ++ )
    if (_linkage.find (*i) == _linkage.end ()) {
      int n = _linkage.size () ;
      _linkage [*i] = n ;
    }
  }
  int cnt = 0 ;
  { for (int i = 0 ; i < psize () ; i ++ )
    if (in [i].end() != find (in [i].begin(), in [i].end(), me)) {
      assert (i != me) ;
      if (_linkage.find (i) == _linkage.end ()) {
        int n = _linkage.size () ;
        _linkage [i] = n ;
        cnt ++ ; 
      }
    }
  }
  return cnt ;
}
