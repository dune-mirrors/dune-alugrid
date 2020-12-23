#ifndef DUNE_ALU3DGRID_FACTORY_IMP_CC
#define DUNE_ALU3DGRID_FACTORY_IMP_CC

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <algorithm>
#include <array>
#include <iostream>
#include <fstream>
#include <list>

#include <dune/common/parallel/mpicommunication.hh>
#include <dune/alugrid/3d/gridfactory.hh>

#include <dune/alugrid/impl/parallel/zcurve.hh>

#include <dune/alugrid/common/bisectioncompatibility.hh>
#include <dune/alugrid/common/faceconsistency.hh>

#include "dune/alugrid/3d/aluinline.hh"

namespace Dune
{
  template< class ALUGrid >
  alu_inline
  ALU3dGridFactory< ALUGrid > :: ~ALU3dGridFactory ()
  {}


  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid > :: insertVertex ( const VertexInputType &pos )
  {
    if (dimension == 2 && elementType == hexa)
      doInsertVertex( pos, vertices_.size()/2 );
    else
      doInsertVertex( pos, vertices_.size() );
  }


  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >::insertVertex ( const VertexInputType &coord, const VertexId globalId )
  {
    // mark that vertices with global id have been inserted
    foundGlobalIndex_ = true;
    doInsertVertex( coord, globalId );
  }

  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >::doInsertVertex ( const VertexInputType &coord, const VertexId globalId )
  {
    VertexType pos ( 0 );
    // copy coordinates since input vertex might only have 2 coordinates
    const int size = coord.size();
    for( int i=0; i<size; ++i )
      pos[ i ] = coord[ i ];

    // nothing to do for 3d
    if(dimension == 3)
    {
      vertices_.push_back( std::make_pair( pos, globalId ) );
    }
    else if (dimension == 2)
    {
      if( elementType == tetra )
      {
        //setting the global id to odd is convenience
        //we are then able to set the is2d() flag in the
        //alugrid implementation just by checking the index
        vertices_.push_back( std::make_pair( pos, 2*globalId+1 ) );
      }
      else if(elementType == hexa)
      {
        // it is here important, that the fake vertices
        // are of globalId +1 to the real ones
        // odd indices are the 2d vertices
        vertices_.push_back( std::make_pair( pos, 2*globalId+1 ) );
        pos[2] += 1.0 ;
        vertices_.push_back( std::make_pair( pos, 2*globalId+2 ) );
      }
    }
  }


  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    :: insertElement ( const GeometryType &geometry,
                       const std::vector< VertexId > &vertices )
  {
    assertGeometryType( geometry );
    const unsigned int boundaryId2d = ALU3DSPACE Gitter::hbndseg_STI::closure_2d;

    if( geometry.dim() != dimension )
      DUNE_THROW( GridError, "Only 3-dimensional elements can be inserted "
                             "into a 3-dimensional ALUGrid." );
    if(dimension == 3){
      if( vertices.size() != numCorners )
        DUNE_THROW( GridError, "Wrong number of vertices." );
      elements_.push_back( vertices );
    }
    else if (dimension == 2)
    {
      std::vector< VertexId > element;
      if( elementType == hexa)
      {
        element.resize( 8 );
        for (int i = 0; i < 4; ++i)
        {
          // multiply original number with 2 to get the indices of the 2d valid vertices
          element[ i ]   = vertices[ i ] * 2; // all even numbers means bottom face
          element[ i+4 ] = element [ i ] + 1; // all odd numbers means top face
        }
        elements_.push_back(element);
        /*
        std::cout << "Insert vertices: [";
        for( int i = 0; i < 4; ++i)
          std::cout << vertices[ i ] << ",";
        std::cout << "]" << std::endl;
        std::cout << "Insert Element: [";
        for( int i = 0; i < 8; ++i)
          std::cout << element[ i ] << ",";
        std::cout << "]" << std::endl;
        */
        insertBoundary(elements_.size()-1,4,boundaryId2d);
        insertBoundary(elements_.size()-1,5,boundaryId2d);
      }
      else if ( elementType == tetra )
      {
        element.resize( 4 );

        // construct element following the DUNE reference tetrahedron
        // do not rotate the vertices - otherwise the boundaries wont fit anymore
        element[0] = 0;
        element[1] = vertices[ 0 ] + 1;
        element[2] = vertices[ 1 ] + 1;
        element[3] = vertices[ 2 ] + 1;
        elements_.push_back(element);
        insertBoundary(elements_.size()-1,3,boundaryId2d);
      }
    }
    for( unsigned int i = 0 ; i < numFaces ; ++i)
    {
      doInsertFace(elements_.size()-1, i);
    }
  }

  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    :: doInsertFace ( const unsigned elIndex, int faceNumber )
  {
    FaceType face;
    generateFace( elements_[ elIndex ], faceNumber, face);
    doInsertFace( face, elIndex );
  }

  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    :: doInsertFace ( FaceType face, const unsigned int elIndex )
  {
    std::sort(face.begin(), face.end());
    auto it = boundaryFaces_.find( face ) ;
    if(it != boundaryFaces_.end() )
    {
      unsigned neighIndex = it->second;
      interiorFaces_.insert( std::make_pair( face, std::make_pair(neighIndex, elIndex ) ) );
      boundaryFaces_.erase( it );
    }
    else
    {
      boundaryFaces_.insert( std::make_pair( face, elIndex ) );
    }
  }

  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    :: insertBoundary ( const GeometryType &geometry,
                        const std::vector< VertexId > &vertices,
                        const int id )
  {
    assertGeometryType( geometry );
    if( geometry.dim() != dimension-1 )
    {
      DUNE_THROW( GridError, "Only 2-dimensional boundaries can be inserted "
                             "into a 3-dimensional ALUGrid." );
    }

    boundaryIds_.insert( makeBndPair( makeFace( vertices ), id ) );
  }


  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    ::insertBoundary ( const int element, const int face, const int id )
  {
    if( (element < 0) || (element >= (int)elements_.size()) )
      DUNE_THROW( RangeError, "ALU3dGridFactory::insertBoundary: invalid element index given." );

   // BndPair boundaryId;
  // generateFace( elements_[ element ], face, boundaryId.first );
  //  std::cout <<  "Element: [" << elements_[element][0] << ","<<elements_[element][1]<<"," << elements_[element][2] << "," << elements_[element][3] <<"] Face: " << face << " Boundary: " <<     boundaryId.first  << std::endl;

    //in 2d the local face ids are correct, because we need the faces 0,1,2 in tetra and 0,1,2,3 for hexas
    //and that is exactly what we get form the 2d dgfparser.

    doInsertBoundary( element, face, id );
  }

  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    ::doInsertBoundary ( const int element, const int face, const int id )
  {
    if( (element < 0) || (element >= (int)elements_.size()) )
      DUNE_THROW( RangeError, "ALU3dGridFactory::insertBoundary: invalid element index given." );

    BndPair boundaryId;
    generateFace( elements_[ element ], face, boundaryId.first );
    boundaryId.second = id;
    boundaryIds_.insert( boundaryId );
  }

  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid > ::
  insertBoundaryProjection( const DuneBoundaryProjectionType& bndProjection, const bool isSurfaceProjection )
  {
#ifndef NDEBUG
    std::cout << "Inserting Surface Projection:" << std::boolalpha << isSurfaceProjection << std::endl << std::endl;
#endif
    if( isSurfaceProjection )
    {
      if( surfaceProjection_ )
        DUNE_THROW(InvalidStateException,"You can only insert one Surface Projection");
      surfaceProjection_.reset(new ALUProjectionType (&bndProjection, ALUProjectionType :: surface) );
    }
    else
    {
      if( globalProjection_ )
        DUNE_THROW(InvalidStateException,"You can only insert one global boundary Projection");
      globalProjection_.reset(new ALUProjectionType (&bndProjection, ALUProjectionType :: global) );
    }
  }


  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid > ::
  insertBoundaryProjection ( const GeometryType &type,
                             const std::vector< VertexId > &vertices,
                             const DuneBoundaryProjectionType *projection )
  {
    if( (int)type.dim() != dimension-1 )
      DUNE_THROW( GridError, "Inserting boundary face of wrong dimension: " << type.dim() );
    alugrid_assert ( type.isCube() || type.isSimplex() );

    FaceType faceId = makeFace( vertices );
    std::sort( faceId.begin(), faceId.end() );

    if( boundaryProjections_.find( faceId ) != boundaryProjections_.end() )
      DUNE_THROW( GridError, "Only one boundary projection can be attached to a face." );

    boundaryProjections_[ faceId ] = projection;
  }


  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    ::insertFaceTransformation ( const WorldMatrix &matrix, const WorldVector &shift )
  {
    faceTransformations_.push_back( Transformation( matrix, shift ) );
  }

  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    ::sortElements( const VertexVector& vertices,
                    const ElementVector& elements,
                    std::vector< unsigned int >& ordering )
  {
    const size_t elemSize = elements.size();
    ordering.resize( elemSize );
    // default ordering
    for( size_t i=0; i<elemSize; ++i ) ordering[ i ] = i;

#ifdef DISABLE_ALUGRID_SFC_ORDERING
    std::cerr << "WARNING: ALUGRID_SFC_ORDERING disabled by DISABLE_ALUGRID_SFC_ORDERING" << std::endl;
    return ;
#endif

    // if type of curve is chosen to be None, nothing more to be done here
    if( curveType_ == SpaceFillingCurveOrderingType :: None )
      return ;

    {
      // apply space filling curve orderung to the inserted elements
      // see common/hsfc.hh for details
      typename ALUGrid::CollectiveCommunication comm( communicator_ );

      // if we are in parallel insertion mode we need communication
      const bool foundGlobalIndex = comm.max( foundGlobalIndex_ );
      if( foundGlobalIndex && comm.size() > 1 )
      {
        if( comm.rank() == 0 )
          std::cerr << "WARNING: Space filling curve ordering does not work for parallel grid factory, yet!" << std::endl;
        return ;
      }

      VertexInputType maxCoord ( std::numeric_limits<double>::min() );
      VertexInputType minCoord ( std::numeric_limits<double>::max() );
      const size_t vertexSize = vertices.size();
      if( vertexSize > 0 )
      {
        for( unsigned int d=0; d<dimensionworld; ++d )
        {
          maxCoord[d] = vertices[ 0 ].first[d];
          minCoord[d] = vertices[ 0 ].first[d];
        }
      }

      for( size_t i=0; i<vertexSize; ++i )
      {
        const VertexType& vx = vertices[ i ].first;
        for( unsigned int d=0; d<dimensionworld; ++d )
        {
          maxCoord[ d ] = std::max( maxCoord[ d ], vx[ d ] );
          minCoord[ d ] = std::min( minCoord[ d ], vx[ d ] );
        }
      }

      // get element's center to Hilbert/Zcurve index mapping
      SpaceFillingCurveOrderingType sfc( curveType_, minCoord, maxCoord, comm );

      typedef std::multimap< double, long int > hsfc_t;
      hsfc_t hsfc;

      for( size_t i=0; i<elemSize; ++i )
      {
        VertexInputType center( 0 );
        // compute barycenter
        const int vxSize = elements[ i ].size();
        for( int vx = 0; vx<vxSize; ++vx )
        {
          const VertexType& vertex = vertices[ elements[ i ][ vx ] ].first;
          for( unsigned int d=0; d<dimensionworld; ++d )
            center[ d ] += vertex[ d ];
        }
        center /= double(vxSize);

        // generate sfc index from element's center and store index
        // make sure that the mapping is unique
        alugrid_assert( hsfc.find( sfc.index( center ) ) == hsfc.end() );
        hsfc.insert( std::make_pair( sfc.index( center ) , i ) );
      }

      typedef typename hsfc_t :: iterator iterator;
      const iterator end = hsfc.end();
      size_t idx = 0;
      for( iterator it = hsfc.begin(); it != end; ++it, ++idx )
      {
        ordering[ idx ] = (*it).second ;
      }
    }
  }

