#if HAVE_ZOLTAN 
template< class Grid >
ZoltanLoadBalanceHandle<Grid>::
ZoltanLoadBalanceHandle(const Grid &grid)
: grid_( grid )
, globalIdSet_( grid.globalIdSet() )
, first_(true)
{
  zz_ = Zoltan_Create(MPI_COMM_WORLD);

  // General parameters
  Zoltan_Set_Param(zz_, "DEBUG_LEVEL", "1");
  Zoltan_Set_Param(zz_, "LB_METHOD", "HYPERGRAPH");   /* partitioning method */
  Zoltan_Set_Param(zz_, "HYPERGRAPH_PACKAGE", "PHG"); /* version of method */
  Zoltan_Set_Param(zz_, "NUM_GID_ENTRIES", "1");      /* global IDs are 1 integers */
  Zoltan_Set_Param(zz_, "NUM_LID_ENTRIES", "1");      /* local IDs are 1 integers */
  Zoltan_Set_Param(zz_, "RETURN_LISTS", "ALL");       /* export AND import lists */
  Zoltan_Set_Param(zz_, "OBJ_WEIGHT_DIM", "0");       /* use Zoltan default vertex weights */
  Zoltan_Set_Param(zz_, "EDGE_WEIGHT_DIM", "0");      /* use Zoltan default hyperedge weights */

  /* PHG parameters  - see the Zoltan User's Guide for many more
   * (The "REPARTITION" approach asks Zoltan to create a partitioning that is
   * better but is not too far from the current partitioning, rather than partitioning 
   * from scratch.  It may be faster but of lower quality that LB_APPROACH=PARTITION.)
  */
  Zoltan_Set_Param(zz_, "LB_APPROACH", "REPARTITION");
  Zoltan_Set_Param(zz_, "REMAP", "1");

  /* Application defined query functions */
  Zoltan_Set_Num_Obj_Fn(zz_, get_number_of_vertices, &hg_);
  Zoltan_Set_Obj_List_Fn(zz_, get_vertex_list, &hg_);
  Zoltan_Set_HG_Size_CS_Fn(zz_, get_hypergraph_size, &hg_);
  Zoltan_Set_HG_CS_Fn(zz_, get_hypergraph, &hg_);

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
 * It needs to setup the hypergraph and call the zoltan partition function
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
  int *tempNborIndex = (int *)malloc(sizeof(int) * (tempNumMyVertices + 1));
  tempNborIndex[0] = 0;

  std::vector<ZOLTAN_ID_TYPE> tempNborGID(0);
  std::vector<int> fixedProcVector(0), fixedElmtVector(0);

  unsigned int element_count = 0;
  const Iterator &end = gridView.template end< 0, partition >();
  for( Iterator it = gridView.template begin< 0, partition >(); it != end; ++it )
  {
	  const Entity &entity = *it;
	  std::vector<int> elementGID(NUM_GID_ENTRIES);
    // use special ALU method that returns a pure integer tuple which is a
    // unique id on the macrolevel
	  // GIdType id = globalIdSet_.id(entity);
	  // id.getKey().extractKey(elementGID);
	  elementGID[0] = gridView.macroId(entity); //   entity.impl().macroID();

	  for (int i=0; i<NUM_GID_ENTRIES; ++i)
	  {
	    tempVtxGID[element_count*NUM_GID_ENTRIES + i] = (ZOLTAN_ID_TYPE)elementGID[i] + 1;   // global identifier of element    ADD ONE BECAUSE WE START COUNTING AT 0 AND ZOLTAN DOES NOT LIKE IT
	    tempEdgeGID[element_count*NUM_GID_ENTRIES + i] = (ZOLTAN_ID_TYPE)elementGID[i] + 1;  // global identifier of hyperedge
	    tempNborGID.push_back((ZOLTAN_ID_TYPE)elementGID[i] + 1);  // the element is a member of the hyperedge
  	}

	  // Find if element is candidate for user-defined partitioning:
    // we keep the center on one process...
	  typename Entity::Geometry::GlobalCoordinate c = entity.geometry().center();
    double vol = 4./double(this->grid_.comm().size()); // this is the size of the domain for proz zero
    double R2 = vol/M_PI*0.5; // correct size for the central circle (half the optimal size)
	  if ( c[0]*c[0]+c[1]*c[1] < R2 )
	  {
	    for (int i=0; i<NUM_GID_ENTRIES; ++i)
	    {
		    fixedElmtVector.push_back((ZOLTAN_ID_TYPE)elementGID[i]+1);
	    }
	    fixedProcVector.push_back(0);
	  }

    // now setup the edges
    const IntersectionIterator iend = gridView.iend( entity );
	  int num_of_neighbors = 0;
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
		    // GIdType id = globalIdSet_.id(neighbor);
		    // id.getKey().extractKey(neighborGID);
	      neighborGID[0] = gridView.macroId(neighbor); //   entity.impl().macroID();
        int weight = gridView.weight( iit );
        if (grid_.comm().rank() == 0 && gridView.master(neighbor)!=0)
          std::cout << weight << " " << gridView.master(neighbor) << std::endl;

		    for (int i=0; i<NUM_GID_ENTRIES; ++i)
		    {
		      tempNborGID.push_back((ZOLTAN_ID_TYPE)neighborGID[i] + 1);
		    }

		    num_of_neighbors++;
	    }

    }
    // add one because not only neighbors are used in graph, but also entity itself
	  tempNborIndex[element_count+1] = tempNborIndex[element_count] + (1+num_of_neighbors); 

	  element_count++;
  }

  // now copy into hypergraph structure
  hg_.numMyVertices = element_count;    // How many global elements there are
  hg_.vtxGID = (ZOLTAN_ID_TYPE *)malloc(sizeof(ZOLTAN_ID_TYPE) * NUM_GID_ENTRIES * element_count);
  std::copy(tempVtxGID, tempVtxGID+element_count*NUM_GID_ENTRIES, hg_.vtxGID);
  hg_.numMyHEdges = element_count;	   // We have the same amount of Hyperedges
  hg_.edgeGID = (ZOLTAN_ID_TYPE *)malloc(sizeof(ZOLTAN_ID_TYPE) * NUM_GID_ENTRIES * element_count);
  std::copy(tempEdgeGID, tempEdgeGID+element_count*NUM_GID_ENTRIES, hg_.edgeGID);
  hg_.nborIndex = (int *)malloc(sizeof(int) * (element_count + 1));
  std::copy(tempNborIndex, tempNborIndex+element_count + 1, hg_.nborIndex);

  hg_.numAllNbors = tempNborGID.size()/NUM_GID_ENTRIES;
  hg_.nborGID = (ZOLTAN_ID_TYPE *)malloc(sizeof(ZOLTAN_ID_TYPE) * tempNborGID.size());
  std::copy(tempNborGID.begin(), tempNborGID.end(),hg_.nborGID);

  ///////// WRITE THE FIXED ELEMENTS INTO THE PROVIDED STRUCTURE
  hg_.fixed_elmts.fixed_GID.resize(fixedElmtVector.size());
  std::copy(fixedElmtVector.begin(), fixedElmtVector.end(), hg_.fixed_elmts.fixed_GID.begin());
  hg_.fixed_elmts.fixed_Process.resize(fixedProcVector.size());
  std::copy(fixedProcVector.begin(), fixedProcVector.end(), hg_.fixed_elmts.fixed_Process.begin());
  hg_.fixed_elmts.fixed_entities = hg_.fixed_elmts.fixed_Process.size();

  free(tempNborIndex);
  free(tempEdgeGID);
  free(tempVtxGID);
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
  }
}

