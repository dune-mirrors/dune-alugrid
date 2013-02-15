// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 
#include <config.h>

#include <algorithm>
#include <iterator>
#include <iostream>
#include <numeric>

#include "alusfc.hh"
#include "alumetis.hh"
#include "aluzoltan.hh"
#include "../serial/gitter_sti.h"
#include "gitter_pll_ldb.h"

namespace ALUGrid
{

  void LoadBalancer::DataBase::edgeUpdate (const GraphEdge & e) 
  {
    if (e.isValid ()) 
    {
      ldb_edge_set_t::iterator it =  _edgeSet.find (e);
      if ( it != _edgeSet.end() )
      {
        _edgeSet.erase (it);
      }
      
      _edgeSet.insert (e);
    }
    return;
  }

  void LoadBalancer::DataBase::vertexUpdate (const GraphVertex & v) 
  {
    assert (v.isValid ());
    _maxVertexLoad = _maxVertexLoad < v.weight () ? v.weight () : _maxVertexLoad;
    _vertexSet.find (v) != _vertexSet.end () ? 
      (_vertexSet.erase (v), _vertexSet [v] = -1) : _vertexSet [v] = -1;
    return;
  }

  int LoadBalancer::DataBase::accVertexLoad () const
  {
    return std::accumulate( _vertexSet.begin(), _vertexSet.end(), 0, AccVertexLoad() );
  }

  int LoadBalancer::DataBase::accEdgeLoad () const
  {
    return std::accumulate( _edgeSet.begin(), _edgeSet.end(), 0, AccEdgeLoad() );
  }

  void LoadBalancer::DataBase::printLoad () const
  {
    std::cout << "INFO: LoadBalancer::DataBase::printLoad () [elt(max)|fce] ";
    std::cout << accVertexLoad () << " " << maxVertexLoad () << " " << accEdgeLoad () << std::endl;
  } 

  void LoadBalancer::DataBase::
  graphCollect ( const MpAccessGlobal &mpa,
                 std::insert_iterator< ldb_vertex_map_t > nodes,
                 std::insert_iterator< ldb_edge_set_t > edges,
                 const bool serialPartitioner ) const 
  {
    // if the number of ranks is small, then use old allgather method 
    if( ALUGridExternalParameters::useAllGather( mpa ) )
    {
      // old method has O(p log p) time complexity 
      // and O(p) memory consumption which is critical 
      // for higher number of cores 
      graphCollectAllgather( mpa, nodes, edges, serialPartitioner );
    }
    else 
    {
      // otherwise use method with O(p log p) time complexity 
      // and O(1) memory consumption
      graphCollectBcast( mpa, nodes, edges, serialPartitioner );
    }
  }