  // This method moves the longest edge to edge 0--3 of the local numbering,
  // so if it is a possibly announced edge, it is taken by the compatibility algorithm
  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >::markLongestEdge ( const bool resortElements )
  {
    std::cerr << "Marking longest edge for initial refinement..." << std::endl;

    // DUNE reference element edge numbering
    // The vertices on edge i
    // (Vertices that are not on edge i are on edge 5-i)
    static const int edges[ 6 ][ 2 ] = { {0,1}, {0,2}, {1,2}, {0,3}, {1,3}, {2,3} };

    //create the vertex priority List
    const int numberOfElements = elements_.size();

    for(int el = 0; el < numberOfElements ; ++el )
    {
      auto& element = elements_[ el ];
      assert( int(element.size()) == 4 ); // 4 vertices for a tetrahedron

      // find longest edge
      int edge = -1;
      double maxLength = 0;
      for( int j = 0; j < 6; ++j )
      {
        const int vx0 = element[ edges[ j ][ 0 ] ];
        const int vx1 = element[ edges[ j ][ 1 ] ];
        double length = (vertices_[ vx0 ].first - vertices_[ vx1 ].first ).two_norm2();
        if( length > maxLength )
        {
          edge = j;
          maxLength = length;
        }
      }

      assert( edge >= 0 && edge <= 5 );

      // mark longest edge as refinement edge

      if( resortElements )
      {
        assert( element.size() == 4 );
        const auto old( element );
        element[0] = old[edges[edge][0]];
        element[1] = old[edges[5-edge][0]];
        element[2] = old[edges[5-edge][1]];
        element[3] = old[edges[edge][1]];
      }


      /*
#ifndef NDEBUG
      // find longest edge, this should now be 0 for all elements
      int longest = -1;
      maxLength = 0;
      for( int j = 0; j < 6; ++j )
      {
        const int vx0 = element[ edges[ j ][ 0 ] ];
        const int vx1 = element[ edges[ j ][ 1 ] ];
        double length = (vertices_[ vx0 ].first - vertices_[ vx1 ].first ).two_norm2();
        if( length > maxLength )
        {
          longest = j;
          maxLength = length;
        }
      }
      assert( longest == 0 );
      // std::cout << "Longest edge = " << longest << "  " << edge << std::endl;
#endif
      */

    }
  }