template< class Grid >
void ZoltanLoadBalanceHandle<Grid>::
get_hypergraph_size(void *data, int *num_lists, int *num_nonzeroes,
                    int *format, int *ierr)
{
  HGraphData *temphg = (HGraphData *)data;
  *ierr = ZOLTAN_OK;

  *num_lists = temphg->numMyHEdges;
  *num_nonzeroes = temphg->numAllNbors;

  *format = ZOLTAN_COMPRESSED_EDGE;

  return;
}

template< class Grid >
void ZoltanLoadBalanceHandle<Grid>::
get_hypergraph(void *data, int sizeGID, int num_edges, int num_nonzeroes,
               int format, ZOLTAN_ID_PTR edgeGID, int *vtxPtr,
               ZOLTAN_ID_PTR vtxGID, int *ierr)
{
  int i;

  HGraphData *temphg = (HGraphData *)data;
  *ierr = ZOLTAN_OK;

  if ( (num_edges != temphg->numMyHEdges) || (num_nonzeroes != temphg->numAllNbors) ||
       (format != ZOLTAN_COMPRESSED_EDGE)) {
    *ierr = ZOLTAN_FATAL;
    return;
  }

  for (i=0; i < num_edges*sizeGID; i++){
    edgeGID[i] = temphg->edgeGID[i];
  }

  for (i=0; i < num_edges; i++){
    vtxPtr[i] = temphg->nborIndex[i];
  }

  for (i=0; i < num_nonzeroes*sizeGID; i++){
    vtxGID[i] = temphg->nborGID[i];
  }

  return;
}

#endif // if HAVE_ZOLTAN
