#ifndef DUNE_ALU3DGRID_MESHQUALITY_HH
#define DUNE_ALU3DGRID_MESHQUALITY_HH

#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#include <dune/grid/common/rangegenerators.hh>
#include <dune/geometry/referenceelements.hh>

namespace Dune {

  template <class GridView>
  static void meshQuality( const GridView& gridView )
  {
    static const int dim = GridView::dimension;

    double minVolEdgeRatio = 1e308;
    double maxVolEdgeRatio = 0;

    for( const auto& element : elements( gridView ) )
    {
      const double vol = element.geometry().volume();

      double minEdge = 1e308 ;
      double maxEdge = 0;

      const int edges = element.subEntities( dim-1 );
      for( int e = 0 ; e < edges ; ++ e )
      {
        const auto& edge = element.template subEntity<dim-1>( e );
        const auto& geo  = edge.geometry();
        assert( geo.corners() == 2 );
        double edgeLength = ( geo.corner( 1 ) - geo.corner( 0 ) ).two_norm();
        minEdge = std::min( minEdge, edgeLength );
        maxEdge = std::max( maxEdge, edgeLength );
      }

      minVolEdgeRatio = std::min( minVolEdgeRatio, ( vol / minEdge ) );
      maxVolEdgeRatio = std::max( maxVolEdgeRatio, ( vol / maxEdge ) );
    }

    minVolEdgeRatio = gridView.grid().comm().min( minVolEdgeRatio );
    maxVolEdgeRatio = gridView.grid().comm().max( maxVolEdgeRatio );

    if( gridView.grid().comm().rank() == 0 )
    {
      std::cout << "MeshQuality: vol/minEdge = " << minVolEdgeRatio << std::endl;
      std::cout << "MeshQuality: vol/maxEdge = " << maxVolEdgeRatio << std::endl;
    }
  }

} // end namespace Dune

#endif // #ifndef DUNE_ALU3DGRID_MESHQUALITY_HH
