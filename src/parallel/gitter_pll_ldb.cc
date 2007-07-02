// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 
#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <time.h>
#include <assert.h>
#include <math.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
  #include <iomanip>
  #include <utility>
  #include <map>
  #include <functional>
  #include <algorithm>
  #include <numeric>
#else
  #include <iostream.h>
  #include <iomanip.h>
  #include <pair.h>
  #include <map.h>
  #include <function.h>
  #include <algo.h>
#endif

#ifdef _DUNE_NOT_USES_ALU3DGRID_
#include "party_lib.h"
#include "metis.h"
#endif
  
#include "gitter_pll_ldb.h" 

void LoadBalancer :: DataBase :: edgeUpdate (const GraphEdge & e) 
{
  if (e.isValid ()) 
  {
    //_edgeSet.find (e) != _edgeSet.end() ? (_edgeSet.erase (_edgeSet.find (e)), _edgeSet.insert (e), 0) : (_edgeSet.insert (e), 0) ;
    ldb_edge_set_t :: iterator it =  _edgeSet.find (e);
    if ( it != _edgeSet.end() )
    {
      _edgeSet.erase (it);
    }
    
    _edgeSet.insert (e);
  }
  return ;
}

void LoadBalancer :: DataBase :: vertexUpdate (const GraphVertex & v) 
{
  assert (v.isValid ()) ;
  _maxVertexLoad = _maxVertexLoad < v.weight () ? v.weight () : _maxVertexLoad ;
  _vertexSet.find (v) != _vertexSet.end () ? 
    (_vertexSet.erase (v), _vertexSet [v] = -1) : _vertexSet [v] = -1 ;
  return ;
}

int LoadBalancer :: DataBase :: accVertexLoad () const {
  return accumulate (_vertexSet.begin (), _vertexSet.end (), 0, AccVertexLoad ()) ;
}

int LoadBalancer :: DataBase :: accEdgeLoad () const {
  return accumulate (_edgeSet.begin (), _edgeSet.end (), 0, AccEdgeLoad ()) ;
}

void LoadBalancer :: DataBase :: printLoad () const {
  cout << "**INFO LoadBalancer :: DataBase :: printLoad () [elt(max)|fce] " 
       << accVertexLoad () << " " << maxVertexLoad () << " " << accEdgeLoad () << endl ;
  return ;
} 

