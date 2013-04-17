#include <algorithm>
#include <iostream>

#include "mpAccess.h"

namespace ALUGrid
{

  void MpAccessLocal::printLinkage ( std::ostream &out ) const
  {
    out << "  MpAccessLocal::printLinkage() " << myrank () << " -> ";
    for (std::map < int, int >::const_iterator i = _linkage.begin (); i != _linkage.end(); ++i )
      out << (*i).first << " ";
    out << std::endl;
  }

  int MpAccessLocal::insertRequest( const std::set< int >& req )
  {
    const int me = myrank ();

    {
      typedef std::map< int, int >::iterator iterator ;
      typedef std::set< int >::const_iterator const_iterator;

      const iterator linkageEnd = _linkage.end ();
      const const_iterator reqEnd = req.end ();
      int link = 0 ;
      for (const_iterator i = req.begin (); i != reqEnd; ++i )
      {
        const int rank = (*i);
        // if rank was not inserted, insert with current link number 
        if( rank != me && (_linkage.find ( rank ) == linkageEnd ) )
        {
          _linkage.insert( std::make_pair( rank, link++) );
        }
      }
    }

    // setup destination vector from linkage map 
    {
      typedef std::map< int, int >::const_iterator const_iterator ;
      _dest.resize( _linkage.size () );
      const const_iterator linkageEnd = _linkage.end ();
      for( const_iterator i = _linkage.begin (); i != linkageEnd; ++i )
      {
        //std::cout << "link: " << (*i).first << " " <<  (*i).second << std::endl;
        _dest[ (*i).second ] = (*i).first;
      }
    }
    return _linkage.size();
  }

  // insertRequestSymmetric needs a global communication 
  // this method is used to build the pattern for the loadBalancing 
  // where we don't know who is sending whom something 
  int MpAccessLocal::insertRequestSymetric ( const std::set< int >& req )
  {
    const int me = myrank ();

    std::vector< int > out;
    out.reserve( req.size() );

    {
      typedef std::map< int, int >::iterator iterator ;
      typedef std::set< int >::const_iterator const_iterator;

      const iterator linkageEnd = _linkage.end ();
      const const_iterator reqEnd = req.end ();
      for (const_iterator i = req.begin (); i != reqEnd; ++i )
      {
        const int rank = (*i);
        if( rank != me && (_linkage.find (rank) == linkageEnd ) )
          out.push_back ( rank );
      }
    }

    std::vector< std::vector< int > > in = gcollect (out);
    { 
      for (std::vector< int >::const_iterator i = out.begin (); i != out.end (); ++i )
      {
        if (_linkage.find (*i) == _linkage.end ()) 
        {
          int n = _linkage.size ();
          _linkage [*i] = n;
        }
      }
    }

    int cnt = 0;
    for( int i = 0; i < psize(); ++i )
    {
      if( std::find (in[ i ].begin(), in[ i ].end(), me) != in[ i ].end()  )
      {
        alugrid_assert (i != me);
        if (_linkage.find (i) == _linkage.end ())
        {
          int n = _linkage.size ();
          _linkage [i] = n;
          cnt ++; 
        }
      }
    }

    // setup destination vector from linkage map 
    {
      typedef std::map< int, int >::const_iterator const_iterator ;
      _dest.resize( _linkage.size () );
      const const_iterator linkageEnd = _linkage.end ();
      for( const_iterator i = _linkage.begin (); i != linkageEnd; ++i )
      {
        //std::cout << "link: " << (*i).first << " " <<  (*i).second << std::endl;
        _dest[ (*i).second ] = (*i).first;
      }
    }
    return _linkage.size();
  }

} // namespace ALUGrid