  template< class ALUGrid >
  alu_inline
  typename ALU3dGridFactory< ALUGrid >::GridPtrType
  ALU3dGridFactory< ALUGrid >::createGrid ()
  {
    return createGrid( true, true, "" );
  }

  template< class ALUGrid >
  alu_inline
  typename ALU3dGridFactory< ALUGrid >::GridPtrType
  ALU3dGridFactory< ALUGrid >::createGrid ( const bool addMissingBoundaries, const std::string dgfName )
  {
    return createGrid( addMissingBoundaries, true, dgfName );
  }

  template< class ALUGrid >
  alu_inline
  typename ALU3dGridFactory< ALUGrid >::GridPtrType
  ALU3dGridFactory< ALUGrid >::createGrid ( const bool addMissingBoundaries, bool temporary, const std::string name )
  {

    if( dimension == 2 && ALUGrid::refinementType == conforming )
    {
      if( markLongestEdge_ )
      {
        markLongestEdge();
      }
    }

    std::vector< unsigned int >& ordering = ordering_;
    // sort element given a hilbert space filling curve (if Zoltan is available)
    sortElements( vertices_, elements_, ordering );

    //the search for the periodic neighbour also deletes the
    //found faces from the boundaryIds_
    if( !faceTransformations_.empty() )
    {
      // make a copy of the boundary face map because the boundaryFaces_
      // is altered during the search for periodic neighbors
      BoundaryFaceMap faceMap(boundaryFaces_);

      for(auto it = boundaryFaces_.begin(); it != boundaryFaces_.end(); ++it)
      {
        //for dimension == 2 we do not want to search
        // the artificially introduced faces
        if( isArtificialFace( it->first ) )
        {
          continue;
        }

        auto pos = faceMap.find( it->first );
        if( pos != faceMap.end() )
          searchPeriodicNeighbor( faceMap, pos, 1 );
      }
    }

    std::vector<int> simplexTypes(elementType == tetra ? elements_.size() : 0,0);
    //use consistency algorithm or bisection compatibility
    bool madeCompatible = correctElementOrientation(simplexTypes);
    madeCompatible = bool(comm().max( int(madeCompatible) ) ) ;
    if( elementType == hexa && madeCompatible )
    {
      const int numNonEmptyPartitions = comm().sum( int( !elements_.empty() ) );
      if( numNonEmptyPartitions > 1)
          DUNE_THROW( GridError, "Cannot consistently orient a parallel hexahedral grid." );
    }


    numFacesInserted_ = boundaryIds_.size();

    // We need to communicate executing recreateboundaryids
    // because there is communication inside
    // recreateBoundaryIds needs to be called, when
    // the grid numbering has changed, i.e. almost always
    const bool recreateBndIds = (dimension ==2) || addMissingBoundaries || bool(comm().max( int(!faceTransformations_.empty() || madeCompatible) ));

    //We need dimension == 2 here, because it is correcting the face orientation
    //as the 2d faces are not necessarily orientated the right way, we cannot
    //guerantee beforehand to have the right 3d face orientation
    if( recreateBndIds )
      recreateBoundaryIds();

    // sort boundary ids to insert real boundaries first and then fake
    // boundaries
    std::vector< BndPair > boundaryIds;
    boundaryIds.reserve( boundaryIds_.size() );
    for( const auto& bndId : boundaryIds_ )
    {
      ALU3DSPACE Gitter::hbndseg::bnd_t bndType = (ALU3DSPACE Gitter::hbndseg::bnd_t ) bndId.second;
      // skip fake boundaries first
      if( dimension == 2 && bndType == ALU3DSPACE Gitter::hbndseg::closure_2d ) continue;
      boundaryIds.push_back( bndId );
    }

    for( const auto& bndId : boundaryIds_ )
    {
      ALU3DSPACE Gitter::hbndseg::bnd_t bndType = (ALU3DSPACE Gitter::hbndseg::bnd_t ) bndId.second;
      // now insert fake boundaries
      if( dimension == 2 && bndType == ALU3DSPACE Gitter::hbndseg::closure_2d )
      {
        boundaryIds.push_back( bndId );
      }
    }

    // We now create the isRear variables for all boundaries
    // and elements
    std::vector<std::vector<bool> > isRearElements(elements_.size(), elementType == tetra ? std::vector<bool>({false, true, false, true}) : std::vector<bool>({false, true, true, false, false ,true}));
    std::unordered_map<FaceType, bool> isRearBoundaries;
    calculateIsRear( isRearElements, isRearBoundaries);

    assert( boundaryIds.size() == boundaryIds_.size() );
    boundaryIds_.clear();

    // if dump file should be written
    if( allowGridGeneration_ && !temporary )
    {
      std::string filename ( name );

      std::ofstream out( filename.c_str() );
      out.setf( std::ios_base::scientific, std::ios_base::floatfield );
      out.precision( 16 );
      // write ALU macro format header
      ALU3DSPACE MacroFileHeader header;
      header.setType( elementType == hexa ?
          ALU3DSPACE MacroFileHeader::hexahedra : ALU3DSPACE MacroFileHeader::tetrahedra );
      header.write( out );

      // write data of macro grid
      const unsigned int numVertices = vertices_.size();
      // now start writing grid
      out << numVertices << std :: endl;
      typedef typename VertexVector::iterator VertexIteratorType;
      const VertexIteratorType endV = vertices_.end();
      for( VertexIteratorType it = vertices_.begin(); it != endV; ++it )
      {
        const VertexType &vertex = it->first;
        const int globalId = it->second;
        out << globalId ;
        for( unsigned int i = 0; i < dimensionworld; ++i )
          out << " " << vertex[ i ];
        out << std :: endl;
      }

      const unsigned int elemSize = elements_.size();
      out << elemSize << std :: endl;
      for( unsigned int el = 0; el<elemSize; ++el )
      {
        const size_t elemIndex = ordering[ el ];
        std::array< unsigned int, numCorners > element;
        for( unsigned int i = 0; i < numCorners; ++i )
        {
          element[ i ] = elements_[ elemIndex ][ i ];
        }

        out << element[ 0 ];
        for( unsigned int i = 1; i < numCorners; ++i )
          out << " " << element[ i ];

        ALU3DSPACE IsRearFlag isRear( isRearElements[elemIndex] );
        out << " " << isRear << std::endl;
      }

      out << int(periodicBoundaries_.size()) << " " << int(boundaryIds.size()) << std :: endl;
      const typename PeriodicBoundaryVector::iterator endP = periodicBoundaries_.end();
      for( typename PeriodicBoundaryVector::iterator it = periodicBoundaries_.begin(); it != endP; ++it )
      {
        const std::pair< BndPair, BndPair > &facePair = *it;
        out << facePair.first.first[ 0 ];
        for( unsigned int i = 1; i < numFaceCorners; ++i )
          out << " " << facePair.first.first[ numFaceCorners == 3 ? (3 - i) % 3 : i ];
        for( unsigned int i = 0; i < numFaceCorners; ++i )
          out << " " << facePair.second.first[ numFaceCorners == 3 ? (3 - i) % 3 : i ];
        out << std::endl;
      }
      const auto endB = boundaryIds.end();
      for( auto it = boundaryIds.begin(); it != endB; ++it )
      {
        const BndPair& boundaryId = *it;
        out << -boundaryId.second;
        for( unsigned int i = 0; i < numFaceCorners; ++i )
          out << " " << boundaryId.first[ i ];
        out << std::endl;
      }

      // no linkage
      out << int(0) << std::endl;

      out.close();
    } // if( allowGridGeneration_ && !temporary )

    // ALUGrid is taking ownership of bndProjections
    // and is going to delete this pointer
    Grid* grid = createGridObj( name );
    alugrid_assert ( grid );

    // insert grid using ALUGrid macro grid builder
    if( !vertices_.empty() )
    {
      ALU3DSPACE MacroGridBuilder mgb ( grid->getBuilder() );

      // now start inserting grid
      const int vxSize = vertices_.size();

      for( int vxIdx = 0; vxIdx < vxSize ; ++vxIdx )
      {
        const VertexType &vertex = position( vxIdx );
        // insert vertex
        mgb.InsertUniqueVertex( vertex[ 0 ], vertex[ 1 ], vertex[ 2 ], globalId( vxIdx ) );
      }

      const size_t elemSize = elements_.size();
      for( size_t el = 0; el<elemSize; ++el )
      {
        const size_t elemIndex = ordering[ el ];
        if( elementType == hexa )
        {
          int element[ 8 ];
          for( unsigned int i = 0; i < 8; ++i )
          {
            element[ i ] = globalId( elements_[ elemIndex ][ i ] );
          }
          ALU3DSPACE IsRearFlag isRear( isRearElements[elemIndex] );
          mgb.InsertUniqueHexa( element, isRear );
        }
        else if( elementType == tetra )
        {
          int element[ 4 ];
          for( unsigned int i = 0; i < 4; ++i )
          {
            element[ i ] = globalId( elements_[ elemIndex ][ i ] );
          }

          // bisection element type: orientation and type (default 0)
          int type = 0;
          if(dimension == 3 && ALUGrid::refinementType == conforming && madeCompatible )
          {
            type = simplexTypes[ elemIndex ];
          }

          ALU3DSPACE IsRearFlag isRear( isRearElements[elemIndex] );
          ALU3DSPACE SimplexTypeFlag simplexTypeFlag( 0, type );
          mgb.InsertUniqueTetra( element, isRear, simplexTypeFlag );
        }
        else
          DUNE_THROW( GridError, "Invalid element type");
      }

      const auto endB = boundaryIds.end();
      for( auto it = boundaryIds.begin(); it != endB; ++it )
      {
        const BndPair &boundaryId = *it;
        ALU3DSPACE Gitter::hbndseg::bnd_t bndType = (ALU3DSPACE Gitter::hbndseg::bnd_t ) boundaryId.second;
        assert( dimension == 3 ? bndType != ALU3DSPACE Gitter::hbndseg::closure_2d : true );

        // only positive boundary id's are allowed
        assert( bndType > 0 );

        // generate boundary segment pointer
        FaceType faceId ( boundaryId.first);
        std::sort( faceId.begin(), faceId.end() );
        const DuneBoundaryProjectionType* projection = boundaryProjections_[ faceId ];

        ALU3DSPACE ProjectVertexPtr pv;

        if( projection )
        {
          pv.reset( new ALUProjectionType( projection, ALUProjectionType::segment ) );
        }
        else if( (it->second == int(ALU3DSPACE Gitter::hbndseg_STI::closure_2d)
                 && surfaceProjection_ ) )
        {
          pv = surfaceProjection_;
        }
        else if ( globalProjection_ )
        {
          pv = globalProjection_;
        }

        int bndface[ numFaceCorners ];
        for( unsigned int i = 0; i < numFaceCorners; ++i )
        {
          bndface[ i ] = globalId( boundaryId.first[ i ] );
        }
        mgb.InsertUniqueHbnd( bndface, bndType, pv );
      }

      const typename PeriodicBoundaryVector::iterator endP = periodicBoundaries_.end();
      for( typename PeriodicBoundaryVector::iterator it = periodicBoundaries_.begin(); it != endP; ++it )
      {
        const std::pair< BndPair, BndPair > &facePair = *it;
        int perel[ numFaceCorners*2 ];
        for( unsigned int i = 0, k=numFaceCorners; i < numFaceCorners; ++i, ++k )
        {
          // first periodic element
          perel[ i ] = globalId( facePair.first.first [ i ] );
          // second periodic element
          perel[ k ] = globalId( facePair.second.first[ i ] );
        }

        typedef typename ALU3DSPACE Gitter::hbndseg::bnd_t bnd_t ;
        bnd_t bndId[ 2 ] = { bnd_t( facePair.first.second ),
                             bnd_t( facePair.second.second ) };
        mgb.InsertUniquePeriodic( perel, bndId );
      }
    }

    // free memory
    boundaryProjections_.clear();

    // clear vertices
    VertexVector().swap( vertices_ );
    // clear elements
    ElementVector().swap( elements_ );

    if( realGrid_ )
    {
      grid->comm().barrier();
      // make changes in macro grid known in every partition
      grid->completeGrid();
    }

    return grid;
  }


  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    ::generateFace ( const ElementType &element, const int f, FaceType &face )
  {
    typedef ElementTopologyMapping< elementType > ElementTopologyMappingType;
    for( unsigned int i = 0; i < numFaceCorners; ++i )
    {
      const int j = ElementTopologyMappingType :: faceVertex( f, i );
      face[ i ] = element[ j ];
    }
  }


