#ifndef ALUGRID_ZOLTAN_H_INCLUDED
#define ALUGRID_ZOLTAN_H_INCLUDED

#if USING_DUNE_PYTHON
// include this header here to avoid warnings about HAVE_PROTOTYPES
#include <dune/python/pybind11/pybind11.h>
#endif

#include <iostream>
#include <cmath>
#include <dune/alugrid/common/alugrid_assert.hh>
#include <sstream>

#include "mpAccess_MPI.h"

// Warning: Zoltan defines HAVE_MPI and HAVE_PARMETIS itself. However, their definition will
//          not match ours. The following complicated preprocessor code tries
//          to cope with this problem.

#if HAVE_ZOLTAN

// if DUNE was built with MPI
#if HAVE_MPI
// undefine our definition of HAVE_MPI before including zoltan_cpp.h
#undef HAVE_MPI
#define HAVE_MPI_WAS_UNDEFED_HERE
#endif

#if HAVE_PARMETIS
#undef HAVE_PARMETIS
#define HAVE_PARMETIS_WAS_UNDEFED_HERE
#endif

// include Zoltan's C++ header
#include <zoltan_cpp.h>

// undefine any definition of HAVE_MPI made by Zoltan
#ifdef HAVE_MPI
#undef HAVE_MPI
#endif // #ifdef HAVE_MPI

// undefine any definition of HAVE_PARMETIS made by Zoltan
#ifdef HAVE_PARMETIS
#undef HAVE_PARMETIS
#endif

#ifdef HAVE_MPI_WAS_UNDEFED_HERE
#ifdef ENABLE_MPI
// redefine our definition of HAVE_MPI if it was undef'd before
#define HAVE_MPI ENABLE_MPI
#else
#define HAVE_MPI 1
#endif
#undef HAVE_MPI_WAS_UNDEFED_HERE
#endif // #ifdef HAVE_MPI_WAS_UNDEFED_HERE

#ifdef HAVE_PARMETIS_WAS_UNDEFED_HERE
// redefine our definition of HAVE_PARMETIS if it was undef'd before
#define HAVE_PARMETIS 1
#undef HAVE_PARMETIS_WAS_UNDEFED_HERE
#endif // #ifdef HAVE_PARMETIS_WAS_UNDEFED_HERE

#endif // #if HAVE_ZOLTAN

namespace ALUGridZoltan
{
  using ::ALUGrid::MpAccessGlobal;
  using ::ALUGrid::MpAccessLocal;

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
    int edgeIdx(int i,int k) { alugrid_assert ( i < (int)_edges.size() && k < (int)_edges[i].size() );
                               return ( (_edges[i][k].second) ?
                                   _edges[i][k].first->rightNode() :
                                   _edges[i][k].first->leftNode() ) ; }
    int edgeMaster(int i,int k) { alugrid_assert ( i < (int)_edges.size() && k < (int)_edges[i].size() );
                               return ( (_edges[i][k].second) ?
                                   _edges[i][k].first->rightMaster() :
                                   _edges[i][k].first->leftMaster() ) ; }
    int edgeWeight(int i,int k) { alugrid_assert ( i < (int)_edges.size() && k < (int)_edges[i].size() );
                                 return _edges[i][k].first->weight(); }

    // query functions that respond to requests from Zoltan

    static int get_number_of_objects(void *data, int *ierr)
    {
      ObjectCollection *objs = static_cast<ObjectCollection *> (data);
      alugrid_assert ( objs );
      *ierr = ZOLTAN_OK;
      return objs->vertexMap().size();
    }

