// (c) bernhard schupp, 1997 - 1998
#ifndef LOCK_H_INCLUDED
#define LOCK_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
#else
  #include <iostream.h>
#endif

	// Einfache Klasse, die w"ahrend ihrer Lebnsdauer ein
	// Lockfile mit einem vorgegebenen Namen (Pfad) h"alt.

class FSLock {
  char * _fname ;
  public :
    FSLock (const char * = "") ;
   ~FSLock () ;
} ;

inline FSLock :: FSLock (const char * name) : _fname (0) {
  _fname = new char [strlen(name) + 100] ;
  assert (_fname) ;
  sprintf (_fname, "%s.lock", name) ;
  FILE * fp = fopen (_fname, "w") ;
  if (fp == NULL) {
    delete [] _fname ;
    _fname = 0 ;
    cerr << "**WARNUNG (IGNORIERT) Lockfile konnte nicht erzeugt werden" << endl ;
  } else {
    
    // only test in debug mode 
#ifndef NDEBUG 
    int test = 
#endif
    fclose (fp) ;
    assert (test == 0) ;
  }
  return ;
}

inline FSLock :: ~FSLock () {
  if (_fname) {
    int test = remove (_fname) ;
    if (test != 0) {
      cerr << "**WARNUNG (IGNORIERT) Lockfile konnte nicht gel\"oscht werden." << endl ;
    }
    delete [] _fname ;
    _fname = 0 ;
  }
  return ;
}

#endif  // LOCK_H_INCLUDED
