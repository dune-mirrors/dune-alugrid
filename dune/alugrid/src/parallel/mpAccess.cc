#include <algorithm>
#include <iostream>

#include "mpAccess.h"

void MpAccessLocal::printLinkage ( std::ostream &out ) const
{
  out << "  MpAccessLocal::printLinkage() " << myrank () << " -> ";
  for (std::map < int, int >::const_iterator i = _linkage.begin (); i != _linkage.end(); ++i )
    out << (*i).first << " ";
  out << std::endl;
}

int MpAccessLocal::insertRequestSymetric ( std::set< int > req )
{
  const int me = myrank ();
  req.erase (me);
  std::vector< int > out;
  {for (std::set< int >::const_iterator i = req.begin (); i != req.end (); i ++ )
    if (_linkage.find (*i) == _linkage.end ()) out.push_back (*i); }
  std::vector< std::vector< int > > in = gcollect (out);
  { for (std::vector< int >::const_iterator i = out.begin (); i != out.end (); i ++ )
    if (_linkage.find (*i) == _linkage.end ()) {
      int n = _linkage.size ();
      _linkage [*i] = n;
    }
  }
  int cnt = 0;
  for( int i = 0; i < psize(); ++i )
  {
    if( std::find (in[ i ].begin(), in[ i ].end(), me) != in[ i ].end()  )
    {
      assert (i != me);
      if (_linkage.find (i) == _linkage.end ())
      {
        int n = _linkage.size ();
        _linkage [i] = n;
        cnt ++; 
      }
    }
  }

  // setup dest vector 
  {
    _dest.resize( _linkage.size () );
    for( std::map< int, int >::const_iterator i = _linkage.begin (); i != _linkage.end (); ++i )
      _dest[(*i).second] = (*i).first;
  }

  return cnt;
}