    static void get_object_list(void *data, int sizeGID, int sizeLID,
                                ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                int wgt_dim, float *obj_wgts, int *ierr)
    {
      ObjectCollection *objs = static_cast<ObjectCollection *> (data);
      alugrid_assert ( objs );
      alugrid_assert (wgt_dim==1);
      *ierr = ZOLTAN_OK;

      ldb_vertex_map_t& vertexMap = objs->vertexMap();

      // In this example, return the IDs of our objects,
      int i = 0;
      typedef typename ldb_vertex_map_t :: iterator iterator ;
      const iterator end = vertexMap.end();
      for ( iterator it = vertexMap.begin(); it != end; ++ it, ++i )
      {
        // std::cout << "[" << objs->rank() << "]: ";
        // std::cout << "vertex(" << i << ") = " << (*it).first.index() << std::endl;
        globalID[ i ] = (*it).first.index() ;
        localID [ i ] = i;
        if (wgt_dim == 1)
          obj_wgts[ i ] = (*it).first.weight();
      }
    }

    static void get_num_edges_list(void *data, int sizeGID, int sizeLID,
                                  int num_obj,
                                  ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                  int *numEdges, int *ierr)
    {
      ObjectCollection *objs = static_cast<ObjectCollection *> (data);
      alugrid_assert ( num_obj == (int)objs->vertexMap().size() );
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

      const int rank = objs->rank();

      typename ldb_edge_set_t::const_iterator iEnd = objs->edgeMap().end();
      for (typename ldb_edge_set_t::const_iterator it = objs->edgeMap().begin (); it != iEnd; ++it )
      {
        if ( it->leftMaster() == rank )
        {
          int node = it->leftNode();
          int i=0;
          // std::cout << "[" << objs->rank() << "]: ";
          // std::cout << "edge(" << node << ") = " << it->rightNode() << " " << it->rightMaster() << std::endl;
          for (;i<num_obj;++i)
            if ((int)globalID[i] == node) break;
          alugrid_assert ( i<num_obj );
          alugrid_assert (it->rightMaster() >= 0);
          alugrid_assert (it->weight() >= 0);
          ++numEdges[i];
          objs->edges()[i].push_back( std::make_pair(it,true) );
        }
        if ( it->rightMaster() == rank )
        {
          int node = it->rightNode();
          // std::cout << "[" << objs->rank() << "]: ";
          // std::cout << "edge(" << node << ") = " << it->leftNode() << " " << it->leftMaster() << std::endl;
          int i=0;
          for (;i<num_obj;++i)
            if ((int)globalID[i] == node) break;
          alugrid_assert ( i<num_obj );
          alugrid_assert (it->leftMaster() >= 0);
          alugrid_assert (it->weight() >= 0);
          ++numEdges[i];
          objs->edges()[i].push_back( std::make_pair(it,false) );
        }
      }
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
          alugrid_assert ( nborProc[k] >= 0 );
          if (wgt_dim==1)
          {
            ewgts[k]=objs->edgeWeight(j,l);
            alugrid_assert ( ewgts[k] >= 0 );
          }
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
      alugrid_assert ( objs );

      if ( (sizeGID != 1) || (sizeLID != 1) || (num_dim != dimension))
      {
        *ierr = ZOLTAN_FATAL;
        return;
      }

      *ierr = ZOLTAN_OK;

      ldb_vertex_map_t& vertexMap = objs->vertexMap();

      int idx = 0;
      typedef typename ldb_vertex_map_t :: iterator iterator ;
      double coord[ 3 ];
      const iterator end = vertexMap.end();
      for ( iterator it = vertexMap.begin(); it != end; ++ it )
      {
        (*it).first.computeBaryCenter( coord );
        for( int d=0; d<dimension; ++d, ++idx )
          geom_vec[ idx ] = coord[ d ];
      }
    }
  };
