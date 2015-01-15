// (c) bernhard schupp, 1997 - 1998
// modification for the dune interface
// (c) Robert Kloefkorn 2004 -- 2005
#include <config.h>

#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

#include <dune/alugrid/impl/macrofileheader.hh>
#include <dune/alugrid/impl/binaryio.hh>

#include "mapp_cube_3d.h"
#include "mapp_tetra_3d.h"
#include "gitter_sti.h"
#include "gitter_mgb.h"
#include "walk.h"

namespace ALUGrid
{

  const std::pair< Gitter::Geometric::hasFace3 *, int > Gitter::Geometric::hface3::face3Neighbour::null
    = std::pair< Gitter::Geometric::hasFace3 *, int > (
        & (Gitter::Geometric::hasFaceEmpty::instance() ), -1);

  const std::pair< Gitter::Geometric::hasFace4 *, int > Gitter::Geometric::hface4::face4Neighbour::null
    = std::pair< Gitter::Geometric::hasFace4 *, int > (
        & (Gitter::Geometric::hasFaceEmpty::instance() ), -1);

  // prototype of Tetra type ( the faces of a tetrahedron )
  const int Gitter::Geometric::Tetra::prototype [4][3] = {{1,3,2},{0,2,3},{0,3,1},{0,1,2}};

  // edge which tell from which face with which edge number we get edge 0 to 5
  const int Gitter::Geometric::Tetra::edgeMap [6][2] = {{3, 0},
                                                              {3, 2},
                                                              {1, 2},
                                                              {0, 2},
                                                              {0, 0},
                                                              {0, 1}};

  // calculation Fomula is
  // edgeTwist = twist(face) < 0 ?
  //          (6 - vertex + twist(face)) % 3 :
  //          (vertex + twist(face)) % 3);
  const int Gitter::Geometric::Tetra::edgeTwist[6][3] = {
                                                                {0, 2, 1}, // twist -3
                                                                {1, 0, 2}, // twist -2
                                                                {2, 1, 0}, // twist -1
                                                                {0, 1, 2}, // twist 0
                                                                {1, 2, 0}, // twist 1
                                                                {2, 0, 1}, // twist 2
                                                                      };

  // calculation Fomula is
  // vertexTwist = (twist(face) < 0 ?
  //                 (7 - vertex + twist(face)) % 3 :
  //                 (vertex + twist(face)) % 3);
  const int Gitter::Geometric::Tetra::vertexTwist[6][3] = {
                                                                {1, 0, 2}, // twist -3
                                                                {2, 1, 0}, // twist -2
                                                                {0, 2, 1}, // twist -1
                                                                {0, 1, 2}, // twist 0
                                                                {1, 2, 0}, // twist 1
                                                                {2, 0, 1}, // twist 2
                                                                      };

  const std::vector< std::vector< int > > Gitter::Geometric::Tetra::_verticesNotOnFace( Gitter::Geometric::Tetra::initVerticesNotOnFace() );
  const std::vector< std::vector< int > > Gitter::Geometric::Tetra::_edgesNotOnFace( Gitter::Geometric::Tetra::initEdgesNotOnFace() );
  const std::vector< std::vector< int > > Gitter::Geometric::Tetra::_facesNotOnFace( Gitter::Geometric::Tetra::initFacesNotOnFace() );

  // return list with edges that lie not on given face
  std::vector< std::vector< int > >  Gitter::Geometric::Tetra::initVerticesNotOnFace()
  {
    std::vector< std::vector< int > > verticesNotFace( 4 );
    for(int f=0; f<4; ++f)
    {
      verticesNotFace[f].resize(1);
      verticesNotFace[f][0] = f;
    }
    return verticesNotFace;
  }

  // return list with edges that lie not on given face
  const std::vector<int> & Gitter::Geometric::Tetra::verticesNotOnFace( const int face )
  {
    alugrid_assert( face >= 0 && face < int(_verticesNotOnFace.size() ) );
    return _verticesNotOnFace[ face ];
  }

  // return list with edges that lie not on given face
  std::vector< std::vector< int > > Gitter::Geometric::Tetra::initEdgesNotOnFace()
  {
    std::vector< std::vector< int > > edgesNotFace(4);
    for(int f = 0; f<4; ++f )
    {
      edgesNotFace[f].resize(3);

      // tell which vertices belong to which edge
      const int protoEdges [6][2] = {{0, 1},
                                     {0, 2},
                                     {0, 3},
                                     {1, 2},
                                     {1, 3},
                                     {2, 3}};

      const int (&edges)[6][2] = protoEdges;
      const int (&vertices)[3] = prototype [ f ];

      int edgeCount = 0;
      for (int e = 0; e < 6; ++e)
      {
        const int (&edgeVx)[2] = edges[e];
        int count = 0;
        for(int v=0; v<3; ++v)
        {
          if( vertices[v] == edgeVx[0] || vertices[v] == edgeVx[1] )
            ++count;
        }
        if (count < 2)
        {
          edgesNotFace[f][edgeCount] = e;
          ++edgeCount;
        }
      }
      alugrid_assert ( edgeCount == 3 );
    }
    return edgesNotFace;
  }

  // return list with edges that lie not on given face
  const std::vector<int> & Gitter::Geometric::Tetra::edgesNotOnFace( const int face )
  {
    alugrid_assert( face >=0 && face < int(_edgesNotOnFace.size()) );
    return _edgesNotOnFace[ face ];
  }

