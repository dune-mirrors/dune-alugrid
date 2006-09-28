// Robert Kloefkorn (c) 2004 - 2005 
#ifndef GITTER_DUNE_IMPL_CC_INCLUDED
#define GITTER_DUNE_IMPL_CC_INCLUDED

#include "gitter_dune_impl.h"

IteratorSTI < Gitter :: helement_STI > * GitterDuneImpl :: 
leafIterator (const helement_STI *) 
{
  return new Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
  TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > (container ()) ;
}

IteratorSTI < Gitter :: helement_STI > * GitterDuneImpl :: 
leafIterator (const IteratorSTI < helement_STI > * p) 
{
  return new Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
  TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > 
  (*(const Insert < PureElementAccessIterator < Gitter :: helement_STI > :: Handle,
  TreeIterator < Gitter :: helement_STI, is_leaf < Gitter :: helement_STI> > > *) p) ;
}


void GitterDuneBasis :: backupIndices (ostream & out)
{
  // backup indices, our index type is hierarchic_index 
  unsigned char indices = hierarchic_index;
  out << indices;

  // store max indices 
  for(int i=0; i< numOfIndexManager ; i++)
    indexManager(i).backupIndexSet(out);

  { // backup index of elements 
    AccessIterator <helement_STI> :: Handle ew (container ()) ;
    for (ew.first () ; ! ew.done () ; ew.next ()) ew.item ().backupIndex (out) ;
  }
  {
    // backup index of vertices 
    LeafIterator < vertex_STI > w ( *this );
    for( w->first(); ! w->done() ; w->next () ) w->item().backupIndex(out);
  }

  return ;
}

// go down all children and check index 
inline void GitterDuneBasis :: 
goDownHelement( Gitter::helement_STI & el , vector<bool> & idxcheck)
{
  typedef Gitter :: helement_STI ElType;
  assert( (static_cast<size_t> (el.getIndex())) < idxcheck.size() );
  idxcheck[ el.getIndex() ] = false;
  for( ElType * ch = el.down() ; ch ; ch = ch->next())
    goDownHelement( *ch , idxcheck );

  return ;
}

void GitterDuneBasis ::restoreIndices (istream & in) 
{
  unsigned char indices = no_index;
  in >> indices;

  // set VERBOSE to 20 and you have the indices value printed 
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: restoreIndices: index flag = " << (int)indices << " file: "
                       << __FILE__ << " line: " << __LINE__ <<") " << endl, 1) : 1) ;
  
  // restore dune indices (see backUpIndices method)
  if(indices == hierarchic_index) 
  {
    for(int i=0; i< numOfIndexManager ; i++)
      this->indexManager(i).restoreIndexSet( in );

    // restore index of elements 
    {
      AccessIterator < helement_STI >:: Handle ew(container());
      for ( ew.first(); !ew.done(); ew.next()) ew.item().restoreIndex (in);
    }
    // restore index of vertices
    {
      LeafIterator < vertex_STI > w ( *this );
      for( w->first(); ! w->done() ; w->next () ) w->item().restoreIndex(in);
    }

    { // reconstruct holes 
      {
        enum { elements = 0 };
        // for elements 
        int idxsize = this->indexManager(elements).getMaxIndex();
        vector < bool > checkidx ( idxsize );
        for(int i=0; i<idxsize; i++) checkidx[i] = true;

        AccessIterator < helement_STI >:: Handle ew(container());
        for ( ew.first(); !ew.done(); ew.next())
        {
          goDownHelement( ew.item() , checkidx );
        }

        for(int i=0; i<idxsize; i++)
        {
          if(checkidx[i] == true)
            this->indexManager(elements).freeIndex(i);
        }
      }
      {
        enum { vertices = 3 };
        // for vertices 
        LeafIterator < vertex_STI > w ( *this );
        int idxsize = this->indexManager(vertices).getMaxIndex();

        vector < bool > checkidx ( idxsize );
        for(int i=0; i<idxsize; i++) checkidx[i] = true;
        for( w->first(); ! w->done() ; w->next () )
        {
          assert( (static_cast<size_t> (w->item().getIndex())) < checkidx.size() );
          checkidx[ w->item().getIndex() ] = false;
        }

        for(int i=0; i<idxsize; i++)
        {
          if(checkidx[i] == true)
            this->indexManager(vertices).freeIndex(i);
        }
      }
    }
    return ;
  }

  if(indices == leaf_index) // convert indices to leafindices 
  {
    int idx = 0;
    PureElementLeafIterator < helement_STI > ew(*this);
    for ( ew->first(); !ew->done(); ew->next()) 
    {
      ew->item().setIndex( idx );
      idx++;
    }
    this->indexManager(0).setMaxIndex ( idx );
    assert (debugOption (20) ? (cout << endl << "**INFO GitterDuneBasis :: restoreIndices: create new leaf indices with size = " << idx << " ! file: "<< __FILE__ << ", line: " << __LINE__ << endl, 1) : 1) ;
    // printsize ();
    return ;
  }

  
  cerr<< "**WARNING: GitterDuneBasis :: restoreIndices: indices (id = " << indices << ") not read! file: "<< __FILE__ << ", line: " << __LINE__ << "\n";
  return ;
}

