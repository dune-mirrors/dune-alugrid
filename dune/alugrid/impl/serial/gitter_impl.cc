// (c) Robert Kloefkorn 2010
#include <config.h>

#include <fstream>

#include "gatherscatter.hh"
#include "gitter_impl.h"
#include "ghost_elements.h"

namespace ALUGrid
{

  /////////////////////////////////////////
  //  read of data
  /////////////////////////////////////////
  void GitterBasis::Objects::TetraEmpty::
  os2VertexData ( ObjectStream &os, GatherScatterType &gs, int borderFace )
  {
      //for the 2d case we do not want to transmit data for
      //non-2d elements
    if(!is2d() || myvertex(borderFace)->is2d())
    // only one opposite vertex for tetras
    gs.setData( os, *myvertex( borderFace ) );
  }

  void GitterBasis::Objects::TetraEmpty ::
  os2EdgeData(ObjectStream & os, GatherScatterType & gs, int borderFace )
  {
    const std::vector<int> & edgesNotOnFace =
      Gitter::Geometric::tetra_GEO::edgesNotOnFace( borderFace );
    const int numEdges = edgesNotOnFace.size();
    alugrid_assert ( numEdges == 3 );
    for(int e = 0; e<numEdges; ++e)
    {
      gs.setData( os, *myhedge1( edgesNotOnFace[e] ) );
    }
  }

  void GitterBasis::Objects::TetraEmpty ::
  os2FaceData(ObjectStream & os, GatherScatterType & gs, int borderFace )
  {
    const std::vector<int> & facesNotOnFace =
      Gitter::Geometric::tetra_GEO::facesNotOnFace( borderFace );
    const int numFaces = facesNotOnFace.size();
    alugrid_assert ( numFaces == 3 );
    for (int i = 0; i <numFaces; ++i)
    {
      //for the 2d case we do not want to transmit data for
      //non-2d elements
      if( !is2d() ||myhface3( facesNotOnFace[i] )->is2d())
      gs.setData( os, *myhface3( facesNotOnFace[i] ) );
    }
  }

  /////////////////////////////////////////
  //  writing of data
  /////////////////////////////////////////
  void GitterBasis::Objects::TetraEmpty ::
  VertexData2os(ObjectStream & os, GatherScatterType & gs, int borderFace )
  {
    // only send one vertex
    //for the 2d case we do not want to transmit data for
    //non-2d elements
    if(!is2d() || myvertex(borderFace)->is2d())
    gs.sendData( os, *myvertex(borderFace) );
  }

  void GitterBasis::Objects::TetraEmpty ::
  EdgeData2os(ObjectStream & os, GatherScatterType & gs, int borderFace)
  {
    const std::vector<int> & edgesNotOnFace =
      Gitter::Geometric::tetra_GEO::edgesNotOnFace( borderFace );
    const int numEdges = edgesNotOnFace.size();
    alugrid_assert ( numEdges == 3 );
    for(int e=0; e<numEdges; ++e)
    {
      gs.sendData( os, *myhedge1( edgesNotOnFace[e] ) );
    }
  }

  void GitterBasis::Objects::TetraEmpty ::
  FaceData2os(ObjectStream & os, GatherScatterType & gs, int borderFace)
  {
    const std::vector<int> & facesNotOnFace =
      Gitter::Geometric::tetra_GEO::facesNotOnFace( borderFace );
    const int numFaces = facesNotOnFace.size();
    alugrid_assert ( numFaces == 3 );
    for (int i = 0; i <numFaces; ++i)
    {
      //for the 2d case we do not want to transmit data for
      //non-2d elements
      if( !is2d() ||myhface3( facesNotOnFace[i] )->is2d())
      gs.sendData( os,  *myhface3( facesNotOnFace[i] ) );
    }
  }

  // declare this element and all parts leaf
  void GitterBasis::Objects::TetraEmpty ::
  attachleafs()
  {
    addleaf();
    for (int i = 0; i < 4; ++i) myhface3(i)->addleaf();
    for (int i = 0; i < 6; ++i) myhedge1(i)->addleaf();
    for (int i = 0; i < 4; ++i) myvertex(i)->addleaf();
  }