  template< class ALUGrid >
  alu_inline
  bool
  ALU3dGridFactory< ALUGrid >::correctElementOrientation ( std::vector<int>& simplexTypes )
  {
    bool result = false;
    //if there are no elements, skip correction
    if(elements_.empty()) return result;
    // apply mesh-consistency algorithm to hexas if not Cartesian mesh
    // and mesh-consistency package was found during library build
    if( elementType == hexa && ! cartesian_ &&
        detail::correctCubeOrientationAvailable() )
    {
      std::vector<Dune::FieldVector<double,3> > vertices(vertices_.size());
      for(unsigned i = 0 ; i < vertices_.size(); ++i)
      {
        vertices[i] = vertices_[i].first;
      }

      std::size_t elementSize = elements_.size();
      if(!faceTransformations_.empty())
      {
        //Add periodic elements to elements_ vector
        for( auto perElem : periodicBoundaries_ )
        {
          FaceType perFace0 = perElem.first.first;
          FaceType perFace1 = perElem.second.first;
          ElementType elem(8);
          for( int i = 0 ; i < 4 ; ++i)
          {
            elem[i] = perFace0[i];
            elem[i+4] = perFace1[i];
          }
          elements_.push_back(elem);
        }
      }

      if(dimension == 3)
      {
        result = detail::correctCubeOrientation(3, vertices, elements_);
        FaceConsistency faceConsistency(elements_, interiorFaces_, boundaryFaces_, elementSize);
        if(!faceConsistency.consistencyCheck(true))
        {
          faceConsistency.makeFaceConsistent();
          faceConsistency.returnElements(elements_);
        }
      }
      else //if dimension == 2
      {
        //get quad grid
        std::vector<std::vector<unsigned int> > quadElements(elements_);
        for( auto&& quad : quadElements )
          quad.resize(4);
        //consistently orient quads
        result = detail::correctCubeOrientation(2, vertices, quadElements);
        if( result )
        {
          for(unsigned int i = 0 ; i < elements_.size() ; ++i)
          {
            /*std::cout << "Correct element [";
            for(int j = 0 ; j < 8 ; ++j)
            {
              std::cout << elements_[i][j] << ",";
            }
            std::cout << "]" << std::endl;
            std::cout << "with element [";
            for(int j = 0 ; j < 8 ; ++j)
            {
              std::cout << quadElements[i][j] << ",";
            }
            std::cout << "]" << std::endl; */

            for(int j = 0 ; j < 4 ; ++j)
            {

              elements_[i][j] = quadElements[i][j];
              elements_[i][j+4] = quadElements[i][j]+1;
            }
          }
        }
      }

      if(!faceTransformations_.empty())
      {
        //periodic elements are regenerated in recreateboundaryids
        elements_.resize(elementSize);
      }
    }

    if(elementType == tetra)
    {
      result = bisectionCompatibility(simplexTypes);
    }
    return result;
  }

