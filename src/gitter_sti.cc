//   Version f"ur DUNE
//
  // (c) bernhard schupp 1997 - 1998

  // $Source$
  // $Revision$
  // $Name$
  // $State$

/* $Id$
 * $Log$
 * Revision 1.12  2005/03/18 19:53:17  robertk
 * added backup and restore for XDRStream, dosent work yet.
 *
 * Revision 1.11  2005/01/13 16:49:37  robertk
 * moved goDownHlement to gitter_dune_impl.cc
 *
 * Revision 1.10  2004/12/10 12:19:45  robertk
 * backupIndices removed from gitter_sti and moved to gitter_dune_impl.cc
 *
 * Revision 1.9  2004/11/29 18:00:45  robertk
 * implemented the hole regeneration after restoreing the grid.
 *
 * Revision 1.8  2004/11/29 13:03:51  robertk
 * number of indexmanagers variable (enum ) now.
 *
 * Revision 1.7  2004/11/29 12:39:17  robertk
 * size of index set is stored aswell. The indexset is completely overwritten
 * after the grid is read from file.
 *
 * Revision 1.6  2004/11/25 18:44:17  robertk
 * added vertex backup and restore. not tested yet.
 *
 * Revision 1.5  2004/11/02 17:12:04  robertk
 * Removed duneBackup and duneRestore, now defined in the inherited
 * GitterDuneImpl class.
 *
 * Revision 1.4  2004/10/28 15:45:17  robertk
 * minor changes.
 *
 * Revision 1.3  2004/10/27 15:07:32  robertk
 * at the end of gitter backup and restore index of elements is backed and
 * restored.
 *
 * Revision 1.2  2004/10/25 16:38:11  robertk
 * All header end with .h now. Like the original.
 *
 * In the .cc this changes are done.
 *
 * Revision 1.1  2004/10/15 09:48:37  robertk
 * Inititial version. Some extenxions for Dune made. Schould be compatible
 * with all other applications done so far.
 *
 * Revision 1.4  2002/04/19 15:36:07  wesenber
 * modifications required for IBM VisualAge C++ Version 5.0
 *
 * Revision 1.3  2001/12/10 13:33:45  wesenber
 * parameter ``filePath'' for backup(), backupCMode(), and restore() added
 *
 ***/
#ifndef GITTER_STI_CC_INCLUDED
#define GITTER_STI_CC_INCLUDED


#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
  #include <fstream>
#else
  #include <iostream.h>
  #include <fstream.h>
#endif

#include "lock.h"
#include "gitter_sti.h"
#include "walk.h"

#include "xdrclass.h"

static volatile char RCSId_gitter_sti_cc [] = "$Id$" ;

extern "C" { double drand48 (void) ; }

#ifndef NDEBUG
Refcount :: Globalcount Refcount :: _g ;

Refcount :: Globalcount :: ~Globalcount () {
  assert (_c ? (cerr << "**WARNUNG Refcount :: Globalcount :: ~Globalcount() " << _c 
           << " objekte sind stehen geblieben" << endl, 1) : 1) ;
  return ;
}
#endif

typedef Wrapper < AccessIterator < Gitter :: vertex_STI > :: Handle, 
  Gitter :: InternalVertex >              leaf_vertex__macro_vertex__iterator ;

typedef Insert < AccessIterator < Gitter :: hedge_STI > :: Handle, 
  TreeIterator < Gitter :: hedge_STI, is_leaf < Gitter :: hedge_STI > > >   leaf_edge__macro_edge__iterator ;

typedef Insert < AccessIterator < Gitter :: hface_STI > :: Handle, 
  TreeIterator < Gitter :: hface_STI, is_leaf < Gitter :: hface_STI > > >   leaf_face__macro_face__iterator ;

typedef Insert < AccessIterator < Gitter :: hbndseg_STI > :: Handle, 
  TreeIterator < Gitter :: hbndseg_STI, is_leaf < Gitter :: hbndseg_STI> > >  leaf_bnd__macro_bnd__iterator ;

typedef Insert < AccessIterator < Gitter :: helement_STI > :: Handle, 
  TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > leaf_element__macro_element__iterator ;