  // return list with edges that lie not on given face
  std::vector< std::vector< int > > Gitter::Geometric::Tetra::initFacesNotOnFace()
  {
    std::vector< std::vector< int > > facesNotFace( 4 );
    for(int f=0; f<4; ++f)
    {
      facesNotFace[f].resize( 3 );
      int count = 0;
      for(int i=0; i<4; ++i)
      {
        if( i == f ) continue;
        facesNotFace[f][count] = i;
        ++count;
      }
      alugrid_assert ( count == 3 );
    }
    return facesNotFace;
  }

  // return list with edges that lie not on given face
  const std::vector<int> & Gitter::Geometric::Tetra::facesNotOnFace( const int face )
  {
    alugrid_assert( face >= 0 && face < int(_facesNotOnFace.size()) );
    return _facesNotOnFace[face];
  }

  // prototype of periodic 3 type
  const int Gitter::Geometric::Periodic3::prototype [2][3] = {{0,1,2},{3,5,4}};

  // prototype of periodic 4 type
  const int Gitter::Geometric::Periodic4::prototype [2][4] = {{0,3,2,1},{4,5,6,7}};


  // #     #
  // #     #  ######  #    #    ##
  // #     #  #        #  #    #  #
  // #######  #####     ##    #    #
  // #     #  #         ##    ######
  // #     #  #        #  #   #    #
  // #     #  ######  #    #  #    #

    //  Prototyp des Hexaeders wie er im Programm verwendet wird.
    //  Eckpunkte und Seitenflaechen:
    //
    //                      x2
    //                      /
    //                     /
    //              7---------6
    //         x3  /.        /|
    //          | / .  1    / |
    //          |/  .      /  |
    //          4---------5   | <-- 4 (hinten)
    //    5 --> |   .     | 3 |
    //  (links) |   3.....|...2
    //          |  .      |  /
    //          | .   2   | / <-- 0 (unten)
    //          |.        |/
    //          0---------1 --x1
    //
    //
    //
    //  edge[0]  = [0,1]   [0 0 0] [1 0 0]
    //  edge[1]  = [0,3]   [0 0 0] [0 1 0]
    //  edge[2]  = [0,4]   [0 0 0] [0 0 1]
    //  edge[3]  = [1,2]   [1 0 0] [1 1 0]
    //  edge[4]  = [1,5]   [1 0 0] [1 0 1]
    //  edge[5]  = [2,3]   [1 1 0] [0 1 0]
    //  edge[6]  = [2,6]   [1 1 0] [1 1 1]
    //  edge[7]  = [3,7]   [0 1 0] [0 1 1]
    //  edge[8]  = [4,5]   [0 0 1] [1 0 1]
    //  edge[9]  = [4,7]   [0 0 1] [0 1 1]
    //  edge[10] = [5,6]   [1 0 1] [1 1 1]
    //  edge[11] = [6,7]   [1 1 1] [0 1 1]
    //
    //

  // defines from which vertices one face is created
  const int Gitter::Geometric::Hexa::prototype [6][4] =
          {{0,3,2,1},{4,5,6,7},{0,1,5,4},{1,2,6,5},{2,3,7,6},{0,4,7,3}};

  const int Gitter::Geometric::Hexa::oppositeFace [6] = { 1 , 0 , 4 , 5 , 2 , 3  }; // opposite face of given face

  const std::vector< std::vector< int > > Gitter::Geometric::Hexa::_verticesNotOnFace( Gitter::Geometric::Hexa::initVerticesNotOnFace() );
  const std::vector< std::vector< int > > Gitter::Geometric::Hexa::_edgesNotOnFace( Gitter::Geometric::Hexa::initEdgesNotOnFace() );
  const std::vector< std::vector< int > > Gitter::Geometric::Hexa::_facesNotOnFace( Gitter::Geometric::Hexa::initFacesNotOnFace() );

  // return list with edges that lie not on given face
  std::vector< std::vector< int > > Gitter::Geometric::Hexa::initVerticesNotOnFace()
  {
    std::vector< std::vector< int > > verticesNotFace( 6 );
    for(int f=0; f<6; ++f)
    {
      verticesNotFace[f].resize( 4 );
      int oppFace = Gitter::Geometric::hexa_GEO::oppositeFace[ f ];

      // get vertices of opposite face of gFace
      const int (& vertices)[4] = Gitter::Geometric::hexa_GEO::prototype [ oppFace ];

      for (int i = 0; i < 4; ++i)
      {
        verticesNotFace[f][i] = vertices[i];
      }
    }
    return verticesNotFace;
  }

  // return list with edges that lie not on given face
  const std::vector<int> & Gitter::Geometric::Hexa::verticesNotOnFace( const int face )
  {
    alugrid_assert( face >= 0 && face < int( _verticesNotOnFace.size() ) );
    return _verticesNotOnFace[ face ];
  }

