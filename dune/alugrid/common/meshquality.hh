#ifndef DUNE_ALU3DGRID_MESHQUALITY_HH
#define DUNE_ALU3DGRID_MESHQUALITY_HH

#include <iomanip>
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
    if ( dim < 3 ) return ;

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

    double minSine = 1e308;
    double maxSine = 0;
    double avgSine = 0;

    // TODO: number of elements connected to one edge
    // TODO: number of elements connected to one vertex

    //in a regular tetrahedron, we have
    // volume = (sqrt(2)/12)*edge^3
    // faceArea = (sqrt(3)/4) *edge^2

    const double factorEdgeTet = std::sqrt(2.) / double(12);
    const double factorFaceEdgeTet =  std::sqrt( 3. ) / 4.;
    const double factorFaceTet = factorEdgeTet * std::pow( ( 1. / factorFaceEdgeTet ), 1.5 )  ;

    std::vector< double > faceVols;

    const auto& indexSet = gridView.indexSet();

    std::vector< int > nElementsPerEdge( indexSet.size( dim-1 ), 0 );
    std::vector< int > nElementsPerVertex( indexSet.size( dim ), 0 );

    size_t nElements = 0;
    for( const auto& element : elements( gridView, Dune::Partitions::interiorBorder ) )
    {
      const double vol = element.geometry().volume();
      const Dune::GeometryType geomType = element.type();

      if( ! geomType.isSimplex() )
      {
        static bool firstCall = true;
        if( gridView.comm().rank() == 0 && firstCall )
        {
          firstCall = false;
          std::cout << "MeshQuality check only works for simplex grids, skipping check!" << std::endl;
        }
        return ;
      }

      const double factorEdge     = geomType.isCube() ? 1.0 : factorEdgeTet;
      const double factorFaceEdge = geomType.isCube() ? 1.0 : factorFaceEdgeTet;
      const double factorFace     = geomType.isCube() ? 1.0 : factorFaceTet;

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

        const int idx = indexSet.subIndex( element, e, dim-1 );
        ++(nElementsPerEdge[ idx ]);
      }

      double smallestFace = 1e308 ;
      double biggestFace = 0;

      const int faces = element.subEntities( dim-2 );
      faceVols.clear();
      faceVols.reserve( faces );
      for( int f = 0 ; f < faces ; ++ f )
      {
        const auto& face = element.template subEntity<dim-2>( f );
        const auto& geo  = face.geometry();
        //assert( geo.corners() == 3 );
        //( geo.corner( 1 ) - geo.corner( 0 ) ).two_norm();
        double faceSize = geo.volume();
        faceVols.push_back( faceSize );

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

      const int vertices = element.subEntities( dim );

      double sumSine = 0 ;
      for ( int i=0; i<vertices; ++i )
      {
        const int idx = indexSet.subIndex( element, i, dim );
        ++(nElementsPerVertex[ idx ]);

        double sine = (vol * 6.0);
        sine *= sine;
        for( int f=0; f<faces; ++f )
        {
          if( f == 3-i ) continue ;
          sine /= (faceVols[ f ] * 2.0);
        }

        minSine  = std::min( minSine, sine );
        maxSine  = std::max( maxSine, sine );
        sumSine += sine ;
      }

      sumSine /= double(vertices);
      avgSine += sumSine ;

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

    int maxElementsEdge = 0;
    int minElementsEdge = 100000000;
    double avgElementsEdge = 0;
    double nEdges = nElementsPerEdge.size();
    for( const auto& nElem : nElementsPerEdge )
    {
      maxElementsEdge = std::max( maxElementsEdge, nElem );
      minElementsEdge = std::min( minElementsEdge, nElem );
      avgElementsEdge += nElem;
    }

    int maxElementsVertex = 0;
    int minElementsVertex = 100000000;
    double avgElementsVertex = 0;
    double nVertices = nElementsPerVertex.size();
    for( const auto& nElem : nElementsPerVertex )
    {
      minElementsVertex = std::min( minElementsVertex, nElem );
      maxElementsVertex = std::max( maxElementsVertex, nElem );
      avgElementsVertex += nElem;
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

    avgSine /= double(nElements);

    minSine = gridView.grid().comm().min( minSine );
    maxSine = gridView.grid().comm().max( maxSine );
    avgSine = gridView.grid().comm().sum( avgSine );

    minElementsEdge   = gridView.grid().comm().min( minElementsEdge );
    minElementsVertex = gridView.grid().comm().min( minElementsVertex );
    maxElementsEdge   = gridView.grid().comm().max( maxElementsEdge );
    maxElementsVertex = gridView.grid().comm().max( maxElementsVertex );

    nEdges    = gridView.grid().comm().sum( nEdges );
    nVertices = gridView.grid().comm().sum( nVertices );

    avgElementsVertex = gridView.grid().comm().sum( avgElementsVertex );
    avgElementsEdge   = gridView.grid().comm().sum( avgElementsEdge );

    avgElementsEdge   /= nEdges;
    avgElementsVertex /= nVertices;

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
      out << std::setw(space) << minAreaShortestEdgeRatio << " " << maxAreaShortestEdgeRatio << " " << avgAreaShortestEdgeRatio << " ";
      out << std::setw(space) << minSine << " " << maxSine << " " << avgSine << " ";
      out << std::setw(space) << minElementsEdge << " " << maxElementsEdge << " " << avgElementsEdge << " ";
      out << std::setw(space) << minElementsVertex << " " << maxElementsVertex << " " << avgElementsVertex << " ";
      out << std::endl;
    }
  }

} // end namespace Dune

#endif // #ifndef DUNE_ALU3DGRID_MESHQUALITY_HH