IteratorSTI < Gitter :: vertex_STI > * Gitter :: iterator (const Gitter :: vertex_STI *) {
  vector < IteratorSTI < vertex_STI > * > _iterators ;
  {
    _iterators.push_back ( new AccessIterator < vertex_STI > :: Handle (container ())) ;
  }
  Insert < AccessIterator < hedge_STI > :: Handle, 
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > dw (container ()) ;
  _iterators.push_back ( new Wrapper < Insert < AccessIterator < hedge_STI > :: Handle, 
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (dw)) ;
  {
    Insert < AccessIterator < hface_STI > :: Handle,
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > > fw (container ()) ;
    _iterators.push_back ( new Wrapper < Insert < AccessIterator < hface_STI > :: Handle,
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (fw)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_vertex < helement_STI > > > ew (container ()) ;
    _iterators.push_back ( new Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_vertex < helement_STI > > >, InternalVertex > (ew)) ;
  }
  {
    Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > > fw (container ()) ;
    Wrapper < Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > df (fw) ;
    Insert < Wrapper < Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > > dif (df) ;
    _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > >, InternalVertex > (dif)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > > ew (container ()) ;
    Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge > de (ew) ;
    Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > > die (de) ;
    _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > >, InternalVertex > (die)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > > ew (container ()) ;
    Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fe (ew) ;
    Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >, 
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > > fie (fe) ;
    _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (fie)) ;
  }
  {
    Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > > ew (container ()) ;
    Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fe (ew) ;
    Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > > fie (fe) ;
    Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > dfie (fie) ;
    Insert < Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > difie (dfie) ;
    _iterators.push_back (new Wrapper < Insert < Wrapper < Insert < Wrapper < 
  Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (difie)) ;
  }
  return new VectorAlign < vertex_STI > (_iterators) ;
}

IteratorSTI < Gitter :: hedge_STI > * Gitter :: iterator (const hedge_STI *) {
  vector < IteratorSTI < hedge_STI > * > _iterators ; 
  _iterators.push_back ( new leaf_edge__macro_edge__iterator (container ())) ;
  Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > > nf (container ()) ;
  Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_edge < helement_STI > > > ne (container ()) ;
  Wrapper < Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > ef (nf) ;
  Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge > ee (ne) ;
  _iterators.push_back ( new  Insert < Wrapper < Insert < AccessIterator < hface_STI > :: Handle, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, is_leaf < hedge_STI > > > (ef)) ;
  _iterators.push_back ( new Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, is_leaf < hedge_STI > > > (ee)) ;
  Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > > nef (container ()) ;
  Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fnef (nef) ;
  Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > > fie (fnef) ;
  Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > efie (fie) ;
  _iterators.push_back (new Insert < Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle, 
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >, 
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >, 
  TreeIterator < hedge_STI, is_leaf < hedge_STI > > > (efie)) ;
  return new VectorAlign < hedge_STI > (_iterators) ;
}

IteratorSTI < Gitter :: hface_STI > * Gitter :: iterator (const hface_STI *) {
  leaf_face__macro_face__iterator w1 (container ()) ;
  Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_face < helement_STI > > > nw (container ()) ;
  Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > ww (nw) ;
  Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, is_leaf < hface_STI > > > www (nw) ;
  return new AlignIterator < leaf_face__macro_face__iterator, 
  Insert < Wrapper < Insert < AccessIterator < helement_STI > :: Handle,
  TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
  TreeIterator < hface_STI, is_leaf < hface_STI > > >, hface_STI > (w1, www) ;
}

IteratorSTI < Gitter :: hbndseg_STI > * Gitter :: iterator (const hbndseg_STI *) {
  return new leaf_bnd__macro_bnd__iterator (container ()) ;
}

IteratorSTI < Gitter :: helement_STI > * Gitter :: iterator (const helement_STI *) {
  return new leaf_element__macro_element__iterator (container ()) ;
}

IteratorSTI < Gitter :: helement_STI > * Gitter :: iterator (const IteratorSTI < helement_STI > * w) {
  return new leaf_element__macro_element__iterator (*(const leaf_element__macro_element__iterator *) w) ;
}

void Gitter :: fullIntegrityCheck () {
  const int start = clock() ;
  int count = 0 ;
  leaf_element__macro_element__iterator w (container ()) ;
  for(w.first () ; !w.done () ; w.next ())
    w.item ().test () ? (cerr << "-> fehler gefunden am element : " << count << "\n" << endl, count++) : (count ++) ;
  if (debugOption (3)) {
    float used = (float)(clock () - start)/(float)(CLOCKS_PER_SEC) ;
    cout << "**INFO Gitter :: fullIntegrityCheck () used : " << used << " sec." << endl ;
  }
  return ;
}