  std::vector< std::vector< int > > Gitter::Geometric::Hexa::initEdgesNotOnFace()
  {
    std::vector< std::vector< int > > edgesNotFace( 6 );
    for(int f = 0; f<6; ++f )
    {
      edgesNotFace[f].resize(8);

      // vertices of the edges of an Hexa
      const int protoEdges [12][2] =
          { {0,1} , {0,3} , {0,4} , {1,2} , {1,5} , {2,3} ,
            {2,6} , {3,7} , {4,5} , {4,7} , {5,6} , {6,7} };

      const int (&edges)[12][2] = protoEdges;
      const int (&vertices)[4]  = prototype [ f ];

      int edgeCount = 0;
      for (int e = 0; e < 12; ++e)
      {
        const int (&edgeVx)[2] = edges[e];
        int count = 0;
        for(int v=0; v<4; ++v)
        {
          if( vertices[v] == edgeVx[0] || vertices[v] == edgeVx[1] )
            ++count;
        }
        if (count < 2)
        {
          edgesNotFace[f][edgeCount] = e;
          ++edgeCount;
        }
      }
      alugrid_assert ( edgeCount == 8 );
    }
    return edgesNotFace;
  }

  const std::vector<int> & Gitter::Geometric::Hexa::edgesNotOnFace( const int face )
  {
    alugrid_assert( face >=0 && face < int(_edgesNotOnFace.size()) );
    return _edgesNotOnFace[ face ];
  }

  // return list with edges that lie not on given face
  std::vector< std::vector< int > > Gitter::Geometric::Hexa::initFacesNotOnFace()
  {
    std::vector< std::vector< int > > facesNotFace( 6 );
    for(int f=0; f<6; ++f)
    {
      facesNotFace[f].resize( 5 );
      int count = 0;
      for(int i=0; i<6; ++i)
      {
        if( i == f ) continue;
        facesNotFace[f][count] = i;
        ++count;
      }
      alugrid_assert ( count == 5 );
    }
    return facesNotFace;
  }

  // return list with edges that lie not on given face
  const std::vector<int> & Gitter::Geometric::Hexa::facesNotOnFace( const int face )
  {
    alugrid_assert( face >= 0 && face < int(_facesNotOnFace.size()) );
    return _facesNotOnFace[ face ];
  }

  // defines how we get an edge from an hexa , first is face , second is edge
  // for example the 0th edge is defined by the 3th edge of the 0th face
  const int Gitter::Geometric::Hexa::edgeMap [12][2] = {{0, 3},
                                                              {0, 0},
                                                              {2, 3},
                                                              {0, 2},
                                                              {2, 1},
                                                              {0, 1},
                                                              {4, 3},
                                                              {4, 1},
                                                              {1, 0},
                                                              {1, 3},
                                                              {1, 1},
                                                              {1, 2}};

  // calculation Fomula is
  // vertexTwist = twist(face) < 0 ?
  //                   (9 - vertex + twist(face)) % 4 :
  //                   (vertex + twist(face)) % 4)
  const int Gitter::Geometric::Hexa::
  vertexTwist[8][4] = {
    {1,0,3,2}, // twist = -4
    {2,1,0,3}, // twist = -3
    {3,2,1,0}, // twist = -2
    {0,3,2,1}, // twist = -1
    {0,1,2,3}, // twist = 0
    {1,2,3,0}, // twist = 1
    {2,3,0,1}, // twist = 2
    {3,0,1,2}  // twist = 3
  };

  // calculation Fomula is
  // edgeTwist = twist(face) < 0 ?
  //          (6 - vertex + twist(face)) % 3 :
  //          (vertex + twist(face)) % 3);
  const int Gitter::Geometric::Hexa::
  edgeTwist[8][4] = {
    {0,3,2,1}, // twist = -4
    {1,0,3,2}, // twist = -3
    {2,1,0,3}, // twist = -2
    {3,2,1,0}, // twist = -1
    {0,1,2,3}, // twist = 0
    {1,2,3,0}, // twist = 1
    {2,3,0,1}, // twist = 2
    {3,0,1,2}  // twist = 3
   };

  const int Gitter::Geometric::Hexa::
  vertex2Face [8][2] = {
    {0,0},// vx = 0
    {0,3},// vx = 1
    {0,2},// vx = 2
    {0,1},// vx = 3
    {1,0},// vx = 4
    {1,1},// vx = 5
    {1,2},// vx = 6
    {1,3} // vx = 7
  };

  int Gitter::Geometric::Hexa::test () const
  {
    const int v0[8][2] = {{0,0},{0,1},{0,2},{0,3},{1,0},{1,1},{1,2},{1,3}};
    const int v1[8][2] = {{2,0},{4,1},{3,1},{2,1},{2,3},{2,2},{3,2},{4,2}};
    const int v2[8][2] = {{5,0},{5,3},{4,0},{3,0},{5,1},{3,3},{4,3},{5,2}};
    int nfaults = 0;
    {
      for(int i = 0; i < 8; i ++ )
      {
        int i0 = v0[i][0], j0 = v0[i][1];
        int i1 = v1[i][0], j1 = v1[i][1];
        int i2 = v2[i][0], j2 = v2[i][1];
        if( myvertex (i0, j0) != myvertex (i1, j1) )
        {
          std::cerr << "ERROR: On level " << level () << " ";
          std::cerr << "vertex (" << i0 << "," << j0 << ") != vertex (" << i1 << "," << j1 << ")";
          std::cerr << "\t(" << i0 << "," << j0 << ") =" << myvertex(i0,j0) << " " << twist (i0);
          std::cerr << "\t(" << i1 << "," << j1 << ") =" << myvertex(i1,j1) << " " << twist (i1);
          std::cerr << std::endl;
          nfaults ++;
        }
        if( myvertex (i0, j0) != myvertex (i2, j2))
        {
          std::cerr << "ERROR: On level " << level () << " ";
          std::cerr << "vertex (" << i0 << "," << j0 << ") != vertex (" << i2 << "," << j2 << ")";
          std::cerr << "\t(" << i0 << "," << j0 << ") =" << myvertex(i0,j0) << " " << twist (i0);
          std::cerr << "\t(" << i2 << "," << j2 << ") =" << myvertex(i2,j2) << " " << twist (i1);
          std::cerr << std::endl;
          nfaults ++;
        }
      }
    }
    return nfaults;
  }