  template< class ALUGrid >
  alu_inline
  void
  ALU3dGridFactory< ALUGrid >::calculateIsRear ( std::vector<std::vector<bool> > & isRearElements, std::unordered_map<FaceType, bool> & isRearBoundaries)
  {
    //In dimension == dimensionworld, we calculate the normal direction of the face and use this for isRear
    //The benefit is, that we do not need communication, as this works for consistently oriented grids
    if(dimension == dimensionworld)
    {
      std::vector<bool> isRear = (elementType == tetra) ?
        std::vector<bool>({false, true, false, true}) : // based on reference tetrahedron
        std::vector<bool>({false, true, true,  false, false, true}); // based on reference hexahedron

      //walk over all elements
      for( std::size_t el =0 ; el < elements_.size(); ++el)
      {
        //calculate det to know whether we have to switch
        auto element = elements_[el];
        auto p0 = position(element[0]);
        auto p1 = position(element[1]);
        auto p2 = position(element[2]);
        auto p3 = position(element[(elementType == tetra) ? 3 : 4]);
        // (p1 - p0) x (p2-p0) * (p3-p0)
        double det = ((p1[1] - p0[1]) * (p2[2] - p0[2]) - (p1[2] - p0[2]) * (p2[1] - p0[1])) * (p3[0] - p0[0])
                   + ((p1[2] - p0[2]) * (p2[0] - p0[0]) - (p1[0] - p0[0]) * (p2[2] - p0[2])) * (p3[1] - p0[1])
                   + ((p1[0] - p0[0]) * (p2[1] - p0[1]) - (p1[1] - p0[1]) * (p2[0] - p0[0])) * (p3[2] - p0[2]);
        for( std::size_t face = 0 ; face < numFaces; ++face)
        {
          //set isRear on all faces correctly
          bool rear = (det < 0) ? isRear[face] : !isRear[face];
          isRearElements[el][face] = rear;
        }
      }
    }
    else // dimension == 2 and dimensionWorld == 3
    {
      //Walk over all elements
      for( std::size_t el =0 ; el < elements_.size(); ++el)
      {
        auto element = elements_[el];
        //walk over all faces
        for( unsigned face = 0 ; face < numFaces; ++face)
        {
          FaceType faceId;
          generateFace( element, face, faceId);
          std::sort(faceId.begin(), faceId.end());
          //check if face has been addressed
          auto faceIt = isRearBoundaries.find(faceId);
          if( faceIt == isRearBoundaries.end() )
          {
            isRearElements[el][face] =false;
            isRearBoundaries.insert(std::make_pair(faceId, true ));
          }
          else
          {
            isRearElements[el][face] = true;
            isRearBoundaries.erase( faceIt );
          }
        }
      }
    }
    assert( isRearElements.size() == elements_.size() );
  }