void Gitter :: printsize () {
  cout << "\n Gitter :: printSize () : \n\n" ;
  if (debugOption (10)) {
    { cout << " - Makroelemente .... "  << AccessIterator < helement_STI > :: Handle (container ()).size() << "\n" ; }
    { cout << " - Makror\"ander ..... " << AccessIterator < hbndseg_STI > :: Handle (container ()).size() << "\n" ; }
    { cout << " - Makrofl\"achen .... " << AccessIterator < hface_STI > :: Handle (container ()).size() << "\n" ; }
    { cout << " - Makrokanten ...... "  << AccessIterator < hedge_STI > :: Handle (container ()).size() << "\n" ; }
    { cout << " - Makrovertices .... "  << AccessIterator < vertex_STI > :: Handle (container ()).size() << "\n" ; }
    cout << "\n" ;
  }
  { cout << " - Elemente ......... "  << LeafIterator < helement_STI > (*this)->size() << "\n" ;}
  { cout << " - R\"ander .......... " << LeafIterator < hbndseg_STI > (*this)->size() << "\n" ;}
  { cout << " - Fl\"achen ......... " << LeafIterator < hface_STI > (*this)->size() << "\n" ;}
  { cout << " - Kanten ........... "  << LeafIterator < hedge_STI > (*this)->size() << "\n" ;}
  { cout << " - Vertices ......... "  << LeafIterator < vertex_STI > (*this)->size() << "\n" ;}
  cout << endl ;
  return ;
}

bool Gitter :: refine () {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: refine ()" << endl, 1) : 1) ;
  bool x = true ;
  {
    leaf_element__macro_element__iterator i (container ()) ;
    for( i.first(); ! i.done() ; i.next()) x &= i.item ().refine () ;
  }
  return x ;
}

void Gitter :: coarse() {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: coarse ()" << endl, 1) : 1) ;
  {AccessIterator < helement_STI > :: Handle i (container ()) ;
    for( i.first(); ! i.done() ; i.next()) i.item ().coarse () ; }
  return ;
}

bool Gitter :: adapt () {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: adapt ()" << endl, 1) : 1) ;
  assert (! iterators_attached ()) ;
  const int start = clock () ;

  bool refined = refine ();
  if (!refined) {
    cerr << "**WARNUNG (IGNORIERT) Verfeinerung nicht vollst\"andig (warum auch immer)\n" ;
    cerr << "  diese Option ist eigentlich dem parallelen Verfeinerer vorbehalten.\n" ;
    cerr << "  Der Fehler trat auf in " << __FILE__ << " " << __LINE__ << endl ;
  }
  int lap = clock () ;
  coarse () ;
  int end = clock () ;
  if (debugOption (1)) {
    float u1 = (float)(lap - start)/(float)(CLOCKS_PER_SEC) ;
    float u2 = (float)(end - lap)/(float)(CLOCKS_PER_SEC) ;
    float u3 = (float)(end - start)/(float)(CLOCKS_PER_SEC) ;
    cout << "**INFO Gitter :: adapt () [ref|cse|all] " << u1 << " " << u2 << " " << u3 << endl ;
  }
  return refined;
}

void Gitter :: backupCMode (ostream & out) {

  // das Kompatibilit"ats - Backup f"ur backups im alten Modus.

  int i = 0 ;
  while(1) {
    Insert < AccessIterator < helement_STI > :: Handle, 
        TreeIterator < helement_STI, any_has_level < helement_STI > > > w (container (),i ++) ;
    if (! w.size()) break ;
    for(w.first() ; ! w.done() ; w.next()) w.item ().backupCMode (out) ;
    out << endl ;
  }
  return ;
}

void Gitter :: backup (ostream & out) {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: backup (ostream & = " << out << ") " << endl, 1) : 1) ;
  {AccessIterator <hedge_STI> :: Handle fw (container ()) ;
    for (fw.first(); !fw.done(); fw.next()) fw.item ().backup (out) ; }
  {AccessIterator <hface_STI>::Handle fw (container ()) ;
    for (fw.first () ; ! fw.done () ; fw.next ()) fw.item().backup(out) ; }
  {AccessIterator <helement_STI> :: Handle ew (container ()) ;
    for (ew.first () ; ! ew.done () ; ew.next ()) ew.item ().backup (out) ; }
    
  return ;
}

