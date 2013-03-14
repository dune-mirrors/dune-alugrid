#ifndef ALUGRID_ZOLTAN_H_INCLUDED
#define ALUGRID_ZOLTAN_H_INCLUDED

#include <cmath>
#include <cassert>
#include <sstream>

#include "mpAccess_MPI.h"

#if HAVE_ZOLTAN 
#define ZOLTAN_CONFIG_H_INCLUDED
#include <zoltan_cpp.h>
#endif 

namespace ALUGridZoltan
{
#if HAVE_ZOLTAN
  template < class ldb_vertex_map_t, class ldb_edge_set_t >
  class ObjectCollection 
  {
    int _rank;
    ldb_vertex_map_t& _vertexMap ;
    ldb_edge_set_t& _edgeMap ;
    typedef typename ldb_edge_set_t::const_iterator edgeType;
    std::vector< std::vector< std::pair<edgeType,bool> > > _edges;
    static const int dimension = 3 ;

  public:
    // constructor
    ObjectCollection( int rank, ldb_vertex_map_t& vertexMap, ldb_edge_set_t& edgeMap ) 
      : _rank( rank ),
        _vertexMap( vertexMap ),
        _edgeMap( edgeMap ),
        _edges(0)
    {}

    int rank() { return _rank; }
    ldb_vertex_map_t& vertexMap() { return _vertexMap; }
    ldb_edge_set_t& edgeMap() { return _edgeMap; }
    std::vector< std::vector<std::pair<edgeType,bool> > >& edges() { return _edges; }
    int edgeIdx(int i,int k) { assert( i < (int)_edges.size() && k < (int)_edges[i].size() ); 
                               return ( (_edges[i][k].second) ?
                                   _edges[i][k].first->rightNode() : 
                                   _edges[i][k].first->leftNode() ) ; }
    int edgeMaster(int i,int k) { assert( i < (int)_edges.size() && k < (int)_edges[i].size() ); 
                               return ( (_edges[i][k].second) ?
                                   _edges[i][k].first->rightMaster() : 
                                   _edges[i][k].first->leftMaster() ) ; }
    int edgeWeight(int i,int k) { assert( i < (int)_edges.size() && k < (int)_edges[i].size() ); 
                                 return _edges[i][k].first->weight(); }

    // query functions that respond to requests from Zoltan 

    static int get_number_of_objects(void *data, int *ierr)
    {
      ObjectCollection *objs = static_cast<ObjectCollection *> (data);
      assert( objs );
      *ierr = ZOLTAN_OK;
      return objs->vertexMap().size();
    }

    static void get_object_list(void *data, int sizeGID, int sizeLID,
                                ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                int wgt_dim, float *obj_wgts, int *ierr)
    {
      ObjectCollection *objs = static_cast<ObjectCollection *> (data);
      assert( objs );
      assert(wgt_dim==1);
      *ierr = ZOLTAN_OK;

      ldb_vertex_map_t& vertexMap = objs->vertexMap();

      // In this example, return the IDs of our objects, but no weights.
      // Zoltan will assume equally weighted objects.
      int i = 0; 
      typedef typename ldb_vertex_map_t :: iterator iterator ;
      const iterator end = vertexMap.end();
      for ( iterator it = vertexMap.begin(); it != end; ++ it, ++i ) 
      {
        // std::cout << "[" << objs->rank() << "]: ";
        // std::cout << "vertex(" << i << ") = " << (*it).first.index() << std::endl;
        globalID[ i ] = (*it).first.index() ;
        localID [ i ] = i;
        obj_wgts[ i ] = (*it).first.weight();
      }
    }

