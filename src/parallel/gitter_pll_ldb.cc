// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 

#include "gitter_pll_ldb.h" 

void LoadBalancer :: DataBase :: edgeUpdate (const GraphEdge & e) 
{
  if (e.isValid ()) 
  {
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

template <class idx_t>
void LoadBalancer :: DataBase :: 
graphCollect (const MpAccessGlobal & mpa, 
              insert_iterator < ldb_vertex_map_t > nodes, 
              insert_iterator < ldb_edge_set_t > edges,
              idx_t* vtxdist, const bool serialPartitioner ) const 
{
  const int allGatherMaxSize = ALUGridExternalParameters :: allGatherMaxSize();
  assert( allGatherMaxSize == mpa.gmax( allGatherMaxSize ) );

  // if the number of ranks is small, then use old allgather method 
  if( mpa.psize() < allGatherMaxSize )
  {
    // old method has O(p log p) time complexity 
    // and O(p) memory consumption which is critical 
    // for higher number of cores 
    graphCollectAllgather( mpa, nodes, edges, vtxdist, serialPartitioner );
  }
  else 
  {
    // otherwise use method with O(p log p) time complexity 
    // and O(1) memory consumption
    graphCollectBcast( mpa, nodes, edges, vtxdist, serialPartitioner );
  }
}

template <class idx_t>
void LoadBalancer :: DataBase :: 
graphCollectAllgather (const MpAccessGlobal & mpa, 
                       insert_iterator < ldb_vertex_map_t > nodes, 
                       insert_iterator < ldb_edge_set_t > edges,
                       idx_t* vtxdist, const bool serialPartitioner ) const 
{
  // for parallel partitioner return local vertices and edges 
  // for serial partitioner these have to be communicates to all
  // processes 
   
  // number of processes 
  const int np = mpa.psize();

  if( ! serialPartitioner || np == 1 )
  {
    const int myrank = mpa.myrank();
    {
      ldb_vertex_map_t :: const_iterator iEnd = _vertexSet.end () ;
      for (ldb_vertex_map_t :: const_iterator i = _vertexSet.begin () ; 
         i != iEnd; ++i ) 
      {
        {
          const GraphVertex& x = (*i).first;
          * nodes ++ = pair < const GraphVertex, int > ( x , myrank) ;
        } 
      }
    }

    {
      ldb_edge_set_t :: const_iterator eEnd = _edgeSet.end () ;
      for (ldb_edge_set_t :: const_iterator e = _edgeSet.begin () ; 
           e != eEnd; ++e) 
      {
        const GraphEdge& x = (*e) ;
        // edges exists twice ( u , v ) and ( v , u )
        // with both orientations 
        * edges ++ = x ;
        * edges ++ = - x ;
      }
    }

    // if vtxdist is given, we set it up here
    if( vtxdist ) 
    {
      // vtxdist always starts with 0 
      // so initialize here 
      vtxdist[ 0 ] = 0 ;
    }
  }

  // for serial calls we are done here 
  if( np == 1 ) return ;

  ObjectStream os ;

  {
    // write number of elements  
    int len = _vertexSet.size () ;
    os.writeObject (len) ;

    if( serialPartitioner )
    {
      // write vertices 
      ldb_vertex_map_t :: const_iterator iEnd = _vertexSet.end () ;
      for (ldb_vertex_map_t :: const_iterator i = _vertexSet.begin () ; i != iEnd; ++i ) 
      {
        os.writeObject ((*i).first) ;
      }

      // write number of edges 
      len = _edgeSet.size () ;
      os.writeObject (len) ;

      // write edges 
      ldb_edge_set_t :: const_iterator eEnd = _edgeSet.end () ;
      for (ldb_edge_set_t :: const_iterator e = _edgeSet.begin () ; e != eEnd; ++e )
      {
        os.writeObject (*e);
      }
    }
  }

  try 
  {

    // exchange data  
    vector < ObjectStream > osv = mpa.gcollect (os) ;

    // free memory 
    os.reset ();

    {
      for (int i = 0 ; i < np ; ++i) 
      {
        int len ;
        ObjectStream& osv_i = osv [i];

        osv_i.readObject (len) ;
        assert (len >= 0) ;

        // read graph for serial partitioner 
        if( serialPartitioner ) 
        {
          for (int j = 0 ; j < len ; ++j) 
          {
            GraphVertex x ;
            osv_i.readObject (x) ;
            * nodes ++ = pair < const GraphVertex, int > (x,i) ;
          } 

          osv_i.readObject (len) ;
          assert (len >= 0) ;

          for (int j = 0 ; j < len ; ++j) 
          {
            GraphEdge x ;
            osv_i.readObject (x) ;
            * edges ++ = x ;
            * edges ++ = - x ;
          }
        }
        else 
        {
          // see above vtxdist [ 0 ] = 0
          // sum up number of vertices for processor i 
          vtxdist[ i + 1 ] = vtxdist[ i ] + len ;
        }

        // free memory of osv[i]
        osv_i.reset();
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

template <class idx_t>
void LoadBalancer :: DataBase :: 
graphCollectBcast (const MpAccessGlobal & mpa, 
                   insert_iterator < ldb_vertex_map_t > nodes, 
                   insert_iterator < ldb_edge_set_t > edges,
                   idx_t* vtxdist, const bool serialPartitioner ) const 
{
  // for parallel partitioner return local vertices and edges 
  // for serial partitioner these have to be communicates to all
  // processes 
   
  // my rank number 
  const int me = mpa.myrank();

  // number of processes 
  const int np = mpa.psize();

  if( ! serialPartitioner || np == 1 )
  {
    {
      ldb_vertex_map_t :: const_iterator iEnd = _vertexSet.end () ;
      for (ldb_vertex_map_t :: const_iterator i = _vertexSet.begin () ; 
         i != iEnd; ++i ) 
      {
        {
          const GraphVertex& x = (*i).first;
          * nodes ++ = pair < const GraphVertex, int > ( x , me ) ;
        } 
      }
    }

    {
      ldb_edge_set_t :: const_iterator eEnd = _edgeSet.end () ;
      for (ldb_edge_set_t :: const_iterator e = _edgeSet.begin () ; 
           e != eEnd; ++e) 
      {
        const GraphEdge& x = (*e) ;
        // edges exists twice ( u , v ) and ( v , u )
        // with both orientations 
        * edges ++ = x ;
        * edges ++ = - x ;
      }
    }

    // if vtxdist is given, we set it up here
    if( vtxdist ) 
    {
      // vtxdist always starts with 0 
      // so initialize here 
      vtxdist[ 0 ] = 0 ;
    }
  }

  // for serial calls we are done here 
  if( np == 1 ) return ;

  // my data stream 
  ObjectStream os;
   
  {
    // write number of elements  
    const int vertexSize = _vertexSet.size () ;
    os.writeObject ( vertexSize ) ;

    if( serialPartitioner )
    {

      // write vertices 
      ldb_vertex_map_t :: const_iterator iEnd = _vertexSet.end () ;
      for (ldb_vertex_map_t :: const_iterator i = _vertexSet.begin () ; i != iEnd; ++i ) 
      {
        os.writeObject ((*i).first) ;
      }

      // write number of edges 
      const int edgeSize = _edgeSet.size () ;
      os.writeObject ( edgeSize ) ;

      // write edges 
      ldb_edge_set_t :: const_iterator eEnd = _edgeSet.end () ;
      for (ldb_edge_set_t :: const_iterator e = _edgeSet.begin () ; e != eEnd; ++e )
      {
        os.writeObject (*e);
      }
    }
  }

  try 
  {
    // get max buffer size (only for serial partitioner we need to communicate)
    const int maxSize = serialPartitioner ? mpa.gmax( os.size() ) : os.size();

    // create bcast buffer  
    ObjectStream sendrecv ;
    sendrecv.reserve( maxSize * sizeof(char) );

    for( int rank = 0; rank < np; ++ rank ) 
    {
      // reset read/write positions 
      sendrecv.clear();

      // write my data 
      if( rank == me ) 
      {
        // write my stream 
        sendrecv.writeStream( os );
        // clear data 
        os.reset();
      }

      // make sure size is still ok 
      assert( sendrecv.capacity() >= maxSize );

      // exchange data 
      mpa.bcast( sendrecv.getBuff(0), maxSize, rank );

      // insert data into graph map 
      {
        // reset read posistion 
        sendrecv.resetReadPosition();
        // adjust write count to max length to avoid eof errors 
        sendrecv.seekp( maxSize );

        int len ;
        sendrecv.readObject ( len ) ;
        assert (len >= 0) ;

        // read graph for serial partitioner 
        if( serialPartitioner ) 
        {
          for (int j = 0 ; j < len ; ++j) 
          {
            GraphVertex x ;
            sendrecv.readObject (x) ;
            * nodes ++ = pair < const GraphVertex, int > (x, rank) ;
          } 

          sendrecv.readObject (len) ;
          assert (len >= 0) ;

          for (int j = 0 ; j < len ; ++j) 
          {
            GraphEdge x ;
            sendrecv.readObject (x) ;
            * edges ++ = x ;
            * edges ++ = - x ;
          }
        }
        else 
        {
          // see above vtxdist [ 0 ] = 0
          // sum up number of vertices for processor rank 
          vtxdist[ rank + 1 ] = vtxdist[ rank ] + len ;
        }
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

template <class real_t, class idx_t>
static void optimizeCoverage (const int nparts, 
                              const int len, 
                              const idx_t* const reference, 
                              const real_t* const weight, 
                              idx_t* const proposal, 
                              const int verbose) 
{
  // 'reference' ist das Referenzarray, das mit dem 'proposal'
  // Vorschlagsvektor optimal abgeglichen werden soll, indem
  // auf 'proposal' eine Indexpermutationangewendet wird.
  // cov ist das 'coverage' Array, das die "Uberdeckung von
  // alter und neuer Teilgebietszuordnung beschreiben soll.

  map < int, pair < int, int >, greater_equal < int > > max ;
  set < int, less < int > > freeIndex ;
  
  {
    vector < vector < int > > cov (nparts, vector < int > (nparts, 0L)) ;

    for (int k = 0 ; k < len ; ++k) cov [reference [k]][proposal[k]] += 1 + int (sqrt(weight [k])) ; 
  
    for (int i = 0 ; i < nparts ; ++i ) 
    {
      freeIndex.insert (i) ;

      vector < int > :: iterator covBegin = cov [i].begin ();
      vector < int > :: const_iterator pos = max_element (covBegin, cov [i].end ()) ;

      int distance = (pos - covBegin);
      pair<int, int> val (i,distance);
      max [ *pos ] = val; 
    } 
  }

  vector < int > renumber (nparts, -1L) ;

  {
    typedef map < int, pair < int, int >, greater_equal < int > > ::
      const_iterator max_const_iterator;
    const max_const_iterator maxEnd = max.end();
    for (max_const_iterator i = max.begin () ; i != maxEnd; ++i ) 
    {
      const pair<int, int> & item = (*i).second;

      if (renumber [item.second] == -1) 
      {
        int neue = item.first ;
        if (freeIndex.find (neue) != freeIndex.end ()) 
        {
          renumber [item.second] = neue ;
          freeIndex.erase (neue) ;
        }
      }
    } 
  }
  
  for (int j = 0; j < nparts ; ++j) 
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
        typedef set < int, less < int > > :: iterator  free_iterator ;
        free_iterator freeIndexBegin = freeIndex.begin () ;

        renumber [j] = * freeIndexBegin;
        freeIndex.erase ( freeIndexBegin ) ;
      }
    }
  }

  /*
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
  */

  { 
    for (int i = 0 ; i < len ; ++i ) proposal [i] = renumber [ proposal [i] ] ; 
  }

  freeIndex.clear();
  return ;
}

template <class real_t, class idx_t>
static bool collectInsulatedNodes (const int nel, 
                                   const real_t* const vertex_w, 
                                   const idx_t* const edge_p, 
                                   const idx_t* const edge, 
                                   const idx_t* const edge_w, 
                                   const int np, 
                                   idx_t* neu) 
{
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
  for (int i = 0 ; i < nel ; ++i ) 
  {
    int j = 0, max = 0 ;
    for (j = max = edge_p [i]; j < edge_p [i+1] ; ++j ) 
    {
      assert (j < ned) ;
      if (neu [i] == neu [edge [j]]) break ;
      else max = edge_w [j] > edge_w [max] ? j : max ;
    }

    if (j == edge_p [i+1]) 
    {
      if (edge_p [i] == edge_p [i+1]) 
      {
        cerr << "**WARNUNG (FEHLER IGNORIERT) Vollst\"andig isolierter Knoten im Grobgittergraph." ;
        cerr << " In Datei: " << __FILE__ << " Zeile: " << __LINE__ << endl ;
      } 
      else 
      {
//        cerr << "!!! Pass mal auf, ich weise dem Knoten " << i << " jetzt das Gebiet " << neu[edge [max]] << " zu." << endl ;
        neu [i] = neu [edge [max]] ;    
        change = true ;
      }
    }
  }
  return change ;
}

bool LoadBalancer :: DataBase :: repartition (MpAccessGlobal & mpa, 
                                              method mth )
{
  vector< int > partition;
  return repartition( mpa, mth, partition, mpa.psize() );
}

vector< int > LoadBalancer :: DataBase :: 
repartition (MpAccessGlobal & mpa, 
             method mth,
             const int np ) 
{
  vector< int > partition( 1 );
  repartition( mpa, mth, partition, np );
  return partition;
}

bool LoadBalancer :: DataBase :: repartition (MpAccessGlobal & mpa, 
                                              method mth,
                                              std::vector< int >& partition,
                                              // number of partitions to be created 
                                              // this is not neccesarily equal to mpa.psize()
                                              const int np )
{
  if (debugOption (3)) printLoad () ;
  
  // if method for load balancing is none, do nothing 
  if (mth == NONE) return false ;

  const int start = clock (), me = mpa.myrank () ;
  // intitial value for change 
  bool change = partition.size() > 0 ;
  
  // flag to indicate whether we use a serial or a parallel partitioner 
  bool serialPartitioner = ( mth <= METIS_PartGraphRecursive ); 
  const bool usePartKway = ( mth == ParMETIS_V3_PartKway );

  // create maps for edges and vertices 
  ldb_edge_set_t    edges ;
  ldb_vertex_map_t  nodes ; 

  typedef ALUGridMETIS :: realtype real_t ;
  typedef ALUGridMETIS :: idxtype  idx_t ;

  // vector of vertex distribution (only for ParMETIS)
  idx_t* vtxdist = ( serialPartitioner ) ?  0 : new idx_t [np + 1];

  // collect graph from all processors 
  // needs a all-to-all (allgather) communication 
  graphCollect( mpa,
                insert_iterator < ldb_vertex_map_t > (nodes,nodes.begin ()),
                insert_iterator < ldb_edge_set_t > (edges,edges.begin ()), 
                vtxdist,
                serialPartitioner 
              ) ;


  // only use ParMETIS_V3_GraphKway for the initial partitioning 
  // this is when all vertices are on proc 0 
  const bool nonDistributedMesh = ( ! serialPartitioner ) ? 
    vtxdist[ 1 ] == vtxdist[ np ] : false ;

  if( nonDistributedMesh && ! serialPartitioner ) 
  {
    serialPartitioner = true ;

    nodes.clear();
    edges.clear();
    delete [] vtxdist; 
    vtxdist = 0;

    // use METIS_PartGraphKway for initial distribution of the mesh 
    mth = METIS_PartGraphKway ;
    
    // redo the graph collect in the case that the mesh is not distributed 
    graphCollect( mpa,
                  insert_iterator < ldb_vertex_map_t > (nodes,nodes.begin ()),
                  insert_iterator < ldb_edge_set_t > (edges,edges.begin ()), 
                  vtxdist,
                  serialPartitioner 
                 ) ;
  }

  // 'ned' ist die Anzahl der Kanten im Graphen, 'nel' die Anzahl der Knoten.
  // Der Container 'nodes' enth"alt alle Knoten des gesamten Grobittergraphen
  // durch Zusammenführen der einzelnen Container aus den Teilgrobgittern.
  // Der Container 'edges' enthält alle Kanten des gesamten Grobgittergraphen
  // doppelt, einmal mit jeder Orientierung (vorw"arts/r"uckw"arts). Diese Form
  // der Datenhaltung ist vorteilhaft, wenn die Eingangsdaten der Partitionierer
  // im CSR Format daraus erstellt werden m"ussen.
  
  const int ned = edges.size () ;
  // for ParMETIS nodes is a local graph that could be empty 
  const int nel = (serialPartitioner) ? nodes.size () : vtxdist[ np ];

  // make sure every process got the same numbers 
  assert( nel == mpa.gmax( nel ) );
  assert( ned == mpa.gmax( ned ) );
  
  //std::cout << "Got " << nel << " number of nodes " << std::endl;
  //std::cout << "Got " << ned << " number of edges " << std::endl;

  // do repartition if edges exist (for serial partitioners) or for
  // parallel partitioners anyway  
  if ( ! serialPartitioner || (ned > 0) ) 
  {
    if( serialPartitioner ) 
    {
      if (!((*edges.rbegin ()).leftNode () < nel)) 
      {
        cerr << "**WARNUNG (FEHLER IGNORIERT) Die Indexmenge ist nicht vollst\"andig\n" ;
        cerr << "  \"uberdeckt zur Neupartitionierung. In " << __FILE__ << " " << __LINE__ << endl ;
        return false ;
      }
    }

    // allocate edge memory for graph partitioners 
    // get memory at once 
    idx_t  * const edge_mem    = new idx_t [(nel + 1) + ned + ned ];

    // set pointer 
    idx_t  * const edge_p      = edge_mem; 
    idx_t  * const edge        = edge_mem + (nel +1);
    idx_t  * const edge_w      = edge + ned; 

    assert ( edge_p && edge && edge_w ) ;
    
    {
      idx_t* edge_pPos = edge_p ;
      int count = 0, index = -1 ;
      
      ldb_edge_set_t :: const_iterator iEnd = edges.end();
      for (ldb_edge_set_t :: const_iterator i = edges.begin () ; i != iEnd ; ++i) 
      {
        const GraphEdge& e = (*i);
        if (e.leftNode () != index) 
        {
          assert ( e.leftNode () < nel) ;
          * edge_pPos ++ = count ;
          index = e.leftNode () ;
        }
        assert ( e.rightNode () < nel) ;
        edge [ count ] = e.rightNode () ;
        edge_w [count ++] = e.weight () ;
      }


      * edge_pPos = count ;
      assert( edge_p [0] == 0 );
      assert( ( serialPartitioner ) ? edge_p [nel] == ned : true ) ;

      // free memory, not needed anymore 
      edges.clear();
    }
    
    // get vertex memory 
    real_t * const vertex_w    = new real_t [nel] ;

    const int sizeNeu = (np > 1) ? nel : 0;
    const int memFactor = ( usePartKway ) ? 2 : 3; // need extra memory for adaptive repartitioning
    idx_t  * vertex_mem = new idx_t [ (memFactor * nel)  + sizeNeu];
    idx_t  * const vertex_wInt = vertex_mem; 
    idx_t  * part              = vertex_mem + nel; 


    real_t  ubvec[1] = {1.2}; // array of length ncon 
    //int options[4] = {0, 1, 15, 1}; // these are the default values 
    real_t* tpwgts = new real_t [ np ]; // ncon * npart, but ncon = 1 
    
    const real_t value = 1.0/ ((real_t) np);
    // set weights (uniform distribution, to be adjusted)
    for(int l=0; l<np; ++l) tpwgts[l] = value;

    assert ( vertex_w && vertex_wInt && part) ;
    {
      vector < int > check (nel, 0L) ;
      ldb_vertex_map_t :: const_iterator iEnd = nodes.end () ;
      for (ldb_vertex_map_t :: const_iterator i = nodes.begin (); i != iEnd; ++i ) 
      {
        const pair< const GraphVertex , int >& item = (*i);
        const int j = item.first.index () ;

        assert (0 <= j && j < nel) ;
        assert (0 <= item.second && item.second < np) ;
        part [j] = item.second ;
        check [j] = 1 ;
        vertex_w [j] = vertex_wInt [j] = item.first.weight () ;
      }

      // store nodes size before clearing   
      const int nNodes = ( serialPartitioner ) ? nel : nodes.size();

      // free memory, not needed anymore
      nodes.clear();
      
      // only for serial partitioners 
      if (nNodes != accumulate (check.begin (), check.end (), 0)) 
      {
        cerr << "**WARNUNG (IGNORIERT) Keine Neupartitionierung wegen fehlgeschlagenem Konsistenzcheck." ;
        cerr << " In Datei: " << __FILE__ << " Zeile: " << __LINE__ << endl ;

        delete [] vertex_w ;
        delete [] vertex_mem;
        delete [] edge_mem;
        delete [] vtxdist ;

        return false ;
      }
    }

    if (np > 1) 
    {
      //int * neu = new int [nel] ;
      idx_t* neu = vertex_mem + (2 * nel);
      assert (neu) ;

      // copy part to neu, this is needed by some of the partitioning tools  
      copy (part, part + nel, neu) ;
      
      idx_t ncon = 1; // number of constraints per vertex, here only one 
      if( ! serialPartitioner ) 
      {
        //cout << "ParMETIS partitioner \n";
        idx_t numflag = 0; // C-style numbering, arrays start with 0  
        idx_t edgecut ;
        idx_t wgtflag = 3; // means weights for vertices and edges 
        idx_t options[4] = {0, 1, 15, 1}; // these are the default values 
        idx_t nparts = np ;

        // get communincator (see mpAccess_MPI.cc)
        MPI_Comm comm = getMPICommunicator( mpa.communicator() );

        // for starting partitions use PartKway
        if( usePartKway ) 
        {
          ALUGridParMETIS :: CALL_ParMETIS_V3_PartKway(vtxdist, edge_p, edge, vertex_wInt, edge_w, 
                                    & wgtflag, & numflag, &ncon, & nparts, tpwgts, 
                                    ubvec, options, & edgecut, neu, & comm ) ;
        }
        else // otherwise do an adaptive repartition 
        {
          // recommmended by ParMETIS docu 
          real_t itr = 1000.0;
          //float itr = 10.0; // like dennis does 
          
          // vsize stores the size of the vertex with respect to
          // redistribution costs (we use the same as vertex_wInt)
          idx_t* vsize = neu + nel; 
          assert ( vsize );
          // for the moment use vertex weights 
          copy(vertex_wInt, vertex_wInt + nel, vsize); 

          // adaptive repartition 
          //cout << "Call AdaptiveRepart \n";
          ALUGridParMETIS :: CALL_ParMETIS_V3_AdaptiveRepart(vtxdist, edge_p, edge, vertex_wInt, vsize, edge_w, 
                                          & wgtflag, & numflag, &ncon, & nparts, tpwgts, 
                                          ubvec, &itr, options, & edgecut, neu, & comm ) ;
        }

        //cout << "Done ParMETIS \n";

        // delete vtxdist and set zero (see below) 
        delete [] vtxdist; vtxdist = 0;
      }
      else 
      {
        // if the number of graph vertices is larger 
        // then the number of partitions 
        if( nel <= np ) 
        {
          std::cout << "Use simple partitioning " << std::endl;
          // set easy partitioning 
          for( int p=0; p<nel; ++ p ) 
          {
            neu[ p ] = p;
          }
        }
        else 
        {
          // serial partitioning methods 
          switch (mth) 
          {

          // METIS methods 
          case METIS_PartGraphKway :
            {
              idx_t wgtflag = 3, numflag = 0, options = 0, edgecut, n = nel, npart = np ;
              ALUGridMETIS :: CALL_METIS_PartGraphKway (&n, &ncon, edge_p, edge, vertex_wInt, edge_w, 
                            & wgtflag, & numflag, & npart, tpwgts, ubvec, & options, & edgecut, neu) ;
            }
            break ;
          case METIS_PartGraphRecursive :
            {
              idx_t wgtflag = 3, numflag = 0, options = 0, edgecut, n = nel, npart = np ;
              ALUGridMETIS :: CALL_METIS_PartGraphRecursive (&n, &ncon, edge_p, edge, vertex_wInt, edge_w, 
                            & wgtflag, & numflag, & npart, tpwgts, ubvec, & options, & edgecut, neu) ;
            }
            break ;

          // the method 'collect' moves all elements to rank 0 
          case COLLECT :
            fill (neu, neu + nel, 0L) ;
            break ;

          default :
            cerr << "**WARNUNG (FEHLER IGNORIERT) Ung\"ultige Methode [" << mth << "] zur\n" ;
            cerr << "  Neupartitionierung angegeben. In " << __FILE__ << " " << __LINE__ << endl ;
              
            delete [] vertex_w ;
            delete [] vertex_mem;
            delete [] edge_mem;
            delete [] tpwgts;
            return false ;
          }
        }
      }

      // only do the following for serialPartitioners and 
      // if we really have a graph much larger then partition number 
      if( serialPartitioner && ( nel > 3*np ) ) 
      {
        // collectInsulatedNodes () sucht alle isolierten Knoten im Graphen und klebt
        // diese einfach mit dem Nachbarknoten "uber die Kante mit dem gr"ossten Gewicht
        // zusammen.
         
        collectInsulatedNodes (nel, vertex_w, edge_p, edge, edge_w, np, neu) ;

        // optimizeCoverage () versucht, die Lastverschiebung durch Permutation der
        // Gebietszuordnung zu beschleunigen. Wenn die alte Aufteilung von der neuen
        // abweicht, dann wird 'change'auf 'true' gestetzt, damit der Lastverschieber
        // in Aktion tritt.
         
        const int verbose = me == 0 ? debugOption (4) : 0;
        optimizeCoverage (np, nel, part, vertex_w, neu, verbose ) ;
      }

      // Vergleichen, ob sich die Aufteilung des Gebiets "uberhaupt ver"andert hat.
      change = ( serialPartitioner ) ? ( ! equal (neu, neu + nel, part) ) : true; 

      // if partition vector is given fill it with the calculated partitioning 
      if( partition.size() > 0 ) 
      { 
        partition.resize( nel );
        copy( neu, neu + nel, partition.begin() );
      }

      // apply partitioning be reassigning a new processor number 
      // to the vertices (elements of the macro mesh) of the graph 
      if (change) 
      {
        // Hier die neue Zuordnung auf den eigenen Lastvertex-Container schreiben.
        // Dadurch werden die Grobgitterelemente an das neue Teilgebiet zugewiesen. 

        ldb_vertex_map_t :: iterator iEnd =  _vertexSet.end () ;
        for (ldb_vertex_map_t :: iterator i = _vertexSet.begin () ; i != iEnd ; ++i)
        {
          // insert and also set partition number new 
          _connect.insert( (*i).second = neu [ (*i).first.index () ]) ;
        }

      }
    }

    delete [] vertex_w ;
    delete [] vertex_mem;
    delete [] edge_mem;
    delete [] vtxdist ;
  }


  if (debugOption (3) && ! me) {
    cout << "**INFO LoadBalancerPll :: DataBase :: repartition ()\n"
         << "       globalen Graphen mit " << (ned/2) << " Kanten und " << nel << " Knoten erzeugt\n"
         << "       und zerlegt mit \"" << methodToString (mth) << "\" in "
         << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec." << endl ;
  }
  return change ;
}

int LoadBalancer :: DataBase :: getDestination (int i) const 
{
  // use constructor to initialize default values 
  GraphVertex e (i) ;
  assert (_vertexSet.find (e) != _vertexSet.end ()) ;
  return (*_vertexSet.find (e)).second ;
}

set < int, less < int > > LoadBalancer :: DataBase :: scan () const {
  return _connect ;
}

const char * LoadBalancer :: DataBase :: methodToString (method m) 
{
  switch (m) {
    case NONE :
      return "no dynamic load balancing" ;
    case COLLECT :
      return "COLLECT" ;
    case METIS_PartGraphKway :
      return "METIS_PartGraphKway" ;
    case METIS_PartGraphRecursive :
      return "METIS_PartGraphRecursive" ;
    case ParMETIS_V3_AdaptiveRepart :
      return "ParMETIS_V3_AdaptiveRepart" ;
    default :
      return "unknown method" ;
  }
  return "" ;
}