  // this element is not leaf anymore
  void GitterBasis::Objects::TetraEmpty ::
  detachleafs()
  {
    removeleaf();
    for (int i = 0; i < 4; ++i) myhface3(i)->removeleaf();
    for (int i = 0; i < 6; ++i) myhedge1(i)->removeleaf();
    for (int i = 0; i < 4; ++i) myvertex(i)->removeleaf();
  }

  // check that all indices are within range of index manager
  void GitterBasis::Objects::TetraEmpty ::
  resetGhostIndices()
  {
    // only set indices for macro level ghosts
    if( this->level() > 0 ) return;

    {
      IndexManagerStorageType& ims = this->myvertex(0)->indexManagerStorage();

      typedef Gitter::Geometric::BuilderIF BuilderIF;
      // only call for ghosts
      alugrid_assert ( this->isGhost() );

      // check my index first
      resetGhostIndex( ims.get( BuilderIF::IM_Elements ));

      {
        // get index manager of faces
        IndexManagerType & im = ims.get(BuilderIF::IM_Faces );
        for (int i = 0; i < 4; ++i) myhface3(i)->resetGhostIndex(im);
      }
      {
        // get index manager of edges
        IndexManagerType & im = ims.get( BuilderIF::IM_Edges );
        for (int i = 0; i < 6; ++i) myhedge1(i)->resetGhostIndex(im);
      }
      {
        // get index manager of vertices
        IndexManagerType & im = ims.get( BuilderIF::IM_Vertices );
        for (int i = 0; i < 4; ++i) myvertex(i)->resetGhostIndex(im);
      }
    }
  }

  //ghost tetra gets indices of grid, to which it belongs actually
  void GitterBasis::Objects::TetraEmpty ::
  setIndicesAndBndId (const hface_STI & f, int face_nr)
  {
    // set all items to ghost bnd id
    setGhostBoundaryIds();

    typedef Gitter::Geometric::BuilderIF BuilderIF;

    typedef Gitter::Geometric::vertex_GEO vertex_GEO;
    typedef Gitter::Geometric::hedge1_GEO hedge1_GEO;

    const myhface3_t & face = static_cast<const myhface3_t &> (f);
    const bndid_t bndid = face.bndId ();

    myhface3_t & myface = *(myhface3(face_nr));

    IndexManagerStorageType& ims = this->myvertex(0)->indexManagerStorage();

    // set index of face
    myface.setIndex( ims.get(BuilderIF::IM_Faces), face.getIndex());
    // set bnd id of face
    myface.setGhostBndId( bndid );

    IndexManagerType & vxIm = ims.get(BuilderIF::IM_Vertices);
    IndexManagerType & edIm = ims.get(BuilderIF::IM_Edges);

    for (int i = 0; i < 3; ++i)
    {
      /*
#ifdef ALUGRIDDEBUG
      // this test will (and should) fail for vertex projections
      if( ! this->myGrid()->vertexProjection() )
      {
        // make sure we got the right face
        alugrid_assert (std::abs(myface.myvertex(i)->Point()[0]-
               face.myvertex(i)->Point()[0])<1e-8);
        alugrid_assert (std::abs(myface.myvertex(i)->Point()[1]-
               face.myvertex(i)->Point()[1])<1e-8);
        alugrid_assert (std::abs(myface.myvertex(i)->Point()[2]-
               face.myvertex(i)->Point()[2])<1e-8);
      }
#endif
*/

      vertex_GEO * vx = myface.myvertex(i);
      vx->setIndex( vxIm , face.myvertex(i)->getIndex() );
      vx->setGhostBndId( bndid );

      hedge1_GEO * edge = myface.myhedge1(i);
      edge->setIndex( edIm , face.myhedge1(i)->getIndex() );
      edge->setGhostBndId( bndid );
    }
  }