void LoadBalancer :: DataBase :: graphCollect (const MpAccessGlobal & mpa, 
  insert_iterator < ldb_vertex_map_t > nodes, insert_iterator < ldb_edge_set_t > edges) const {
  
  const int np = mpa.psize () ;
  ObjectStream os ;

  {
    int len = _vertexSet.size () ;
    os.writeObject (len) ;
    {
      ldb_vertex_map_t :: const_iterator iEnd = _vertexSet.end () ;
      for (ldb_vertex_map_t :: const_iterator i = _vertexSet.begin () ; 
        i != iEnd; os.writeObject ((*i++).first)) ;
    }
    
    len = _edgeSet.size () ;
    os.writeObject (len) ;
    {
      ldb_edge_set_t :: const_iterator iEnd = _edgeSet.end () ;
      for (ldb_edge_set_t :: const_iterator i = _edgeSet.begin () ; 
        i != iEnd; os.writeObject (*i++)) ;
    }
  }

  try 
  {
    // exchange data 
    vector < ObjectStream > osv = mpa.gcollect (os) ;

    // free memory 
    os.reset ();

    {
      for (int i = 0 ; i < np ; i ++) 
      {
        int len ;
        osv [i].readObject (len) ;
        assert (len >= 0) ;
        {
          for (int j = 0 ; j < len ; ++j) 
          {
            GraphVertex x ;
            osv [i].readObject (x) ;
            * nodes ++ = pair < const GraphVertex, int > (x,i) ;
          } 
        }
        osv [i].readObject (len) ;
        assert (len >= 0) ;
        {
          for (int j = 0 ; j < len ; ++j) 
          {
            GraphEdge x ;
            osv [i].readObject (x) ;
            * edges ++ = x ;
            * edges ++ = - x ;
          }
        }

        // free memory of osv[i]
        osv[i].reset();
      }
    }
  } 
  catch (ObjectStream :: EOFException) 
  {
    cerr << "**FEHLER (FATAL) EOF gelesen in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  } 
  catch (ObjectStream :: OutOfMemoryException) 
  {
    cerr << "**FEHLER (FATAL) Out Of Memory in " << __FILE__ << " " << __LINE__ << endl ;
    abort () ;
  }
  return ;
}

static void optimizeCoverage (const int nparts, const int len, const int * const reference, const float * const weight, int * const proposal, const int verbose) {

  // 'reference' ist das Referenzarray, das mit dem 'proposal'
  // Vorschlagsvektor optimal abgeglichen werden soll, indem
  // auf 'proposal' eine Indexpermutationangewendet wird.
  // cov ist das 'coverage' Array, das die "Uberdeckung von
  // alter und neuer Teilgebietszuordnung beschreiben soll.

  vector < vector < int > > cov (nparts, vector < int > (nparts, 0L)) ;

  { 
    for (int k = 0 ; k < len ; k ++) cov [reference [k]][proposal[k]] += 1 + int (sqrt(weight [k])) ; 
  }
  
  map < int, pair < int, int >, greater_equal < int > > max ;
  set < int, less < int > > freeIndex ;
  
  {
    for (int i = 0 ; i < nparts ; ++i ) 
    {
      freeIndex.insert (i) ;
      vector < int > :: iterator covBegin = cov [i].begin ();
      vector < int > :: const_iterator pos = max_element (covBegin, cov [i].end ()) ;
      int distance = (pos - covBegin);
      pair<int, int> val (i,distance);
      max [*pos] = val; 
    } 
  }

  vector < int > renumber (nparts, -1L) ;
  {
    for (map < int, pair < int, int >, greater_equal < int > > :: const_iterator i = max.begin () ; 
         i != max.end () ; i ++ ) 
    {
      if (renumber [(*i).second.second] == -1) 
      {
        int neue = (*i).second.first ;
        if (freeIndex.find (neue) != freeIndex.end ()) 
        {
          renumber [(*i).second.second] = neue ;
          freeIndex.erase (neue) ;
        }
      }
    } 
  }
  
  for (int j = 0; j != nparts ; ++j) 
  {
    if (renumber [j] == -1) 
    {
      if (freeIndex.find (j) != freeIndex.end ()) 
      {
        renumber [j] = j ;
        freeIndex.erase (j) ;
      } 
      else 
      {
        renumber [j] = * freeIndex.begin () ;
        freeIndex.erase (freeIndex.begin ()) ;
      }
    }
  }

  if (verbose) 
  {
    cout << "**INFO optimizeCoverage (): " << endl ;
    for (int i = 0 ; i < nparts ; i ++) 
    {
      for (int j = 0 ; j < nparts ; j ++)
        cout << "  " << setw (4) << cov [i][j] << " " ;

      cout << "| " << i << " -> " << renumber [i] << endl ;
    }
  }
  { 
    for (int i = 0 ; i < len ; i ++ ) proposal [i] = renumber [proposal [i]] ; 
  }

  freeIndex.clear();
  return ;
}

static bool collectInsulatedNodes (const int nel, const float * const vertex_w, const int * const edge_p, const int * const edge, const int * const edge_w, const int np, int * neu) {

  // 'collectInsulatedNodes (.)' ist eine Behelfsl"osung, damit der MHD Code
  // mit seinen periodischen Randelementen nicht zu Bruch geht. Da es sich
  // bei den periodischen Randelementen nur um Adapter ohne eigenen Daten-
  // inhalt handelt, d"urfen diese niemals isoliert von ihren Nachbarelementen
  // in ein Teilgebiet zugewiesen werden. Diese Prozedur sammelt grunds"atzlich
  // alle isolierten Elemente auf eine einigermassen vern"unftige Art zusammen,
  // weil sich auf dem Niveau des Partitionierers 'echte' Elemente und die
  // periodischen Adapter nur indirekt, d.h. durch die Anzahl der abgehenden
  // Kanten, unterscheiden lassen (das ist aber ein zu schwaches Kriterium).

#ifndef NDEBUG
  const int ned = edge_p [nel] ;
#endif
  assert (edge_p [0] == 0) ;
  bool change = false ;
  for (int i = 0 ; i < nel ; i++ ) {
    int j = 0, max = 0 ;
    for (j = max = edge_p [i]; j < edge_p [i+1] ; j++ ) {
      assert (j < ned) ;
      if (neu [i] == neu [edge [j]]) break ;
      else max = edge_w [j] > edge_w [max] ? j : max ;
    }
    if (j == edge_p [i+1]) {
      if (edge_p [i] == edge_p [i+1]) {
        cerr << "**WARNUNG (FEHLER IGNORIERT) Vollst\"andig isolierter Knoten im Grobgittergraph." ;
        cerr << " In Datei: " << __FILE__ << " Zeile: " << __LINE__ << endl ;
      } else {
//        cerr << "!!! Pass mal auf, ich weise dem Knoten " << i << " jetzt das Gebiet " << neu[edge [max]] << " zu." << endl ;
        neu [i] = neu [edge [max]] ;    
        change = true ;
      }
    }
  }
  return change ;
}

bool LoadBalancer :: DataBase :: repartition (MpAccessGlobal & mpa, method mth) {
  const int start = clock (), np = mpa.psize (), me = mpa.myrank () ;
  bool change (false) ;
  
  if (debugOption (3)) printLoad () ;
  
  if (mth == NONE) return false ;
  
  // create maps for edges and vertices 
  ldb_edge_set_t   edges ;
  ldb_vertex_map_t nodes ; 

  graphCollect (mpa,
                insert_iterator < ldb_vertex_map_t > (nodes,nodes.begin ()),
                insert_iterator < ldb_edge_set_t > (edges,edges.begin ())
               ) ;
  
  // 'ned' ist die Anzahl der Kanten im Graphen, 'nel' die Anzahl der Knoten.
  // Der Container 'nodes' enth"alt alle Knoten des gesamten Grobittergraphen
  // durch Zusammenführen der einzelnen Container aus den Teilgrobgittern.
  // Der Container 'edges' enthält alle Kanten des gesamten Grobgittergraphen
  // doppelt, einmal mit jeder Orientierung (vorw"arts/r"uckw"arts). Diese Form
  // der Datenhaltung ist vorteilhaft, wenn die Eingangsdaten der Partitionierer
  // im CSR Format daraus erstellt werden m"ussen.
  
  const int ned = edges.size () ;
  const int nel = nodes.size () ;
  
  if (edges.size ()) 
  {
    if (!((*edges.rbegin ()).leftNode () < nel)) 
    {
      cerr << "**WARNUNG (FEHLER IGNORIERT) Die Indexmenge ist nicht volls\"andig\n" ;
      cerr << "  \"uberdeckt zur Neupartitionierung. In " << __FILE__ << " " << __LINE__ << endl ;
      return false ;
    }

    // allocate edge memory for graph partitioners 
    int    * const edge_p      = new int [nel + 1] ;
    int    * const edge        = new int [ned] ;
    int    * const edge_w      = new int [ned] ;

    assert ( edge_p && edge && edge_w ) ;
    
    {
      int * edge_pPos = edge_p ;
      int count = 0, index = -1 ;
      
      for (ldb_edge_set_t :: const_iterator i = edges.begin () ; 
           i != edges.end () ; i ++) 
      {
        if ((*i).leftNode () != index) 
        {
          assert ((*i).leftNode () < nel) ;
          * edge_pPos ++ = count ;
          index = (*i).leftNode () ;
        }
        assert ((*i).rightNode () < nel) ;
        edge [count] = (*i).rightNode () ;
        edge_w [count ++] = (*i).weight () ;
      }
      * edge_pPos = count ;
      assert (edge_p [0] == 0 && edge_p [nel] == ned) ;

      // free memory, not needed anymore 
      edges.clear();
    }
    
    // get vertex memory 
    float  * const vertex_w    = new float [nel] ;
    int    * const vertex_wInt = new int [nel] ;
    int    * part              = new int [nel] ;
    
    assert ( vertex_w && vertex_wInt && part) ;
    {
      vector < int > check (nel, 0L) ;
      for (ldb_vertex_map_t :: const_iterator i = nodes.begin () ; i != nodes.end () ; i ++ ) 
      {
        int j = (*i).first.index () ;
        assert (0 <= j && j < nel) ;
        assert (0 <= (*i).second && (*i).second < np) ;
        part [j] = (*i).second ;
        check [j] = 1 ;
        vertex_w [j] = vertex_wInt [j] = (*i).first.weight () ;
      }

      // free memory, not needed anymore
      nodes.clear();
      
      if (nel != accumulate (check.begin (), check.end (), 0)) 
      {
        cerr << "**WARNUNG (IGNORIERT) Keine Neupartitionierung wegen fehlgeschlagenem Konsistenzcheck." ;
        cerr << " In Datei: " << __FILE__ << " Zeile: " << __LINE__ << endl ;
        delete [] part ;
        delete [] vertex_wInt ;
        delete [] vertex_w ;
        delete [] edge_w ;
        delete [] edge ;
        delete [] edge_p ;
        return false ;
      }
    }

    if (np > 1) 
    {
      // Abfangen, falls nur ein teilgebiet gebildet werden soll,
      // sonst Speicherallocationsfehler in den Partitionierern,
      // zumindest bei PARTY 1.1.

      int * neu = new int [nel] ;
      assert (neu) ;
      copy (part, part + nel, neu) ;
      
      switch (mth) 
      {
        // Die Methode 'collect' sammelt alle Elemente in einer Partition ein.

        case COLLECT :
          fill (neu, neu + nel, 0L) ;
          break ;
          
        case PARTY_linear :
          global_linear (nel, vertex_w, np, 0.0, neu) ;
          break ;
          
        case PARTY_gain :
          global_gain (nel, vertex_w, edge_p, edge, edge_w, np, 0.0, neu) ;
          break ;
          
        case PARTY_farhat :
          global_farhat (nel, vertex_w, edge_p, edge, edge_w, np, 0.0, neu) ;
          break ;
          
        case PARTY_kernighanLin :
          // Die dreifache Anwendung der Helpful-Set bzw. Kenighan-Lin Heuristik
          // basiert auf Erfahrungswerten und liefert einigermassen ausiterierte
          // Partitionen.
        
          local_kl (nel, vertex_w, edge_p, edge, edge_w, np, 3.0, neu, 0) ;
          local_kl (nel, vertex_w, edge_p, edge, edge_w, np, 1.0, neu, 0) ;
          local_kl (nel, vertex_w, edge_p, edge, edge_w, np, 0.0, neu, 0) ;
          break ;
          
        case PARTY_helpfulSet :
                local_hs (nel, vertex_w, edge_p, edge, edge_w, np, 2.0, neu, 0) ;
          local_hs (nel, vertex_w, edge_p, edge, edge_w, np, 1.0, neu, 0) ;
          local_hs (nel, vertex_w, edge_p, edge, edge_w, np, 0.0, neu, 0) ;
          break ;

        case METIS_PartGraphKway :
          {
            int wgtflag = 3, numflag = 0, options = 0, edgecut, n = nel, npart = np ;
            :: METIS_PartGraphKway (&n, edge_p, edge, vertex_wInt, edge_w, 
                    & wgtflag, & numflag, & npart, & options, & edgecut, neu) ;
          }
          break ;
        case METIS_PartGraphRecursive :
          {
            int wgtflag = 3, numflag = 0, options = 0, edgecut, n = nel, npart = np ;
            :: METIS_PartGraphRecursive (&n, edge_p, edge, vertex_wInt, edge_w, 
                    & wgtflag, & numflag, & npart, & options, & edgecut, neu) ;
          }
          break ;

        default :
          cerr << "**WARNUNG (FEHLER IGNORIERT) Ung\"ultige Methode [" << mth << "] zur\n" ;
          cerr << "  Neupartitionierung angegeben. In " << __FILE__ << " " << __LINE__ << endl ;
            
          delete [] neu ;
          delete [] part ;
          delete [] vertex_wInt ;
          delete [] vertex_w ;
          delete [] edge_w ;
          delete [] edge ;
          delete [] edge_p ;
          return false ;
      }

      // collectInsulatedNodes () sucht alle isolierten Knoten im Graphen und klebt
      // diese einfach mit dem Nachbarknoten "uber die Kante mit dem gr"ossten Gewicht
      // zusammen.

      collectInsulatedNodes (nel, vertex_w, edge_p, edge, edge_w, np, neu) ;

      // optimizeCoverage () versucht, die Lastverschiebung durch Permutation der
      // Gebietszuordnung zu beschleunigen. Wenn die alte Aufteilung von der neuen
      // abweicht, dann wird 'change'auf 'true' gestetzt, damit der Lastverschieber
      // in Aktion tritt.

      optimizeCoverage (np, nel, part, vertex_w, neu, me == 0 ? debugOption (4) : 0) ;

      // Vergleichen, ob sich die Aufteilung des Gebiets "uberhaupt ver"andert hat.

      change = ! equal (neu, neu + nel, part) ;

      if (change) 
      {
        // Hier die neue Zuordnung auf den eigenen Lastvertex-Container schreiben.
        // Dadurch werden die Grobgitterelemente an das neue Teilgebiet zugewiesen. 

        for (ldb_vertex_map_t :: iterator i = _vertexSet.begin () ; i != _vertexSet.end () ; i ++)
          _connect.insert ((*i).second = neu [(*i).first.index ()]) ;
      }
      
      delete [] neu ;
    }

    delete [] part ;
    delete [] vertex_wInt ;
    delete [] vertex_w ;
    delete [] edge_w ;
    delete [] edge ;
    delete [] edge_p ;
  }


  if (debugOption (3) && ! me) {
    cout << "**INFO LoadBalancerPll :: DataBase :: repartition ()\n"
         << "       globalen Graphen mit " << (ned/2) << " Kanten und " << nel << " Knoten erzeugt\n"
         << "       und zerlegt mit \"" << methodToString (mth) << "\" in "
         << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec." << endl ;
  }
  return change ;
}

int LoadBalancer :: DataBase :: getDestination (int i) const {
  const double p [3] = {0.0,0.0,0.0} ;
  GraphVertex e (i,0,p) ;
  assert (_vertexSet.find (e) != _vertexSet.end ()) ;
  return (*_vertexSet.find (e)).second ;
}

set < int, less < int > > LoadBalancer :: DataBase :: scan () const {
  return _connect ;
}

const char * LoadBalancer :: DataBase :: methodToString (method m) {
  switch (m) {
    case NONE :
      return "keine dynamische Lastverteilung" ;
    case COLLECT :
      return "COLLECT" ;
    case PARTY_helpfulSet :
      return "PARTY_helpfulSet" ;
    case PARTY_kernighanLin :
      return "PARTY_kernighanLin" ;
    case PARTY_linear :
      return "PARTY_linear" ;
    case PARTY_gain :
      return "PARTY_gain" ;
    case PARTY_farhat :
      return "PARTY_farhat" ;
    case METIS_PartGraphKway :
      return "METIS_PartGraphKway" ;
    case METIS_PartGraphRecursive :
      return "METIS_PartGraphRecursive" ;
    default :
      return "unbekannte Methode" ;
  }
  return "" ;
}



