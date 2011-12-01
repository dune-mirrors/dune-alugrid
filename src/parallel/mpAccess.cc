#include "mpAccess.h"

void MpAccessLocal :: printLinkage (ostream & out) const {
  out << "  MpAccessLocal :: printLinkage () " << myrank () << " -> " ;
  { for (map < int, int, less < int > > :: const_iterator i = _linkage.begin () ; 
  	i != _linkage.end () ; i ++) out << (*i).first << " " ; }
  out << endl ;
  return ;
}

int MpAccessLocal :: insertRequestSymetric (set < int, less < int > > req) 
{
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

  // setup dest vector 
  {
    _dest.resize( _linkage.size () ) ;
    for (map < int, int, less < int > > :: const_iterator i = _linkage.begin () ; 
  	  i != _linkage.end () ; i ++) _dest[(*i).second] = (*i).first ;
  }

  return cnt ;
}
