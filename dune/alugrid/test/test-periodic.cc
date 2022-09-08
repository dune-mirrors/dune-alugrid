// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include "config.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <dune/common/parallel/mpihelper.hh>

#include <dune/alugrid/grid.hh>
#include <dune/alugrid/dgf.hh>

#include <dune/grid/io/file/vtk/vtkwriter.hh>

using namespace Dune;

template <class Point>
bool  comparePoints(const Point& a, const Point& b )
{
  auto diff = a - b;

  if( std::abs(diff.two_norm() - 1.0 ) > 1e-12 )
  {
    std::cout << a << " " << b << " " << diff << std::endl;
    return false;
  }
  return true;
}

template <class GridView>
void checkPeriodic( const GridView& gv )
{
  for(const auto& elem : Dune::elements( gv ) )
  {
    for( const auto& intersection : Dune::intersections( gv, elem ))
    {
      // check periodic boundary
      if( intersection.neighbor() && intersection.boundary() )
      {
        const auto& inside = intersection.inside();
        const auto& outside = intersection.outside();

        auto insideGeom  = intersection.geometryInInside();
        auto outsideGeom = intersection.geometryInOutside();
        auto centerInside  = insideGeom.center();
        auto centerOutside = outsideGeom.center();

        auto gIn  = inside.geometry().global ( centerInside );
        auto gOut = outside.geometry().global ( centerOutside );
        bool ok = comparePoints( gIn, gOut );
        if( ! ok )
          DUNE_THROW(Dune::InvalidStateException,"Centers on periodic boundary differ!");

        if ( GridView::dimension == 2 )
        {
          auto twistInside  = intersection.impl().twistInInside();
          auto twistOutside = intersection.impl().twistInOutside();
          // obtain mapped points on each side of the periodic boundary
          auto pIn  = inside.geometry().global ( insideGeom.corner( twistInside(0) ) );
          auto pOut = outside.geometry().global ( outsideGeom.corner( twistOutside(0) ) );
          ok = comparePoints( pIn, pOut );
          if( ! ok )
          {
            for( int i=0; i<2; ++i )
            {
              std::cout << inside.geometry().global ( insideGeom.corner( i ) )  <<
                " == " << outside.geometry().global ( outsideGeom.corner( i ) ) << std::endl;
            }
            DUNE_THROW(Dune::InvalidStateException,"Points on periodic boundary differ!");
          }
        }
      }
    }
  }
}

int main( int argc, char** argv )
try
{
  static const int dim = 2;
  //using GridType = Dune::ALUGrid<dim, dim, Dune::simplex, Dune::conforming>;
  //using GridType = Dune::ALUGrid<dim, dim, Dune::simplex, Dune::nonconforming>;
  using GridType = Dune::ALUGrid<dim, dim, Dune::cube, Dune::nonconforming>;

  MPIHelper::instance( argc, argv );

  std::string filename = "dgf/periodic" + std::to_string( dim ) + ".dgf";
  if( argc > 1 )
  {
    filename = std::string(argv[1]);
  }

  Dune::GridPtr< GridType > gridPtr( filename );
  GridType& grid = *gridPtr;
  grid.loadBalance();

  checkPeriodic( grid.leafGridView() );

  for( int i=0; i<3; ++i )
  {
    grid.globalRefine( 1 );
    checkPeriodic( grid.leafGridView() );
  }

  Dune::VTKWriter<typename GridType::LeafGridView> vtkWriter( grid.leafGridView());
  vtkWriter.write( "periodic-out" );

  return 0;
}
catch ( const Dune::Exception &e )
{
  std::cerr << e << std::endl;
  return 1;
}
catch (const std::exception &e) {
  std::cerr << e.what() << std::endl;
  return 1;
}
catch ( ... )
{
  std::cerr << "Generic exception!" << std::endl;
  return 2;
}
