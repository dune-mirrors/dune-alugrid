	// (c) bernhard schupp 1997 - 1998

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
 * Revision 1.7  2002/04/26 12:54:31  dedner
 * Laufzeiterfassung
 *
 * Revision 1.6  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.5  2001/12/21 15:18:31  dedner
 * GROSSER UMBAU: Berechnen von Adaptindikator und divBint und auch Zeitschritt bei Flussberechnung
 *
 * Revision 1.4  2001/12/10 13:56:37  wesenber
 * RCS Log history and/or RCSId-variable added
 *
 ***/

#ifndef MPACCESS_H_INCLUDED
#define MPACCESS_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <assert.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
  #include <strstream>
  #include <vector>
  #include <functional>
  #include <set>
  #include <map>
#else
  #include <iostream.h>
  #include <strstream.h>
  #include <vector.h>
  #include <function.h>
  #include <set.h>
  #include <map.h>
#endif

#include "serialize.h"

static volatile char RCSId_mpAccess_h [] = "$Id$" ;

class MpAccessGlobal {
  public :
    inline virtual ~MpAccessGlobal () ;
    virtual int psize () const = 0 ;
    virtual int myrank () const = 0 ;
    virtual int barrier () const = 0 ;
    virtual int gmax (int) const = 0 ;
    virtual int gmin (int) const = 0 ;
    virtual int gsum (int) const = 0 ;
    virtual long gmax (long) const = 0 ;
    virtual long gmin (long) const = 0 ;
    virtual long gsum (long) const = 0 ;
    virtual double gmax (double) const = 0 ;
    virtual double gmin (double) const = 0 ;
    virtual double gsum (double) const = 0 ;
    virtual void gmax (double*,int,double*) const = 0 ;
    virtual void gmin (double*,int,double*) const = 0 ;
    virtual void gsum (double*,int,double*) const = 0 ;
    virtual pair<double,double> gmax (pair<double,double>) const = 0 ;
    virtual pair<double,double> gmin (pair<double,double>) const = 0 ;
    virtual pair<double,double> gsum (pair<double,double>) const = 0 ;
    virtual vector < int > gcollect (int) const = 0 ;
    virtual vector < double > gcollect (double) const = 0 ;
    virtual vector < vector < int > > gcollect (const vector < int > &) const = 0 ;
    virtual vector < vector < double > > gcollect (const vector < double > &) const = 0 ;
    virtual vector < ObjectStream > gcollect (const ObjectStream &) const = 0 ;
} ;

class MpAccessLocal : public MpAccessGlobal {
  map < int, int, less < int > > linkage ;
  public :
    inline virtual ~MpAccessLocal () ;
    void printLinkage (ostream &) const ;
    inline void removeLinkage () ;
    inline int nlinks () const ;
    inline int link (int) const ;
    vector < int > dest () const ;
    int insertRequestSymetric (set < int, less < int > >) ;
    virtual vector < vector < int > > exchange (const vector < vector < int > > &) const = 0 ;
    virtual vector < vector < double > > exchange (const vector < vector < double > > &) const = 0 ;
    virtual vector < vector < char > > exchange (const vector < vector < char > > &) const = 0 ;
    virtual vector < ObjectStream > exchange (const vector < ObjectStream > &) const = 0 ;
} ;


	//
	//    #    #    #  #          #    #    #  ######
	//    #    ##   #  #          #    ##   #  #
	//    #    # #  #  #          #    # #  #  #####
	//    #    #  # #  #          #    #  # #  #
	//    #    #   ##  #          #    #   ##  #
	//    #    #    #  ######     #    #    #  ######
	//

inline MpAccessGlobal :: ~MpAccessGlobal () {
}

inline MpAccessLocal :: ~MpAccessLocal () {
}

inline int MpAccessLocal :: link (int i) const {
  assert (linkage.end () != linkage.find (i)) ;
  return (* linkage.find (i)).second ;
}

inline int MpAccessLocal :: nlinks () const {
  return linkage.size () ;
}

inline void MpAccessLocal :: removeLinkage () {
  linkage.erase (linkage.begin (), linkage.end ()) ;
  return ;
}

#endif
