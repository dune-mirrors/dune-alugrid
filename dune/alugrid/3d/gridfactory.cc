#ifndef DUNE_ALU3DGRID_FACTORY_CC
#define DUNE_ALU3DGRID_FACTORY_CC

#if COMPILE_ALUGRID_INLINE == 0
#include <config.h>
#endif

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <list>

#include <dune/common/parallel/mpicollectivecommunication.hh>
#include <dune/alugrid/3d/gridfactory.hh>

#include <dune/alugrid/impl/parallel/zcurve.hh>

#include <utils/bisection-compatibility/bisectioncompatibility.hh>

#if COMPILE_ALUGRID_INLINE
#define alu_inline inline
#else
#define alu_inline
#endif

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
        if(vertices_.size() == 0)
        {
          // fake vertex that every tetra is connected to
          vertices_.push_back( std::make_pair( VertexType(1.0), 0 ) );
        }

        //setting the global id to odd is convenience
        //we are then able to set the is2d() flag in the
        //alugrid implementation just by checking the index
        vertices_.push_back( std::make_pair( pos, 2*globalId+1 ) );
      }
      else if(elementType == hexa)
      {
        // it is here important, that the fake vertices
        // are of globalId +1 to the real ones
        vertices_.push_back( std::make_pair( pos, 2*globalId+1 ) );
        VertexType pos1 (pos);
        pos1[2] += 1.0 ;
        vertices_.push_back( std::make_pair( pos1, 2*globalId+2 ) );
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
          // multiply original number with 2 to get the indices of the 2dvalid vertices
          element[ i ]    = vertices[ i ] * 2;
          element[ i+4 ]  = element [ i ] + 1;
        }
        elements_.push_back(element);
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
  insertBoundaryProjection( const DuneBoundaryProjectionType& bndProjection )
  {
    if( globalProjection_ )
      DUNE_THROW(InvalidStateException,"You can only insert one globalProjection");

    globalProjection_ = &bndProjection;
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

  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >::markLongestEdge ( std::vector< bool >& elementOrientation )
  {
    std::cerr << "Marking longest edge for initial refinement..." << std::endl;

    // DUNE reference element edge numbering
    static const int edges[ 6 ][ 2 ] = { {0,1}, {0,2}, {1,2}, {0,3}, {1,3}, {2,3} };

    static const int swapSuccessor[ 6 ] = { -1, 1, -1, -1, 3, -1 };

    // DUNE reference element
    static const int shift[ 6 ] = { 0, 0, 1, 3, 0, 2 };

    const int numVertices = 4;

    //create the vertex priority List
    const int numberOfElements = elements_.size();

    for(int el = 0; el < numberOfElements ; ++el )
    {
      auto& element = elements_[ el ];
      assert( int(element.size()) == numVertices );

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

      // rotate element
      if( shift[ edge ] > 0 )
      {
        assert( element.size() == 4 );
        const auto old( element );
        const int s = shift[ edge ];
        for( int j = 0; j < numVertices; ++j )
        {
          element[ j ] = old[ (j+s) % numVertices ];
        }
      }

      if( swapSuccessor[ edge ] > 0 )
      {
        std::swap( element[  swapSuccessor[ edge ] ],
                   element[ (swapSuccessor[ edge ] + 1) % numVertices ] );
        //std::swap( elements_[ el ][ 1 ], elements_[ el ][ 2 ] );
      }

      Dune::FieldMatrix< double, 4, 3 > p( 0 );
      Dune::FieldMatrix< double, 3, 3 > matrix( 0 );

      for( int j=0; j<4; ++j )
      {
        p[ j ] = vertices_[ element[ j ] ].first;
      }

      for( int j=0; j<3; ++j )
      {
        matrix [j] = p[j+1] - p[0];
      }

      double det = matrix.determinant();
      std::cout << "el = " << el << "  det = " << det << std::endl;
      if( det > 0 )
      {
        std::swap( element[ 2 ], element[ 3 ] );
      }

      for( int j=0; j<4; ++j )
      {
        p[ j ] = vertices_[ element[ j ] ].first;
      }

      for( int j=0; j<3; ++j )
      {
        matrix [j] = p[j+1] - p[0];
      }
      det = matrix.determinant();
      std::cout << "el = " << el << "  det = " << det << std::endl;
    }
  }

  template< class ALUGrid >
  alu_inline
  ALUGrid *ALU3dGridFactory< ALUGrid >::createGrid ()
  {
    return createGrid( true, true, "" );
  }

  template< class ALUGrid >
  alu_inline
  ALUGrid *ALU3dGridFactory< ALUGrid >
    ::createGrid ( const bool addMissingBoundaries, const std::string dgfName )
  {
    return createGrid( addMissingBoundaries, true, dgfName );
  }

  template< class ALUGrid >
  alu_inline
  ALUGrid *ALU3dGridFactory< ALUGrid >
    ::createGrid ( const bool addMissingBoundaries, bool temporary, const std::string name )
  {
    typedef typename BoundaryIdMap :: iterator  BoundaryIdIteratorType;
    BoundaryProjectionVector* bndProjections = 0;

    correctElementOrientation();

    std::vector< unsigned int >& ordering = ordering_;
    // sort element given a hilbert space filling curve (if Zoltan is available)
    sortElements( vertices_, elements_, ordering );


    bool make6 = true;
    std::vector< bool > elementOrientation( elements_.size(), true );
    std::vector< int  > simplexTypes;

    if(dimension == 3 && ALUGrid::refinementType == conforming )
    {
      BisectionCompatibility< VertexVector > bisComp( vertices_, elements_, false);
      if(bisComp.make6CompatibilityCheck())
      {
        std::cout << "Grid is compatible!" << std::endl;
      }
      else
      {
        make6 = false;

        // mark longest edge for initial refinement
        // successive refinement is done via Newest Vertex Bisection
        // markLongestEdge( elementOrientation );

        std::cout << "Making compatible" << std::endl;
        if( bisComp.type0Algorithm() )
        {
          std::cout << "Grid is compatible!!" << std::endl;

          // obtain new element sorting, orientations, and types
          bisComp.returnElements( elements_, elementOrientation, simplexTypes );
        }
        else
          std::cout << "Could not make compatible!" << std::endl;
      }
    }

    numFacesInserted_ = boundaryIds_.size();

    //We need dimension == 2 here, because it is correcting the face orientation
    //as the 2d faces are not necessarily orientated the right way, we cannot
    //guerantee beforehand to have the right 3d face orientation
    //
    //Another way would be to store faces as element number + local face index and
    // create them AFTER correctelementorientation was called!!
    if( addMissingBoundaries || ! faceTransformations_.empty() || dimension == 2 )
      recreateBoundaryIds();

    // if dump file should be written
    if( allowGridGeneration_ && !temporary )
    {
      std::string filename ( name );

      std::ofstream out( filename.c_str() );
      out.setf( std::ios_base::scientific, std::ios_base::floatfield );
      out.precision( 16 );
      if( elementType == tetra )
        out << "!Tetrahedra";
      else
        out << "!Hexahedra";

      const unsigned int numVertices = vertices_.size();
      // print information about vertices and elements
      // to header to have an easy check
      out << "  ( noVertices = " << numVertices;
      out << " | noElements = " << elements_.size() << " )" << std :: endl;

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
      out << elemSize << " " << int(numCorners) << std :: endl;
      for( unsigned int el = 0; el<elemSize; ++el )
      {
        const size_t elemIndex = ordering[ el ];
        array< unsigned int, numCorners > element;
        for( unsigned int i = 0; i < numCorners; ++i )
        {
          const unsigned int j = ElementTopologyMappingType::dune2aluVertex( i );
          element[ j ] = elements_[ elemIndex ][ i ];
        }

        out << element[ 0 ];
        for( unsigned int i = 1; i < numCorners; ++i )
          out << " " << element[ i ];
        out << std :: endl;
      }

      out << int(periodicBoundaries_.size()) << " " << int(boundaryIds_.size()) << std :: endl;
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
      const BoundaryIdIteratorType endB = boundaryIds_.end();
      for( BoundaryIdIteratorType it = boundaryIds_.begin(); it != endB; ++it )
      {
        const std::pair< FaceType, int > &boundaryId = *it;
        out << -boundaryId.second;
        for( unsigned int i = 0; i < numFaceCorners; ++i )
          out << " " << boundaryId.first[ i ];
        out << std::endl;
      }

      // no linkage
      out << int(0) << std::endl;

      out.close();
    } // if( allowGridGeneration_ && !temporary )

    const size_t boundarySegments = boundaryIds_.size();

    const size_t bndProjectionSize = boundaryProjections_.size();
    if( bndProjectionSize > 0 || (globalProjection_ && dimension == 2) )
    {
      // the memory is freed by the grid on destruction
      bndProjections = new BoundaryProjectionVector( boundarySegments,
                                                    (DuneBoundaryProjectionType*) 0 );
      const BoundaryIdIteratorType endB = boundaryIds_.end();
      int segmentIndex = 0;
      for( BoundaryIdIteratorType it = boundaryIds_.begin(); it != endB; ++it, ++segmentIndex )
      {
        // generate boundary segment pointer
        FaceType faceId ( (*it).first);
        std::sort( faceId.begin(), faceId.end() );

        const DuneBoundaryProjectionType* projection = boundaryProjections_[ faceId ];

        // if no projection given we use global projection, otherwise identity
        if( ! projection && !(it->second == int(boundaryId2d)) && globalProjection_ )
        {
          typedef BoundaryProjectionWrapper< dimensionworld > ProjectionWrapperType;
          // we need to wrap the global projection because of
          // delete in desctructor of ALUGrid
          projection = new ProjectionWrapperType( *globalProjection_ );
          alugrid_assert ( projection );
        }

        // copy pointer
        (*bndProjections)[ segmentIndex ] = projection;
      }
    }

    // free memory
    boundaryProjections_.clear();

    // if we have a vector reset global projection
    // because empty positions are filled with global projection anyway
    if( bndProjections ) globalProjection_ = 0;

    // ALUGrid is taking ownership of bndProjections
    // and is going to delete this pointer
    Grid* grid = createGridObj( bndProjections , name );
    alugrid_assert ( grid );

    // remove pointers
    globalProjection_ = 0;
    // is removed by grid instance
    bndProjections    = 0;

    // insert grid using ALUGrid macro grid builder
    if( !vertices_.empty() )
    {
      ALU3DSPACE MacroGridBuilder mgb ( grid->getBuilder(), grid->vertexProjection() );

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
            const unsigned int j = ElementTopologyMappingType::dune2aluVertex( i );
            element[ j ] = globalId( elements_[ elemIndex ][ i ] );
          }
          mgb.InsertUniqueHexa( element );
        }
        else if( elementType == tetra )
        {
          int element[ 4 ];
          for( unsigned int i = 0; i < 4; ++i )
          {
            const unsigned int j = ElementTopologyMappingType::dune2aluVertex( i );
            element[ j ] = globalId( elements_[ elemIndex ][ i ] );
          }

          // bisection element type: orientation and type (default 0)
          int type = 0;
          int orientation = (dimension == 3 ? (elemIndex % 2) : 0);
          if(dimension == 3 && ALUGrid::refinementType == conforming && !(make6) )
          {
            assert( !elementOrientation.empty() );
            orientation = (elementOrientation[ elemIndex ]);

            assert( !simplexTypes.empty() );
            type = simplexTypes[ elemIndex ];
          }
          ALU3DSPACE SimplexTypeFlag simplexTypeFlag( orientation, type );
          mgb.InsertUniqueTetra( element, simplexTypeFlag );
        }
        else
          DUNE_THROW( GridError, "Invalid element type");
      }

      const BoundaryIdIteratorType endB = boundaryIds_.end();
      for( BoundaryIdIteratorType it = boundaryIds_.begin(); it != endB; ++it )
      {
        const BndPair &boundaryId = *it;
        ALU3DSPACE Gitter::hbndseg::bnd_t bndType = (ALU3DSPACE Gitter::hbndseg::bnd_t ) boundaryId.second;

        if( elementType == hexa )
        {
          int bndface[ 4 ];
          for( unsigned int i = 0; i < numFaceCorners; ++i )
          {
            bndface[ i ] = globalId( boundaryId.first[ i ] );
          }
          mgb.InsertUniqueHbnd4( bndface, bndType );
        }
        else if( elementType == tetra )
        {
          int bndface[ 3 ];
          for( unsigned int i = 0; i < numFaceCorners; ++i )
          {
            bndface[ i ] = globalId( boundaryId.first[ i ] );
          }
          mgb.InsertUniqueHbnd3( bndface, bndType );
        }
        else
          DUNE_THROW( GridError, "Invalid element type");
      }

      const typename PeriodicBoundaryVector::iterator endP = periodicBoundaries_.end();
      for( typename PeriodicBoundaryVector::iterator it = periodicBoundaries_.begin(); it != endP; ++it )
      {
        const std::pair< BndPair, BndPair > &facePair = *it;
        if( elementType == hexa )
        {
          int perel[ 8 ];
          for( unsigned int i = 0; i < numFaceCorners; ++i )
          {
            perel[ i+0 ] = globalId( facePair.first.first[ i ] );
            perel[ i+4 ] = globalId( facePair.second.first[ i ] );
          }

          typedef typename ALU3DSPACE Gitter::hbndseg::bnd_t bnd_t ;
          bnd_t bndId[ 2 ] = { bnd_t( facePair.first.second ),
                               bnd_t( facePair.second.second ) };
          mgb.InsertUniquePeriodic4( perel, bndId );

        }
        else if( elementType == tetra )
        {
          int perel[ 6 ];
          for( unsigned int i = 0; i < 3; ++i )
          {
            perel[ i+0 ] = globalId( facePair.first.first[ (3 - i) % 3 ] );
            perel[ i+3 ] = globalId( facePair.second.first[ (3 - i) % 3 ] );
          }
          typedef typename ALU3DSPACE Gitter::hbndseg::bnd_t bnd_t ;
          bnd_t bndId[ 2 ] = { bnd_t( facePair.first.second ),
                               bnd_t( facePair.second.second ) };
          mgb.InsertUniquePeriodic3( perel, bndId );
        }
        else
          DUNE_THROW( GridError, "Invalid element type" );
      }
    }

    // clear vertices
    VertexVector().swap( vertices_ );
    // clear elements
    ElementVector().swap( elements_ );
    // free memory
    boundaryIds_.clear();

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
    const int falu = ElementTopologyMappingType :: generic2aluFace( f );
    for( unsigned int i = 0; i < numFaceCorners; ++i )
    {
      const int j = ElementTopologyMappingType :: faceVertex( falu, i );
      const int k = ElementTopologyMappingType :: alu2genericVertex( j );
      face[ i ] = element[ k ];
    }
  }


  template< class ALUGrid >
  alu_inline
  void
  ALU3dGridFactory< ALUGrid >::correctElementOrientation ()
  {
    //if there are no elements, do not correct Orientation
    if(elements_.begin() == elements_.end()) return;

    //for 2,3 we orient the surface -
    // we choose the orientation on one element
    // and then set the orientation on the neighbour to be the same
    // and iterate that over the whole grid, in hope that we have an
    // orientable surface.
    if(dimension ==2 && dimensionworld == 3 )
    {
      // A 2d face type, as we want to work in 2d
      typedef std::array<unsigned int, 2>  Face2Type;

      const int numFaces = (elementType == tetra) ? 3 : 4;
      //the nextIndex denotes the indices of the 2d element
      //inside the 3d element in circular order
      std::vector <unsigned int> nextIndex ({1,2,3});
      if(elementType == hexa)
      {
        nextIndex.resize(4,0);
        nextIndex[0]=0;
        nextIndex[1]=1;
        nextIndex[2]=3;
        nextIndex[3] =2;
      }

      //The sorted faces that are pending to be worked on with
      //the corresponding twist
      typedef std::map< Face2Type, int > FaceMap ;
      typedef FaceMap::iterator FaceIterator;
      FaceMap activeFaces;

      //returns true if element is done
      std::vector<bool> doneElements(elements_.size(), false);

      //The Faces already worked on
      std::set< Face2Type > doneFaces;

      //get first element
      ElementType &element = elements_[0];
      //choose orientation as given by first inserted element and
      //build oriented faces and add to list of active faces
      for(int i = 0; i < numFaces ; ++i)
      {
        Face2Type face = {{element[ nextIndex[i] ], element[ nextIndex[ (i+1)%numFaces ] ]}};
        //this is the twist with respect to the global face orientation
        // we need it once from each side
        int twist = face[0] < face[1] ? 0 : -1;
        std::sort(face.begin(),face.end());
        activeFaces.insert( std::make_pair ( face, twist ) );
      }
      doneElements[0] =true;

      while(!(activeFaces.empty()))
      {
        //get iterator
        FaceIterator faceBegin = activeFaces.begin();

        const Face2Type &currentFace = faceBegin->first;

        //if face is in doneFaces, just remove the face from the active face list
        //this should actually never happen, but just to be sure
        if(doneFaces.find(currentFace) != doneFaces.end())
        {
          activeFaces.erase(currentFace);
          //while loop continue
          continue;
        }

        //get twist
        int twist = faceBegin->second;
        bool found = false;
        int cnt = 0;
        //find face in element list
        const typename ElementVector::iterator elementEnd = elements_.end();
        for( typename ElementVector::iterator elementIt = elements_.begin();
          elementIt != elementEnd; ++elementIt, ++cnt )
        {
          ElementType &outerElement  = *elementIt;
          //we already treated this element if doneElements is true
          if ( doneElements[cnt] ) continue;
          for(int i=0; i<3 ;++i)
          {
            if (outerElement[nextIndex[i]] == currentFace[0])
            {
              if( outerElement[ nextIndex[(i+1)%numFaces] ] == currentFace[1]  )
              {
                if(twist == 0)
                {
                  //correct element orientation
                  std::swap(outerElement[1], outerElement[2]);
                  if(elementType == hexa )
                    std::swap(outerElement[5], outerElement[6]);
                }
                found =true;
              }
              else if(outerElement[nextIndex[(i-1+numFaces)%numFaces]] == currentFace[1] )
              {
                if(twist < 0)
                {
                  //correct element orientation
                  std::swap(outerElement[1], outerElement[2]);
                  if(elementType == hexa )
                    std::swap(outerElement[5], outerElement[6]);
                }
                found = true;
              }
              else //this is not the element you are looking for - break innermost for
                break;

              //build the faces of outerElement with twists
              for (int f = 0 ; f< numFaces ; ++f)
              {
                Face2Type face =  {{ outerElement[ nextIndex[ f%numFaces ] ],outerElement[nextIndex[(f+1)%numFaces]] }} ;
                int twist = face[0] < face[1] ? 0 : -1;
                std::sort(face.begin(),face.end());
                if(face == currentFace) continue;

                //check that it is not in doneFaces
                if(doneFaces.find(face) == doneFaces.end())
                {
                //check that it is not in activeFaces
                  if(activeFaces.find(face) == activeFaces.end())
                     activeFaces.insert(std::make_pair( face , twist ) );
                  //here we can make the orientability check - see assert
                  else
                  {
                    // alugrid_assert(std::abs(activeFaces.find(face)->second - twist) == 1);
                    activeFaces.erase(face);
                  }
                }
              }
              //break inner for loop, as we do not need it anymore
              break;
            }
          }

          //break element for loop if we found the element
          //and set doneElements true
          if(found)
          {
            doneElements[cnt] = true;
            break;
          }
        }
        // add the sorted face to doneFaces with innerElement and outerElement
        doneFaces.insert( currentFace );
        //remove face from activeFaces (if not found it is a boundary)
        activeFaces.erase(currentFace);
      } //end while
      return;
    }

    //for all other cases we orient the elements by having a positive 3d volume

      const typename ElementVector::iterator elementEnd = elements_.end();
      for( typename ElementVector::iterator elementIt = elements_.begin();
           elementIt != elementEnd; ++elementIt )
      {
        ElementType &element = *elementIt;

        const VertexType &p0 = position( element[ 0 ] );
        VertexType p1, p2, p3;

        if( elementType == tetra )
        {
          p1 = position( element[ 1 ] );
          p2 = position( element[ 2 ] );
          p3 = position( element[ 3 ] );
        }
        else
        {
          p1 = position( element[ 1 ] );
          p2 = position( element[ 2 ] );
          p3 = position( element[ 4 ] );
        }
        p1 -= p0; p2 -= p0; p3 -= p0;

        VertexType n;
        n[ 0 ] = p1[ 1 ] * p2[ 2 ] - p2[ 1 ] * p1[ 2 ];
        n[ 1 ] = p1[ 2 ] * p2[ 0 ] - p2[ 2 ] * p1[ 0 ];
        n[ 2 ] = p1[ 0 ] * p2[ 1 ] - p2[ 0 ] * p1[ 1 ];

        if( n * p3 > 0 )
          continue;



        if( elementType == hexa )
        {
        //we changed this,because for the 2d case it is important, that the valid vertices 0,1,2,3 remain the vertices 0,1,2,3
        //  for( int i = 0; i < 4; ++i )
        //    std::swap( element[ i ], element[ i+4 ] );
          std::swap( element[ 5 ], element[ 6 ] );
          std::swap( element[ 1 ], element[ 2 ] );
        }
        else
          std::swap( element[ 2 ], element[ 3 ] );
      } // end of loop over all elements
  }


  template< class ALUGrid >
  alu_inline
  bool ALU3dGridFactory< ALUGrid >
    ::identifyFaces ( const Transformation &transformation,
                      const FaceType &key1, const FaceType &key2,
                      const int defaultId )
  {
    FaceType key0;
    if(dimension == 3 || elementType ==hexa)
    {
      WorldVector w = transformation.evaluate( inputPosition( key1[ 0 ] ) );
      int org = -1;
      for( unsigned int i = 0; i < numFaceCorners; ++i )
      {
        if( (w - inputPosition( key2[ i ] )).two_norm() < 1e-6 )
        {
          org = i;
          break;
        }
      }
      if( org < 0 )
        return false;

      key0[ 0 ] = key2[ org ];
      for( unsigned int i = 1; i < numFaceCorners; ++i )
      {
        w = transformation.evaluate( inputPosition( key1[ i ] ) );
        const int j = ((org+numFaceCorners)-i) % numFaceCorners;
        if( (w - inputPosition( key2[ j ] )).two_norm() >= 1e-6 )
          return false;
        key0[ i ] = key2[ j ];
      }
    }
    else //if dimension == 2 && elementType == tetra
    {
      if(key1[0] != 0 || key2[0] != 0) return false;
      WorldVector w = transformation.evaluate( inputPosition( key1[ 1 ] ) );
      int org = -1;
      for( unsigned int i = 1; i < numFaceCorners; ++i )
      {
        if( (w - inputPosition( key2[ i ] )).two_norm() < 1e-6 )
        {
          org = i;
          break;
        }
      }
      if( org < 0 )
        return false;

      key0[ 0 ] = 0;
      key0[ 1 ] = key2[ org ];
      w = transformation.evaluate( inputPosition( key1[ 2 ] ) );
      const int j = (org == 1) ? 2 : 1;
      if( (w - inputPosition( key2[ j ] )).two_norm() >= 1e-6 )
        return false;
      key0[ 2 ] = key2[ j ];
    }

    int bndId[ 2 ] = { 20, 20 };
    FaceType keys[ 2 ] = { key1, key2 };

    for( int i=0; i<2; ++i )
    {
      typedef typename BoundaryIdMap :: iterator iterator ;
      iterator pos = boundaryIds_.find( keys[ i ] );

      if( pos != boundaryIds_.end() )
      {
        bndId[ i ] = (*pos).second ;
        boundaryIds_.erase( pos );
      }
    }

    BndPair bnd0 ( key0, bndId[ 0 ] );
    BndPair bnd1 ( key1, bndId[ 1 ] );

    periodicBoundaries_.push_back( std::make_pair( bnd0, bnd1 ) );

    return true;
  }


  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    ::searchPeriodicNeighbor ( FaceMap &faceMap, const typename FaceMap::iterator &pos,
                               const int defaultId )
  {
    typedef typename FaceTransformationVector::const_iterator TrafoIterator;
    typedef typename FaceMap::iterator FaceMapIterator;

    if( !faceTransformations_.empty() )
    {
      FaceType key1;
      generateFace( pos->second, key1 );

      const FaceMapIterator fend = faceMap.end();
      for( FaceMapIterator fit = faceMap.begin(); fit != fend; ++fit )
      {
        FaceType key2;
        generateFace( fit->second, key2 );

        const TrafoIterator trend = faceTransformations_.end();
        for( TrafoIterator trit = faceTransformations_.begin(); trit != trend; ++trit )
        {
          if( identifyFaces( *trit, key1, key2, defaultId) ||
              identifyFaces( *trit, key2, key1, defaultId) )
          {
            faceMap.erase( fit );
            faceMap.erase( pos );
            return;
          }
        }
      }
    }
  }


  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    ::reinsertBoundary ( const FaceMap &faceMap, const typename FaceMap::const_iterator &pos, const int id )
  {
    doInsertBoundary( pos->second.first, pos->second.second, id );
  }


  template< class ALUGrid >
  alu_inline
  void ALU3dGridFactory< ALUGrid >
    ::recreateBoundaryIds ( const int defaultId )
  {
    typedef typename FaceMap::iterator FaceIterator;
    FaceMap faceMap;

    const unsigned int numElements = elements_.size();
    for( unsigned int n = 0; n < numElements; ++n )
    {
      for( unsigned int face = 0; face < numFaces; ++face )
      {
        FaceType key;
        generateFace( elements_[ n ], face, key );
        std::sort( key.begin(), key.end() );

        const FaceIterator pos = faceMap.find( key );
        if( pos != faceMap.end() )
          faceMap.erase( key );
        else
        {
          faceMap.insert( std::make_pair( key, SubEntity( n, face ) ) );
        }
      }
    }


    // swap current boundary ids with an empty vector
    BoundaryIdMap boundaryIds;
    boundaryIds_.swap( boundaryIds );
    alugrid_assert ( boundaryIds_.size() == 0 );

    // add all current boundary ids again (with their reordered keys)
    typedef typename BoundaryIdMap::iterator BoundaryIterator;
    const BoundaryIterator bndEnd = boundaryIds.end();
    for( BoundaryIterator bndIt = boundaryIds.begin(); bndIt != bndEnd; ++bndIt )
    {
      FaceType key = bndIt->first;
      std::sort( key.begin(), key.end() );
      const FaceIterator pos = faceMap.find( key );

      if( pos == faceMap.end() )
      {
        std::cerr << "Key: " << key << std::endl;
        DUNE_THROW( GridError, "Inserted boundary segment is not part of the boundary." );
      }

      reinsertBoundary( faceMap, pos, bndIt->second );
      faceMap.erase( pos );
    }

    //the search for the periodic neighbour also deletes the
    //found faces from the boundaryIds_ - thus it has to be done
    //after the recreation of the Ids_, because of correctElementOrientation
    for(auto it = faceMap.begin(); it!=faceMap.end(); ++it)
    {
      searchPeriodicNeighbor( faceMap, it, defaultId );
    }

    // communicate unidentified boundaries and find process borders)
    // use the Grids communicator (ALUGridNoComm or ALUGridMPIComm)
    typename ALUGrid::CollectiveCommunication comm( communicator_ );

    int numBoundariesMine = faceMap.size();
    std::vector< int > boundariesMine( numFaceCorners * numBoundariesMine );
    typedef std::map< FaceType, FaceType, FaceLess > GlobalToLocalFaceMap;
    GlobalToLocalFaceMap globalFaceMap;
    {
      const FaceIterator faceEnd = faceMap.end();
      int idx = 0;
      for( FaceIterator faceIt = faceMap.begin(); faceIt != faceEnd; ++faceIt )
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
            const FaceIterator pos = faceMap.find( pos_gl->second );
            if ( pos != faceMap.end() )
            {
              reinsertBoundary( faceMap, pos, ALU3DSPACE ProcessorBoundary_t );
              faceMap.erase( pos );
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
    const FaceIterator faceEnd = faceMap.end();
    for( FaceIterator faceIt = faceMap.begin(); faceIt != faceEnd; ++faceIt )
      reinsertBoundary( faceMap, faceIt, defaultId );

  }

#if COMPILE_ALUGRID_LIB
  template class ALU3dGridFactory< ALUGrid< 3, 3, cube, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 3, 3, simplex, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 3, 3, simplex, conforming > >;

  template class ALU3dGridFactory< ALUGrid< 2, 3, cube, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 2, 3, simplex, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 2, 3, simplex, conforming > >;

  template class ALU3dGridFactory< ALUGrid< 2, 2, cube, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 2, 2, simplex, nonconforming > >;
  template class ALU3dGridFactory< ALUGrid< 2, 2, simplex, conforming > >;
#endif
}
#undef alu_inline

#endif // end DUNE_ALU3DGRID_FACTORY_CC
