	//	(c) christian badura 1998
	//      -*- C++ -*-

	// $Source$
	// $Revision$
	// $Name$
	// $State$

/* $Id$
 * $Log$
 * Revision 1.1  2004/10/25 16:39:54  robertk
 * Some off the headers are old and changed from .hh to .h.
 * All changes are made in the headers aswell.
 *
 * Some new file are the parallel grid files mostly have a _pll_ in then name.
 * There some Constructors of Tetra and Hbdn3Top had to be adapted.
 *
 * Revision 1.1  2004/10/15 09:48:38  robertk
 * Inititial version. Some extenxions for Dune made. Schould be compatible
 * with all other applications done so far.
 *
 * Revision 1.3  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.2  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

#ifndef MYALLOC_H_INCLUDED
#define MYALLOC_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#ifdef _ANSI_HEADER
  using namespace std;
  #include <memory>      // Def. von size_t, malloc (), free ()
#else
  #include <memory.h>    // Def. von size_t, malloc (), free ()
#endif

static volatile char RCSId_myalloc_h [] = "$Id$" ;

class MyAlloc {
  static const long MAX_HOLD_ADD ;
  static const double MAX_HOLD_MULT ;
  static long _init ;
  public :
    class Initializer {
	// initializer versucht, die statischen Objekte der Speicherverwaltung
	// vor allem anderen zu initialisieren, damit keine Fehler auftreten,
	// falls statische Objekte irgendwo Instanzen mit MyAlloc als Basis-
	// klasse angelegen.
      public :
        Initializer () ;
       ~Initializer () ;
    } ;
    class OutOfMemoryException { };
    friend class Initializer;
  protected :
    MyAlloc () {}
   ~MyAlloc () {}
  public :
    void * operator new (size_t) throw (OutOfMemoryException) ;
    void operator delete (void *,size_t) ;
} ;

static MyAlloc :: Initializer allocatorInitializer ;

#endif // MYALLOC_H_INCLUDED