void Gitter :: backup (XDRstream_out & out) {
/*
  //assert (debugOption (20) ? (cout << "**INFO Gitter :: backup (ostream & = " << out << ") " << endl, 1) : 1) ;
  {AccessIterator <hedge_STI> :: Handle fw (container ()) ;
    for (fw.first(); !fw.done(); fw.next()) fw.item ().backup (out) ; }
  {AccessIterator <hface_STI>::Handle fw (container ()) ;
    for (fw.first () ; ! fw.done () ; fw.next ()) fw.item().backup(out) ; }
  {AccessIterator <helement_STI> :: Handle ew (container ()) ;
    for (ew.first () ; ! ew.done () ; ew.next ()) ew.item ().backup (out) ; }
*/
  return ;
}

void Gitter ::restore (istream & in) {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: restore (istream & = " << in << ") " << endl, 1) : 1) ;  
  {AccessIterator < hedge_STI > :: Handle ew (container ());
    for (ew.first () ; !ew.done () ; ew.next ()) ew.item ().restore (in) ; }
  {AccessIterator < hface_STI >:: Handle fw(container());
    for ( fw.first(); !fw.done (); fw.next()) fw.item().restore (in); }
  {AccessIterator < helement_STI >:: Handle ew(container());
    for ( ew.first(); !ew.done(); ew.next()) ew.item().restore (in); }
  {AccessIterator < hbndseg_STI > :: Handle bw (container ()) ;
    for (bw.first () ; ! bw.done () ; bw.next ()) bw.item ().restoreFollowFace () ; }
  notifyGridChanges () ;
  return ;
}

void Gitter ::restore (XDRstream_in & in) {
  //assert (debugOption (20) ? (cout << "**INFO Gitter :: restore (istream & = " << in << ") " << endl, 1) : 1) ;  
/*
  {AccessIterator < hedge_STI > :: Handle ew (container ());
    for (ew.first () ; !ew.done () ; ew.next ()) ew.item ().restore (in) ; }
  {AccessIterator < hface_STI >:: Handle fw(container());
    for ( fw.first(); !fw.done (); fw.next()) fw.item().restore (in); }
  {AccessIterator < helement_STI >:: Handle ew(container());
    for ( ew.first(); !ew.done(); ew.next()) ew.item().restore (in); }
  {AccessIterator < hbndseg_STI > :: Handle bw (container ()) ;
    for (bw.first () ; ! bw.done () ; bw.next ()) bw.item ().restoreFollowFace () ; }
  notifyGridChanges () ;
*/
  return ;
}

void Gitter :: backup (const char * filePath, const char * fileName) {

  // Die Konstruktion des Backup und Restore ist folgendermassen gedacht:
  // Jede "uberschreibende Methode mit Signatur (const char *) modifiziert
  // lediglich den Dateinamen und delegiert dann an Gitter :: backup / restore.
  // Gitter :: backup erzeugt dann event. Lockfiles usw. und in jedem Fall die
  // Datenstr"ome, mit denen dann die Methoden mit der Signatur (.stream &)
  // aufgerufen werden. Diese wiederum sollen polymorph reimplementiert
  // werden, falls ein erweitertes backup / restore erforderlich ist. Dabei
  // mu"s immer als erstes die backup / restore Methode der direkten Basisklasse
  // aufgerufen werden. Auf diese Art kommt es zu einem strukturierten Konzept
  // f"ur die Sicherung und Wiederherstellung, bei dem auch die objektorientierte
  // Struktur der Gitterklassen ber"ucksichtigt wird.
  

  assert (debugOption (20) ? (cout << "**INFO Gitter :: backup (const char * = \""
                 << filePath << ", const char * =\""
                       << fileName << "\") " << endl, 1) : 1) ;

  char *fullName = new char[strlen(filePath)+strlen(fileName)+1];
  sprintf(fullName,"%s%s",filePath,fileName);

  ofstream out (fullName) ;
  if (!out) {
    cerr << "**WARNUNG (IGNORIERT) Gitter :: backup (const char *, const char *) Fehler beim Anlegen von < " 
         << (fullName ? fullName : "null") << " >" << endl ;
  } else {
    FSLock lock (fullName) ;
    backup (out) ;
    container ().backup (filePath, fileName) ;
  }

  delete [] fullName;

  return ;
}


