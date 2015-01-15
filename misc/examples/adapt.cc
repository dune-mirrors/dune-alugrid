
#include <config.h>
#include <iostream>
#include <string>

// include serial part of ALUGrid
#include <dune/alugrid/grid.hh>

using namespace ALUGrid;
using namespace std;

// using namespace ALUGridSpace ;

typedef Hmesh GridType;


void globalRefine(GridType& grid, int refCount )
{
  if( refCount <= 0 ) return ;

  for (int j = 0; j < refCount; ++j)
  {
    Listwalkptr <Hmesh_basic::helement_t > walk( grid );
    for( walk->first() ; ! walk->done() ; walk->next())
    {
      Element & tr = walk->getitem();
      tr.Refco_el::mark(Refco::ref);
    } // end element loop

    // refine the mesh
    grid.refine();
  }
}

void hierarchicClear (Hmesh_basic::helement_t* el)
{
  // clear actual tag
  el->Refco_el::clear();
  // clear refined tag
  el->Refco_el::clearWas();
  // go to children
  for(Hmesh_basic::helement_t* child = el->down(); child; child = child->next())
  {
    // clear marker for child
    hierarchicClear(child);
  }
}

void postAdapt(GridType& grid)
{
  typedef Macro < Element > macro_t;
  // get macro element iterator
  Listwalkptr <macro_t> walk( grid );
  for( walk->first() ; ! walk->done() ; walk->next())
  {
    // get element pointer
    Hmesh_basic::helement_t* el = walk->getitem().operator ->();
    // hierarchically clear all markers
    hierarchicClear(el);
  }
}

int size(GridType& grid)
{
  int count = 0;
  Listwalkptr < Vertex > walk( grid );
  for( walk->first() ; ! walk->done() ; walk->next())
  {
    ++count ;
  }
  return count ;
}


int main (int argc, char ** argv )
{
  // The grid dimension
  const int dim = 2;

  std::string filename ("square.triangle");

  Hmesh* gridptr = 0;
  int ref = 1;
  if( argc > 2 )
  {
    // create conforming grid
    gridptr = new Hmesh( filename.c_str() );
    ref = 2 ;
    cout << "Create conforming grid " << endl;
  }
  else
  {
    // create non-conforming grid
    gridptr = new Hmesh( filename.c_str() , 1,   Refco::quart );
    cout << "Create non-conforming grid " << endl;
  }

  Hmesh& grid = *gridptr ;

  const int gRefine = 6;
  const int Nstep = (argc > 1) ? atoi( argv[1] ) : 5 ;
  const int N = (ref == 2) ? (ref * Nstep + 1) : Nstep;

  cout << "Refine step = " << N << endl ;

  std::vector<double> refTimes( N, 0.0);
  std::vector<double> crsTimes( N, 0.0);

  //some uniform pre refinement
  for (int i=0; i<gRefine; ++i)
  {
    int oldSize = size(grid);
    Timer timerGlobalRefine;
    globalRefine( grid , ref );
    std::cout << "Globalrefine " << oldSize << " -> " << size(grid) << ": " << timerGlobalRefine.elapsed() << " seconds." << std::endl;
  }

  for (int i = 0; i<N; ++i)
  {
    Timer timerRefine;
    Listwalkptr <Hmesh_basic::helement_t > walk( grid );
    for( walk->first() ; ! walk->done() ; walk->next())
    {
      Hmesh_basic::helement_t& tr = walk->getitem();
      assert( tr.down() == 0 );
      for (int j=0; j<dim+1; ++j)
      {
        const double (&p)[2] = tr.getVertex( j )->coord();
        const double two_norm = sqrt( p[0]*p[0] + p[1]*p[1] );

        if (two_norm <=1.0 && (two_norm>1.0 - pow(2.0,-4.0)) )
        {
          tr.Refco_el::mark(Refco::ref);
          break;
        }
      }
    }

    std::cout << "Refine cycle " << i << ":" << std::endl << "marking: " << timerRefine.elapsed() << std::endl;
    refTimes[i] += timerRefine.elapsed();
    timerRefine.reset();
    int oldSize = size(grid);
    grid.refine();
    std::cout << "adapt() " << oldSize << " -> " << size(grid) << ": " <<  timerRefine.elapsed() << std::endl;
    refTimes[i] += timerRefine.elapsed();

    timerRefine.reset();
    postAdapt(grid);
    std::cout << "postAdapt(): " << timerRefine.elapsed() << std::endl ;
    refTimes[i] +=  timerRefine.elapsed();

    if( i > 0 )
      std::cout << "Factor to previous run: " << refTimes[i]/refTimes[i-1] << std::endl << std::endl;
    else
      std::cout << std::endl;
  }

  for (int i = 0; i<N; ++i)
  {
    Timer timerRefine;
    Listwalkptr <Hmesh_basic::helement_t > walk( grid );

    for( walk->first() ; ! walk->done() ; walk->next())
    {
      Hmesh_basic::helement_t& tr = walk->getitem();
      tr.Refco_el::mark( Refco::crs );
    }

    std::cout << "Coarsen cycle " << i << ":" << std::endl << "marking: " << timerRefine.elapsed() << std::endl;
    crsTimes[i] += timerRefine.elapsed();
    timerRefine.reset();

    int oldSize = size(grid);
    grid.coarse();
    std::cout << "adapt() " << oldSize << " -> " << size(grid) << ": " << timerRefine.elapsed() << std::endl;
    crsTimes[i] += timerRefine.elapsed();

    timerRefine.reset();
    postAdapt(grid);
    std::cout << "postAdapt(): " << timerRefine.elapsed() << std::endl ;
    refTimes[i] +=  timerRefine.elapsed();

    if( i > 0 )
      std::cout << "Factor to previous run: " << crsTimes[i]/crsTimes[i-1] << std::endl << std::endl ;
    else
      std::cout << std::endl;
  }
  return 0;
}