  int Gitter::Geometric::Hexa::tagForGlobalRefinement () {
    return (request (myrule_t::regular), 1);
  }

  int Gitter::Geometric::Hexa::tagForGlobalCoarsening () {
    return (request (myrule_t::crs), 1);
  }

  int Gitter::Geometric::Hexa::resetRefinementRequest () {
    return (request (myrule_t::nosplit), 1);
  }

  static inline bool insideBall (const alucoord_t (&p)[3], const alucoord_t (&c)[3], double r) {
    /*
    bool inside=false;
    double q[3];
    int x,y,z;
    for (x=0 , q[0]=p[0]-c[0]-2.0; !inside && x<3; x++) {
      for (y=0 , q[1]=p[1]-c[1]-2.0; !inside && y<3; y++) {
        for (z=0 , q[2]=p[2]-c[2]-2.0; !inside && z<3; z++) {
          inside = ( q[0]*q[0]+q[1]*q[1]+q[2]*q[2] < r*r );
          q[2]+=2.0;
        }
        q[1]+=2.0;
      }
      q[0]+=2.0;
    }
    return inside;
    */
    return
         (((p [0] - c [0]) * (p [0] - c [0]) + (p [1] - c [1]) * (p [1] - c [1])
         + (p [2] - c [2]) * (p [2] - c [2])) < (r * r)) ? true : false;
  }

  int Gitter::Geometric::Hexa::tagForBallRefinement (const alucoord_t (&center)[3], double radius, int limit) {
    bool hit = false;
    for (int i = 0; i < 8; i ++) {
      const alucoord_t (&p)[3] = myvertex (i)->Point ();
      if (insideBall (p,center,radius)) { hit = true; break; }
    }
    if (!hit) {
      const int resolution = 50;
      TrilinearMapping map (myvertex(0)->Point(), myvertex(1)->Point(),
          myvertex(2)->Point(), myvertex(3)->Point(), myvertex(4)->Point(),
          myvertex(5)->Point(), myvertex(6)->Point(), myvertex(7)->Point());
      alucoord_t p [3];
      for (int i = 0; i < resolution; i ++ ) {
        map.map2world (2.0 * drand48 () - 1.0, 2.0 * drand48 () - 1.0, 2.0 * drand48 () - 1.0, p);
        if (insideBall (p,center,radius)) { hit = true; break; }
      }
    }
    return hit ? (level () < limit ? (request (myrule_t::regular), 1)
           : (request (myrule_t::nosplit), 0)) : (request (myrule_t::crs), 1);
  }

  // #######
  //    #     ######   #####  #####     ##
  //    #     #          #    #    #   #  #
  //    #     #####      #    #    #  #    #
  //    #     #          #    #####   ######
  //    #     #          #    #   #   #    #
  //    #     ######     #    #    #  #    #

  /*
  //          z          y
  //         3 |-------- 2
  //           |\      .|     faces opposite to vertices
  //           | \    . |
  //           |  \  .  |
  //           |   \.   |     math. positive orientation from INSIDE
  //           |   .\   |
  //           |  .  \  |
  //           | .    \ |
  //           |.      \|1
  //         0 ------------ x
  //
  // face 0 = { 1, 3, 2 }   NOTE: all faces are oriented such that when one looks from
  // face 1 = { 0, 2, 3 }         the inside, they are oriented math. positive
  // face 2 = { 0, 3, 1 }
  // face 3 = { 0, 1, 2 }
  //
  // edge 0 = {0,1}
  // edge 1 = {0,2}
  // edge 2 = {0,3}
  // edge 3 = {1,2}
  // edge 4 = {1,3}
  // edge 5 = {2,3}
  //
  //
  //   edgeMap[6][2] = {{3, 0},  (face ,edge)
  //                    {3, 2},
  //                    {1, 2},
  //                    {0, 2},
  //                    {0, 0},
  //                    {0, 1}};
  //
  //
  //  Edge numbering in the triangle
  //
  //    2
  //    |\
  //    | \
  //    |  \
  //  2 |   \ 1
  //    |    \
  //    |     \
  //    |      \
  //  0 -------- 1
  //        0
  //
  */

  int Gitter::Geometric::Tetra::test () const {
    //cerr << "**WARNUNG (IGNORIERT) Tetra::test () nicht implementiert, in " <<  __FILE__ << " " << __LINE__  << endl;
    return 0;
  }

  int Gitter::Geometric::Tetra::tagForGlobalRefinement ()
  {
    // check whether bisection should be used
    if( this->myvertex(0)->myGrid()->conformingClosureNeeded() )
      return (request (myrule_t::bisect), 1);
    else
      return (request (myrule_t::regular), 1);
  }

