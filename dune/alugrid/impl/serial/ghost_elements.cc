// (c) Robert Kloefkorn 2004 - 2005
#include <config.h>

#include "ghost_elements.h"
#include "gitter_impl.h"

namespace ALUGrid
{

  MacroGhostBuilder::MacroGhostBuilder (BuilderIF & bi) : MacroGridBuilder(bi, bool(false) )
  {
    // create Builder with empty lists
    this->_initialized = true;
  }

  // desctructor
  MacroGhostBuilder:: ~MacroGhostBuilder ()
  {
    finalize();
  }

  // insert new Vertex without linkagePattern
  bool MacroGhostBuilder::
  InsertNewUniqueVertex (double x, double y, double z, int i)
  {
    vertexMap_t::const_iterator hit = this->_vertexMap.find (i);
    if (hit == _vertexMap.end ())
    {
      VertexGeo * v = myBuilder ().insert_ghostvx (x,y,z,i);
      this->_vertexMap [i] = v;
      return true;
    }
    return false;
  }

  // delete all elementes and stuff
  void MacroGhostBuilder::finalize ()
  {
    // empty all maps
    this->_hexaMap.clear();
    this->_tetraMap.clear();

    alugrid_assert ( this->_hbnd3Int.empty ());
    alugrid_assert ( this->_hbnd4Int.empty ());

    alugrid_assert ( this->_hbnd3Map.empty ());
    alugrid_assert ( this->_hbnd4Map.empty ());

    // faces
    this->_face4Map.clear();
    this->_face3Map.clear();

    // edges
    this->_edgeMap.clear();

    // vertices
    this->_vertexMap.clear();

    // finalized
    this->_finalized = true;
  }

  ////////////////////////////////////////////////////////////////
  //
  //  --MacroGhostTetra
  //
  ////////////////////////////////////////////////////////////////

  MacroGhostTetra ::
  MacroGhostTetra( BuilderIF & bi,
                   MacroGhostInfoTetra * allp,
                   const hface3_GEO * face) :
    _ghInfoPtr( allp ),
    _ghostPair( (GhostElement_t *)0 , -1)
  {
    //create macro ghost builder to create ghost element
    MacroGhostBuilder mgb( bi );

    MacroGhostInfoTetra& ghInfo = *_ghInfoPtr;

    typedef Gitter::Geometric::VertexGeo VertexGeo;

    const alucoord_t (&p)[1][3]  = ghInfo.getPoints();
    const int (&oppVerts)[1] = ghInfo.getOuterVertices();

    // here all entities have to be created new, because otherwise
    // the index generation will fail
    for(int i=0; i<3; ++i)
    {
      const VertexGeo * vx = face->myvertex(i);
#ifdef ALUGRIDDEBUG
      int idx = vx->ident();
      bool found = false;
      for(int j=0; j<4; ++j)
        if(ghInfo.vertices()[j] == idx) found = true;
      alugrid_assert ( found );
#endif
      const alucoord_t (&point)[3] = vx->Point();
      mgb.InsertNewUniqueVertex(point[0],point[1],point[2],vx->ident());
    }

#ifdef ALUGRIDDEBUG
    int idx = oppVerts[0];
    bool found = false;
    for(int j=0; j<4; ++j)
      if( ghInfo.vertices()[j] == idx) found = true;
    alugrid_assert ( found );
#endif
    const alucoord_t (&px)[3] = p[0];
    //cout << "Insert new point " << oppVerts[0] << endl;
#ifdef ALUGRIDDEBUG
    bool wasNewlyInserted =
#endif
      mgb.InsertNewUniqueVertex(px[0],px[1],px[2],oppVerts[0]);

    alugrid_assert ( wasNewlyInserted );

    // InsertUniqueHexa gets the global vertex numbers
    GhostTetra_t * ghost = mgb.InsertUniqueTetra ( ghInfo.vertices(), allp->simplexTypeFlag() ).first;

    // set ghost and number
    _ghostPair.first = ghost;
    alugrid_assert ( _ghostPair.first );
    _ghostPair.second = ghInfo.internalFace();
    alugrid_assert ( _ghostPair.second >= 0 );

    // NOTE: we do not insert boundary faces, because we don't need them
    // here. This is ok because of the hasFaceEmpty class (gitter_sti.h)
    // which acts as empty boundary.
  }

  //alternative Konstruktor fuer die Geister, die an Periodischen
  //Raendern haengen
  //sign = +/- 1  und ist dafuer da, um den Vektor
  //nicht mit -1 durchmultiplizieren zu muessen fuer anderen Geist
  MacroGhostTetra::
  MacroGhostTetra( BuilderIF & bi, MacroGhostInfoTetra * allp,
                   Gitter::Geometric::tetra_GEO * orig,
                   alucoord_t (&vec)[3] , double sign) :
    _ghInfoPtr(allp),
    _ghostPair( (GhostElement_t *)0, -1)
  {
    //create macro ghost builder to create ghost element
    MacroGhostBuilder mgb( bi );

    MacroGhostInfoTetra& ghInfo = *_ghInfoPtr;

    for (int i = 0; i < 4; ++i)
    {
      mgb.InsertNewUniqueVertex(orig->myvertex(i)->Point()[0] + sign*vec[0],
                                orig->myvertex(i)->Point()[1] + sign*vec[1],
                                orig->myvertex(i)->Point()[2] + sign*vec[2],
                                orig->myvertex(i)->ident()   );
    }

    GhostTetra_t * ghost = mgb.InsertUniqueTetra ( ghInfo.vertices(), orig->simplexTypeFlag() ).first;
    _ghostPair.first = ghost;
    alugrid_assert ( _ghostPair.first );
    _ghostPair.second = ghInfo.internalFace();
    alugrid_assert ( _ghostPair.second >= 0 );

    // NOTE: we do not insert boundary faces, because we don't need them
    // here. This is ok because of the hasFaceEmpty class (gitter_sti.h)
    // which acts as empty boundary.
  }