  template< class ALUGrid >
  alu_inline
  bool
  ALU3dGridFactory< ALUGrid >::bisectionCompatibility ( std::vector<int> & simplexTypes)
  {
    bool madeCompatible = false;
    //pass empty vertexWeights to construct them on the fly in
    //the reordering algorithm
    std::vector<double> vertexWeights;
    if( ALUGrid::elementType == tetra && ! elements_.empty() )
    {
      const std::size_t vSize = vertices_.size();
      for(std::size_t i = 0; i<vSize; ++i)
      {
        //Dim = 2 is only needed for consistency,
        //initial refinement edge is always the longest
        if(dimension == 2)
        {
          vertexWeights.push_back( globalId(i) + 1 );
        }
        //For dim = 2 the ordering presets the refinement edge
        //for structured (and axis-aligned) grids the following results in
        //criss-cross reffinement
        else
        {
          const VertexType& pos = position( i );
          //auto pos = inputPosition(i);
          double weight = 256 * pos[0] + 16 * pos[1] + pos[2];
          vertexWeights.push_back(weight);
        }
      }

      Dune::Timer timer;

      BisectionCompatibility< VertexVector, PeriodicBoundaryVector > bisComp( vertices_, elements_, periodicBoundaries_);

      std::string rankstr ;
      {
        std::stringstream str;
        str << "P[ " << rank_ << " ]: ";
        rankstr = str.str();
      }

      if( bisComp.compatibilityCheck()  )
      {
#ifndef NDEBUG
        std::cout << rankstr << "Grid is compatible!" << std::endl;
#endif
      }
      else
      {
        // mark longest edge for initial refinement
        // successive refinement is done via Newest Vertex Bisection
        if( markLongestEdge_ )
        {
          markLongestEdge( );
        }
#ifndef NDEBUG
        std::cout << rankstr << "Making compatible" << std::endl;
#endif
        if(  bisComp.type0Algorithm( vertexWeights ) )
        {
          madeCompatible = true;
#ifndef NDEBUG
          std::cout << rankstr << "Grid is compatible!!" << std::endl;
          bisComp.stronglyCompatibleFaces();
#endif
          // obtain new element sorting, orientations, and types
          bisComp.returnElements( elements_, simplexTypes );
        }
#ifndef NDEBUG
        else
          std::cout << rankstr << "Could not make compatible!" << std::endl;
#endif
      }
#ifndef NDEBUG
      std::cout << rankstr << "BisectionCompatibility done:" << std::endl;
      std::cout << rankstr << "Elements: " << elements_.size() << " " << timer.elapsed() << " seconds used. " << std::endl;
#endif
    }
    return madeCompatible;
  }

  namespace detail
  {
    template <int dim>
    struct SimpleIntegerSet
    {
      std::array< int, dim > data_;
      unsigned int counter_;
      SimpleIntegerSet() : data_(), counter_( 0 )
      {
        for( int i=0; i<dim; ++i )
        {
          data_[ i ] = -1;
        }
      }

      // only store numbers smaller then dimension
      void insert( int item )
      {
        assert( counter_ < (data_.size()));
        assert( item < dim );
        data_[ counter_ ] = item;
        ++counter_;
      }

      bool contains( int item ) const
      {
        for( unsigned int i=0; i<counter_; ++i )
          if( item == data_[ i ] )
            return true;

        return false;
      }

      int operator [] ( const int i ) const
      {
        assert( i < int(counter_) );
        return data_[ i ];
      }

      unsigned int size() const { return counter_; }

      void insertMissing()
      {
        // this should only be called for hexas
        // after 3 corners have been identified
        // then we insert the forth corner at the end
        std::array< bool, dim > found;
        for( int i=0; i<dim; ++i ) found[ i ] = false;
        assert( counter_ == 3 );
        for( unsigned int i = 0 ; i<counter_; ++i )
          found[ data_[ i ] ] = true ;

        for( int i=0; i<dim; ++i )
        {
          if( ! found[ i ] )
          {
            insert( i );
            return ;
          }
        }
      }
    };
  }