  int Gitter::Geometric::Tetra::tagForGlobalCoarsening () {
    return (request (myrule_t::crs), 1);
  }

  int Gitter::Geometric::Tetra::resetRefinementRequest () {
    return (request (myrule_t::nosplit), 1);
  }

  int Gitter::Geometric::Tetra::tagForBallRefinement (const alucoord_t (&center)[3], double radius, int limit) {
    bool hit = false;
    for (int i = 0; i < 4; i ++) {
      const alucoord_t (&p)[3] = myvertex (i)->Point ();
      if (insideBall (p,center,radius)) { hit = true; break; }
    }
    if (!hit) return (request (myrule_t::crs), 1);
    if (level()>limit) return (request (myrule_t::nosplit), 0);
    return tagForGlobalRefinement();
  }

  // ######                                                           #####
  // #     #  ######  #####      #     ####   #####      #     ####  #     #
  // #     #  #       #    #     #    #    #  #    #     #    #    #       #
  // ######   #####   #    #     #    #    #  #    #     #    #       #####
  // #        #       #####      #    #    #  #    #     #    #            #
  // #        #       #   #      #    #    #  #    #     #    #    # #     #
  // #        ######  #    #     #     ####   #####      #     ####   #####

  // #include <iomanip.h>

  int Gitter::Geometric::Periodic3::test () const
  {
    std::cerr << "**WARNING (ignored): Periodic3::test () not implemented." << std::endl;
  //  const int digits = 3;
  //  cout << "Fl\"ache: 0, Twist: " << twist (0) << "\t";
  //  const VertexGeo * vx = myvertex (0,0);
  //  cout << "(" << setw (digits) << vx->Point ()[0] << "," << setw (digits) << vx->Point ()[1] << "," << setw (digits) << vx->Point ()[2] << ") ";
  //  vx = myvertex (0,1);
  //  cout << "(" << setw (digits) << vx->Point ()[0] << "," << setw (digits) << vx->Point ()[1] << "," << setw (digits) << vx->Point ()[2] << ") ";
  //  vx = myvertex (0,2);
  //  cout << "(" << setw (digits) << vx->Point ()[0] << "," << setw (digits) << vx->Point ()[1] << "," << setw (digits) << vx->Point ()[2] << ") \n";
  //  cout << "Fl\"ache: 1, Twist: " << twist (1) << "\t";
  //  vx = myvertex (1,0);
  //  cout << "(" << setw (digits) << vx->Point ()[0] << "," << setw (digits) << vx->Point ()[1] << "," << setw (digits) << vx->Point ()[2] << ") ";
  //  vx = myvertex (1,2);
  //  cout << "(" << setw (digits) << vx->Point ()[0] << "," << setw (digits) << vx->Point ()[1] << "," << setw (digits) << vx->Point ()[2] << ") ";
  //  vx = myvertex (1,1);
  //  cout << "(" << setw (digits) << vx->Point ()[0] << "," << setw (digits) << vx->Point ()[1] << "," << setw (digits) << vx->Point ()[2] << ") \n" << endl;
    return 0;
  }

  int Gitter::Geometric::Periodic3::tagForGlobalRefinement () {
    return 0;
  }

  int Gitter::Geometric::Periodic3::tagForGlobalCoarsening () {
    return 0;
  }

  int Gitter::Geometric::Periodic3::resetRefinementRequest () {
    return 0;
  }

  int Gitter::Geometric::Periodic3::tagForBallRefinement (const alucoord_t (&center)[3], double radius, int limit) {
    return 0;
  }

  // ######                                                          #
  // #     #  ######  #####      #     ####   #####      #     ####  #    #
  // #     #  #       #    #     #    #    #  #    #     #    #    # #    #
  // ######   #####   #    #     #    #    #  #    #     #    #      #    #
  // #        #       #####      #    #    #  #    #     #    #      #######
  // #        #       #   #      #    #    #  #    #     #    #    #      #
  // #        ######  #    #     #     ####   #####      #     ####       #

  int Gitter::Geometric::Periodic4::test () const
  {
    std::cerr << "WARNING (ignored): Periodic4::test () not implemented." << std::endl;
    return 0;
  }

  int Gitter::Geometric::Periodic4::tagForGlobalRefinement ()
  {
    return 0;
  }

  int Gitter::Geometric::Periodic4::tagForGlobalCoarsening ()
  {
    return 0;
  }

  int Gitter::Geometric::Periodic4::resetRefinementRequest ()
  {
    return 0;
  }

  int Gitter::Geometric::Periodic4::tagForBallRefinement (const alucoord_t (&center)[3], double radius, int limit)
  {
    return 0;
  }

