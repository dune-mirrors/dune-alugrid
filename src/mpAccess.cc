	// (c) bernhard schupp 1997 - 1998

	// $Source$
	// $Revision$
	// $Name$
	// $State$

/* $Id$
 * $Log$
 * Revision 1.1  2004/10/25 16:41:20  robertk
 * Parallel grid implementations.
 *
 * Revision 1.4  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.3  2001/12/10 13:57:23  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/ 

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

static volatile char RCSId_mpAccess_cc [] = "$Id$" ;

vector < int > MpAccessLocal :: dest () const {
  vector < int > d (linkage.size ()) ;
  for (map < int, int, less < int > > :: const_iterator i = linkage.begin () ; 
  	i != linkage.end () ; i ++) d [(*i).second] = (*i).first ;
  return d ;
}

void MpAccessLocal :: printLinkage (ostream & out) const {
  out << "  MpAccessLocal :: printLinkage () " << myrank () << " -> " ;
  { for (map < int, int, less < int > > :: const_iterator i = linkage.begin () ; 
  	i != linkage.end () ; i ++) out << (*i).first << " " ; }
  out << endl ;
  return ;
}

int MpAccessLocal :: insertRequestSymetric (set < int, less < int > > req) {
  const int me = myrank () ;
  req.erase (me) ;
  vector < int > out ;
  {for (set < int, less < int > > :: const_iterator i = req.begin () ; i != req.end () ; i ++ )
    if (linkage.find (*i) == linkage.end ()) out.push_back (*i) ; }
  vector < vector < int > > in = gcollect (out) ;
  { for (vector < int > :: const_iterator i = out.begin () ; i != out.end () ; i ++ )
    if (linkage.find (*i) == linkage.end ()) {
      int n = linkage.size () ;
      linkage [*i] = n ;
    }
  }
  int cnt = 0 ;
  { for (int i = 0 ; i < psize () ; i ++ )
    if (in [i].end() != find (in [i].begin(), in [i].end(), me)) {
      assert (i != me) ;
      if (linkage.find (i) == linkage.end ()) {
        int n = linkage.size () ;
        linkage [i] = n ;
        cnt ++ ; 
      }
    }
  }
  return cnt ;
}