void Gitter :: backupCMode (const char * filePath, const char * fileName) {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: backupCMode (const char * = \""
                                   << filePath << ", const char * = \""
                 << fileName << "\") " << endl, 1) : 1) ;  

  char *fullName = new char[strlen(filePath)+strlen(fileName)+1];
  sprintf(fullName,"%s%s",filePath,fileName);

  ofstream out (fullName) ;
  if (!out) {
    cerr << "**WARNUNG (IGNORIERT) Gitter :: backupCMode (const char *) Fehler beim Anlegen von < " 
         << (fullName ? fullName : "null") << " >" << endl ;
  } else {
    backupCMode (out) ;
    container ().backupCMode (filePath, fileName) ;
  }

  delete [] fullName;

  return ;
}

void Gitter :: restore (const char * filePath, const char * fileName) {

  // Erkl"arung siehe Gitter :: backup (const char *, const char *) in gitter_sti.cc ;

  assert (debugOption (20) ? (cout << "**INFO Gitter :: restore (const char * = \""
                                   << filePath << ", const char * = \""
                 << fileName << "\") " << endl, 1) : 1) ;

  char *fullName = new char[strlen(filePath)+strlen(fileName)+1];
  sprintf(fullName,"%s%s",filePath,fileName);

  ifstream in (fullName) ;
  if (!in) {
    cerr << "**WARNUNG (IGNORIERT) Gitter :: restore (const char *, const char*) Fehler beim \"Offnen von < " 
         << (fullName ? fullName : "null") << " > " << endl ;
  } else {
    restore (in) ;
  }

  delete [] fullName;

  return ;
}

void Gitter :: refineGlobal () {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: refineGlobal () " << endl, 1) : 1) ;
  const int start = clock () ;
  {leaf_element__macro_element__iterator w (container ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item (). tagForGlobalRefinement () ; }
  adapt () ;
  if (debugOption (2))
    cout << "  Gitter :: refineGlobal () used " 
         << (double)(clock () - start)/(double)(CLOCKS_PER_SEC) << " sec." << endl ;
  return ;
}

void Gitter :: refineRandom (double p) {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: refineRandom (double = " << p << ") " << endl, 1) : 1) ;
  const int start = clock () ;
  if (p < .0 || p > 1.) {
    cerr << "**WARNUNG (IGNORIERT) Gitter :: refineRandom (double = " << p << ") Argument muss zwischen 0 und 1 liegen" << endl ;
  } else {
    {
       leaf_element__macro_element__iterator w (container ()) ;
       for (w.first () ; ! w.done () ; w.next ()) 
         drand48 () < p ? (w.item (). tagForGlobalRefinement (), 0) : 0 ;
    }
    adapt () ;
    if (debugOption (2))
      cout << "**INFO Gitter :: refineRandom () used " 
           << (double)(clock () - start)/(double)(CLOCKS_PER_SEC) << " sec." << endl ;
  }
  return ;
}

void Gitter :: refineBall (const double (&center)[3], double radius, int limit) {
  if (radius < .0) {
    cerr << "**WARNUNG (IGNORIERT) Gitter :: refineBall (center = ?, radius = " 
   << radius << ") Radius darf nicht negativ sein" << endl ;
  } else {
    const int start = clock () ;
    {
       leaf_element__macro_element__iterator w (container ()) ;
       for (w.first () ; ! w.done () ; w.next ())
         w.item (). tagForBallRefinement (center,radius,limit) ;
    }
    adapt () ;
    if (debugOption (2))
      cout << "**INFO Gitter :: refineBall () used " 
           << (double)(clock () - start)/(double)(CLOCKS_PER_SEC) << " sec." << endl ;
  }
  return ;
}

void Gitter :: notifyGridChanges () {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: notifyGridChanges () " << endl, 1) : 1) ;
  return ;
}

void Gitter :: notifyMacroGridChanges () {
  assert (debugOption (20) ? (cout << "**INFO Gitter :: notifyMacroGridChanges () " << endl, 1) : 1) ;
  return ;
}

Gitter :: ~Gitter () {
  if (ref)
    cerr << "**WARNUNG (IGNORIERT) Gitter-Referenzz\"ahler beim L\"oschen [" << ref << "]" << endl ;
  return ;
}

int Gitter :: Makrogitter :: iterators_attached () const {
  return AccessIterator < vertex_STI > :: ref + AccessIterator < hedge_STI > :: ref + 
   AccessIterator < hface_STI > :: ref + AccessIterator < helement_STI > :: ref +
   AccessIterator < hbndseg_STI > :: ref ;
}

Gitter :: Makrogitter :: ~Makrogitter () {
  if (iterators_attached())
    cerr << "**WARNUNG (IGNORIERT) in " << __FILE__ << " " << __LINE__ << endl ;
  return ;
}

#endif