  Gitter::Geometric::BuilderIF::~BuilderIF ()
  {
    if( iterators_attached () )
      std::cerr << "WARNING (ignored): Non-zero iterator count while deleting BuilderIF [" << iterators_attached () << "]" << std::endl;

    {for (hexalist_t ::iterator i = _hexaList.begin (); i != _hexaList.end (); delete (*i++)); }
    {for (tetralist_t::iterator i = _tetraList.begin (); i != _tetraList.end (); delete (*i++)); }
    {for (periodic3list_t::iterator i = _periodic3List.begin (); i != _periodic3List.end (); delete (*i++)); }
    {for (periodic4list_t::iterator i = _periodic4List.begin (); i != _periodic4List.end (); delete (*i++)); }
    {for (hbndseg4list_t ::iterator i = _hbndseg4List.begin (); i != _hbndseg4List.end (); delete (*i++)); }
    {for (hbndseg3list_t ::iterator i = _hbndseg3List.begin (); i != _hbndseg3List.end (); delete (*i++)); }
    {for (hface4list_t ::iterator i = _hface4List.begin (); i != _hface4List.end (); delete (*i++)); }
    {for (hface3list_t ::iterator i = _hface3List.begin (); i != _hface3List.end (); delete (*i++)); }
    {for (hedge1list_t ::iterator i = _hedge1List.begin (); i != _hedge1List.end (); delete (*i++)); }
    {for (vertexlist_t ::iterator i = _vertexList.begin (); i != _vertexList.end (); delete (*i++)); }
  }

  IteratorSTI < Gitter::vertex_STI > * Gitter::Geometric::BuilderIF::iterator (const vertex_STI *) const {
    ListIterator < VertexGeo > w (_vertexList);
    return new Wrapper < ListIterator < VertexGeo >, InternalVertex > (w);
  }

  IteratorSTI < Gitter::vertex_STI > * Gitter::Geometric::BuilderIF::iterator (const IteratorSTI < vertex_STI > * w) const {
    return new Wrapper < ListIterator < VertexGeo >, InternalVertex > (*(const Wrapper < ListIterator < VertexGeo >, InternalVertex > *) w);
  }

  IteratorSTI < Gitter::hedge_STI > * Gitter::Geometric::BuilderIF::iterator (const hedge_STI *) const {
    ListIterator < hedge1_GEO > w (_hedge1List);
    return new Wrapper < ListIterator < hedge1_GEO >, InternalEdge > (w);
  }

  IteratorSTI < Gitter::hedge_STI > * Gitter::Geometric::BuilderIF::iterator (const IteratorSTI < hedge_STI > * w) const {
    return new Wrapper < ListIterator < hedge1_GEO >, InternalEdge > (*(const Wrapper < ListIterator < hedge1_GEO >, InternalEdge > *)w);
  }

  IteratorSTI < Gitter::hface_STI > * Gitter::Geometric::BuilderIF::iterator (const hface_STI *) const {
    ListIterator < hface4_GEO > w1 (_hface4List);
    ListIterator < hface3_GEO > w2 (_hface3List);
    return new AlignIterator < ListIterator < hface4_GEO >, ListIterator < hface3_GEO >, hface_STI > (w1,w2);
  }

  IteratorSTI < Gitter::hface_STI > * Gitter::Geometric::BuilderIF::iterator (const IteratorSTI < hface_STI > * w) const {
    return new AlignIterator < ListIterator < hface4_GEO >, ListIterator < hface3_GEO >, hface_STI >
      (*(const AlignIterator < ListIterator < hface4_GEO >, ListIterator < hface3_GEO >, hface_STI > *)w);
  }

  IteratorSTI < Gitter::hbndseg_STI > * Gitter::Geometric::BuilderIF::iterator (const hbndseg_STI *) const {
    ListIterator < hbndseg4_GEO > w1 (_hbndseg4List);
    ListIterator < hbndseg3_GEO > w2 (_hbndseg3List);
    return new AlignIterator < ListIterator < hbndseg4_GEO >, ListIterator < hbndseg3_GEO >, hbndseg_STI > (w1,w2);
  }

  IteratorSTI < Gitter::hbndseg_STI > * Gitter::Geometric::BuilderIF::iterator (const IteratorSTI < hbndseg_STI > * w) const {
    return new AlignIterator < ListIterator < hbndseg4_GEO >, ListIterator < hbndseg3_GEO >, hbndseg_STI >
              (*(const AlignIterator < ListIterator < hbndseg4_GEO >, ListIterator < hbndseg3_GEO >, hbndseg_STI > *)w);;
  }

  IteratorSTI < Gitter::helement_STI > * Gitter::Geometric::BuilderIF::iterator (const helement_STI *a ) const
  {
    ListIterator < hexa_GEO > w1 (_hexaList);
    ListIterator < tetra_GEO > w2 (_tetraList);

    return new AlignIterator < ListIterator < hexa_GEO >, ListIterator < tetra_GEO >, helement_STI > (w1,w2);
  }

  IteratorSTI < Gitter::helement_STI > * Gitter::Geometric::BuilderIF::iterator (const IteratorSTI < helement_STI > * w) const {
    return new AlignIterator < ListIterator < hexa_GEO >, ListIterator < tetra_GEO >, helement_STI >
      (*(const AlignIterator < ListIterator < hexa_GEO >, ListIterator < tetra_GEO >, helement_STI > *)w);
  }

  IteratorSTI < Gitter::hperiodic_STI > * Gitter::Geometric::BuilderIF::iterator (const hperiodic_STI *a ) const
  {
    ListIterator < periodic3_GEO > w1 (_periodic3List);
    ListIterator < periodic4_GEO > w2 (_periodic4List);

    return new AlignIterator < ListIterator < periodic3_GEO >, ListIterator < periodic4_GEO >, hperiodic_STI > (w1, w2 );
  }

