
static volatile char RCSId_main2d_cc [] = "$Id$";

#include <time.h>
#include <math.h>
#include <string.h>
#include <alloca.h>
#include <stdlib.h>
#include <malloc.h>

#include "grid.h"
#include "triang.h"
#include "handle.h"

#ifndef EPS
  #define EPS 1e-12
#endif

void draw_grid(int argc, char ** argv, Hmesh &hdl) {
  char header[500];
  Listwalkptr < Hmesh_basic::helement_t > walk(hdl) ;
  Listwalkptr < Hmesh_basic::hbndel_t > walkb(hdl) ;
  Listwalkptr < Vertex > walkv(hdl) ;

  sprintf(header,"#elements: %d, #vertices: %d, #belements: %d"
                ,walk->size(),walkv->size(),walkb->size());
  cerr << "INDEXSETS:" << endl;
  hdl.printIndex();
  Xdisplay disp(argc,argv,header,"Grid",
  	350.,450.,450.);
  hdl.draw(disp);
}

char ** g_argv;
int g_argc;
int main(int argc, char ** argv, char ** envp)
{
g_argc=argc;
g_argv=argv;
struct mallinfo prog_meminfo;
get_memory(prog_meminfo);
write_memory(cerr,"start of program");

{ // Main program
  double t;
  long unsigned int n;
  //Hmesh hdl("triang.git"); 
  //Hmesh hdl("triang.git",Refco::quart); 
  for (int k=0;k<3;k++)
    Hmesh hdl("triang.git",1,Refco::quart); 
  //cout << "Grid 0" << endl;
  //draw_grid(argc,argv,hdl);
  /* refine macrogrid */

  Hmesh hdl("triang.git",1,Refco::quart); 
  // Hmesh hdl("triang.git"); 
  draw_grid(argc,argv,hdl);
for (int k=0;k<1;k++) {
    for (int i=0;i<2;i++) //
  {
    {
      Listwalkptr < Hmesh_basic::helement_t > walk(hdl);
      for( walk->first() ; ! walk->done() ; walk->next()) {
	Element & tr = walk->getitem();
	tr.Refco_el::mark(Refco::ref);
      }
    }
    hdl.refine();
    //draw_grid(argc,argv,hdl);
  }
  //cout << "Grid 0" << endl;
  //draw_grid(argc,argv,hdl);
  for (int i=0;i<5;i++) // 3 
  {
    {
      Listwalkptr < Hmesh_basic::helement_t > walk(hdl);
      for( walk->first() ; ! walk->done() ; walk->next()) {
	Element & tr = walk->getitem();
	double bary[3]={1./3.,1./3.,1./3.};
	double p[2];
	tr.fromlocal(bary,p);
	p[0]+=0.6;
	p[1]-=0.366667;
	if (p[0]*p[0]+p[1]*p[1]<0.1)
	  tr.Refco_el::mark(Refco::ref);
      }
    }
    hdl.refine();
    draw_grid(argc,argv,hdl);
  }
  draw_grid(argc,argv,hdl);
  for (int i=0;i<2;i++) // 2 
  {
    {
      Listwalkptr < Hmesh_basic::helement_t > walk(hdl);
      for( walk->first() ; ! walk->done() ; walk->next()) {
	Element & tr = walk->getitem();
	double bary[3]={1./3.,1./3.,1./3.};
	double p[2];
	tr.fromlocal(bary,p);
	tr.Refco_el::mark(Refco::crs);
      }
    }
    hdl.coarse();
  } 
  draw_grid(argc,argv,hdl);

  for (int i=0;i<3;i++) // 3
  {
    {
      Listwalkptr < Hmesh_basic::helement_t > walk(hdl);
      for( walk->first() ; ! walk->done() ; walk->next()) {
	Element & tr = walk->getitem();
	double bary[3]={1./3.,1./3.,1./3.};
	double p[2];
	tr.fromlocal(bary,p);
	p[0]+=0.6;
	p[1]-=0.366667;
	if (p[0]*p[0]+p[1]*p[1]<0.1)
	  tr.Refco_el::mark(Refco::ref);
      }
    }
    hdl.refine();
  }
  draw_grid(argc,argv,hdl);
  hdl.storeGrid("checkpoint",0,0);
  Hmesh hdl2("checkpoint"); 
  draw_grid(argc,argv,hdl2);
  if (k==2) {
  for (int i=0;i<2;i++) 
  {
    {
      Listwalkptr < Hmesh_basic::helement_t > walk(hdl);
      for( walk->first() ; ! walk->done() ; walk->next()) {
	Element & tr = walk->getitem();
	double bary[3]={1./3.,1./3.,1./3.};
	double p[2];
	tr.fromlocal(bary,p);
	p[0]+=0.6;
	p[1]-=0.366667;
	if (p[0]*p[0]+p[1]*p[1]<0.1)
	  tr.Refco_el::mark(Refco::ref);
      }
    }
    hdl.refine();
  }
  draw_grid(argc,argv,hdl);
  }
  for (int i=0;i<15;i++) 
  {
    {
      Listwalkptr < Hmesh_basic::helement_t > walk(hdl);
      for( walk->first() ; ! walk->done() ; walk->next()) {
	Element & tr = walk->getitem();
	double bary[3]={1./3.,1./3./1./3.};
	double p[2];
	tr.fromlocal(bary,p);
	tr.Refco_el::mark(Refco::crs);
      }
    }
    hdl.coarse();
  }
    

  draw_grid(argc,argv,hdl);
  }
}
write_memory(cerr,"end of program");
write_memory_diff(cerr,"end of program",prog_meminfo);

  return 0;
}


