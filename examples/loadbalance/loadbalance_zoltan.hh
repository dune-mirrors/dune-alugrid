#if HAVE_ZOLTAN 
template< class Grid >
ZoltanLoadBalanceHandle<Grid>::
ZoltanLoadBalanceHandle(const Grid &grid)
: grid_( grid )
, globalIdSet_( grid.globalIdSet() )
, first_(true)
, ldbUnder_(0), ldbOver_(1.2)
, fix_bnd_(true)
{
  zz_ = Zoltan_Create(MPI_COMM_WORLD);

  // General parameters
  Zoltan_Set_Param(zz_, "DEBUG_LEVEL", "0");
  if (!fix_bnd_) // fixing element requires using hypergraph partitioning (which is perhaps better anyway?)
    Zoltan_Set_Param(zz_, "LB_METHOD", "HYPERGRAPH");        /* partitioning method */
  else
    Zoltan_Set_Param(zz_, "LB_METHOD", "HYPERGRAPH");        /* partitioning method */
  Zoltan_Set_Param(zz_, "HYPERGRAPH_PACKAGE", "PHG"); /* version of method */
  Zoltan_Set_Param(zz_, "NUM_GID_ENTRIES", "1");      /* global IDs are 1 integers */
  Zoltan_Set_Param(zz_, "NUM_LID_ENTRIES", "1");      /* local IDs are 1 integers */
  Zoltan_Set_Param(zz_, "RETURN_LISTS", "ALL");       /* export AND import lists */
  Zoltan_Set_Param(zz_, "OBJ_WEIGHT_DIM", "1");       /* provide a weight for graph nodes */
  Zoltan_Set_Param(zz_, "EDGE_WEIGHT_DIM", "1");      /* provide a weight for graph edge */
  Zoltan_Set_Param(zz_, "GRAPH_SYM_WEIGHT","MAX");
  Zoltan_Set_Param(zz_, "GRAPH_SYMMETRIZE","NONE" );
  Zoltan_Set_Param(zz_, "PHG_EDGE_SIZE_THRESHOLD", ".25");
  Zoltan_Set_Param(zz_, "CHECK_HYPERGRAPH", "0"); 
  Zoltan_Set_Param(zz_, "CHECK_GRAPH", "0"); 

  /* PHG parameters  - see the Zoltan User's Guide for many more
  */
  Zoltan_Set_Param(zz_, "LB_APPROACH", "REPARTITION");
  Zoltan_Set_Param(zz_, "REMAP", "1");
  Zoltan_Set_Param(zz_, "IMBALANCE_TOL", "1.05" );

  /* Application defined query functions */
  Zoltan_Set_Num_Obj_Fn(zz_, get_number_of_vertices, &hg_);
  Zoltan_Set_Obj_List_Fn(zz_, get_vertex_list, &hg_);
  Zoltan_Set_Num_Edges_Multi_Fn(zz_, get_num_edges_list, &hg_);
  Zoltan_Set_Edge_List_Multi_Fn(zz_, get_edge_list, &hg_);

  /* Register fixed object callback functions */
  if (Zoltan_Set_Fn(zz_, ZOLTAN_NUM_FIXED_OBJ_FN_TYPE,
        (void (*)()) get_num_fixed_obj,
        (void *) &hg_) == ZOLTAN_FATAL) {
    return;
  }
  if (Zoltan_Set_Fn(zz_, ZOLTAN_FIXED_OBJ_LIST_FN_TYPE,
        (void (*)()) get_fixed_obj_list,
        (void *) &hg_) == ZOLTAN_FATAL) {
    return;
  }

  /******************************************************************
  ** Zoltan can now partition the vertices of hypergraph.
  ** In this simple example, we assume the number of partitions is
  ** equal to the number of processes.  Process rank 0 will own
  ** partition 0, process rank 1 will own partition 1, and so on.
  ******************************************************************/

  // read config file if avaialble
  std::ifstream in( "alugrid.cfg" );
  if( in )
  {
    in >> ldbUnder_;
    in >> ldbOver_;
  }
}
template <class Grid>
ZoltanLoadBalanceHandle<Grid>::
~ZoltanLoadBalanceHandle()
{
  if (!first_)
  {
    Zoltan_LB_Free_Part(&(new_partitioning_.importGlobalGids), 
                 &(new_partitioning_.importLocalGids), 
                 &(new_partitioning_.importProcs), 
                 &(new_partitioning_.importToPart) );
    Zoltan_LB_Free_Part(&(new_partitioning_.exportGlobalGids), 
                 &(new_partitioning_.exportLocalGids), 
                 &(new_partitioning_.exportProcs), 
                 &(new_partitioning_.exportToPart) );
  }
  Zoltan_Destroy(&zz_);
}

/********************************************************************************
 * This method is called in the repartition callback function.
 * It needs to setup the graph/hypergraph and call the zoltan partition function
 ********************************************************************************/