  void LoadBalancer::DataBase::
  graphCollectAllgather ( const MpAccessGlobal &mpa,
                          std::insert_iterator< ldb_vertex_map_t > nodes, 
                          std::insert_iterator< ldb_edge_set_t > edges,
                          const bool serialPartitioner ) const 
  {
    assert( serialPartitioner );
    // for parallel partitioner return local vertices and edges 
    // for serial partitioner these have to be communicates to all
    // processes 
     
    // number of processes 
    const int np = mpa.psize();

    if( ! serialPartitioner || np == 1 )
    {
      const int myrank = mpa.myrank();
      {
        ldb_vertex_map_t::const_iterator iEnd = _vertexSet.end ();
        for (ldb_vertex_map_t::const_iterator i = _vertexSet.begin (); 
             i != iEnd; ++i, ++nodes ) 
        {
          {
            const GraphVertex& x = (*i).first;
            *nodes = std::pair< const GraphVertex, int > ( x , myrank);
          } 
        }
      }

      {
        ldb_edge_set_t::const_iterator eEnd = _edgeSet.end ();
        for (ldb_edge_set_t::const_iterator e = _edgeSet.begin (); 
             e != eEnd; ++e ) 
        {
          const GraphEdge& x = (*e);
          // edges exists twice ( u , v ) and ( v , u )
          // with both orientations 
          * edges = x;
          ++ edges;
          * edges = - x;
          ++ edges;
        }
      }
    }

    // for serial calls we are done here 
    if( np == 1 ) return;

    ObjectStream os;

    {
      const int noPeriodicFaces = int( _noPeriodicFaces );
      os.writeObject( noPeriodicFaces );

      // write number of elements  
      const int vertexSize = _vertexSet.size ();
      os.writeObject ( vertexSize );

      if( serialPartitioner )
      {
        // write vertices 
        ldb_vertex_map_t::const_iterator iEnd = _vertexSet.end ();
        for (ldb_vertex_map_t::const_iterator i = _vertexSet.begin (); i != iEnd; ++i ) 
        {
          // write graph vertex to stream 
          (*i).first.writeToStream( os );
        }

        // write number of edges 
        const int edgeSize = _edgeSet.size ();
        os.writeObject ( edgeSize );

        // write edges 
        ldb_edge_set_t::const_iterator eEnd = _edgeSet.end ();
        for (ldb_edge_set_t::const_iterator e = _edgeSet.begin (); e != eEnd; ++e )
        {
          // write graph edge to stream 
          (*e).writeToStream( os );
        }
      }
    }

    try 
    {
      // exchange data  
      std::vector< ObjectStream > osv = ( _graphSizes.size() > 0 ) ? 
            mpa.gcollect (os, _graphSizes) : mpa.gcollect( os );

      // free memory 
      os.reset ();

      {
        for (int i = 0; i < np; ++i) 
        {
          ObjectStream& osv_i = osv [i];

          int noPeriodicFaces = 1;
          osv_i.readObject( noPeriodicFaces );
          // store result 
          _noPeriodicFaces &= bool( noPeriodicFaces );

          int len = -1;
          osv_i.readObject (len);
          assert (len >= 0);

          // read graph for serial partitioner 
          if( serialPartitioner ) 
          {
            for (int j = 0; j < len; ++j, ++ nodes ) 
            {
              // constructor taking stream reads values form ObjectStream 
              GraphVertex x( osv_i );
              (*nodes) = std::pair< const GraphVertex, int > (x,i);
            } 

            osv_i.readObject (len);
            assert (len >= 0);

            for (int j = 0; j < len; ++j) 
            {
              // constructor taking stream reads values form ObjectStream 
              GraphEdge x( osv_i );
              (*edges) =  x;
              ++ edges;
              (*edges) = -x;
              ++ edges;
            }
          }

          // free memory of osv[i]
          osv_i.reset();
        }
      }
    } 
    catch (ObjectStream::EOFException) 
    {
      std::cerr << "ERROR (fatal): EOF encountered." << std::endl;
      abort();
    } 
    catch( ObjectStream::OutOfMemoryException )
    {
      std::cerr << "ERRPR (fatal): Out Of Memory." << std::endl;
      abort();
    }
  }