  //Get two sorted face keys,
  //If the faces match by world transformation,
  //key1 and resorted key2 are added into periodicBoundaryVector
  //Also checks boundaryIds_ and erases face there
  template< class ALUGrid >
  alu_inline
  bool ALU3dGridFactory< ALUGrid >::
    identifyFaces ( const Transformation &transformation,
                    const FaceType &key1, const FaceType &key2,
                    const int defaultId )
  {
    const ctype tolerance2 = 1e-12;
    WorldVector w;
    VertexInputType fakeVx;
    VertexType transVx;

    unsigned int startCount = 0;
    // for hexahedral (2d and 3d) we have to check 3 vertices
    // and for tetrahedral element also 3 vertices
    static const unsigned int numCornersToCheck = 3 ;

    // contains all local vertex numbers that have been matched
    detail::SimpleIntegerSet< numFaceCorners > matched;

    // for triangles (tetra and 2d)
    // we can skip the vertex 0 because that is always the same
    if constexpr (elementType == tetra && dimension == 2)
    {
      startCount = 1;
      matched.insert( 0 );
    }

    for( unsigned int i = startCount; i < numCornersToCheck; ++i )
    {
      const VertexType& realVx = position( key1[ i ] );
      if constexpr ( dimension == 2 )
      {
        for(unsigned int d = 0 ; d < dimensionworld ; ++d)
          fakeVx[ d ] = realVx[ d ];

        // evaluate transformation of vertex position
        w = transformation.evaluate( fakeVx );

        for(unsigned int d = 0 ; d < dimensionworld ; ++d)
          transVx[ d ] = w[ d ];
        transVx[ 2 ] = realVx[ 2 ];
      }
      else
      {
        // evaluate transformation of vertex position
        transVx = transformation.evaluate( realVx );
      }

      bool foundOne = false ;
      for( unsigned int j = startCount; j < numCornersToCheck; ++j )
      {
        // if j was already matched then simply skip here
        if( matched.contains( j ) )
          continue;

        // check that difference is small then we have a match
        if( (transVx - position( key2[ j ] )).two_norm2() < tolerance2 )
        {
          matched.insert( j );
          foundOne = true;
          break;
        }
      }

      // if no match was found then faces don't match
      if( !foundOne) return false ;
    }

    if( matched.size() < numCornersToCheck )
      return false;

    if( numCornersToCheck < numFaceCorners )
    {
      // insert the missing vertex number
      matched.insertMissing();
    }

    // at this point matched should contain numFaceCorners numbers
    assert( matched.size() == numFaceCorners );

    FaceType key0; // resorted face
    for( unsigned int i = 0; i < numFaceCorners; ++i )
    {
      key0[ i ] = key2[ matched[ i ] ];
    }

    int bndId[ 2 ] = { 20, 20 };
    FaceType keys[ 2 ] = { key1, key2 };

    for( int i=0; i<2; ++i )
    {
      auto it = boundaryFaces_.find( keys[ i ] );
      int localFaceIndex = getFaceIndex( it->second, it->first );
      FaceType key;
      generateFace( elements_[it->second], localFaceIndex, key );

      auto pos = boundaryIds_.find( key );

      if( pos != boundaryIds_.end() )
      {
        bndId[ i ] = (*pos).second ;
        boundaryIds_.erase( pos );
      }
    }

    // convention is that both periodic faces
    // are inserted with matching vertices
    BndPair bnd0 ( key0, bndId[ 0 ] );
    BndPair bnd1 ( key1, bndId[ 1 ] );

    periodicBoundaries_.push_back( std::make_pair( bnd0, bnd1 ) );

    return true;
  }

  template< class ALUGrid >
  alu_inline
  bool ALU3dGridFactory< ALUGrid >
    ::isArtificialFace( const FaceType& face ) const
  {
    if( dimension == 2 )
    {
      // if the first vertex is not the artificial vertex
      // then the face is the bottom face which is the artificial face
      if(elementType == tetra && face[0] != 0)
        return true;

      //if the first and third vertex do not differ by 1
      if(elementType == hexa )
      {
        //&& (face[0] != face[1]-1) )
        int odd = 0;
        int even = 0;
        for( const auto& vx : face )
        {
          if( vx % 2 == 0 )
            ++even;
          if( vx % 2 == 1 )
            ++odd;
        }

        if( even == 4 || odd == 4 )
          return true;
      }
    }

    return false;
  }



  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    ::searchPeriodicNeighbor ( BoundaryFaceMap &boundaryFaceMap,
                               typename BoundaryFaceMap::iterator& pos,
                               const int defaultId )
  {
    typedef typename FaceTransformationVector::const_iterator TrafoIterator;
    typedef typename BoundaryFaceMap::iterator BoundaryFaceMapIterator;

    if( !faceTransformations_.empty() )
    {
      FaceType key1 = pos->first;

      for( BoundaryFaceMapIterator fit = boundaryFaceMap.begin(); fit != boundaryFaceMap.end(); ++fit )
      {
        if( fit == pos ) continue;

        //for dimension == 2 we do not want to search
        // the artificially introduced faces
        if( isArtificialFace( fit->first ) )
        {
          continue;
        }

        FaceType key2 = fit->first;

        const TrafoIterator trend = faceTransformations_.end();
        for( TrafoIterator trit = faceTransformations_.begin(); trit != trend; ++trit )
        {
          if( identifyFaces( *trit, key1, key2, defaultId) ||
              identifyFaces( *trit, key2, key1, defaultId) )
          {
            fit = boundaryFaceMap.erase( fit );
            pos = boundaryFaceMap.erase( pos );
            return;
          }
        }
      }
    }
  }


  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    ::reinsertBoundary ( const typename BoundaryFaceMap::const_iterator &pos, const int id )
  {
    doInsertBoundary( pos->second, getFaceIndex(pos->second, pos->first), id );
  }

