#ifndef __TRANS2BSMACRO_C__
#define __TRANS2BSMACRO_C__

static REAL_D * coords = NULL; 
static FILE * file = NULL; 
static int bound = 0;

static int map [N_VERTICES] = { 0, 3, 2 ,1 }; 

static int faces[4][3] = { { 3 , 1 , 2 },
                           { 3 , 2 , 0 },
                           { 3 , 0 , 1 },
                           { 2 , 0 , 1 }
                         };

/* fill coord vector */
static void fillCoords(const EL_INFO * elf)
{
  int k,j;
  assert(elf->level == 0);
  for(k=0; k<N_VERTICES; k++)
  {
    int dof = elf->el->dof[k][0];
    for(j=0; j<DIM_OF_WORLD; j++)
      coords[dof][j] = elf->coord[k][j];
  }
}

/* count boundary faces */
static void calcBound(const EL_INFO * elf)
{
  int k=0;
  assert(elf->level == 0);
  for(k=0; k<N_VERTICES; k++)
  {
    if(elf->neigh[k] == NULL)
    {
      bound++;
    }
  }
}

/* write boundary faces */
static void writeBound(const EL_INFO * elf)
{
  int k;
  DOF ** dof = elf->el->dof;  

  assert(elf->level == 0);
  
  for(k=0; k<N_VERTICES; k++)
  {
    if(elf->neigh[k] == NULL) 
    {
      fprintf(file,"%d 3 ",-ABS(elf->boundary[k]->bound));
      /* the three vertices */
      if(elf->orientation < 0)
      {
        fprintf(file,"%d ",dof[faces[k][2]][0]);
        fprintf(file,"%d ",dof[faces[k][1]][0]);
        fprintf(file,"%d ",dof[faces[k][0]][0]);
      }
      else 
      {
        fprintf(file,"%d ",dof[faces[k][0]][0]);
        fprintf(file,"%d ",dof[faces[k][1]][0]);
        fprintf(file,"%d ",dof[faces[k][2]][0]);
      }
      
      fprintf(file,"\n");
    }
  }
}

void trans2BSMacroGrid(MESH * mesh, const char * filename)
{
  int k;
  MACRO_EL * mel = NULL;

  file = fopen(filename,"w");
  assert(file);
  
  coords = MEM_ALLOC(mesh->n_vertices, REAL_D );
  assert(coords);
  /* calc coords */
  mesh_traverse(mesh,0,CALL_LEAF_EL|FILL_COORDS , fillCoords );

  fprintf(file,"!Tetraeder\n");
  fprintf(file,"%d\n",mesh->n_vertices);
  for(k=0; k<mesh->n_vertices; k++)
  {
    fprintf(file,"%e %e %e\n",coords[k][0],coords[k][1],coords[k][2]);
  }
  
  MEM_FREE(coords,mesh->n_vertices, REAL_D ); 
  coords = NULL;
  
  /************************************************/
  /* write Tetras */
  /************************************************/
  fprintf(file,"\n%d\n",mesh->n_macro_el);
  for( mel = mesh->first_macro_el; mel ; mel = mel->next )
  {
    EL * el = mel->el; 
    assert(el);
    
    /* if orientation is negative, then switch */
    if(mel->orientation < 0)
    {
      for(k=0; k<N_VERTICES; k++)
        fprintf(file,"%d ",el->dof[map[k]][0]);
    }
    else 
    {
      for(k=0; k<N_VERTICES; k++)
        fprintf(file,"%d ",el->dof[k][0]);
    }
    fprintf(file,"\n");
  }

  /************************************************/
  /* write Boundary */
  /************************************************/

  bound = 0;
  mesh_traverse(mesh,0,CALL_LEAF_EL|FILL_ANY , calcBound );
  fprintf(file,"\n%d\n",bound); 

  /* write boundary faces */
  mesh_traverse(mesh,0,CALL_LEAF_EL|FILL_ANY , writeBound );

  /* write default parallelisation id */
  fprintf(file,"\n");
  for(k=0; k<mesh->n_vertices; k++)
    fprintf(file,"%d -1\n",k);

  fclose ( file ); file = NULL;
  return;
}


#endif