  //ghost tetra gets indices of grid, to which it belongs actually
  void GitterBasis::Objects::TetraEmpty ::
  setGhostBoundaryIds()
  {
    const bndid_t bndid = Gitter::hbndseg_STI::ghost_closure;

    // value of ghost_closure
    this->setGhostBndId( bndid );
    for( int i=0; i<4; ++i) myhface3(i)->setGhostBndId( bndid );
    for( int i=0; i<6; ++i) myhedge1(i)->setGhostBndId( bndid );
    for( int i=0; i<4; ++i) myvertex(i)->setGhostBndId( bndid );
  }


  ////////////////////////////////////////////////
  // --HexaEmpty  read of data
  ////////////////////////////////////////////////
  // scatter only on ghosts
  void GitterBasis::Objects::HexaEmpty ::
  os2VertexData(ObjectStream & os, GatherScatterType & gs, int borderFace )
  {
    const std::vector<int> & verticesNotOnFace =
      Gitter::Geometric::hexa_GEO::verticesNotOnFace( borderFace );
    const int numVertices = verticesNotOnFace.size();
    alugrid_assert ( numVertices == 4 );
    for (int i = 0; i <numVertices; ++i)
    {
      //for the 2d case we do not want to transmit data for
      //non-2d elements
      if(!is2d() || myvertex(verticesNotOnFace[i])->is2d() )
      gs.setData( os, *myvertex( verticesNotOnFace[i] ) );
    }
  }

  // scatter data on ghost edges
  void GitterBasis::Objects::HexaEmpty ::
  os2EdgeData(ObjectStream & os, GatherScatterType & gs, int borderFace )
  {
    const std::vector<int> & edgesNotOnFace =
      Gitter::Geometric::hexa_GEO::edgesNotOnFace( borderFace );
    const int numEdges = edgesNotOnFace.size();
    alugrid_assert ( numEdges == 8 );
    for(int e = 0; e<numEdges; ++e)
    {
      gs.setData( os, *myhedge1( edgesNotOnFace[e] ) );
    }
  }

  // scatter data on ghost faces
  void GitterBasis::Objects::HexaEmpty ::
  os2FaceData(ObjectStream & os, GatherScatterType & gs, int borderFace )
  {
    const std::vector<int> & facesNotOnFace =
      Gitter::Geometric::hexa_GEO::facesNotOnFace( borderFace );
    const int numFaces = facesNotOnFace.size();
    alugrid_assert ( numFaces == 5 );
    for (int i = 0; i <numFaces; ++i)
    {
      //for the 2d case we do not want to transmit data for
      //non-2d elements
      if(!is2d() || myhface4( facesNotOnFace[i])->is2d())
      gs.setData( os, *myhface4( facesNotOnFace[i] ) );
    }
  }

  //////////////////////////////////////////
  //  writing of data
  //////////////////////////////////////////
  void GitterBasis::Objects::HexaEmpty ::
  VertexData2os(ObjectStream & os, GatherScatterType & gs, int borderFace )
  {
    const std::vector<int> & verticesNotOnFace =
      Gitter::Geometric::hexa_GEO::verticesNotOnFace( borderFace );
    const int numVertices = verticesNotOnFace.size();
    alugrid_assert ( numVertices == 4 );
    for (int i = 0; i <numVertices; ++i)
    {
      //for the 2d case we do not want to transmit data for
      //non-2d elements
      if(!is2d() || myvertex(verticesNotOnFace[i])->is2d())
      gs.sendData( os, *myvertex( verticesNotOnFace[i] ) );
    }
  }

  void GitterBasis::Objects::HexaEmpty ::
  EdgeData2os(ObjectStream & os, GatherScatterType & gs, int borderFace)
  {
    const std::vector<int> & edgesNotOnFace =
      Gitter::Geometric::hexa_GEO::edgesNotOnFace( borderFace );
    const int numEdges = edgesNotOnFace.size();
    alugrid_assert ( numEdges == 8 );
    for(int e=0; e<numEdges; ++e)
    {
      gs.sendData( os, *myhedge1( edgesNotOnFace[e] ) );
    }
  }