  //clears the boundaryFaces_ to recreate all boundaries
  //also finds process boundaries
  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    ::recreateBoundaryIds ( const int defaultId )
  {
    typedef typename BoundaryFaceMap::iterator FaceIterator;

    // swap current boundary ids with an empty vector
    BoundaryIdMap boundaryIds;
    boundaryIds_.swap( boundaryIds );
    alugrid_assert ( boundaryIds_.size() == 0 );

    // list of face that should be removed
    std::set< FaceType > toBeDeletedFaces;

    // add all current boundary ids again (with their reordered keys)
    typedef typename BoundaryIdMap::iterator BoundaryIterator;
    const BoundaryIterator bndEnd = boundaryIds.end();
    for( BoundaryIterator bndIt = boundaryIds.begin(); bndIt != bndEnd; ++bndIt )
    {
      FaceType key = bndIt->first;
      std::sort( key.begin(), key.end() );
      FaceIterator pos = boundaryFaces_.find( key );

      if( pos == boundaryFaces_.end() )
      {
        DUNE_THROW( GridError, "Inserted boundary segment is not part of the boundary." );
      }

      reinsertBoundary( pos, bndIt->second );
      toBeDeletedFaces.insert( key );
    }

    if(!faceTransformations_.empty() )
    {
      //restore periodic boundaries and
      //add periodic boundaries to deleted faces
      for( auto & periodicPair : periodicBoundaries_ )
      {
        FaceType key0 = periodicPair.first.first;
        std::sort(key0.begin(), key0.end());
        FaceIterator pos = boundaryFaces_.find( key0 );
        if( pos == boundaryFaces_.end() )
        {
          DUNE_THROW( GridError, "Inserted periodic boundary segment is not part of the boundary." );
        }
        generateFace( elements_[pos->second], getFaceIndex(pos->second, pos->first), periodicPair.first.first);
        toBeDeletedFaces.insert( key0 );

        FaceType key1 = periodicPair.second.first;
        std::sort(key1.begin(), key1.end());
        pos = boundaryFaces_.find( key1 );
        if( pos == boundaryFaces_.end() )
        {
          DUNE_THROW( GridError, "Inserted periodic boundary segment is not part of the boundary." );
        }
        generateFace( elements_[pos->second], getFaceIndex(pos->second, pos->first), periodicPair.second.first);
        toBeDeletedFaces.insert( key1 );
      }
    }

    // erase faces that are boundries
    for( const auto& key : toBeDeletedFaces )
    {
      boundaryFaces_.erase( key );
    }

    // communicate unidentified boundaries and find process borders)
    // use the Grids communicator (ALUGridNoComm or ALUGridMPIComm)
    typename ALUGrid::CollectiveCommunication comm( communicator_ );

    int numBoundariesMine = boundaryFaces_.size();
    std::vector< int > boundariesMine( numFaceCorners * numBoundariesMine );
    typedef std::map< FaceType, FaceType, FaceLess > GlobalToLocalFaceMap;
    GlobalToLocalFaceMap globalFaceMap;
    {
      const FaceIterator faceEnd = boundaryFaces_.end();
      int idx = 0;
      for( FaceIterator faceIt = boundaryFaces_.begin(); faceIt != faceEnd; ++faceIt )
      {
        FaceType key;
        for( unsigned int i = 0; i < numFaceCorners; ++i )
          key[ i ] = vertices_[ faceIt->first[ i ] ].second;
        std::sort( key.begin(), key.end() );
        globalFaceMap.insert( std::make_pair(key, faceIt->first) );

        for( unsigned int i = 0; i < numFaceCorners; ++i )
          boundariesMine[ idx++ ] = key[ i ];
      }
    }

    const int numBoundariesMax = comm.max( numBoundariesMine );


    // get out of here, if the face maps on all processors are empty (all boundaries have been inserted)
    if( numBoundariesMax == 0 ) return ;

    // get internal boundaries from each process
    std::vector< std::vector< int > > boundariesEach( comm.size() );

#if HAVE_MPI
    if( comm.size() > 1 )
    {
      ALU3DSPACE MpAccessMPI mpAccess( Dune::MPIHelper::getCommunicator() );
      // collect data from all processes (use MPI_COMM_WORLD here) since in this case the
      // grid must be parallel if we reaced this point
      boundariesEach = mpAccess.gcollect( boundariesMine );
#ifndef NDEBUG
      // make sure everybody is on the same page
      mpAccess.barrier();
#endif
    }
#endif // #if HAVE_MPI

    boundariesMine.clear();

    for( int p = 0; p < comm.size(); ++p )
    {
      if( p != comm.rank() )
      {
        const std::vector< int >& boundariesRank = boundariesEach[ p ];
        const int bSize = boundariesRank.size();
        for( int idx = 0; idx < bSize; idx += numFaceCorners )
        {
          FaceType key;
          for( unsigned int i = 0; i < numFaceCorners; ++i )
            key[ i ] = boundariesRank[ idx + i ];

          const typename GlobalToLocalFaceMap :: const_iterator pos_gl = globalFaceMap.find( key );
          if( pos_gl != globalFaceMap.end() )
          {
            FaceIterator pos = boundaryFaces_.find( pos_gl->second );
            if ( pos != boundaryFaces_.end() )
            {
              reinsertBoundary(  pos, ALU3DSPACE ProcessorBoundary_t );
              pos = boundaryFaces_.erase( pos );
            }
            else
            {
              // should never get here but does when this method is called to
              // construct the "reference" elements for alu
            }
          }
        }
      }
      boundariesEach[ p ].clear();
    } // end for all p

    // add all new boundaries (with defaultId)
    const FaceIterator faceEnd = boundaryFaces_.end();
    for( FaceIterator faceIt = boundaryFaces_.begin(); faceIt != faceEnd; ++faceIt )
      reinsertBoundary( faceIt, defaultId );

  }

#if ! COMPILE_ALUGRID_INLINE
  // Instantiation -3-3
  template class ALUGrid< 3, 3, cube, nonconforming >;
  template class ALUGrid< 3, 3, simplex, nonconforming >;
  template class ALUGrid< 3, 3, simplex, conforming >;

  template class ALU3dGridFactory< ALUGrid< 3, 3, cube, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 3, 3, simplex, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 3, 3, simplex, conforming > >;

  // Instantiation -2-3
  template class ALUGrid< 2, 3, cube, nonconforming >;
  template class ALUGrid< 2, 3, simplex, nonconforming >;
  template class ALUGrid< 2, 3, simplex, conforming >;

  template class ALU3dGridFactory< ALUGrid< 2, 3, cube, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 2, 3, simplex, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 2, 3, simplex, conforming > >;

  // Instantiation -2-2
  template class ALUGrid< 2, 2, cube, nonconforming >;
  template class ALUGrid< 2, 2, simplex, nonconforming >;
  template class ALUGrid< 2, 2, simplex, conforming >;

  template class ALU3dGridFactory< ALUGrid< 2, 2, cube, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 2, 2, simplex, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 2, 2, simplex, conforming > >;
#endif // ! COMPILE_ALUGRID_INLINE
}

#endif // end DUNE_ALU3DGRID_FACTORY_CC
