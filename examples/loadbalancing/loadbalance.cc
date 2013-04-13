#ifndef ZOLTANINTERFACE_HH 
#define ZOLTANINTERFACE_HH

#if HAVE_ZOLTAN 
#include "loadbalance.hh"

template< class Grid >
void ZoltanLoadBalanceHandle<Grid>::
write_hypergraph(int myRank, int numProcs, const GridType &grid)
{
  int myRank = grid.comm.rank();
  int numProcs = grid.comm.size();

  static const Dune::PartitionIteratorType partition = Dune::Interior_Partition;
  typedef typename Grid::LevelGridView GridView;
  const GridView &gridView = grid.levelView(0);
  typedef typename Codim< 0 >::template Partition< partition >::Iterator Iterator;
  typedef typename Codim< 0 >::Entity Entity;
  typedef typename Entity::EntityPointer EntityPointer;

  typedef typename GridView::IntersectionIterator IntersectionIterator;
  typedef typename IntersectionIterator::Intersection Intersection;

  int tempNumMyVertices = gridView.size(0);
  ZOLTAN_ID_TYPE *tempVtxGID = (ZOLTAN_ID_TYPE *)malloc(sizeof(ZOLTAN_ID_TYPE) * NUM_GID_ENTRIES * tempNumMyVertices);
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
	  gIdType bla = globalIdSet.id(entity);
	  std::vector<int> elementGID(NUM_GID_ENTRIES);
	  bla.getKey().extractKey(elementGID);
	  //elementGID[0] = entity.impl().macroID();

	  for (int i=0; i<NUM_GID_ENTRIES; ++i)
	  {
	    tempVtxGID[element_count*NUM_GID_ENTRIES + i] = (ZOLTAN_ID_TYPE)elementGID[i] + 1;   // global identifier of element    ADD ONE BECAUSE WE START COUNTING AT 0 AND ZOLTAN DOES NOT LIKE IT
	    tempEdgeGID[element_count*NUM_GID_ENTRIES + i] = (ZOLTAN_ID_TYPE)elementGID[i] + 1;  // global identifier of hyperedge
	    tempNborGID.push_back((ZOLTAN_ID_TYPE)elementGID[i] + 1);  // the element is a member of the hyperedge
  	}

	  //// Find if element is candidate for user-defined partitioning
	  typename Entity::Geometry::GlobalCoordinate c = entity.geometry().center();
	  double r=abs(std::complex<double>(c[0],c[1]));
	  if (r < 0.5)
	  {
	    for (int i=0; i<NUM_GID_ENTRIES; ++i)
	    {
		    fixedElmtVector.push_back((ZOLTAN_ID_TYPE)elementGID[i]+1);
	    }
	    fixedProcVector.push_back(0);
	  }
	  //// END: Find if element is candidate for user-defined partitioning


    const IntersectionIterator iend = gridView.iend( entity );
	  int num_of_neighbors = 0;
    for( IntersectionIterator iit = gridView.ibegin( entity ); iit != iend; ++iit )
    {
      const Intersection &intersection = *iit;
      if( intersection.neighbor() )
	    {
        const EntityPointer pOutside = intersection.outside();
		    const Entity &neighbor = *pOutside;
		    gIdType blah = globalIdSet.id(neighbor);
		    std::vector<int> globalID(NUM_GID_ENTRIES);
		    blah.getKey().extractKey(globalID);
		    // globalID[0] = neighbor.impl().macroID();

		    for (int i=0; i<NUM_GID_ENTRIES; ++i)
		    {
		      tempNborGID.push_back((ZOLTAN_ID_TYPE)globalID[i] + 1);
		    }

		    num_of_neighbors++;
	    }

    }
	  tempNborIndex[element_count+1] = tempNborIndex[element_count] + (1+num_of_neighbors); // plus one because not only neighbors are written, but also entity itself

	  element_count++;
  }

  hg->numMyVertices = element_count;    // How many global elements there are
  hg->vtxGID = (ZOLTAN_ID_TYPE *)malloc(sizeof(ZOLTAN_ID_TYPE) * NUM_GID_ENTRIES * element_count);
  std::copy(tempVtxGID, tempVtxGID+element_count*NUM_GID_ENTRIES, hg->vtxGID);
  hg->numMyHEdges = element_count;	   // We have the same amount of Hyperedges
  hg->edgeGID = (ZOLTAN_ID_TYPE *)malloc(sizeof(ZOLTAN_ID_TYPE) * NUM_GID_ENTRIES * element_count);
  std::copy(tempEdgeGID, tempEdgeGID+element_count*NUM_GID_ENTRIES, hg->edgeGID);
  hg->nborIndex = (int *)malloc(sizeof(int) * (element_count + 1));
  std::copy(tempNborIndex, tempNborIndex+element_count + 1, hg->nborIndex);

  hg->numAllNbors = tempNborGID.size()/NUM_GID_ENTRIES;
  hg->nborGID = (ZOLTAN_ID_TYPE *)malloc(sizeof(ZOLTAN_ID_TYPE) * tempNborGID.size());
  std::copy(tempNborGID.begin(), tempNborGID.end(),hg->nborGID);

  ///////// WRITE THE FIXED ELEMENTS INTO THE PROVIDED STRUCTURE
  fixed_elmts.fixed_GID.resize(fixedElmtVector.size());
  std::copy(fixedElmtVector.begin(), fixedElmtVector.end(), fixed_elmts.fixed_GID.begin());
  fixed_elmts.fixed_Process.resize(fixedProcVector.size());
  std::copy(fixedProcVector.begin(), fixedProcVector.end(), fixed_elmts.fixed_Process.begin());
  fixed_elmts.fixed_entities = fixed_elmts.fixed_Process.size();
}