  void GitterBasis::Objects::HexaEmpty ::
  FaceData2os(ObjectStream & os, GatherScatterType & gs, int borderFace)
  {
    const std::vector<int> & facesNotOnFace =
      Gitter::Geometric::hexa_GEO::facesNotOnFace( borderFace );
    const int numFaces = facesNotOnFace.size();
    alugrid_assert ( numFaces == 5 );
    for (int i = 0; i < numFaces; ++i)
    {
      //for the 2d case we do not want to transmit data for
      //non-2d elements
      if( !is2d() ||myhface4( facesNotOnFace[i] )->is2d())
      gs.sendData( os, *myhface4( facesNotOnFace[i] ) );
    }
  }

  void GitterBasis::Objects::HexaEmpty ::
  attachleafs()
  {
    alugrid_assert (this->leafRefCount()==0);
    addleaf();
    for (int i = 0; i < 6; ++i) myhface4(i)->addleaf();
    for (int i = 0; i < 12; ++i) myhedge1(i)->addleaf();
    for (int i = 0; i < 8; ++i) myvertex(i)->addleaf();
  }

  void GitterBasis::Objects::HexaEmpty ::
  detachleafs()
  {
    alugrid_assert (this->leafRefCount()==1);
    removeleaf();
    for (int i = 0; i < 6; ++i) myhface4(i)->removeleaf();
    for (int i = 0; i < 12; ++i) myhedge1(i)->removeleaf();
    for (int i = 0; i < 8; ++i) myvertex(i)->removeleaf();
  }

  // check that all indices are within range of index manager
  void GitterBasis::Objects::HexaEmpty ::
  resetGhostIndices()
  {
    // only set indices for macro level ghosts
    if( this->level() > 0 ) return;

    {
      IndexManagerStorageType& ims = this->myvertex(0)->indexManagerStorage();

      typedef Gitter::Geometric::BuilderIF BuilderIF;
      // only call for ghosts
      alugrid_assert ( this->isGhost() );

      // check my index first
      resetGhostIndex( ims.get(BuilderIF::IM_Elements ));

      {
        // get index manager of faces
        IndexManagerType & im = ims.get(BuilderIF::IM_Faces);
        for (int i = 0; i < 6; ++i) myhface4(i)->resetGhostIndex(im);
      }
      {
        // get index manager of edges
        IndexManagerType & im = ims.get(BuilderIF::IM_Edges);
        for (int i = 0; i < 12; ++i) myhedge1(i)->resetGhostIndex(im);
      }
      {
        // get index manager of vertices
        IndexManagerType & im = ims.get(BuilderIF::IM_Vertices);
        for (int i = 0; i < 8; ++i) myvertex(i)->resetGhostIndex(im);
      }
    }
  }

  //ghost hexa gets indices of grid, to which it belongs actually
  void GitterBasis::Objects::HexaEmpty ::
  setIndicesAndBndId (const hface_STI & f, int face_nr)
  {
    // set all items to ghost bnd id
    setGhostBoundaryIds();

    typedef Gitter::Geometric::BuilderIF BuilderIF;

    typedef Gitter::Geometric::vertex_GEO vertex_GEO;
    typedef Gitter::Geometric::hedge1_GEO hedge1_GEO;

    const myhface4_t & face = static_cast<const myhface4_t &> (f);
    const bndid_t bndid = face.bndId();

    myhface4_t & myface = *(myhface4(face_nr));

    IndexManagerStorageType& ims = this->myvertex(0)->indexManagerStorage();

    IndexManagerType & vxIm = ims.get(BuilderIF::IM_Vertices);
    IndexManagerType & edIm = ims.get(BuilderIF::IM_Edges);

    // set index of face
    myface.setIndex( ims.get(BuilderIF::IM_Faces) , face.getIndex ());
    // set bnd id of face
    myface.setGhostBndId( bndid );

    for (int i = 0; i < 4; ++i)
    {
      /*
#ifdef ALUGRIDDEBUG
      // this test will (and should) fail for vertex projections
      if( ! this->myGrid()->vertexProjection() )
      {
        // make sure we got the right face
        alugrid_assert (fabs(myface.myvertex(i)->Point()[0]-
               face.myvertex(i)->Point()[0])<1e-8);
        alugrid_assert (fabs(myface.myvertex(i)->Point()[1]-
               face.myvertex(i)->Point()[1])<1e-8);
        alugrid_assert (fabs(myface.myvertex(i)->Point()[2]-
               face.myvertex(i)->Point()[2])<1e-8);
      }
#endif
*/

      vertex_GEO * vx = myface.myvertex(i);
      vx->setIndex(vxIm, face.myvertex(i)->getIndex());
      vx->setGhostBndId( bndid );

      hedge1_GEO * edge = myface.myhedge1(i);
      edge->setIndex(edIm, face.myhedge1(i)->getIndex());
      edge->setGhostBndId( bndid );
    }
  }