template< class Grid >
void ZoltanLoadBalanceHandle<Grid>::
generateHypergraph()
{
  hg_.freeMemory();
  // setup the hypergraph by iterating over the macro level 
  // (ALU can only partition on the macro level)
  const Dune::PartitionIteratorType partition = Dune::Interior_Partition;
  typedef typename Grid::MacroGridView GridView;
  const GridView &gridView = grid_.macroView();
  typedef typename GridView::template Codim< 0 >::template Partition< partition >::Iterator Iterator;
  typedef typename Codim< 0 >::Entity Entity;
  typedef typename Entity::EntityPointer EntityPointer;

  typedef typename GridView::IntersectionIterator IntersectionIterator;
  typedef typename IntersectionIterator::Intersection Intersection;

  int tempNumMyVertices = gridView.size(0);
  ZOLTAN_ID_TYPE *tempVtxGID  = (ZOLTAN_ID_TYPE *)malloc(sizeof(ZOLTAN_ID_TYPE) * NUM_GID_ENTRIES * tempNumMyVertices);
  ZOLTAN_ID_TYPE *tempEdgeGID = (ZOLTAN_ID_TYPE *)malloc(sizeof(ZOLTAN_ID_TYPE) * NUM_GID_ENTRIES * tempNumMyVertices);
  float* tempVtxWeight = (float*)malloc(sizeof(float)*tempNumMyVertices);

  std::vector<ZOLTAN_ID_TYPE> tempNborGID(0);
  std::vector<float> tempNborWeight(0);
  std::vector<int> tempNborProc(0);
  std::vector<int> fixedProcVector(0), fixedElmtVector(0);
  int *tempNborIndex = (int *)malloc(sizeof(int) * (tempNumMyVertices + 1));
  tempNborIndex[0] = 0;

  unsigned int element_count = 0;
  const Iterator &end = gridView.template end< 0, partition >();
  for( Iterator it = gridView.template begin< 0, partition >(); it != end; ++it )
  {
	  const Entity &entity = *it;
	  std::vector<int> elementGID(NUM_GID_ENTRIES);
    // use special ALU method that returns a pure integer tuple which is a
    // unique id on the macrolevel
	  elementGID[0] = gridView.macroId(entity); 

	  for (int i=0; i<NUM_GID_ENTRIES; ++i)
	  {
	    tempVtxGID[element_count*NUM_GID_ENTRIES + i] = (ZOLTAN_ID_TYPE)elementGID[i] + 1;   // global identifier of element    ADD ONE BECAUSE WE START COUNTING AT 0 AND ZOLTAN DOES NOT LIKE IT
	    tempEdgeGID[element_count*NUM_GID_ENTRIES + i] = (ZOLTAN_ID_TYPE)elementGID[i] + 1;  // global identifier of hyperedge
  	}
    // get weight associated with entity using ALU specific function
    tempVtxWeight[element_count] = gridView.weight(entity);

    // now setup the edges
    const IntersectionIterator iend = gridView.iend( entity );
	  int num_of_neighbors = 0;
    float weight = 0;
    for( IntersectionIterator iit = gridView.ibegin( entity ); iit != iend; ++iit )
    {
      const Intersection &intersection = *iit;
      if( intersection.neighbor() )
	    {
        const EntityPointer pOutside = intersection.outside();
		    const Entity &neighbor = *pOutside;
		    std::vector<int> neighborGID(NUM_GID_ENTRIES);
        // use special ALU method that returns a pure integer tuple which is a
        // unique id on the macrolevel
	      neighborGID[0] = gridView.macroId(neighbor); 
        // use the alu specific weight function between neighboring elements
        weight += gridView.weight( iit );

		    for (int i=0; i<NUM_GID_ENTRIES; ++i)
		    {
		      tempNborGID.push_back((ZOLTAN_ID_TYPE)neighborGID[i] + 1);
		    }
        tempNborWeight.push_back( gridView.weight( iit ) );
        tempNborProc.push_back( gridView.master( neighbor ) );

		    num_of_neighbors++;
	    }
      else
      {
        // Find if element is candidate for user-defined partitioning:
        // we keep the left boundary on process 0
        if ( intersection.centerUnitOuterNormal()[0]<-0.9)
        {
          for (int i=0; i<NUM_GID_ENTRIES; ++i)
          {
            fixedElmtVector.push_back((ZOLTAN_ID_TYPE)elementGID[i]+1);
          }
          fixedProcVector.push_back(0);
        }
      }

    }
    // add one because not only neighbors are used in graph, but also entity itself
	  tempNborIndex[element_count+1] = tempNborIndex[element_count] + num_of_neighbors; 

	  element_count++;
  }

  assert( tempNumMyVertices >= element_count );

  // now copy into hypergraph structure
  hg_.numMyVertices = element_count;    // How many global elements there are
  hg_.numMyHEdges = element_count;	   // We have the same amount of Hyperedges
  std::swap(tempVtxGID, hg_.vtxGID);
  std::swap(tempVtxWeight, hg_.vtxWEIGHT);
  std::swap(tempEdgeGID, hg_.edgeGID);
  std::swap(tempNborIndex, hg_.nborIndex);

  hg_.numAllNbors = tempNborGID.size()/NUM_GID_ENTRIES;
  hg_.nborGID = (ZOLTAN_ID_TYPE *)malloc(sizeof(ZOLTAN_ID_TYPE) * tempNborGID.size());
  std::copy(tempNborGID.begin(), tempNborGID.end(),hg_.nborGID);
  hg_.nborWEIGHT = (float *)malloc(sizeof(float) * tempNborGID.size());
  std::copy(tempNborWeight.begin(), tempNborWeight.end(),hg_.nborWEIGHT);
  hg_.nborPROC = (int *)malloc(sizeof(int) * tempNborGID.size());
  std::copy(tempNborProc.begin(), tempNborProc.end(),hg_.nborPROC);

  ///////// WRITE THE FIXED ELEMENTS INTO THE PROVIDED STRUCTURE
  if (fix_bnd_) // fixing element requires using hypergraph partitioning (which is perhaps better anyway?)
  {
    hg_.fixed_elmts.fixed_GID.resize(fixedElmtVector.size());
    std::copy(fixedElmtVector.begin(), fixedElmtVector.end(), hg_.fixed_elmts.fixed_GID.begin());
    hg_.fixed_elmts.fixed_Process.resize(fixedProcVector.size());
    std::copy(fixedProcVector.begin(), fixedProcVector.end(), hg_.fixed_elmts.fixed_Process.begin());
    hg_.fixed_elmts.fixed_entities = hg_.fixed_elmts.fixed_Process.size();
  }
}