  IteratorSTI < Gitter::hperiodic_STI > * Gitter::Geometric::BuilderIF::iterator (const IteratorSTI < hperiodic_STI > * w) const
  {
    typedef AlignIterator < ListIterator < periodic3_GEO >, ListIterator < periodic4_GEO >, hperiodic_STI > Iterator;
    return new Iterator( (*(const Iterator *) w ) );
  }

  MacroFileHeader Gitter::Geometric::BuilderIF::
  dumpMacroGrid ( std::ostream &os, const MacroFileHeader::Format format ) const
  {
    MacroFileHeader header;
    if( _tetraList.empty() )
      header.setType( MacroFileHeader::hexahedra );
    else if( _hexaList.empty() )
      header.setType( MacroFileHeader::tetrahedra );
    else
    {
      std::cerr << "ERROR (fatal) Gitter::Geometric::BuilderIF::dumpMacroGrid( std::ostream & ) can only write pure tetrahedral or pure hexahedral grids." << std::endl;
      std::abort();
    }

    header.setFormat( format );
    header.setSystemByteOrder();

    if( format == MacroFileHeader::ascii )
    {
      // write header, for ascii it's complete here
      header.write( os );
      // set precision for ostreams (different for each stream)
      os.setf( std::ios::fixed, std::ios::floatfield );
      os.precision( ALUGridExternalParameters::precision() );
      os << std::scientific;
      dumpMacroGridImpl( os );
    }
    else // binary or zbinary
    {
      ObjectStream data;
      dumpMacroGridImpl( data );
      data.put( char(' ') ); // make consistent with ascii stream

      // write binary data to stream
      writeHeaderAndBinary( os, data, header );
    }

    // return header in case of further writing to stream
    return header;
  }

  template<class ostream_t>
  void Gitter::Geometric::BuilderIF::dumpMacroGridImpl (ostream_t & os ) const
  {
    // Bisher enth"alt die erste Zeile der Datei entweder "!Tetraeder"
    // oder "!Hexaeder" je nachdem, ob ein reines Tetraeder- oder
    // Hexaedernetz vorliegt. Gemischte Netze sind bez"uglich ihres
    // Dateiformats noch nicht spezifiziert.
    const int vertexListSize = _vertexList.size();
    const int tetraListSize  = _tetraList.size ();
    const int hexaListSize   = _hexaList.size  ();

    StandardWhiteSpace_t ws ;

    {
      os << vertexListSize << std::endl;
      const vertexlist_t::const_iterator end = _vertexList.end ();
      for (vertexlist_t::const_iterator i = _vertexList.begin (); i != end; ++i)
      {
        vertex_GEO* vertex = (*i);
        os << vertex->ident() << ws << vertex->Point() << std::endl;
      }
    }

    if( hexaListSize > 0 )
    {
      alugrid_assert (_hbndseg3List.size () == 0);

      os << hexaListSize << std::endl;
      const hexalist_t::const_iterator end = _hexaList.end ();
      for (hexalist_t::const_iterator i = _hexaList.begin (); i != end; ++i )
      {
        for (int j = 0; j < 7; ++ j )
        {
          os << (*i)->myvertex (j)->ident() << ws ;
        }
        os << (*i)->myvertex (7)->ident() << std::endl ;
      }

      os << int(_periodic4List.size ()) << ws << int(_hbndseg4List.size ()) << std::endl;
      const periodic4list_t::const_iterator pend = _periodic4List.end ();
      for (periodic4list_t::const_iterator i = _periodic4List.begin (); i != pend; ++i)
      {
        for (int j = 0; j < 7; ++j )
        {
          os << (*i)->myvertex (j)->ident() << ws ;
        }
        os << (*i)->myvertex (7)->ident() << std::endl;
      }
      const hbndseg4list_t::const_iterator hend = _hbndseg4List.end ();
      for (hbndseg4list_t::const_iterator i = _hbndseg4List.begin (); i != hend; ++i)
      {
        const int bndtype = (int)(*i)->bndtype ();
        // write negative bnd value in case of exterior bnd
        if( bndtype != hbndseg_STI::closure )
          os << -bndtype << ws ;
        for (int j = 0; j < 3; ++ j )
        {
          os << (*i)->myvertex (0,j)->ident() << ws ;
        }
        os << (*i)->myvertex (0,3)->ident() << std::endl ;
      }
    }
    else if( tetraListSize > 0 )
    {
      os << tetraListSize << std::endl;
      const tetralist_t::const_iterator end = _tetraList.end ();
      for (tetralist_t::const_iterator i = _tetraList.begin (); i != end; ++i )
      {
        for (int j = 0; j < 3; ++ j )
        {
          os << (*i)->myvertex (j)->ident() << ws ;
        }
        os << (*i)->myvertex (3)->ident() << std::endl ;
      }

      os << int(_periodic3List.size ()) << ws << int(_hbndseg3List.size ()) << std::endl;
      const periodic3list_t::const_iterator pend = _periodic3List.end ();
      for (periodic3list_t::const_iterator i = _periodic3List.begin (); i != pend; ++i )
      {
        for (int j = 0; j < 5; ++j )
        {
          os << (*i)->myvertex (j)->ident() << ws ;
        }
        os << (*i)->myvertex (5)->ident() << std::endl;
      }

      const hbndseg3list_t::const_iterator hend = _hbndseg3List.end ();
      for (hbndseg3list_t::const_iterator i = _hbndseg3List.begin (); i != hend; ++i)
      {
        const int bndtype = (int)(*i)->bndtype ();
        // write negative bnd value in case of exterior bnd
        if( bndtype != hbndseg_STI::closure )
          os << -bndtype << ws ;
        for( int j = 0; j < 2; ++ j )
        {
          os << (*i)->myvertex(0,j)->ident() << ws ;
        }
        os << (*i)->myvertex (0,2)->ident() << std::endl ;
      }
    }

    // backup linkage (for parallel backup/restore)
    linkagePatternMap_t& linkagePattern = const_cast< BuilderIF * > (this)->indexManagerStorage().linkagePatterns();
    // write size of pattern, 0 == no patterns
    // the null patterns is always there
    const int linkPatternSize = linkagePattern.size()-1;
    os << linkPatternSize << std::endl;
    if( linkPatternSize > 0 )
    {
      std::vector< int > refCount( linkPatternSize+1, -1 );
      typedef linkagePatternMap_t :: iterator iterator ;
      int idx = 0;
      const iterator endL = linkagePattern.end();
      for( iterator it = linkagePattern.begin(); it != endL; ++it, ++idx )
      {
        // save ref count
        refCount[ idx ] = (*it).second ;
        // overwrite ref count with position
        (*it).second = idx ;

        // store linkage to backup stream
        const std::vector<int>& linkage = (*it).first ;
        const int linkageSize = linkage.size();
        // the null pattern should be the first entry
        alugrid_assert( linkageSize == 0 ? idx == 0 : true );
        if( linkageSize > 0 )
        {
          os << linkageSize << ws;
          for( int i=0; i<linkageSize; ++i )
            os << linkage[ i ] << ws;
          os << std::endl;
        }
      }


      vertexlist_t::const_iterator i = _vertexList.begin ();
      const int hasElementLinkage = (i != _vertexList.end() ) ? ! (*i)->linkedElements().inactive() : 0;

      // write flag for stored element linkage
      os << hasElementLinkage << std::endl;

      idx = 0 ;
      // store position of vertex linkage in the map if vertex is border vertex
      const vertexlist_t::const_iterator end = _vertexList.end ();
      for (vertexlist_t::const_iterator i = _vertexList.begin (); i != end; ++i, ++idx)
      {
        vertex_GEO* vertex = (*i);
        // linkage info is only needed for border vertices
        if( vertex->isBorder() )
        {
          os << idx << ws << vertex->linkagePosition();
          // the methods linkedElements and linkagePosition will fail for serial vertices
          // but for these isBorder should be false anyway
          if( hasElementLinkage )
          {
            typedef vertex_GEO :: ElementLinkage_t ElementLinkage_t;
            const ElementLinkage_t& linkedElements = vertex->linkedElements();
            const int size = linkedElements.size();
            os << ws << size ;
            for( int k=0; k<size; ++k )
              os << ws << linkedElements[ k ];
          }
          os << std::endl;
        }
      }
      os << int(-1) << std::endl; // end marker for vertex position list

      // restore refcount
      idx = 0;
      for( iterator it = linkagePattern.begin(); it != endL; ++it, ++idx )
      {
        // restore ref count to map entry
        (*it).second = refCount[ idx ] ;
      }
    } // end linkage backup
  }