  //ghost tetra gets indices of grid, to which it belongs actually
  void GitterBasis::Objects::HexaEmpty ::
  setGhostBoundaryIds()
  {
    const bndid_t bndid = Gitter::hbndseg_STI::ghost_closure;

    // value of ghost_closure
    this->setGhostBndId( bndid );
    for( int i=0; i<6; ++i) myhface4(i)->setGhostBndId( bndid );
    for( int i=0; i<12; ++i) myhedge1(i)->setGhostBndId( bndid );
    for( int i=0; i<8; ++i) myvertex(i)->setGhostBndId( bndid );
  }

  //////////////////////////////////////////////////////////////////
  //
  //  --GitterBasisImpl
  //
  //////////////////////////////////////////////////////////////////
  GitterBasisImpl::GitterBasisImpl ( const int dim ) : _macrogitter (0)
  {
    ProjectVertexPtrPair ppv; // = std::make_pair( ProjectVertexPtr(), ProjectVertexPtr() );
    _macrogitter = new MacroGitterBasis ( dim, this, ppv );
    alugrid_assert (_macrogitter);
    notifyMacroGridChanges ();
    return;
  }

  GitterBasisImpl::GitterBasisImpl ( const int dim, std::istream &in, const ProjectVertexPtrPair& ppv )
  : _macrogitter( 0 )
  {
    _macrogitter = new MacroGitterBasis ( dim, this, ppv, in );
    alugrid_assert (_macrogitter);
    _macrogitter->dumpInfo();
    notifyMacroGridChanges ();
    return;
  }

  GitterBasisImpl::GitterBasisImpl ( const int dim, const char *file, const ProjectVertexPtrPair& ppv )
  : _macrogitter( 0 )
  {
    std::ifstream in( file );
    if( !in )
    {
      std::cerr << "ERROR (ignored): Cannot open file '" << (file ? file : "") << "' in GitterBasisImpl::GitterBasisImpl( const char * )." << std::endl;
      _macrogitter = new MacroGitterBasis( dim, this, ppv );
    }
    else
      _macrogitter = new MacroGitterBasis( dim, this, ppv, in );

    _macrogitter->dumpInfo();
    alugrid_assert ( _macrogitter );
    notifyMacroGridChanges();
  }

  GitterBasisImpl::~GitterBasisImpl () { delete _macrogitter; }

  GitterBasis::MacroGitterBasis::MacroGitterBasis ( const int dim, Gitter *mygrid, const ProjectVertexPtrPair& ppv, std::istream &in )
  {
    this->indexManagerStorage().setDimAndGrid( dim, mygrid );
    this->setProjections( ppv );
    macrogridBuilder( in );
  }

  GitterBasis::MacroGitterBasis::MacroGitterBasis ( const int dim, Gitter * mygrid, const ProjectVertexPtrPair& ppv )
  {
    this->indexManagerStorage().setDimAndGrid( dim, mygrid );
    this->setProjections( ppv );
  }

  GitterBasis::VertexGeo * GitterBasis::MacroGitterBasis::insert_vertex (double x, double y, double z, int id) {
    return new Objects::VertexEmptyMacro (x, y, z, id, indexManagerStorage() );
  }

