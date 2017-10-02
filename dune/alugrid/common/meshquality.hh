#ifndef DUNE_ALU3DGRID_MESHQUALITY_HH
#define DUNE_ALU3DGRID_MESHQUALITY_HH

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>

#include <dune/grid/common/rangegenerators.hh>
#include <dune/geometry/referenceelements.hh>

namespace Dune {

  template <class GridView>
  static void meshQuality( const GridView& gridView, const double time, std::ostream& out )
  {
    static const int dim = GridView::dimension;

    double minVolShortestEdgeRatio = 1e308;
    double maxVolShortestEdgeRatio = 0;
    double avgVolShortestEdgeRatio = 0;

    double minVolLongestEdgeRatio = 1e308;
    double maxVolLongestEdgeRatio = 0;
    double avgVolLongestEdgeRatio = 0;

    double minVolSmallestFaceRatio = 1e308;
    double maxVolSmallestFaceRatio = 0;
    double avgVolSmallestFaceRatio = 0;

    double minVolBiggestFaceRatio = 1e308;
    double maxVolBiggestFaceRatio = 0;
    double avgVolBiggestFaceRatio = 0;

    double minAreaShortestEdgeRatio = 1e308;
    double maxAreaShortestEdgeRatio = 0;
    double avgAreaShortestEdgeRatio = 0;

    double minAreaLongestEdgeRatio = 1e308;
    double maxAreaLongestEdgeRatio = 0;
    double avgAreaLongestEdgeRatio = 0;

    //in a regular tetrahedron, we have
    // volume = (sqrt(2)/12)*edge^3
    // faceArea = (sqrt(3)/4) *edge^2
    static const double factorEdge = std::sqrt(2.) / double(12);
    static const double factorFaceEdge =  std::sqrt( 3. ) / 4.;
    static const double factorFace = factorEdge * std::pow( ( 1. / factorFaceEdge ), 1.5 )  ;

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

      double smallestFace = 1e308 ;
      double biggestFace = 0;

      const int faces = element.subEntities( dim-2 );
      for( int f = 0 ; f < faces ; ++ f )
      {
        const auto& face = element.template subEntity<dim-2>( f );
        const auto& geo  = face.geometry();
        assert( geo.corners() == 3 );
        //( geo.corner( 1 ) - geo.corner( 0 ) ).two_norm();
        double faceSize = geo.volume();
        smallestFace = std::min( smallestFace, faceSize );
        biggestFace  = std::max( biggestFace,  faceSize );

        double shortestFaceEdge = 1e308 ;
        double longestFaceEdge = 0;

        const int faceEdges = face.subEntities( dim-1 );
        for( int e = 0 ; e < faceEdges ; ++ e )
        {
          const int fe = Dune::ReferenceElements<double, dim>::simplex().subEntity(f,1,e,2);
          const auto& edge = element.template subEntity<dim-1>( fe );
          const auto& geo  = edge.geometry();
          assert( geo.corners() == 2 );
          //( geo.corner( 1 ) - geo.corner( 0 ) ).two_norm();
          double edgeLength = geo.volume();
          shortestFaceEdge = std::min( shortestFaceEdge, edgeLength );
          longestFaceEdge  = std::max( longestFaceEdge,  edgeLength );
        }

        //in a regular tetrahedron, we have
        // volume = (sqrt(2)/12)*edge^3
        shortestFaceEdge = factorFaceEdge * std::pow( shortestFaceEdge, 2 );
        longestFaceEdge = factorFaceEdge * std::pow( longestFaceEdge, 2 );


        const double areaShortest = faceSize / shortestFaceEdge ;
        minAreaShortestEdgeRatio  = std::min( minAreaShortestEdgeRatio, areaShortest );
        maxAreaShortestEdgeRatio  = std::max( maxAreaShortestEdgeRatio, areaShortest );
        avgAreaShortestEdgeRatio += areaShortest;

        const double areaLongest = faceSize / longestFaceEdge ;
        minAreaLongestEdgeRatio  = std::min( minAreaLongestEdgeRatio, areaLongest );
        maxAreaLongestEdgeRatio = std::max( maxAreaLongestEdgeRatio, areaLongest );
        avgAreaLongestEdgeRatio += areaLongest;
      }

      //in a regular tetrahedron, we have
      // volume = (sqrt(2)/12)*edge^3
      shortestEdge = factorEdge * std::pow( shortestEdge, 3 );
      longestEdge = factorEdge * std::pow( longestEdge, 3 );

      const double volShortest = vol / shortestEdge;
      minVolShortestEdgeRatio  = std::min( minVolShortestEdgeRatio, volShortest );
      maxVolShortestEdgeRatio  = std::max( maxVolShortestEdgeRatio, volShortest );
      avgVolShortestEdgeRatio += volShortest;

      const double volLongest = vol / longestEdge ;
      minVolLongestEdgeRatio = std::min( minVolLongestEdgeRatio, volLongest );
      maxVolLongestEdgeRatio = std::max( maxVolLongestEdgeRatio, volLongest );
      avgVolLongestEdgeRatio += volLongest;

      //in a regular tetrahedron, we have
      // volume = (sqrt(2)/12)*edge^3
      // faceArea = (sqrt(3)/4) *edge^2
      smallestFace = factorFace * std::pow( smallestFace, 1.5 );
      biggestFace = factorFace * std::pow( biggestFace, 1.5 );

      const double volSmallest = vol / smallestFace;
      minVolSmallestFaceRatio = std::min( minVolSmallestFaceRatio, volSmallest );
      maxVolSmallestFaceRatio  = std::max( maxVolSmallestFaceRatio, volSmallest );
      avgVolSmallestFaceRatio += volSmallest;

      const double volBiggest = vol / biggestFace ;
      minVolBiggestFaceRatio = std::min( minVolBiggestFaceRatio, volBiggest );
      maxVolBiggestFaceRatio = std::max( maxVolBiggestFaceRatio, volBiggest );
      avgVolBiggestFaceRatio += volBiggest;

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

    minVolSmallestFaceRatio = gridView.grid().comm().min( minVolSmallestFaceRatio );
    maxVolSmallestFaceRatio = gridView.grid().comm().max( maxVolSmallestFaceRatio );
    avgVolSmallestFaceRatio = gridView.grid().comm().sum( avgVolSmallestFaceRatio );

    minVolBiggestFaceRatio = gridView.grid().comm().min( minVolBiggestFaceRatio );
    maxVolBiggestFaceRatio = gridView.grid().comm().max( maxVolBiggestFaceRatio );
    avgVolBiggestFaceRatio = gridView.grid().comm().sum( avgVolBiggestFaceRatio );

    avgVolSmallestFaceRatio /= double(nElements);
    avgVolBiggestFaceRatio /= double(nElements);

    minAreaShortestEdgeRatio = gridView.grid().comm().min( minAreaShortestEdgeRatio );
    maxAreaShortestEdgeRatio = gridView.grid().comm().max( maxAreaShortestEdgeRatio );
    avgAreaShortestEdgeRatio = gridView.grid().comm().sum( avgAreaShortestEdgeRatio );

    minAreaLongestEdgeRatio = gridView.grid().comm().min( minAreaLongestEdgeRatio );
    maxAreaLongestEdgeRatio = gridView.grid().comm().max( maxAreaLongestEdgeRatio );
    avgAreaLongestEdgeRatio = gridView.grid().comm().sum( avgAreaLongestEdgeRatio );

    avgAreaShortestEdgeRatio /= double(4 * nElements);
    avgAreaLongestEdgeRatio /= double(4 * nElements);

    if( gridView.grid().comm().rank() == 0 )
    {
      int space = 18;
      out << "# MeshQuality: time=1    V/LE(min=2,max=3,avg=4)      V/SE(min=5,max=6,avg=7)     V/LF(min=8,max=9,avg=10)     V/SF(min=11,max=12,avg=13)     A/LE(min=14,max=15,avg=16)     A/SE(min=17,max=18,avg=19)  " << std::endl;
      out << time << " ";
      out << std::setw(space) << minVolLongestEdgeRatio << " " << maxVolLongestEdgeRatio << " " << avgVolLongestEdgeRatio << " ";
      out << std::setw(space) << minVolShortestEdgeRatio << " " << maxVolShortestEdgeRatio << " " << avgVolShortestEdgeRatio << " " ;
      out << std::setw(space) << minVolBiggestFaceRatio << " " << maxVolBiggestFaceRatio << " " << avgVolBiggestFaceRatio << " ";
      out << std::setw(space) << minVolSmallestFaceRatio << " " << maxVolSmallestFaceRatio << " " << avgVolSmallestFaceRatio << " ";
      out << std::setw(space) << minAreaLongestEdgeRatio << " " << maxAreaLongestEdgeRatio << " " << avgAreaLongestEdgeRatio << " ";
      out << std::setw(space) << minAreaShortestEdgeRatio << " " << maxAreaShortestEdgeRatio << " " << avgAreaShortestEdgeRatio << std::endl;
    }
  }

} // end namespace Dune

#endif // #ifndef DUNE_ALU3DGRID_MESHQUALITY_HH
