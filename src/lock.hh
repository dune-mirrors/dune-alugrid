	// (c) bernhard schupp, 1997 - 1998
	
	// $Source$
	// $Revision$
	// $Name$
	// $State$

/* $Id$
 * $Log$
 * Revision 1.1  2004/10/15 09:48:37  robertk
 * Inititial version. Some extenxions for Dune made. Schould be compatible
 * with all other applications done so far.
 *
 * Revision 1.4  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.3  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

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

static volatile char RCSId_lock_h [] = "$Id$";

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
    int test = fclose (fp) ;
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
