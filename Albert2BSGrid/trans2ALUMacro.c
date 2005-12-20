#ifndef __TRANS2BSMACRO_C__
#define __TRANS2BSMACRO_C__

#include <assert.h>

/* face identifier type for periodic boundaries */
static const int periodic = -20;
static const double epsilon = 1e-12;

static REAL_D * coords = NULL; 
static FILE * file = NULL; 
static int bound = 0;

static const int map [N_VERTICES] = { 0, 3, 2 ,1 }; 

static const int faces[4][3] = { { 3 , 1 , 2 },
                                 { 3 , 2 , 0 },
                                 { 3 , 0 , 1 },
                                 { 2 , 1 , 0 }
                               };

typedef double knots_t[3]; 
typedef int    pface_t[3];
typedef int    perface_t[6];

/* maximal number of periodic faces allowd */
#define  maxPeriodicFaces 100000

static pface_t   * pface = NULL; 
static knots_t   * knots = NULL;
//static perface_t * periodicface = NULL;
static int periodicface[maxPeriodicFaces][6];
static int perfaces = 0;
static int anzperifaces = 0;

static int refinedlevel = 0;

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
      int boundval = -ABS(elf->boundary[k]->bound);
      if(boundval != periodic) bound++;
    }
  }
}

/* write boundary faces, all but periodic  */
static void writeBound(const EL_INFO * elf)
{
  int k;
  DOF ** dof = elf->el->dof;  

  assert(elf->level == 0);
  
  for(k=0; k<N_VERTICES; k++)
  {
    if(elf->neigh[k] == NULL) 
    {
      int boundval = -ABS(elf->boundary[k]->bound);
      if(boundval != periodic)
      {
        fprintf(file,"%d 3 ",boundval);
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
}

/* locate bnd knots */
static void buildKnots(const EL_INFO * elf)
{
  int k,j; 
  DOF ** dof = elf->el->dof;  
  int mapf[3] = {0,1,2};
  assert(elf->level == 0);

  if(elf->orientation < 0)
  {
    mapf[0] = 2;
    mapf[1] = 1;
    mapf[2] = 0;
  }
  
  for(k=0; k<N_VERTICES; k++)
  {
    if(elf->neigh[k] == NULL) 
    {
      int boundval = -ABS(elf->boundary[k]->bound);
      if(boundval == periodic)
      {
        for(j=0; j<3; j++)
        {
          knots[dof[faces[k][mapf[0]]][0]][j] = elf->coord[faces[k][mapf[0]]][j];
          knots[dof[faces[k][mapf[1]]][0]][j] = elf->coord[faces[k][mapf[1]]][j];
          knots[dof[faces[k][mapf[2]]][0]][j] = elf->coord[faces[k][mapf[2]]][j];
        }
        pface[perfaces][0] = dof[faces[k][mapf[0]]][0];
        pface[perfaces][1] = dof[faces[k][mapf[1]]][0];
        pface[perfaces][2] = dof[faces[k][mapf[2]]][0];
        perfaces++;
      }
    }
  }
}

/* vergleich der flaechen */
static int actface = 0;
int vergleicheGesicht(int g1, int g2 )
{
  int erg[3][3] = { {0,0,0}, {0,0,0},{0,0,0} };
  int knot[3]   = { -1,-1,-1 };
  int gleich = 0;
  int l,j,i;
  int * v1 = pface[g1];
  int * v2 = pface[g2];
  int mapf[3] = {2,1,0};

  for(j=0; j<3; j++)
  {
    int chekit = 1; 

    for(l=0; l<3; l++)
    {
      for(i=0; i<3; i++)
      {
        erg[l][i] = 0;
        if(ABS(knots[v1[l]][i] - knots[v2[mapf[(l+j)%3]]][i] ) < epsilon)
        {
          erg[l][i] = 1;      
        }
      }
    }
    
    chekit = 1; 
    for(l=0; l<3; l++)
    {
      for(i=0; i<3; i++) 
      if( (erg[l][i] != erg[(l+1)%3][i]) || (erg[l][i] < 0) ) 
        chekit = 0;
      if( erg[l][0] + erg[l][1] + erg[l][2] != 2 )
        chekit = 0;
    }
    if(chekit ==1)
    {
      for(l=0; l<3; l++)
      {
        knot[l] = (l+j)%3;
      }
      break;
    }
  }
  
  /* wenn offset gleich ==> gleich */
  if((knot[0] >= 0 ) && (knot[1] >= 0 ) && (knot[2] >= 0 ) )
  {
    gleich = 1;
    for(i=0; i<3; i++) periodicface[actface][i] = v1[i];
    for(i=0; i<3; i++) periodicface[actface][i+3] = v2[mapf[knot[i]]];
    actface++;

  }
  return gleich;
}

/* locate bnd knots */
static void locateKnots()
{
  int k=0, m=0;
  int visited[maxPeriodicFaces]; 
  
  actface = 0;
  
  assert(knots);
  assert(pface);

  assert(perfaces < maxPeriodicFaces);
  for(k=0; k<perfaces; k++) visited[k] = 0;

  assert(perfaces % 2 == 0);
  anzperifaces = (int) (perfaces/2);
 
  /*
   * for some reason this doesnt work , donno why 
    periodicface = MEM_ALLOC(anzperifaces, perface_t );
    assert(periodicface);
  */
 
  for(k=0; k<perfaces; k++)
  {
    if(visited[k] == 0)
    {
      for(m=0; m<perfaces; m++)
      {
        if((visited[m] == 0) && (k != m)) 
        {
          if(vergleicheGesicht(k,m)) 
          {
            visited[m] = 1; 
            visited[k] = 1;
            break;
          }
        }
      }
    }
  }
}

void trans2BSMacroGrid(MESH * mesh, const char * filename, int level)
{
  int k;
  MACRO_EL * mel = NULL;

  refinedlevel = level;

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

  knots = MEM_ALLOC(mesh->n_vertices, knots_t );
  assert(knots);
  
  pface = MEM_ALLOC(mesh->n_vertices, pface_t );
  assert(pface);

  perfaces = 0;
  
  /* generate list of periodic faces */
  mesh_traverse(mesh,0,CALL_LEAF_EL|FILL_ANY , buildKnots );
  printf("found %d periodic faces! \n",perfaces);

  /* locate periodic boundaries */
  locateKnots();
  
  fprintf(file,"\n%d\n",bound+anzperifaces); 

  /* write boundary faces */
  mesh_traverse(mesh,0,CALL_LEAF_EL|FILL_ANY , writeBound );

  for(k=0; k<anzperifaces; k++)
  {
    fprintf(file,"%d 6 %d %d %d %d %d %d\n",periodic,
          periodicface[k][0],periodicface[k][1],periodicface[k][2],
          periodicface[k][3],periodicface[k][4],periodicface[k][5]);
  }

  /*
    MEM_FREE(knots,mesh->n_vertices, knots_t ); 
    MEM_FREE(pface,mesh->n_vertices, pface_t ); 
  */
    
  /* write default parallelisation id */
  fprintf(file,"\n");
  for(k=0; k<mesh->n_vertices; k++)
    fprintf(file,"%d -1\n",k);

  fclose ( file ); file = NULL;
  return;
}

/* write boundary faces, all but periodic  */
static void changeBnd(const EL_INFO * elf)
{
  int k,i;
  assert(elf->level == 0);
  
  for(k=0; k<N_VERTICES; k++)
  {
    if(elf->neigh[k] == NULL) 
    {
      if(elf->boundary[k]->bound == -1)
      {
        double sum = 0.0;
        BOUNDARY * bnd = ((BOUNDARY *) elf->boundary[k]);
        for(i=0; i<3; i++)
        {
          sum += elf->coord[(k+i+1)%4][2];
        }

        printf("sum on bnd 1 was %f \n",sum);
        
        //if(sum <= 0.0) bnd->bound=-91;
        if(sum >= 3.0) 
        { 
          printf("Change bnd type \n");
          bnd->bound=-4;
        }
      }
    }
  }
}

void changeBoundary(MESH * mesh)
{
  /* write boundary faces */
  mesh_traverse(mesh,0,CALL_LEAF_EL|FILL_ANY , changeBnd );
}

#endif