// wird von Dune verwendet 
void GitterDuneBasis :: duneBackup (const char * fileName) 
{
  // diese Methode wird von der Dune Schnittstelle aufgerufen und ruft
  // intern lediglich backup (siehe oben) und backupCMode des Makrogitters
  // auf, allerdings wird hier der path und filename in einer variablen
  // uebergeben 

  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: duneBackup (const char * = \""
                       << fileName << "\") " << endl, 1) : 1) ;

  ofstream out (fileName) ;
  if (!out) {
    cerr << "**WARNUNG (IGNORIERT) GitterDuneBasis :: duneBackup (const char *, double) Fehler beim Anlegen von < "
         << (fileName ? fileName : "null") << " >" << endl ;
  }
  else
  {
    FSLock lock (fileName) ;
    Gitter :: backup (out) ;
    GitterDuneBasis :: backupIndices (out) ;

    {
      char *fullName = new char[strlen(fileName)+20];
      if(!fullName)
      {
        cerr << "**WARNUNG GitterDuneBasis :: duneBackup (, const char *, double) :";
        cerr << "couldn't allocate fullName! " << endl;
        abort();
      }
      sprintf(fullName,"%s.macro",fileName);
      ofstream macro (fullName) ;

      if(!macro)
      {
        cerr << "**WARNUNG (IGNORIERT) GitterDuneBasis :: duneBackup (const char *, const char *) Fehler beim Anlegen von < "
         << (fullName ? fullName : "null") << " >" << endl ;
      }
      else
      {
        container ().backupCMode (macro) ;
      }
      delete [] fullName;
    }
  }
  return ;
}

// wird von Dune verwendet 
void GitterDuneBasis :: duneRestore (const char * fileName)
{
  // diese Methode wird von der Dune Schnittstelle aufgerufen 
  // diese Methode ruft intern restore auf, hier wird lediglich 
  // der path und filename in einer variablen uebergeben

  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: duneRestore (const char * = \""
                 << fileName << "\") " << endl, 1) : 1) ;

  ifstream in (fileName) ;
  if (!in) {
    cerr << "**WARNUNG (IGNORIERT) GitterDuneBasis :: duneRestore (const char *, double & ) Fehler beim \"Offnen von < "
         << (fileName ? fileName : "null") << " > " << endl ;
  } else {
    Gitter :: restore (in) ;
    GitterDuneBasis :: restoreIndices (in);
  }
  return ;
}

int GitterDuneBasis :: preCoarsening  (Gitter::helement_STI & elem)
{
  // if _arp is set then the extrenal preCoarsening is called 
  if(_arp) return (*_arp).preCoarsening(elem);
  else return 0;
}

int GitterDuneBasis :: postRefinement (Gitter::helement_STI & elem)
{
  // if _arp is set then the extrenal postRefinement is called 
  if(_arp) return (*_arp).postRefinement(elem);
  else return 0;
}

int GitterDuneBasis :: preCoarsening  (Gitter::hbndseg_STI & bnd)
{
  // if _arp is set then the extrenal preCoarsening is called 
  if(_arp) return (*_arp).preCoarsening(bnd);
  else return 0;
}

int GitterDuneBasis :: postRefinement (Gitter::hbndseg_STI & bnd)
{
  // if _arp is set then the extrenal postRefinement is called 
  if(_arp) return (*_arp).postRefinement(bnd);
  else return 0;
}

void GitterDuneBasis ::
setAdaptRestrictProlongOp( AdaptRestrictProlongType & arp )
{
  if(_arp) 
  {
    cerr<< "WARNING: GitterDuneBasis :: setAdaptRestrictProlongOp: _arp not NULL! in:" <<  __FILE__ << " line="<<__LINE__ << endl; 
  }
  _arp = &arp;
}

void GitterDuneBasis :: removeAdaptRestrictProlongOp()
{
  _arp = 0;
}

bool GitterDuneBasis :: refine () {
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: refine ()" << endl, 1) : 1) ;
  bool x = true ;
  {
    leaf_element__macro_element__iterator i (container ()) ;
    for( i.first(); ! i.done() ; i.next()) 
    {
      x &= i.item ().refine () ;
    }
  }
  return x ;
}

bool GitterDuneBasis :: coarse() {
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: coarse ()" << endl, 1) : 1) ;
  bool x = true ;
  {
    AccessIterator < helement_STI > :: Handle i (container ()) ;
    for( i.first(); ! i.done() ; i.next()) 
    {
      x &= i.item ().coarse () ; 
    }
  }
  return x;
}

bool GitterDuneBasis :: duneAdapt (AdaptRestrictProlongType & arp) {
  assert (debugOption (20) ? (cout << "**INFO GitterDuneBasis :: duneAdapt ()" << endl, 1) : 1) ;
  assert (! iterators_attached ()) ;
  const int start = clock () ;

  setAdaptRestrictProlongOp(arp); 
  bool refined = this->refine ();
  if (!refined) {
    cerr << "**WARNUNG (IGNORIERT) Verfeinerung nicht vollst\"andig (warum auch immer)\n" ;
    cerr << "  diese Option ist eigentlich dem parallelen Verfeinerer vorbehalten.\n" ;
    cerr << "  Der Fehler trat auf in " << __FILE__ << " " << __LINE__ << endl ;
  }
  int lap = clock () ;
  bool coarsened = this->coarse () ;
  int end = clock () ;
  if (debugOption (1)) {
    float u1 = (float)(lap - start)/(float)(CLOCKS_PER_SEC) ;
    float u2 = (float)(end - lap)/(float)(CLOCKS_PER_SEC) ;
    float u3 = (float)(end - start)/(float)(CLOCKS_PER_SEC) ;
    cout << "**INFO GitterDuneBasis :: duneAdapt () [ref|cse|all] " << u1 << " " << u2 << " " << u3 << endl ;
  }
  removeAdaptRestrictProlongOp ();

  if( !refined && !coarsened ) return false;
  return true;
}






#endif