  GitterBasis::VertexGeo * GitterBasis::MacroGitterBasis::insert_ghostvx (double x, double y, double z, int id)
  {
    return new Objects::VertexEmptyMacro (x, y, z, id, indexManagerStorage() );
  }

  GitterBasis::hedge1_GEO * GitterBasis::MacroGitterBasis::insert_hedge1 (VertexGeo * a, VertexGeo * b) {
    return new Objects::hedge1_IMPL (0, a, b );
  }

  GitterBasis::hface3_GEO * GitterBasis::MacroGitterBasis::insert_hface3 (hedge1_GEO *(&e)[3], int (&s)[3]) {
    return new Objects::hface3_IMPL (0,e[0],s[0],e[1],s[1],e[2],s[2]);
  }

  GitterBasis::hface4_GEO * GitterBasis::MacroGitterBasis::insert_hface4 (hedge1_GEO *(&e)[4], int (&s)[4]) {
    return new Objects::hface4_IMPL (0, e[0],s[0],e[1],s[1],e[2],s[2],e[3],s[3]);
  }

  GitterBasis::tetra_GEO * GitterBasis::MacroGitterBasis::
  insert_tetra (hface3_GEO *(&f)[4], int (&t)[4], SimplexTypeFlag simplexType )
  {
    return new Objects::tetra_IMPL (0,f[0],t[0],f[1],t[1],f[2],t[2],f[3],t[3], simplexType );
  }

  GitterBasis::periodic3_GEO *
  GitterBasis::MacroGitterBasis::
  insert_periodic3 (hface3_GEO *(&f)[2], int (&t)[2],
                    const Gitter:: hbndseg_STI::bnd_t (&bnd)[2] )
  {
    return new Objects::periodic3_IMPL (0,f[0],t[0],f[1],t[1], bnd);
  }

  GitterBasis::periodic4_GEO * GitterBasis::MacroGitterBasis::
  insert_periodic4 (hface4_GEO *(&f)[2], int (&t)[2],
                    const Gitter:: hbndseg_STI::bnd_t (&bnd)[2] )
  {
    return new Objects::periodic4_IMPL (0, f [0], t[0], f [1], t[1], bnd );
  }

  GitterBasis::hexa_GEO * GitterBasis::MacroGitterBasis::
  insert_hexa (hface4_GEO *(&f)[6], int (&t)[6])
  {
    return new Objects::hexa_IMPL (0,f[0],t[0],f[1],t[1],f[2],t[2],f[3],t[3],f[4],t[4],f[5],t[5]);
  }

  GitterBasis::hbndseg3_GEO * GitterBasis::MacroGitterBasis::
  insert_hbnd3 (hface3_GEO * f, int i,
                Gitter::hbndseg_STI::bnd_t b)
  {
    // the NULL pointer is the pointer to the father which does not exists
    return new Objects::hbndseg3_IMPL ( 0, f, i, b );
  }

  GitterBasis::hbndseg3_GEO * GitterBasis::MacroGitterBasis::
  insert_hbnd3 (hface3_GEO * f, int i,
                Gitter::hbndseg_STI::bnd_t b, MacroGhostInfoTetra* )
  {
    return insert_hbnd3(f,i,b);
  }

  GitterBasis::hbndseg4_GEO * GitterBasis::MacroGitterBasis::
  insert_hbnd4 (hface4_GEO * f, int i, Gitter::hbndseg_STI::bnd_t b)
  {
    return new Objects::hbndseg4_IMPL ( 0, f, i, b );
  }

  GitterBasis::hbndseg4_GEO * GitterBasis::MacroGitterBasis::
  insert_hbnd4 (hface4_GEO * f, int i,
                Gitter::hbndseg_STI::bnd_t b, MacroGhostInfoHexa* )
  {
    return insert_hbnd4 (f,i,b);
  }