  // desctructor deleting _ghInforPtr
  MacroGhostTetra::~MacroGhostTetra ()
  {
    // store all sub items of the ghost element before deleting it
    tetra_GEO* tetra = (tetra_GEO *) _ghostPair.first;
    alugrid_assert ( tetra );

    VertexGeo* vertices[ 4 ] = {
      tetra->myvertex(0),
      tetra->myvertex(1),
      tetra->myvertex(2),
      tetra->myvertex(3) };

    hedge1_GEO* edges[ 6 ] = {
      tetra->myhedge(0),
      tetra->myhedge(1),
      tetra->myhedge(2),
      tetra->myhedge(3),
      tetra->myhedge(4),
      tetra->myhedge(5)
    };

    hface3_GEO* faces[ 4 ] = {
      tetra->myhface( 0 ),
      tetra->myhface( 1 ),
      tetra->myhface( 2 ),
      tetra->myhface( 3 )
    };

    // delete element
    delete tetra;

    // delete faces
    for( int i=0; i<4; ++i ) delete faces[ i ];
    // delete edges
    for( int i=0; i<6; ++i ) delete edges[ i ];
    // detele vertices
    for( int i=0; i<4; ++i ) delete vertices[ i ];

    alugrid_assert ( _ghInfoPtr );
    delete _ghInfoPtr;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //  --MacroGhostHexa
  //
  ///////////////////////////////////////////////////////////////////

  // constructor
  MacroGhostHexa::
  MacroGhostHexa( BuilderIF & bi, MacroGhostInfoHexa* allp, const hface4_GEO * face) :
    _ghInfoPtr(allp),
    _ghostPair( (GhostElement_t *)0 , -1)
  {
    //create macro ghost builder to create ghost element
    MacroGhostBuilder mgb( bi );

    MacroGhostInfoHexa& ghInfo = *_ghInfoPtr;

    typedef Gitter::Geometric::VertexGeo VertexGeo;

    const alucoord_t (&p)[4][3]  = ghInfo.getPoints();
    const int (&oppVerts)[4] = ghInfo.getOuterVertices();

    // here all entities have to be created new, because otherwise
    // the index generation will fail
    for(int i=0; i<4; ++i)
    {
      const VertexGeo * vx = face->myvertex(i);
      const alucoord_t (&p)[3] = vx->Point();
      mgb.InsertNewUniqueVertex(p[0],p[1],p[2],vx->ident());
    }

    // isnert the face, we need this because we wnat to have the same
    // numbers for the face (Lagrange Elements)
    for(int i=0; i<4; ++i)
    {
      const alucoord_t (&px)[3] = p[i];
#ifdef ALUGRIDDEBUG
      bool wasNewlyInserted =
#endif
        mgb.InsertNewUniqueVertex(px[0],px[1],px[2],oppVerts[i]);
      alugrid_assert ( wasNewlyInserted );
    }

    // InsertUniqueHexa gets the global vertex numbers
    hexa_GEO * ghost = mgb.InsertUniqueHexa ( ghInfo.vertices() ).first;
    alugrid_assert ( ghost );

    // set ghost values
    _ghostPair.first  = ghost;
    _ghostPair.second = ghInfo.internalFace();

    // NOTE: we do not insert boundary faces, because we don't need them
    // here. This is ok because of the hasFaceEmpty class (gitter_sti.h)
    // which acts as empty boundary.
  }

  MacroGhostHexa ::~MacroGhostHexa ()
  {
    // store all sub items of the ghost element before deleting it
    hexa_GEO* hexa = (hexa_GEO *) _ghostPair.first;
    alugrid_assert ( hexa );

    VertexGeo* vertices[ 8 ] = {
      hexa->myvertex(0),
      hexa->myvertex(1),
      hexa->myvertex(2),
      hexa->myvertex(3),
      hexa->myvertex(4),
      hexa->myvertex(5),
      hexa->myvertex(6),
      hexa->myvertex(7)
    };

    hedge1_GEO* edges[ 12 ] = {
      hexa->myhedge(0),
      hexa->myhedge(1),
      hexa->myhedge(2),
      hexa->myhedge(3),
      hexa->myhedge(4),
      hexa->myhedge(5),
      hexa->myhedge(6),
      hexa->myhedge(7),
      hexa->myhedge(8),
      hexa->myhedge(9),
      hexa->myhedge(10),
      hexa->myhedge(11),
    };

    hface4_GEO* faces[ 6 ] = {
      hexa->myhface( 0 ),
      hexa->myhface( 1 ),
      hexa->myhface( 2 ),
      hexa->myhface( 3 ),
      hexa->myhface( 4 ),
      hexa->myhface( 5 )
    };

    // delete element
    delete hexa;

    // delete faces
    for( int i=0; i<6; ++i ) delete faces[ i ];
    // delete edges
    for( int i=0; i<12; ++i ) delete edges[ i ];
    // detele vertices
    for( int i=0; i<8; ++i ) delete vertices[ i ];

    alugrid_assert ( _ghInfoPtr );
    delete _ghInfoPtr;
  }

  // instantiation
  class MacroGhostBuilder;
  class MacroGhostTetra;
  class MacroGhostHexa;

} // namespace ALUGrid
