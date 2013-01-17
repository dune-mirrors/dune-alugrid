// (c) bernhard schupp, 1997 - 1998
#ifndef LOCK_H_INCLUDED
#define LOCK_H_INCLUDED

#include <cassert>
#include <cstring>
#include <iostream>

class FSLock
{
  char * _fname;

public:
  FSLock ( const char * = "" );
 ~FSLock ();
};

inline FSLock::FSLock ( const char * name )
: _fname( 0 )
{
  _fname = new char[ std::strlen( name ) + 100 ];
  assert( _fname );
  sprintf (_fname, "%s.lock", name) ;
  FILE *fp = std::fopen( _fname, "w" );
  if( !fp )
  {
    delete[] _fname;
    _fname = 0;
    std::cerr << "WARNING (ignored): Could not create lock file." << std::endl;
  }
  else
  {
    // only test in debug mode 
#ifndef NDEBUG 
    int test = 
#endif
    std::fclose( fp );
    assert (test == 0) ;
  }
}

inline FSLock::~FSLock ()
{
  if( _fname )
  {
    int test = std::remove( _fname );
    if( test != 0 )
      std::cerr << "WARNING (ignored): Could not remove lock file." << std::endl;
    delete[] _fname;
    _fname = 0;
  }
}

#endif  // LOCK_H_INCLUDED