  size_t Gitter::Geometric::BuilderIF::memUsage ()
  {
    size_t mySize = 0;
    mySize += _vertexList.size()*sizeof(VertexGeo *);
    mySize += _hedge1List.size()*sizeof(hedge1_GEO *);
    mySize += sizeof(hface4_GEO *) * _hface4List.size();
    mySize += sizeof(hface3_GEO *) * _hface3List.size();
    mySize += sizeof(tetra_GEO * ) * _tetraList.size();
    mySize += sizeof(periodic3_GEO *) * _periodic3List.size();
    mySize += sizeof(periodic4_GEO *) * _periodic4List.size();
    mySize += sizeof(hexa_GEO *)      * _hexaList.size();
    mySize += sizeof(hbndseg3_GEO *)  * _hbndseg3List.size();
    mySize += sizeof(hbndseg4_GEO *)  * _hbndseg4List.size();

    mySize *= 3;

    mySize += sizeof(BuilderIF);

    for(int i=0; i<numOfIndexManager; ++i)
    {
      mySize += indexManager(i).memUsage();
    }

    return mySize;
  }

  IndexManagerType&  Gitter::Geometric::BuilderIF::indexManager(int codim)
  {
    alugrid_assert ( codim >= 0 && codim < numOfIndexManager );
    return _indexManagerStorage.get( codim );
  }

  IndexManagerStorageType&  Gitter::Geometric::BuilderIF::indexManagerStorage()
  {
    return _indexManagerStorage;
  }

  size_t Gitter::Geometric::BuilderIF::numMacroBndSegments() const
  {
    // count periodic boundaries twice
    return _hbndseg3List.size() +
           _hbndseg4List.size() +
           (2 * _periodic3List.size()) +
           (2 * _periodic4List.size());
  }

  // compress all index manager
  void Gitter::Geometric::BuilderIF::compressIndexManagers()
  {
    _indexManagerStorage.compress();
  }

} // namespace ALUGrid