/*****************************************************************************/
/********** Define, which elements have to be on which process ***************/
/*****************************************************************************/

template< class Grid >
static int ZoltanLoadBalanceHandle<Grid>::
get_num_fixed_obj(void *data, int *ierr)
{
  return fixed_elmts.fixed_entities;
}
template< class Grid >
static void ZoltanLoadBalanceHandle<Grid>::
get_fixed_obj_list(void *data, int num_fixed_obj,
                   int num_gid_entries, ZOLTAN_ID_PTR fixed_gids, int *fixed_part, int *ierr)
{
  HGRAPH_DATA *graph = (HGRAPH_DATA *)data;
  *ierr = ZOLTAN_OK;

  for (int i=0; i<num_fixed_obj*num_gid_entries; i++)
  {
    fixed_gids[i] = fixed_elmts.fixed_GID[i];
    //printf("%d(%d): %d %d\n",NPROC,i,graph->vtxGID[i],fixed_part[i]);
  }

  for (int i=0; i<num_fixed_obj; i++)
  {
    fixed_part[i] = fixed_elmts.fixed_Process[i];
  }
}


template< class Grid >
static int ZoltanLoadBalanceHandle<Grid>::
get_number_of_vertices(void *data, int *ierr)
{
  HGRAPH_DATA *temphg = (HGRAPH_DATA *)data;
  *ierr = ZOLTAN_OK;
  return temphg->numMyVertices;
}

template< class Grid >
static void ZoltanLoadBalanceHandle<Grid>::
get_vertex_list(void *data, int sizeGID, int sizeLID,
                ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                int wgt_dim, float *obj_wgts, int *ierr)
{
  int i;

  HGRAPH_DATA *temphg= (HGRAPH_DATA *)data;
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
static void ZoltanLoadBalanceHandle<Grid>::
get_hypergraph_size(void *data, int *num_lists, int *num_nonzeroes,
                    int *format, int *ierr)
{
  HGRAPH_DATA *temphg = (HGRAPH_DATA *)data;
  *ierr = ZOLTAN_OK;

  *num_lists = temphg->numMyHEdges;
  *num_nonzeroes = temphg->numAllNbors;

  *format = ZOLTAN_COMPRESSED_EDGE;

  return;
}

template< class Grid >
static void ZoltanLoadBalanceHandle<Grid>::
get_hypergraph(void *data, int sizeGID, int num_edges, int num_nonzeroes,
               int format, ZOLTAN_ID_PTR edgeGID, int *vtxPtr,
               ZOLTAN_ID_PTR vtxGID, int *ierr)
{
  int i;

  HGRAPH_DATA *temphg = (HGRAPH_DATA *)data;
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

#endif 