  void GitterBasisImpl::printMemUsage ()
  {
    typedef GitterBasis::DuneIndexProvider DuneIndexProvider;
    typedef GitterBasis::Objects::tetra_IMPL tetra_IMPL;
    typedef GitterBasis::Objects::hexa_IMPL  hexa_IMPL;
    typedef GitterBasis::Objects::hbndseg3_IMPL hbndseg3_IMPL;
    typedef GitterBasis::Objects::hbndseg4_IMPL hbndseg4_IMPL;
    typedef GitterBasis::Objects::hface3_IMPL hface3_IMPL;
    typedef GitterBasis::Objects::hface4_IMPL hface4_IMPL;
    typedef GitterBasis::Objects::hedge1_IMPL hedge1_IMPL;
    typedef GitterBasis::Objects::VertexEmptyMacro VertexEmptyMacro;
    typedef GitterBasis::Objects::VertexEmpty VertexEmpty;
    typedef Gitter::Geometric::VertexGeo VertexGeo;
    std::cout << "bool   = " << sizeof(bool) << std::endl;
    std::cout << "char   = " << sizeof(unsigned char) << std::endl;
    std::cout << "signed char   = " << sizeof(signed char) << std::endl;
    std::cout << "MyAlloc = " << sizeof(MyAlloc) << "\n";
    std::cout << "Refcount = " << sizeof(Refcount) << "\n";
    std::cout << "HedgeRule  = " << sizeof(Gitter::Geometric::Hedge1Rule) <<"\n";
    std::cout << "Hface3Rule = " << sizeof(Gitter::Geometric::Hface3Rule) <<"\n";
    std::cout << "Hface4Rule = " << sizeof(Gitter::Geometric::Hface4Rule) <<"\n";
    std::cout << "DuneIndexProvider = "<< sizeof(DuneIndexProvider) << "\n\n";

    std::cout << "******** TETRA *************************8\n";
    std::cout << "Tetrasize = " << sizeof(tetra_IMPL) << std::endl;
    std::cout << "MacroGhostTetra = " << sizeof(MacroGhostTetra) << std::endl;
    std::cout << "Hface3_IMPL = " << sizeof(hface3_IMPL) << std::endl;
    std::cout << "Hface3_GEO = " << sizeof( Gitter::Geometric::hface3_GEO ) << std::endl;
    std::cout << "Hface3::nb = " << sizeof( Gitter::Geometric::hface3::face3Neighbour ) << std::endl;
    std::cout << "HEdge1_IMPL = " << sizeof(hedge1_IMPL) << std::endl;
    std::cout << "HEdge1_GEO = " << sizeof(Gitter::Geometric ::hedge1_GEO) << std::endl;
    std::cout << "VertexMacro = " << sizeof(VertexEmptyMacro) << std::endl;
    std::cout << "VertexGeo   = " << sizeof(VertexGeo) << std::endl;
    std::cout << "Vertex = " << sizeof(VertexEmpty) << std::endl;
    std::cout << "Hbnd3_IMPL  = " << sizeof(hbndseg3_IMPL) << std::endl;
    std::cout << "MacroGhostInfoTetra = " << sizeof(MacroGhostInfoTetra) << std::endl << std::endl;

    std::cout << "******** HEXA *************************8\n";
    std::cout << "Hexasize = " << sizeof(hexa_IMPL) << std::endl;
    std::cout << "MacroGhostHexa = " << sizeof(MacroGhostHexa) << std::endl;
    std::cout << "Hface4_IMPL = " << sizeof(hface4_IMPL) << std::endl;
    std::cout << "Hface4_GEO = " << sizeof( Gitter::Geometric::hface4_GEO ) << std::endl;
    std::cout << "Hface4::nb = " << sizeof( Gitter::Geometric::hface4::face4Neighbour ) << std::endl;
    std::cout << "Hbnd4_IMPL  = " << sizeof(hbndseg4_IMPL) << std::endl;
    std::cout << "MacroGhostInfoHexa = " << sizeof(MacroGhostInfoHexa) << std::endl << std::endl;

    std::cout << "******** Number of Elements ************************8\n";
    {
      size_t totalSize = 0;
      bool simplex = false;
      {
        AccessIterator < helement_STI >::Handle iter (container ());
        int size = iter.size();
        iter.first();
        if( !iter.done() )
        {
          if( iter.item().type() == tetra )
          {
            simplex = true;
            size *= sizeof(tetra_IMPL);
          }
          else
          {
            size *= sizeof(hexa_IMPL);
          }
        }
        totalSize += size;
        std::cout << "Macro elements: size = " << size/1024/1024 << " MB \n";
      }

      {
        int size = AccessIterator < hbndseg_STI >::Handle (container ()).size();
        size *= (simplex) ?  sizeof(hbndseg3_IMPL) : sizeof(hbndseg4_IMPL);
        std::cout << "Macro boundary : size = " << size/1024/1024 << " MB \n";
        totalSize += size;
      }

      {
        int size = AccessIterator < hface_STI >::Handle (container ()).size();
        size *= (simplex) ?  sizeof(hface3_IMPL) : sizeof(hface4_IMPL);
        std::cout << "Macro faces : size = " << size/1024/1024 << " MB \n";
        totalSize += size;
      }

      {
        int size = AccessIterator < hedge_STI >::Handle (container ()).size();
        size *= sizeof(hedge1_IMPL);
        std::cout << "Macro edges : size = " << size/1024/1024 << " MB \n";
        totalSize += size;
      }

      {
        int size = AccessIterator < vertex_STI >::Handle (container ()).size();
        size *= sizeof(VertexEmptyMacro);
        std::cout << "Macro vertices : size = " << size/1024/1024 << " MB \n";
        totalSize += size;
      }

      size_t allSize = 0;
      size_t numElements = 0;
      {
        LeafIterator< helement_STI > it( *this );
        int size = it->size();
        numElements = size;
        size *= (simplex ? sizeof( tetra_IMPL ) : sizeof( hexa_IMPL ));
        std::cout << "Elements : size = " << size/1024/1024 << " MB" << std::endl;
        allSize += size;
      }

      {
        LeafIterator< hbndseg_STI > it( *this );
        const int size = it->size() * (simplex ? sizeof( hbndseg3_IMPL ) : sizeof( hbndseg4_IMPL ));
        std::cout << "Boundaries : size = " << size/1024/1024 << " MB" << std::endl;
        allSize += size;
      }

      {
        LeafIterator< hface_STI > it( *this );
        const int size = it->size() * (simplex ? sizeof( hface3_IMPL ) : sizeof( hface4_IMPL ));
        std::cout << "Faces : size = " << size/1024/1024 << " MB" << std::endl;
        allSize += size;
      }

      {
        LeafIterator< hedge_STI > it( *this );
        const int size = it->size() * sizeof( hedge1_IMPL );
        std::cout << "Edges : size = " << size/1024/1024 << " MB" << std::endl;
        allSize += size;
      }

      {
        LeafIterator< vertex_STI > it( *this );
        const int size = it->size() * sizeof( VertexEmpty );
        std::cout << "Vertices : size = " << size/1024/1024 << " MB" << std::endl;
        allSize += size;
      }

      {
        size_t indexMem = 0;
        for(int i=0; i<4; ++i)
          indexMem += indexManager( i ).getMaxIndex() * sizeof( int );
        std::cout << "Indices : size = " << indexMem/1024/1024 << " MB" << std::endl;
        allSize += indexMem;
      }

      std::cout << "All leaf size : " << allSize << " MB" << std::endl;
      std::cout << "bytes per Element: " << allSize/numElements << std::endl;
      std::cout << "Estimated all size : " << (9*long(allSize) / 8) << " MB" << std::endl;

      size_t build = container().memUsage();
      std::cout << "BuilderIF size = " << build/1024/1024 << " MB" << std::endl;
      totalSize += build;
      std::cout << "Overall size = " << totalSize/1024/1024 << " MB" << std::endl;
      std::cout << std::endl;
    }
  }

  // instantiation

  class GitterBasis;
  class GitterBasisImpl;

} // namespace ALUGrid
