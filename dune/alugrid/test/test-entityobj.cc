// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include "config.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <optional>

#include <dune/common/parallel/mpihelper.hh>

#include <dune/grid/yaspgrid.hh>

#include <dune/alugrid/grid.hh>
#include <dune/alugrid/dgf.hh>

#include <dune/grid/io/file/vtk/vtkwriter.hh>

using namespace Dune;

template <class Grid>
struct GridFunction_Obj
{
  typedef typename Grid::template Codim<0>::Entity Element;

  const Grid& grid_;
  Element element_;

  GridFunction_Obj( const Grid& grid )
    : grid_( grid ), element_()
  {}

  void bind(const Element& element) { element_ = element; }
  void unbind() {}
};

template <class Grid>
struct GridFunction_Ptr
{
  typedef typename Grid::template Codim<0>::Entity Element;

  const Grid& grid_;
  const Element* element_ = nullptr;

  GridFunction_Ptr( const Grid& grid )
    : grid_( grid )
  {}

  void bind(const Element& element) { element_ = &element; }
  void unbind() { element_ = nullptr; }
};

template <class Grid>
struct GridFunction_Optional
{
  typedef typename Grid::template Codim<0>::Entity Element;

  const Grid& grid_;
  std::optional<Element> element_ = {};

  GridFunction_Optional( const Grid& grid )
    : grid_( grid )
  {}

  void bind(const Element& element) { element_ = element; }
  void unbind() { element_.reset(); }
};


template <class Grid, class GridFctObj>
void timing(const Grid& grid, const GridFctObj& grdFct, const std::string& descr)
{
  std::vector< GridFctObj > vec(25, grdFct);
  std::cout << "---------------------------------------" << std::endl;
  std::cout << descr << " sizeof(GrdFct) = " << sizeof(grdFct)
            << " grid size: " << grid.size(0)
            << std::endl;

  Dune::Timer timer;
  for(const auto& entity : Dune::elements( grid.leafGridView() ) )
  {
    for( auto& fct : vec )
      fct.bind( entity );
  }
  std::cout << descr << " " << timer.elapsed() << std::endl;
}


int main( int argc, char** argv )
try
{
  auto& mpiHelper = MPIHelper::instance( argc, argv );
  const int rank = mpiHelper.rank();

  std::string filename;
  if( argc > 1 )
  {
    filename = std::string(argv[1]);
  }
  else
  {
    std::cerr << "usage: " << argv[0] << " <filename>" << std::endl;
    return 0;
  }

#if 1
  using GridType = Dune::ALUGrid<3, 3, Dune::cube, Dune::nonconforming>;
  // using GridType = Dune::YaspGrid< 3 >;
  Dune::GridPtr< GridType > gridPtr( filename );
  GridType& grid = *gridPtr;
  grid.loadBalance();

  /*
  if( rank == 0 )
  {
    for(const auto& elem : Dune::elements( grid.leafGridView() ) )
    {
      for( const auto& intersection : Dune::intersections( grid.leafGridView(), elem ))
      {
        if( intersection.boundary() )
        {
          if( intersection.impl().boundaryId() < 1 ||
              intersection.impl().boundaryId() > 4)
          {
            std::cout << intersection.impl().boundaryId() << std::endl;
          }
        }
      }
      grid.mark( 1, elem );
    }
  }
  */

  //grid.preAdapt();
  //grid.adapt();
  //grid.postAdapt();

  std::cout << "*****************************\n";
  for(int i=0; i<4; ++i )
  {
    grid.globalRefine( 1 );

    GridFunction_Obj< GridType > fctO( grid );
    timing( grid, fctO, "GrdFct with Obj:" );

    GridFunction_Ptr< GridType > fctP( grid );
    timing( grid, fctP, "GrdFct with Ptr:" );

    GridFunction_Optional< GridType > fctOpt( grid );
    timing( grid, fctOpt, "GrdFct with Optinal:" );

    std::cout << "--------------------------------------------------\n";
  }
  std::cout << "*****************************\n";

  //Dune::VTKWriter<typename GridType::LeafGridView> vtkWriter( grid.leafGridView());
  //vtkWriter.write( "sphere-out" );
#else
  {
    using GridType = Dune::ALUGrid<3, 3, Dune::simplex, Dune::conforming>;
    Dune::GridPtr< GridType > gridPtr( filename );
    gridPtr.loadBalance();

    // grid is ready and load balanced at that point

    std::cout << "P[ " << rank << " ] parameters = " << gridPtr.nofParameters( 0 ) << std::endl;
    auto lvlView = gridPtr->levelGridView( 0 );
    const auto end = lvlView.end< 0 > ();
    for( auto it = lvlView.begin< 0 > (); it != end; ++it )
    {
      const auto& entity = *it;
      std::cout << "P[ " << rank << " ], entity " << lvlView.indexSet().index( entity );
      const auto& param  = gridPtr.parameters( entity );
      for( const auto& p : param )
      {
        std::cout << " " << p;
      }
      std::cout << std::endl;
    }
  }
#endif

  return 0;
}
catch ( Dune::Exception &e )
{
  std::cerr << e << std::endl;
  return 1;
}
catch (std::exception &e) {
  std::cerr << e.what() << std::endl;
  return 1;
}
catch ( ... )
{
  std::cerr << "Generic exception!" << std::endl;
  return 2;
}