  void LoadBalancer::DataBase::
  graphCollectBcast ( const MpAccessGlobal &mpa, 
                      std::insert_iterator< ldb_vertex_map_t > nodes, 
                      std::insert_iterator< ldb_edge_set_t > edges,
                      const bool serialPartitioner ) const 
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
        ldb_vertex_map_t::const_iterator iEnd = _vertexSet.end ();
        for (ldb_vertex_map_t::const_iterator i = _vertexSet.begin (); 
             i != iEnd; ++i, ++nodes ) 
        {
          {
            const GraphVertex& x = (*i).first;
            *nodes = std::pair< const GraphVertex, int > ( x , me );
          } 
        }
      }

      {
        ldb_edge_set_t::const_iterator eEnd = _edgeSet.end ();
        for (ldb_edge_set_t::const_iterator e = _edgeSet.begin (); 
             e != eEnd; ++e ) 
        {
          const GraphEdge& x = (*e);
          // edges exists twice ( u , v ) and ( v , u )
          // with both orientations 
          *edges =   x;
          ++ edges;
          *edges = - x;
          ++ edges;
        }
      }
    }

    // for serial calls we are done here 
    if( np == 1 ) return;

    // my data stream 
    ObjectStream os;
     
    {
      const int noPeriodicFaces = int( _noPeriodicFaces );
      os.writeObject( noPeriodicFaces );

      // write number of elements  
      const int vertexSize = _vertexSet.size ();
      os.writeObject ( vertexSize );

      if( serialPartitioner )
      {
        // write vertices 
        ldb_vertex_map_t::const_iterator iEnd = _vertexSet.end ();
        for (ldb_vertex_map_t::const_iterator i = _vertexSet.begin (); i != iEnd; ++i ) 
        {
          // write graph vertex to stream 
          (*i).first.writeToStream( os );
        }

        // write number of edges 
        const int edgeSize = _edgeSet.size ();
        os.writeObject ( edgeSize );

        // write edges 
        ldb_edge_set_t::const_iterator eEnd = _edgeSet.end ();
        for (ldb_edge_set_t::const_iterator e = _edgeSet.begin (); e != eEnd; ++e )
        {
          // write graph edge to stream 
          (*e).writeToStream( os );
        }
      }
    }

    try 
    {
      const bool havePrecomputedSizes = (_graphSizes.size() == size_t(np)); 
      int maxSize = 0;
      // make each proc is on the same track
      assert( havePrecomputedSizes == mpa.gmax( havePrecomputedSizes ) );
      if( havePrecomputedSizes ) 
      {
        for( int rank = 0; rank < np; ++ rank ) 
          maxSize = std::max( maxSize, _graphSizes[ rank ] );
      }
      else
      {
        // get max buffer size (only for serial partitioner we need to communicate)
        maxSize = serialPartitioner ? mpa.gmax( os.size() ) : os.size();
      }

      // create bcast buffer and reserve memory 
      ObjectStream sendrecv;
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

        // get message size for current rank 
        const int msgSize = havePrecomputedSizes ? _graphSizes[ rank ] : maxSize;

        // exchange data 
        mpa.bcast( sendrecv.getBuff(0), msgSize, rank );

        // insert data into graph map 
        {
          // reset read posistion 
          sendrecv.resetReadPosition();
          // adjust write count to max length to avoid eof errors 
          sendrecv.seekp( msgSize );

          int noPeriodicFaces = 1;
          sendrecv.readObject( noPeriodicFaces );
          // store result 
          _noPeriodicFaces &= bool( noPeriodicFaces );

          int len;
          sendrecv.readObject ( len );
          assert (len >= 0);

          // read graph for serial partitioner 
          if( serialPartitioner ) 
          {
            for (int j = 0; j < len; ++j, ++ nodes ) 
            {
              // constructor taking stream reads values form ObjectStream 
              GraphVertex x( sendrecv );
              *nodes = std::pair< const GraphVertex, int > (x, rank);
            } 

            sendrecv.readObject (len);
            assert (len >= 0);

            for (int j = 0; j < len; ++j) 
            {
              // constructor taking stream reads values form ObjectStream 
              GraphEdge x( sendrecv );
              * edges = x;
              ++ edges;
              * edges = - x;
              ++ edges;
            }
          }
        }
      }
    } 
    catch( ObjectStream::EOFException )
    {
      std::cerr << "ERROR (fatal): EOF encountered." << std::endl;
      abort();
    } 
    catch( ObjectStream::OutOfMemoryException )
    {
      std::cerr << "ERROR (fatal): Out of memory." << std::endl;
      abort();
    }
  }

  template <class idx_t>
  static void optimizeCoverage (const int nparts, 
                                const int len, 
                                const idx_t* const reference, 
                                const idx_t* const weight, 
                                idx_t* const proposal, 
                                const int verbose) 
  {
    // 'reference' ist das Referenzarray, das mit dem 'proposal'
    // Vorschlagsvektor optimal abgeglichen werden soll, indem
    // auf 'proposal' eine Indexpermutationangewendet wird.
    // cov ist das 'coverage' Array, das die "Uberdeckung von
    // alter und neuer Teilgebietszuordnung beschreiben soll.

    std::map< int, std::pair< int, int >, std::greater_equal< int > > max;
    std::set< int > freeIndex;
    
    {
      std::vector< std::vector< int > > cov (nparts, std::vector< int > (nparts, 0L));

      for (int k = 0; k < len; ++k) 
      {
        cov [reference [k]][proposal[k]] += 1 + int (sqrt(double(weight [k])));
      }
    
      for (int i = 0; i < nparts; ++i ) 
      {
        freeIndex.insert (i);

        std::vector< int >::iterator covBegin = cov [i].begin ();
        std::vector< int >::const_iterator pos = max_element (covBegin, cov [i].end ());

        int distance = (pos - covBegin);
        std::pair<int, int> val (i,distance);
        max [ *pos ] = val; 
      } 
    }

    std::vector< int > renumber (nparts, -1L);

    {
      typedef std::map< int, std::pair< int, int >, std::greater_equal< int > >::const_iterator max_const_iterator;
      const max_const_iterator maxEnd = max.end();
      for (max_const_iterator i = max.begin (); i != maxEnd; ++i ) 
      {
        const std::pair<int, int> & item = (*i).second;

        if (renumber [item.second] == -1) 
        {
          int neue = item.first;
          if (freeIndex.find (neue) != freeIndex.end ()) 
          {
            renumber [item.second] = neue;
            freeIndex.erase (neue);
          }
        }
      } 
    }
    
    for (int j = 0; j < nparts; ++j) 
    {
      if (renumber [j] == -1) 
      {
        if (freeIndex.find (j) != freeIndex.end ()) 
        {
          renumber [j] = j;
          freeIndex.erase (j);
        } 
        else 
        {
          typedef std::set< int >::iterator  free_iterator;
          free_iterator freeIndexBegin = freeIndex.begin ();

          renumber [j] = * freeIndexBegin;
          freeIndex.erase ( freeIndexBegin );
        }
      }
    }

    /*
    if (verbose) 
    {
      cout << "**INFO optimizeCoverage (): " << endl;
      for (int i = 0; i < nparts; i ++) 
      {
        for (int j = 0; j < nparts; j ++)
          cout << "  " << setw (4) << cov [i][j] << " ";

        cout << "| " << i << " -> " << renumber [i] << endl;
      }
    }
    */

    { 
      for (int i = 0; i < len; ++i ) proposal [i] = renumber [ proposal [i] ]; 
    }

    freeIndex.clear();
    return;
  }

  template <class idx_t>
  static bool collectInsulatedNodes (const int nel, 
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
    const int ned = edge_p [nel];
#endif
    assert (edge_p [0] == 0);
    bool change = false;
    for (int i = 0; i < nel; ++i ) 
    {
      int j = 0, max = 0;
      for (j = max = edge_p [i]; j < edge_p [i+1]; ++j ) 
      {
        assert (j < ned);
        if (neu [i] == neu [edge [j]]) break;
        else max = edge_w [j] > edge_w [max] ? j : max;
      }

      if (j == edge_p [i+1]) 
      {
        if (edge_p [i] == edge_p [i+1]) 
          std::cerr << "WARNING (ignored): Isolated node in macro grid graph." << std::endl;
        else 
        {
          neu[ i ] = neu[ edge[ max ] ];
          change = true;
        }
      }
    }
    return change;
  }

  bool LoadBalancer::DataBase::repartition ( MpAccessGlobal &mpa, method mth )
  {
    std::vector< int > partition;
    return repartition( mpa, mth, partition, mpa.psize() );
  }

  std::vector< int > LoadBalancer::DataBase::
  repartition (MpAccessGlobal & mpa, 
               method mth,
               const int np ) 
  {
    std::vector< int > partition( 1 );
    repartition( mpa, mth, partition, np );
    return partition;
  }

  bool LoadBalancer::DataBase::repartition (MpAccessGlobal & mpa, 
                                            method mth,
                                            std::vector< int >& partition,
                                            // number of partitions to be created 
                                            // this is not neccesarily equal to mpa.psize()
                                            const int np )
  {
    if (debugOption (3)) printLoad ();
    
    // if method for load balancing is none, do nothing 
    if (mth == NONE) return false;

    // ZOLTAN partitioning 
    if (mth >= ZOLTAN_LB_HSFC ) 
    {
      // if no Zoltan was found the return value will be false 
      ALUGridZoltan :: CALL_Zoltan_LB_Partition( mpa, _vertexSet, _edgeSet,  _connect );
      return true;
    }

    const int start = clock (), me = mpa.myrank ();
    // intitial value for change 
    bool change = partition.size() > 0;
    
    // flag to indicate whether we use a serial or a parallel partitioner 
    bool serialPartitioner    = serialPartitionerUsed( mth );
    const bool noEdgesInGraph = ( mth == ALUGRID_SpaceFillingCurveNoEdges );

    // create maps for edges and vertices 
    ldb_edge_set_t    edges;
    ldb_vertex_map_t  nodes; 

    typedef ALUGridMETIS::realtype real_t;
    typedef ALUGridMETIS::idxtype  idx_t;

    // for the first SFC approach we don't have edges in the graph 
    assert( noEdgesInGraph ? _edgeSet.size() == 0 : true ); 

    // collect graph from all processors 
    // needs a all-to-all (allgather) communication 
    graphCollect( mpa,
                  std::insert_iterator< ldb_vertex_map_t > (nodes,nodes.begin ()),
                  std::insert_iterator< ldb_edge_set_t > (edges,edges.begin ()), 
                  serialPartitioner 
                );

    // 'ned' ist die Anzahl der Kanten im Graphen, 'nel' die Anzahl der Knoten.
    // Der Container 'nodes' enth"alt alle Knoten des gesamten Grobittergraphen
    // durch Zusammenführen der einzelnen Container aus den Teilgrobgittern.
    // Der Container 'edges' enthält alle Kanten des gesamten Grobgittergraphen
    // doppelt, einmal mit jeder Orientierung (vorw"arts/r"uckw"arts). Diese Form
    // der Datenhaltung ist vorteilhaft, wenn die Eingangsdaten der Partitionierer
    // im CSR Format daraus erstellt werden m"ussen.
    
    const int ned = edges.size ();
    const int nel = nodes.size ();

    // make sure every process got the same numbers 
    assert( nel == mpa.gmax( nel ) );
    assert( ned == mpa.gmax( ned ) );
    
    // do repartition if edges exist (for serial partitioners) or for SFC and 
    // parallel partitioners anyway  
    if ( noEdgesInGraph || (ned > 0) ) 
    {
      if( serialPartitioner && (ned > 0) ) 
      {
        if (!((*edges.rbegin ()).leftNode () < nel)) 
        {
          std::cerr << "WARNING (ignored): Incomplete index set during repartitioning." << std::endl;
          return false;
        }
      }

      // allocate edge memory for graph partitioners 
      // get memory at once 
      idx_t  * const edge_mem    = new idx_t [(nel + 1) + ned + ned ];

      // set pointer 
      idx_t  * const edge_p      = edge_mem; 
      idx_t  * const edge        = edge_mem + (nel +1);
      idx_t  * const edge_w      = edge + ned; 

      assert ( edge_p && edge && edge_w );
      
      {
        idx_t* edge_pPos = edge_p;
        int count = 0, index = -1;
        
        ldb_edge_set_t::const_iterator iEnd = edges.end();
        for (ldb_edge_set_t::const_iterator i = edges.begin (); i != iEnd; ++i, ++count ) 
        {
          const GraphEdge& e = (*i);
          if (e.leftNode () != index) 
          {
            assert ( serialPartitioner ? e.leftNode () < nel : true );
            *edge_pPos = count;
            ++edge_pPos ; 
            index = e.leftNode ();
          }
          assert ( serialPartitioner ? e.rightNode () < nel : true );
          edge   [ count ] = e.rightNode ();
          edge_w [ count ] = e.weight ();
        }

        * edge_pPos = count;
        assert( edge_p [0] == 0 );
        assert( ( serialPartitioner && ned > 0 ) ? edge_p [nel] == ned : true );

        // free memory, not needed anymore 
        // needed to determine graphSizes later 
        // edges.clear();
      }
      
      const int sizeNeu = (np > 1) ? nel : 0;
      const int memFactor = 2 ; // need extra memory for adaptive repartitioning
      idx_t  * vertex_mem = new idx_t [ (memFactor * nel)  + sizeNeu];
      idx_t  * const vertex_wInt = vertex_mem; 
      idx_t  * part              = vertex_mem + nel; 
      idx_t ncon = 1 ;

      real_t  ubvec[1] = {1.2}; // array of length ncon 
      real_t* tpwgts = new real_t [ np ]; // ncon * npart, but ncon = 1 
      
      const real_t value = 1.0/ ((real_t) np);
      // set weights (uniform distribution, to be adjusted)
      for(int l=0; l<np; ++l) tpwgts[l] = value;

      assert ( vertex_wInt && part);
      {
        std::vector< int > check (nel, 0L);
        ldb_vertex_map_t::const_iterator iEnd = nodes.end ();
        for (ldb_vertex_map_t::const_iterator i = nodes.begin (); i != iEnd; ++i ) 
        {
          const std::pair< const GraphVertex , int >& item = (*i);
          const int j = item.first.index ();

          assert ( serialPartitioner ? 0 <= j && j < nel : true );
          assert (0 <= item.second && item.second < np);
          part [j] = item.second;
          check [j] = 1;
          vertex_wInt [j] = item.first.weight ();
        }

        // store nodes size before clearing   
        const int nNodes = ( serialPartitioner ) ? nel : nodes.size();

        // free memory, not needed anymore
        nodes.clear();
        
        // only for serial partitioners 
        if (nNodes != accumulate (check.begin (), check.end (), 0)) 
        {
          std::cerr << "WARNING (ignored): No repartitioning due to failed consistency check." << std::endl;

          delete[] vertex_mem;
          delete[] edge_mem;
          return false;
        }
      }

      if (np > 1) 
      {
        idx_t* neu = vertex_mem + (2 * nel);
        assert (neu);

        // copy part to neu, this is needed by some of the partitioning tools  
        std::copy( part, part + nel, neu );
        
        {
          // if the number of graph vertices is smaller  
          // then the number of partitions 
          if( nel <= np ) 
          {
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

            // space filling curve approach 
            case ALUGRID_SpaceFillingCurveNoEdges:
              {
                idx_t n = nel ;
                ALUGridMETIS::CALL_spaceFillingCurveNoEdges( mpa, n, vertex_wInt, neu );
              }
              break;
            case ALUGRID_SpaceFillingCurve:
              {
                idx_t n = nel ;
                ALUGridMETIS::CALL_spaceFillingCurve( mpa, n, vertex_wInt, neu);
              }
              break;

            // METIS methods 
            case METIS_PartGraphKway :
              {
                idx_t wgtflag = 3, numflag = 0, options = 0, edgecut, n = nel, npart = np;
                ALUGridMETIS::CALL_METIS_PartGraphKway (&n, &ncon, edge_p, edge, vertex_wInt, edge_w, 
                              & wgtflag, & numflag, & npart, tpwgts, ubvec, & options, & edgecut, neu);
              }
              break;
            case METIS_PartGraphRecursive :
              {
                idx_t wgtflag = 3, numflag = 0, options = 0, edgecut, n = nel, npart = np;
                ALUGridMETIS::CALL_METIS_PartGraphRecursive (&n, &ncon, edge_p, edge, vertex_wInt, edge_w, 
                              & wgtflag, & numflag, & npart, tpwgts, ubvec, & options, & edgecut, neu);
              }
              break;

            // the method 'collect' moves all elements to rank 0 
            case COLLECT:
              std::fill( neu, neu + nel, 0L );
              break;

            default :
              std::cerr << "WARNING (ignored): Invalid repartitioning method [" << mth << "]." << std::endl;
                
              delete[] vertex_mem;
              delete[] edge_mem;
              delete[] tpwgts;
              return false;
            }
          }
        } // end serialPartitioner 

        // only do the following for serialPartitioners and 
        // if we really have a graph much larger then partition number 
        if( serialPartitioner && ( nel > 3*np ) && ( mth > ALUGRID_SpaceFillingCurve ) ) 
        {
          // collectInsulatedNodes () sucht alle isolierten Knoten im Graphen und klebt
          // diese einfach mit dem Nachbarknoten "uber die Kante mit dem gr"ossten Gewicht
          // zusammen.
           
          collectInsulatedNodes (nel, edge_p, edge, edge_w, np, neu);

          // optimizeCoverage () versucht, die Lastverschiebung durch Permutation der
          // Gebietszuordnung zu beschleunigen. Wenn die alte Aufteilung von der neuen
          // abweicht, dann wird 'change'auf 'true' gestetzt, damit der Lastverschieber
          // in Aktion tritt.
           
          const int verbose = me == 0 ? debugOption (4) : 0;
          optimizeCoverage (np, nel, part, vertex_wInt, neu, verbose );
        }

        // Vergleichen, ob sich die Aufteilung des Gebiets "uberhaupt ver"andert hat.
        change = (serialPartitioner ? ! std::equal (neu, neu + nel, part) : true);

        // if partition vector is given fill it with the calculated partitioning 
        if( partition.size() > 0 ) 
        { 
          partition.resize( nel );
          std::copy( neu, neu + nel, partition.begin() );
        }

        // apply partitioning be reassigning a new processor number 
        // to the vertices (elements of the macro mesh) of the graph 
        if (change) 
        {
          // Hier die neue Zuordnung auf den eigenen Lastvertex-Container schreiben.
          // Dadurch werden die Grobgitterelemente an das neue Teilgebiet zugewiesen. 

          ldb_vertex_map_t::iterator iEnd =  _vertexSet.end ();
          for (ldb_vertex_map_t::iterator i = _vertexSet.begin (); i != iEnd; ++i)
          {
            // insert and also set partition number new 
            _connect.insert( (*i).second = neu [ (*i).first.index () ]);
          }

          // in case of the serial partitioners we are able to store the sizes 
          // to avoid a second communication during graphCollect 
          // this is only needed for the allgatherv communication 
          assert( _noPeriodicFaces == mpa.gmax( _noPeriodicFaces ) );
          if( _noPeriodicFaces ) 
          {
            // resize vector 
            _graphSizes.resize( np );

            // clear _graphSizes vector, default is sizeof for the sizes that are send 
            // at the beginning of the object streams 
            // (one for _noPeriodicFaces flag, one for the vertices, and one for the edges)
            const int initSize = 3 * sizeof(int); // see graphCollect 
            std::fill( _graphSizes.begin(), _graphSizes.end(), initSize );

            // count number of graph vertices each process contains 
            for( int i=0; i<nel; ++i ) 
            {
              _graphSizes[ neu[ i ] ] += GraphVertex :: sizeOfData ;
            }

            // add edge sizes 
            ldb_edge_set_t::const_iterator iEnd = edges.end();
            for (ldb_edge_set_t::const_iterator i = edges.begin (); i != iEnd; ++i) 
            {
              const GraphEdge& e = (*i);
              // only do something when the left node is smaller then the right node 
              // since each edge exists twice in the edges set 
              if( e.leftNode() < e.rightNode() ) 
              {
                // increase size of message to be passed on the next repatition
                _graphSizes[ neu[ e.leftNode() ] ] += GraphEdge :: sizeOfData ;
              }
            }
          }
          else // otherwise disable this feature be clearing the vector 
          {
            clearGraphSizesVector ();
          }
        }
      }

      delete [] vertex_mem;
      delete [] edge_mem;
      delete [] tpwgts;
    }


    if( debugOption( 3 ) && !me )
    {
      std::cout << "INFO: LoadBalancerPll::DataBase::repartition() partitioned global graph with ";
      std::cout << (ned/2) << " edges and " << nel << " nodes using method " << methodToString( mth );
      std::cout << " in " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " s." << std::endl;
    }
    return change;
  }

  int LoadBalancer::DataBase::destination (int i) const 
  {
    // use constructor to initialize default values 
    GraphVertex e (i);
    assert (_vertexSet.find (e) != _vertexSet.end ());
    return (*_vertexSet.find (e)).second;
  }

  const char * LoadBalancer::DataBase::methodToString (method m) 
  {
    switch (m) {
      case NONE :
        return "no dynamic load balancing";
      case COLLECT :
        return "COLLECT";
      case ALUGRID_SpaceFillingCurveNoEdges:
        return "ALUGRID_SpaceFillingCurveNoEdges";
      case ALUGRID_SpaceFillingCurve:
        return "ALUGRID_SpaceFillingCurve";
      case METIS_PartGraphKway :
        return "METIS_PartGraphKway";
      case METIS_PartGraphRecursive :
        return "METIS_PartGraphRecursive";
      case ZOLTAN_LB_HSFC :
        return "ZOLTAN_LB_HSFC";
      case ZOLTAN_LB_GraphPartitioning:
        return "ZOLTAN_LB_GraphPartitioning";
      default :
        return "unknown method";
    }
    return "";
  }

} // namespace ALUGrid