// implement the required zoltan callback functions
template< class Grid >
int ZoltanLoadBalanceHandle<Grid>::
get_num_fixed_obj(void *data, int *ierr)
{
  HGraphData *graph = (HGraphData *)data;
  return graph->fixed_elmts.fixed_entities;
}
template< class Grid >
void ZoltanLoadBalanceHandle<Grid>::
get_fixed_obj_list(void *data, int num_fixed_obj,
                   int num_gid_entries, ZOLTAN_ID_PTR fixed_gids, int *fixed_part, int *ierr)
{
  HGraphData *graph = (HGraphData *)data;
  *ierr = ZOLTAN_OK;

  for (int i=0; i<num_fixed_obj*num_gid_entries; i++)
  {
    fixed_gids[i] = graph->fixed_elmts.fixed_GID[i];
  }

  for (int i=0; i<num_fixed_obj; i++)
  {
    fixed_part[i] = graph->fixed_elmts.fixed_Process[i];
  }
}


template< class Grid >
int ZoltanLoadBalanceHandle<Grid>::
get_number_of_vertices(void *data, int *ierr)
{
  HGraphData *temphg = (HGraphData *)data;
  *ierr = ZOLTAN_OK;
  return temphg->numMyVertices;
}

template< class Grid >
void ZoltanLoadBalanceHandle<Grid>::
get_vertex_list(void *data, int sizeGID, int sizeLID,
                ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                int wgt_dim, float *obj_wgts, int *ierr)
{
  int i;

  HGraphData *temphg= (HGraphData *)data;
  *ierr = ZOLTAN_OK;

  for (i=0; i<temphg->numMyVertices*sizeGID; i++)
  {
    globalID[i] = temphg->vtxGID[i];
  }

  for (i=0; i<temphg->numMyVertices; i++)
  {
    localID[i] = i;
    if (wgt_dim == 1)
      obj_wgts[i] = temphg->vtxWEIGHT[i];
  }
}

template< class Grid >
void ZoltanLoadBalanceHandle<Grid>::
get_num_edges_list(void *data, int sizeGID, int sizeLID,
                   int num_obj,
                   ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                   int *numEdges, int *ierr)
{
  HGraphData *temphg = (HGraphData *)data;
  *ierr = ZOLTAN_OK;
  for (int i=0;i<num_obj;++i)
    numEdges[i] = temphg->nborIndex[i+1]-temphg->nborIndex[i]; 
}
template< class Grid >
void ZoltanLoadBalanceHandle<Grid>::
get_edge_list(void *data, int sizeGID, int sizeLID,
              int num_obj, ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
              int *num_edges,
              ZOLTAN_ID_PTR nborGID, int *nborProc,
              int wgt_dim, float *ewgts, int *ierr)
{
  HGraphData *temphg = (HGraphData *)data;
  *ierr = ZOLTAN_OK;
  int k=0;
  for (int i=0;i<num_obj;++i)
  {
    int l = temphg->nborIndex[i]; 
    for (int j=0;j<num_edges[i];++j)
    {
      nborGID[k]  = temphg->nborGID[l];
      nborProc[k] = temphg->nborPROC[l];
      if (wgt_dim==1)
        ewgts[k] = temphg->nborWEIGHT[l];
      ++l;
      ++k;
    }
  }
}
#endif // if HAVE_ZOLTAN