    static void get_num_edges_list(void *data, int sizeGID, int sizeLID,
                                  int num_obj,
                                  ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                  int *numEdges, int *ierr)
    {
      ObjectCollection *objs = static_cast<ObjectCollection *> (data);
      assert( num_obj == (int)objs->vertexMap().size() );
      if (num_obj == 0)
      {
        *ierr = ZOLTAN_OK;
        return;
      }

      for (int i=0;i<num_obj;++i)
      {
        numEdges[i] = 0;
      }
      objs->edges().resize(num_obj);

      ldb_vertex_map_t& vertexMap = objs->vertexMap();
      typename ldb_vertex_map_t :: iterator vertexIt = vertexMap.begin();
      int i = 0;
      int vertex = vertexIt->first.index() ;

#if 0
      typename ldb_edge_set_t::const_iterator iEnd = objs->edgeMap().end();
      for (typename ldb_edge_set_t::const_iterator it = objs->edgeMap().begin (); it != iEnd; ++it ) 
      {
        std::cout << "edges for " << vertex << std::endl;
        if ( it->leftMaster() != objs->rank() ) // is ghost
          continue;
        int leftNode = it->leftNode();
        std::cout << "[" << objs->rank() << "]: ";
        while (leftNode != vertex)
        {
          ++vertexIt;
          vertex = vertexIt->first.index() ;
          ++i;
          assert( i < num_obj );
          assert( vertexIt != vertexMap.end() );
        }

        std::cout << "[" << objs->rank() << "]: ";
        std::cout << "edge(" << i << ") = " << it->rightNode() << " " << it->rightMaster() << std::endl;
        assert( leftNode == vertex );
        ++numEdges[i];
        // assert( it->leftMaster() == objs->rank() );
        objs->edges()[i].push_back( it );
      }
#else
      typename ldb_edge_set_t::const_iterator iEnd = objs->edgeMap().end();
      for (typename ldb_edge_set_t::const_iterator it = objs->edgeMap().begin (); it != iEnd; ++it ) 
      {
        if ( it->leftMaster() == objs->rank() ) 
        {  
          int node = it->leftNode();
          int i=0;
          for (;i<num_obj;++i)
            if (globalID[i] == node) break;
          assert( i<num_obj );
          //std::cout << "[" << objs->rank() << "]: ";
          //std::cout << "edge(" << i << ") = " << it->rightNode() << " " << it->rightMaster() << std::endl;
          ++numEdges[i];
          objs->edges()[i].push_back( std::make_pair(it,true) );
        }
        if ( it->rightMaster() == objs->rank() ) 
        {  
          int node = it->rightNode();
          int i=0;
          for (;i<num_obj;++i)
            if (globalID[i] == node) break;
          assert( i<num_obj );
          //std::cout << "[" << objs->rank() << "]: ";
          //std::cout << "edge(" << i << ") = " << it->leftNode() << " " << it->leftMaster() << std::endl;
          ++numEdges[i];
          objs->edges()[i].push_back( std::make_pair(it,false) );
        }
      }
#endif
      *ierr = ZOLTAN_OK;
    }
    static void get_edge_list(void *data, int sizeGID, int sizeLID,
                              int num_obj, ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                              int *num_edges,
                              ZOLTAN_ID_PTR nborGID, int *nborProc,
                              int wgt_dim, float *ewgts, int *ierr)
    {
      ObjectCollection *objs = static_cast<ObjectCollection *> (data);
      if (num_obj == 0)
      {
        *ierr = ZOLTAN_OK;
        return;
      }
      int k=0;
      for (int j=0;j<num_obj;++j)
      {
        for (int l=0;l<num_edges[j];++l)
        {
          // std::cout << "[" << objs->rank() << "]: ";
          // std::cout << "v(" << j << ")(" << l << ")=" << objs->edgeIdx(j,l) << " " << objs->edgeMaster(j,l) << std::endl;
          nborGID[k]  = objs->edgeIdx(j,l);
          nborProc[k] = objs->edgeMaster(j,l);
          assert( nborProc[k] >= 0 );
          if (wgt_dim==1)
            ewgts[k]=objs->edgeWeight(j,l);
          ++k;
        }
      }
      *ierr = ZOLTAN_OK;
    }

    // return dimension of coordinates 
    static int get_num_geometry(void *data, int *ierr)
    {
      *ierr = ZOLTAN_OK;
      return dimension;
    }

