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

    double minVolShortestEdgeRatio = 1e308;
    double maxVolShortestEdgeRatio = 0;
    double avgVolShortestEdgeRatio = 0;

    double minVolLongestEdgeRatio = 1e308;
    double maxVolLongestEdgeRatio = 0;
    double avgVolLongestEdgeRatio = 0;

    size_t nElements = 0;
    for( const auto& element : elements( gridView, Dune::Partitions::interiorBorder ) )
    {
      const double vol = element.geometry().volume();

      double shortestEdge = 1e308 ;
      double longestEdge = 0;

      const int edges = element.subEntities( dim-1 );
      for( int e = 0 ; e < edges ; ++ e )
      {
        const auto& edge = element.template subEntity<dim-1>( e );
        const auto& geo  = edge.geometry();
        assert( geo.corners() == 2 );
        //( geo.corner( 1 ) - geo.corner( 0 ) ).two_norm();
        double edgeLength = geo.volume();
        shortestEdge = std::min( shortestEdge, edgeLength );
        longestEdge  = std::max( longestEdge,  edgeLength );
      }

      const double volShortest = vol / shortestEdge;
      minVolShortestEdgeRatio  = std::min( minVolShortestEdgeRatio, volShortest );
      maxVolShortestEdgeRatio  = std::max( maxVolShortestEdgeRatio, volShortest );
      avgVolShortestEdgeRatio += volShortest;

      const double volLongest = vol / longestEdge ;
      minVolLongestEdgeRatio = std::min( minVolLongestEdgeRatio, volLongest );
      maxVolLongestEdgeRatio = std::max( maxVolLongestEdgeRatio, volLongest );
      avgVolLongestEdgeRatio += volLongest;

      ++ nElements;
    }

    nElements = gridView.grid().comm().sum( nElements );

    minVolShortestEdgeRatio = gridView.grid().comm().min( minVolShortestEdgeRatio );
    maxVolShortestEdgeRatio = gridView.grid().comm().max( maxVolShortestEdgeRatio );
    avgVolShortestEdgeRatio = gridView.grid().comm().sum( avgVolShortestEdgeRatio );

    minVolLongestEdgeRatio = gridView.grid().comm().min( minVolLongestEdgeRatio );
    maxVolLongestEdgeRatio = gridView.grid().comm().max( maxVolLongestEdgeRatio );
    avgVolLongestEdgeRatio = gridView.grid().comm().sum( avgVolLongestEdgeRatio );

    avgVolShortestEdgeRatio /= double(nElements);
    avgVolLongestEdgeRatio /= double(nElements);

    if( gridView.grid().comm().rank() == 0 )
    {
      std::cout << "MeshQuality: Volume / Longest   " << minVolLongestEdgeRatio << " " << maxVolLongestEdgeRatio << " " << avgVolLongestEdgeRatio << std::endl;
      std::cout << "MeshQuality: Volume / Shortest  " << minVolShortestEdgeRatio << " " << maxVolShortestEdgeRatio << " " << avgVolShortestEdgeRatio << std::endl;
    }
  }

} // end namespace Dune

#endif // #ifndef DUNE_ALU3DGRID_MESHQUALITY_HH