#endif // #if HAVE_ZOLTAN

  enum method_t {HSFC, PHG, PARMETIS};
  template< class ldb_vertex_map_t, class ldb_edge_set_t, class ldb_connect_set_t >
  bool CALL_Zoltan_LB_Partition( method_t method,
                                 MpAccessGlobal &mpa,
                                 ldb_vertex_map_t& vertexMap,
                                 ldb_edge_set_t& edgeSet,
                                 ldb_connect_set_t& connect,
                                 const double givenTolerance,
                                 const bool verbose )
  {
#if HAVE_ZOLTAN && HAVE_MPI
    using ::ALUGrid::MpAccessMPI;

    MpAccessMPI* mpaMPI = dynamic_cast<MpAccessMPI *> (&mpa);
    if( mpaMPI == 0 )
    {
      std::cerr << "ERROR: wrong mpAccess object, couldn't convert to MpAccessMPI!! in: " << __FILE__ << " line : " << __LINE__ << std::endl;
      std::abort();
    }

    // get communincator (see mpAccess_MPI.cc
    MPI_Comm comm = mpaMPI->communicator();
    int rank = mpaMPI->myrank();

    typedef ObjectCollection< ldb_vertex_map_t, ldb_edge_set_t > ObjectCollectionType;

    ObjectCollectionType objects( rank, vertexMap, edgeSet );
    std::unique_ptr< Zoltan > zz( new Zoltan( comm ) );
    alugrid_assert ( zz );

    // General parameters
    const char* debug = ( verbose ) ? "1" : "0";
    zz->Set_Param( "DEBUG_LEVEL", debug );
    zz->Set_Param( "OBJ_WEIGHT_DIM", "1");
    zz->Set_Param( "NUM_GID_ENTRIES", "1");
    zz->Set_Param( "NUM_LID_ENTRIES", "1");
    zz->Set_Param( "RETURN_LISTS", "ALL");

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
      zz->Set_Param( "LB_APPROACH", "REPARTITION");
      // zz->Set_Param( "LB_APPROACH","PARTITION"); // give an error in PARMETIS with an
      //       empty partitioning - no idea why...
      // zz->Set_Param( "LB_APPROACH", "REFINE");  // gives bad loadbalance with
      //       PARMETOS and the rest of the setting - no idea why
      zz->Set_Param( "EDGE_WEIGHT_DIM","1");
      zz->Set_Param( "GRAPH_SYMMETRIZE","NONE" );
      // zz->Set_Param( "GRAPH_SYMMETRIZE","TRANSPOSE");
      zz->Set_Param( "GRAPH_SYM_WEIGHT","MAX");
      // zz->Set_Param( "GRAPH_BUILD_TYPE","FAST_NO_DUP");
#if HAVE_PARMETIS
      if (method == PARMETIS)
        zz->Set_Param( "GRAPH_PACKAGE","PARMETIS");
#elif defined(HAVE_SCOTCH)
      zz->Set_Param( "GRAPH_PACKAGE","SCOTCH");
#endif
      zz->Set_Param( "CHECK_GRAPH", "0");
      zz->Set_Param( "PHG_EDGE_SIZE_THRESHOLD", ".25");
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
        alugrid_assert ( vertex != vertexMap.end () );
        (*vertex).second = exportProcs[ i ];
      }

      const iterator iEnd = vertexMap.end ();
      const int myrank = mpaMPI->myrank();
      for ( iterator i = vertexMap.begin (); i != iEnd; ++i )
      {
        int& moveTo = i->second;
        // insert and also set partition number new (including own number)
        if ( moveTo == -1 ) moveTo = myrank ;
        connect.insert( MpAccessLocal::sendRank( moveTo ) );
      }

      // insert also process number that I will receive objects from
      for (int i=0; i < numImport; ++i)
      {
        connect.insert( MpAccessLocal::recvRank( importProcs[ i ] ) );
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
    zz.reset();

    // return true if partition changed
    return (changes > 0);
#else
    std::cerr << "ERROR: Zoltan library not found, cannot use Zoltan partitioning! " << std::endl;
    std::abort();
    return false ;
#endif // #if HAVE_ZOLTAN && HAVE_MPI
  } // CALL_Zoltan_LB_Partition

} // namespace ALUGridZoltan

#endif // #ifndef ALUGRID_ZOLTAN_H_INCLUDED