    static void get_geometry_list(void *data, int sizeGID, int sizeLID,
                                  int num_obj,
                                  ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                  int num_dim, double *geom_vec, int *ierr)
    {
      ObjectCollection *objs = static_cast<ObjectCollection *> (data);
      assert( objs );

      if ( (sizeGID != 1) || (sizeLID != 1) || (num_dim != dimension))
      {
        *ierr = ZOLTAN_FATAL;
        return;
      }

      *ierr = ZOLTAN_OK;

      ldb_vertex_map_t& vertexMap = objs->vertexMap();

      int idx = 0; 
      typedef typename ldb_vertex_map_t :: iterator iterator ;
      const iterator end = vertexMap.end();
      for ( iterator it = vertexMap.begin(); it != end; ++ it ) 
      {
        const double (&coord)[ 3 ] = (*it).first.center();
        for( int d=0; d<dimension; ++d, ++idx )
          geom_vec[ idx ] = coord[ d ];
      }
    }
  };
#endif // #if HAVE_ZOLTAN

  enum method_t {HSFC, PHG, PARMETIS};
  template< class ldb_vertex_map_t, class ldb_edge_set_t, class ldb_connect_set_t >
  bool CALL_Zoltan_LB_Partition( method_t method,
                                 ALUGrid::MpAccessGlobal &mpa,
                                 ldb_vertex_map_t& vertexMap,
                                 ldb_edge_set_t& edgeSet,
                                 ldb_connect_set_t& connect,
                                 const double givenTolerance,
                                 const bool verbose )
  {
#if HAVE_ZOLTAN 
    ALUGrid::MpAccessMPI* mpaMPI = dynamic_cast<ALUGrid::MpAccessMPI *> (&mpa);
    if( mpaMPI == 0 )
    {
      std::cerr << "ERROR: wrong mpAccess object, couldn't convert to MpAccessMPI!! in: " << __FILE__ << " line : " << __LINE__ << std::endl;
      abort();
    }

    // get communincator (see mpAccess_MPI.cc
    MPI_Comm comm = mpaMPI->communicator();
    int rank = mpaMPI->myrank();

    typedef ObjectCollection< ldb_vertex_map_t, ldb_edge_set_t > ObjectCollectionType;

    ObjectCollectionType objects( rank, vertexMap, edgeSet );
    Zoltan *zz = new Zoltan( comm );
    assert( zz );

    // General parameters 
    const char* debug = ( verbose ) ? "1" : "0";
    zz->Set_Param( "DEBUG_LEVEL", debug );
    zz->Set_Param( "OBJ_WEIGHT_DIM", "1");
    zz->Set_Param( "NUM_GID_ENTRIES", "1");
    zz->Set_Param( "NUM_LID_ENTRIES", "1");
    zz->Set_Param( "RETURN_LISTS", "EXPORT");

    if ( method == HSFC ) // edgeSet.size() == 0 )
    {
      zz->Set_Param( "LB_METHOD", "HSFC");
      //zz->Set_Param( "KEEP_CUTS", "1" );
      zz->Set_Param( "RCB_OUTPUT_LEVEL", "0");
      zz->Set_Param( "RCB_RECTILINEAR_BLOCKS", "1");
      zz->Set_Param( "LB_APPROACH","REPARTITION");
    }
    else
    {
      zz->Set_Param( "LB_METHOD", "GRAPH");
      // zz->Set_Param( "LB_APPROACH","PARTITION");
      static bool first = true;
      if (first)
        zz->Set_Param( "LB_APPROACH", "REPARTITION"); 
      else
        zz->Set_Param( "LB_APPROACH", "REFINE"); 
      first = false;
      zz->Set_Param( "EDGE_WEIGHT_DIM","1");
      zz->Set_Param( "OBJ_WEIGHT_DIM", "1");
      zz->Set_Param( "GRAPH_SYMMETRIZE","NONE" ); 
      // zz->Set_Param( "GRAPH_SYMMETRIZE","TRANSPOSE");
      zz->Set_Param( "GRAPH_SYM_WEIGHT","MAX");
#ifdef HAVE_PARMETIS
      if (method == PARMETIS)
        zz->Set_Param( "GRAPH_PACKAGE","PARMETIS");
#elif  HAVE_SCOTCH
      zz->Set_Param( "GRAPH_PACKAGE","SCOTCH");
#endif
      zz->Set_Param( "CHECK_GRAPH", "2"); 
      zz->Set_Param( "PHG_EDGE_SIZE_THRESHOLD", ".35");
    }


    
    zz->Set_Num_Obj_Fn ( ObjectCollectionType::get_number_of_objects, &objects);
    zz->Set_Obj_List_Fn( ObjectCollectionType::get_object_list, &objects);
    zz->Set_Num_Geom_Fn( ObjectCollectionType::get_num_geometry, &objects);
    zz->Set_Geom_Multi_Fn( ObjectCollectionType::get_geometry_list, &objects);
    zz->Set_Num_Edges_Multi_Fn(ObjectCollectionType::get_num_edges_list, &objects);
    zz->Set_Edge_List_Multi_Fn(ObjectCollectionType::get_edge_list, &objects);

    int changes = 0;
    int numGidEntries = 0;
    int numLidEntries = 0;
    int numImport = 0;
    ZOLTAN_ID_PTR importGlobalIds = 0;
    ZOLTAN_ID_PTR importLocalIds  = 0;
    int *importProcs  = 0;
    int *importToPart = 0;
    int numExport = 0;
    ZOLTAN_ID_PTR exportGlobalIds = 0;
    ZOLTAN_ID_PTR exportLocalIds  = 0;
    int *exportProcs  = 0;
    int *exportToPart = 0;

    double tolerance = givenTolerance ; 
    int rc = ZOLTAN_OK + 1 ;
    int count = 0 ;
    while( rc != ZOLTAN_OK ) 
    {
      // std::cout << "[" << rank << "]: ";
      // std::cout << "zoltan partitioning tolerance: " << tolerance << std::endl;
      // tolerance for load imbalance 
      {
        std::stringstream tol; 
        tol << tolerance ;
        zz->Set_Param( "IMBALANCE_TOL", tol.str() );
      }

      rc = zz->LB_Partition(changes, numGidEntries, numLidEntries,
                            numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
                            numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);

      // increase imbalance tolerance and try again 
      if( rc != ZOLTAN_OK )
      {
        tolerance *= 1.05 ;
        ++ count ;
        if( count > 3 ) break ;
      }
    }

    // if new partitioning has been calculated 
    if (rc == ZOLTAN_OK) 
    {
      typedef typename ldb_vertex_map_t::iterator iterator;
      for (int i=0; i < numExport; ++i)
      {
        iterator vertex = vertexMap.find( exportGlobalIds[ i ] );
        assert( vertex != vertexMap.end () );
        (*vertex).second = exportProcs[ i ];
      }

      const iterator iEnd = vertexMap.end ();
      const int myrank = mpaMPI->myrank();
      for ( iterator i = vertexMap.begin (); i != iEnd; ++i )
      {
        int& moveTo = i->second;
        // insert and also set partition number new (including own number)
        if ( moveTo == -1 ) moveTo = myrank ;
        connect.insert( moveTo );
      }
    }
    else 
    {
      if( verbose && mpa.myrank() == 0 ) 
        std::cerr << "ERROR: Zoltan partitioning failed, partitioning won't change! " << std::endl; 
      // no changes 
      changes = 0;
    }

    ////////////////////////////////////////////////////////////////
    // Free the arrays allocated by LB_Partition, and free
    // the storage allocated for the Zoltan structure and the mesh.
    ////////////////////////////////////////////////////////////////

    Zoltan::LB_Free_Part(&importGlobalIds, &importLocalIds, &importProcs, &importToPart);
    Zoltan::LB_Free_Part(&exportGlobalIds, &exportLocalIds, &exportProcs, &exportToPart);

    // delete zoltan structure 
    delete zz;

    // return true if partition changed 
    return (changes > 0);
#else 
    std::cerr << "ERROR: Zoltan library not found, cannot use Zoltan partitioning! " << std::endl;
    exit(1);
    return false ;
#endif // #if HAVE_ZOLTAN
  } // CALL_Zoltan_LB_Partition 

} // namespace ALUGridZoltan

#endif // #ifndef ALUGRID_ZOLTAN_H_INCLUDED
