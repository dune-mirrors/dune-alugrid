#ifndef DUNE_ALUGRID_DGF_HH
#define DUNE_ALUGRID_DGF_HH

#if HAVE_ALUGRID
#include <dune/alugrid/grid.hh>
#include <dune/grid/io/file/dgfparser/dgfalu.hh>
#else

#include <dune/grid/common/intersection.hh>
#include <dune/grid/io/file/dgfparser/dgfparser.hh>
#include <dune/grid/io/file/dgfparser/parser.hh>
#include <dune/grid/io/file/dgfparser/blocks/projection.hh>

#include <dune/alugrid/grid.hh>

namespace Dune
{

  namespace
  {

    // GlobalVertexIndexBlock
    // ----------------------

    class GlobalVertexIndexBlock
    : public dgf::BasicBlock
    {
      unsigned int nofvtx;
      bool goodline;       

    public:
      GlobalVertexIndexBlock ( std :: istream &in )
      : dgf::BasicBlock( in, "GlobalVertexIndex" ),
        goodline( true )
      {}

      bool next ( int &index )
      {
        assert( ok() );
        if( !getnextline() )
          return (goodline = false);

        if( !getnextentry( index ) )
        {
          DUNE_THROW ( DGFException, "Error in " << *this << ": "
                                     << "Wrong global vertex indices " );
        }
        return (goodline = true);
      }

      // some information
      bool ok ()
      {
        return goodline;
      }
    };



    // ALUParallelBlock
    // ----------------

    class ALUParallelBlock
    : public dgf::BasicBlock
    {
      unsigned int nofvtx;
      bool goodline;       

    public:
      ALUParallelBlock ( std :: istream &in )
      : dgf::BasicBlock( in, "ALUParallel" ),
        goodline( true )
      {}

      bool next ( std::string &name )
      {
        assert( ok() );
        if( !getnextline() )
          return (goodline = false);

        if( !getnextentry( name ) )
        {
          DUNE_THROW ( DGFException, "Error in " << *this << ": "
                                     << "Wrong global vertex indices " );
        }
        return (goodline = true);
      }

      // some information
      bool ok ()
      {
        return goodline;
      }
    };

  } // end empty namespace 



  // DGFGridInfo (specialization for ALUGrid)
  // ----------------------------------------

  template<int dimg, int dimw, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm >
  struct DGFGridInfo< Dune::ALUGrid< dimg, dimw, eltype, refinementtype, Comm > >
  {
    static int refineStepsForHalf () { return ( refinementtype == conforming ) ? dimg : 1; }
    static double refineWeight () { return ( refinementtype == conforming ) ? 0.5 : 1.0/(std::pow( 2.0, double(dimg))); }
  };
  /** \endcond */



  // DGFGridFactory for AluGrid
  // --------------------------

  // template< int dim, int dimworld > // for a first version
  template< class G >
  struct DGFBaseFactory
  {
    typedef G  Grid;
    const static int dimension = Grid::dimension;
    typedef MPIHelper::MPICommunicator MPICommunicatorType;
    typedef typename Grid::template Codim<0>::Entity Element;
    typedef typename Grid::template Codim<dimension>::Entity Vertex;
    typedef Dune::GridFactory<Grid> GridFactory;

    DGFBaseFactory ()
      : factory_( ),
        dgf_( 0, 1 )
    {}

    explicit DGFBaseFactory ( MPICommunicatorType comm ) 
      : factory_(),
        dgf_( rank(comm), size(comm) )
    {}

    Grid *grid () const
    {
      return grid_;
    }

    template< class Intersection >
    bool wasInserted ( const Intersection &intersection ) const
    {
      return factory_.wasInserted( intersection );
    }

    template< class GG, class II >
    int boundaryId ( const Intersection< GG, II > & intersection ) const
    {
      typedef Dune::Intersection< GG, II > Intersection;
      typename Intersection::EntityPointer inside = intersection.inside();
      const typename Intersection::Entity & entity = *inside;
      const int face = intersection.indexInInside();

      const ReferenceElement< double, dimension > & refElem = 
        ReferenceElements< double, dimension >::general( entity.type() );
      int corners = refElem.size( face, 1, dimension );
      std :: vector< unsigned int > bound( corners );
      for( int i=0; i < corners; ++i )
      {
        const int k =  refElem.subEntity( face, 1, i, dimension );
        bound[ i ] = factory_.insertionIndex( *entity.template subEntity< dimension >( k ) );
      }

      DuneGridFormatParser::facemap_t::key_type key( bound, false );
      const DuneGridFormatParser::facemap_t::const_iterator pos = dgf_.facemap.find( key );
      if( pos != dgf_.facemap.end() )
        return dgf_.facemap.find( key )->second.first;
      else
        return (intersection.boundary() ? 1 : 0);
    }

    template< class GG, class II >
    const typename DGFBoundaryParameter::type & 
      boundaryParameter ( const Intersection< GG, II > & intersection ) const
    {
      typedef Dune::Intersection< GG, II > Intersection;
      typename Intersection::EntityPointer inside = intersection.inside();
      const typename Intersection::Entity & entity = *inside;
      const int face = intersection.indexInInside();

      const ReferenceElement< double, dimension > & refElem = 
        ReferenceElements< double, dimension >::general( entity.type() );
      int corners = refElem.size( face, 1, dimension );
      std :: vector< unsigned int > bound( corners );
      for( int i=0; i < corners; ++i )
      {
        const int k =  refElem.subEntity( face, 1, i, dimension );
        bound[ i ] = factory_.insertionIndex( *entity.template subEntity< dimension >( k ) );
      }

      DuneGridFormatParser::facemap_t::key_type key( bound, false );
      const DuneGridFormatParser::facemap_t::const_iterator pos = dgf_.facemap.find( key );
      if( pos != dgf_.facemap.end() )
        return dgf_.facemap.find( key )->second.second;
      else
        return DGFBoundaryParameter::defaultValue();
    }

    template< int codim >
    int numParameters () const
    {
      if( codim == 0 )
        return dgf_.nofelparams;
      else if( codim == dimension )
        return dgf_.nofvtxparams;
      else
        return 0;
    }

    // return true if boundary parameters found
    bool haveBoundaryParameters () const
    {
      return dgf_.haveBndParameters;
    }

    std::vector< double > &parameter ( const Element &element )
    {
      if( numParameters< 0 >() <= 0 )
      {
        DUNE_THROW( InvalidStateException,
                    "Calling DGFGridFactory::parameter is only allowed if there are parameters." );
      }
      return dgf_.elParams[ factory_.insertionIndex( element ) ];
    }

    std::vector< double > &parameter ( const Vertex &vertex )
    {
      if( numParameters< dimension >() <= 0 )
      {
        DUNE_THROW( InvalidStateException,
                    "Calling DGFGridFactory::parameter is only allowed if there are parameters." );
      }
      return dgf_.vtxParams[ factory_.insertionIndex( vertex ) ];
    }

  protected:
    bool generateALUGrid( const ALUGridElementType eltype,
                          std::istream &file, 
                          MPICommunicatorType communicator,
                          const std::string &filename );

    bool generateALU2dGrid( const ALUGridElementType eltype,
                            std::istream &file, 
                            MPICommunicatorType communicator,
                            const std::string &filename );

    static Grid* callDirectly( const char* gridname,
                               const int rank, 
                               const char *filename,
                               MPICommunicatorType communicator )
    {
      if( ! Conversion< MPICommunicatorType , No_Comm > :: sameType ) 
      {
        // in parallel runs add rank to filename 
        std :: stringstream tmps;
        tmps << filename << "." << rank;
        const std :: string &tmp = tmps.str();

        // if file exits then use it 
        if( fileExists( tmp.c_str() ) )
          return new Grid( tmp.c_str(), communicator );
      }

      // for rank 0 we also check the normal file name 
      if( rank == 0 ) 
      {
        if( fileExists( filename ) )
          return new Grid( filename , communicator );

        // only throw this exception on rank 0 because 
        // for the other ranks we can still create empty grids 
        DUNE_THROW( GridError, "Unable to create " << gridname << " from '"
                    << filename << "'." );
      }
      // don't create messages in every proc, this does not work for many cores.
      //else 
      //{
      //  dwarn << "WARNING:  P[" << rank << "]: Creating empty grid!" << std::endl;
      //}

      // return empty grid on all other processes 
      return new Grid( communicator );
    }
    static bool fileExists ( const char *fileName )
    {
      std :: ifstream testfile( fileName );
      if( !testfile )
        return false;
      testfile.close();
      return true;
    }
    static int rank( MPICommunicatorType MPICOMM )
    {
      int rank = 0;
#if HAVE_MPI
      MPI_Comm_rank( MPICOMM, &rank );
#endif
      return rank;
    }
    static int size( MPICommunicatorType MPICOMM )
    {
      int size = 1;
#if HAVE_MPI
      MPI_Comm_size( MPICOMM, &size );
#endif
      return size;
    }
    Grid *grid_;
    GridFactory factory_;
    DuneGridFormatParser dgf_;
  };

  template < ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm >
  struct DGFGridFactory< ALUGrid<3,3, eltype, refinementtype, Comm > > : 
    public DGFBaseFactory< ALUGrid<3,3, eltype, refinementtype, Comm > >
  {
    typedef ALUGrid<3,3, eltype, refinementtype, Comm > DGFGridType;
    typedef DGFBaseFactory< DGFGridType > BaseType;
    typedef typename BaseType :: MPICommunicatorType MPICommunicatorType;
  protected:  
    using BaseType :: grid_;
    using BaseType :: callDirectly;
  public:  
    explicit DGFGridFactory ( std::istream &input,
                              MPICommunicatorType comm = MPIHelper::getCommunicator() )
    : BaseType( comm )
    {
      input.clear();
      input.seekg( 0 );
      if( !input )
        DUNE_THROW( DGFException, "Error resetting input stream." );
      generate( input, comm );
    }

    explicit DGFGridFactory ( const std::string &filename, 
                              MPICommunicatorType comm = MPIHelper::getCommunicator()) 
    : BaseType( comm ) 
    {
      std::ifstream input( filename.c_str() );
      bool fileFound = input.is_open() ;
      if( fileFound ) 
        fileFound = generate( input, comm, filename );

      if( ! fileFound ) 
        grid_ = callDirectly( "ALUGrid< 3, 3, eltype, ref, comm >", this->rank( comm ), filename.c_str(), comm );
    }

  protected:
    bool generate( std::istream &file, MPICommunicatorType comm, const std::string &filename = "" );
  };

  template < int dimw, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm >
  struct DGFGridFactory< ALUGrid<2, dimw, eltype, refinementtype, Comm > > : 
    public DGFBaseFactory< ALUGrid< 2, dimw, eltype, refinementtype, Comm > >
  {
    typedef ALUGrid< 2, dimw, eltype, refinementtype, Comm > DGFGridType;
    typedef DGFBaseFactory< DGFGridType > BaseType;
    typedef typename BaseType :: MPICommunicatorType MPICommunicatorType;
    typedef typename BaseType::Grid Grid;
    const static int dimension = Grid::dimension;
    typedef typename BaseType::GridFactory GridFactory;

    explicit DGFGridFactory ( std::istream &input,
                              MPICommunicatorType comm = MPIHelper::getCommunicator() )
    : BaseType( comm ) 
    {
      input.clear();
      input.seekg( 0 );
      if( !input )
        DUNE_THROW( DGFException, "Error resetting input stream." );
      generate( input, comm );
    }

    explicit DGFGridFactory ( const std::string &filename, 
                              MPICommunicatorType comm = MPIHelper::getCommunicator()) 
      : BaseType( comm )
    {
      std::ifstream input( filename.c_str() );
      if( !input )
        DUNE_THROW( DGFException, "Macrofile '" << filename << "' not found." );
      if( !generate( input, comm, filename ) )
      {
        if( BaseType::fileExists( filename.c_str() ) )
          grid_ = new Grid( filename );
        else
          DUNE_THROW( GridError, "Unable to create a 2d ALUGrid from '" << filename << "'." );
      }
    }

  protected:
    bool generate( std::istream &file, MPICommunicatorType comm, const std::string &filename = "" );
    using BaseType::grid_;
    using BaseType::factory_;
    using BaseType::dgf_;
  };



  namespace dgf
  {

    struct ALU2dGridParameterBlock
    : public GridParameterBlock
    {
      ALU2dGridParameterBlock( std::istream &in, const bool verbose )
      : GridParameterBlock( in ),
        tolerance_( 1e-8 )
      {
        if( findtoken( "tolerance" ) )
        {
          double x;
          if( getnextentry(x) )
            tolerance_ = x;
          else 
          {
            if( verbose ) 
            {
              dwarn << "GridParameterBlock: found keyword `tolerance' but no value, "
                    << "defaulting to `" <<  tolerance_ <<"'!" << std::endl;
            }
          }
          if( tolerance_ <= 0 )
            DUNE_THROW( DGFException, "Nonpositive tolerance specified!" );
        }
        else
        {
          if( verbose )
          {
            dwarn << "GridParameterBlock: Parameter 'tolerance' not specified, "
                  << "defaulting to `" <<  tolerance_ <<"'!" << std::endl;
          }
        }
      }

      double tolerance () const { return tolerance_; }

    protected:
      double tolerance_;
    };

  }

  template < class G > 
  inline bool DGFBaseFactory< G > :: 
  generateALUGrid( const ALUGridElementType eltype, 
                   std::istream &file, MPICommunicatorType communicator, 
                   const std::string &filename )
  {
    typedef G DGFGridType ;

    const int dimworld = DGFGridType :: dimensionworld ; 
    dgf_.element = ( eltype == simplex) ? 
                        DuneGridFormatParser::Simplex : 
                        DuneGridFormatParser::Cube ;
    dgf_.dimgrid = dimworld;
    dgf_.dimw = dimworld;

    const bool isDGF = dgf_.isDuneGridFormat( file );
    file.seekg( 0 );
    if( !isDGF )
      return false;

    int rank = 0;
#if ALU3DGRID_PARALLEL
    MPI_Comm_rank( communicator, &rank );
#endif

    dgf::GridParameterBlock parameter( file );

    typedef FieldVector< typename DGFGridType :: ctype, dimworld > CoordinateType ;

    ALUParallelBlock aluParallelBlock( file );
    const bool readFromParallelDGF = aluParallelBlock.isactive();
    bool parallelFileExists = false;

    std::string newfilename;
    if (readFromParallelDGF)
    {
      bool ok = true;
      for (int p=0;p<=rank && ok;++p)
        ok = aluParallelBlock.next(newfilename);
      if (ok)
      {
        parallelFileExists = true;
        std::ifstream newfile(newfilename.c_str());
        return generateALUGrid(eltype,newfile,communicator,filename);
      }
    }

    GlobalVertexIndexBlock vertexIndex( file );
    const bool globalVertexIndexFound = vertexIndex.isactive();
    if( rank == 0 || globalVertexIndexFound )
    {
      if( !dgf_.readDuneGrid( file, dimworld, dimworld ) )
        DUNE_THROW( InvalidStateException, "DGF file not recognized on second call." );

      if( eltype == simplex ) 
      {
        dgf_.setOrientation( 2, 3 );
      }

      if( parallelFileExists && !globalVertexIndexFound )
        DUNE_THROW( DGFException, "Parallel DGF file requires GLOBALVERTEXINDEX block." );

      for( int n = 0; n < dgf_.nofvtx; ++n )
      {
        CoordinateType pos;
        for( int i = 0; i < dimworld; ++i )
          pos[ i ] = dgf_.vtx[ n ][ i ];
        if ( !globalVertexIndexFound )
          factory_.insertVertex( pos );
        else
        {
          int globalIndex;
          bool ok = vertexIndex.next(globalIndex);
          if (!ok)
            DUNE_THROW( DGFException, "Not enough values in GlobalVertexIndex block" );
          factory_.insertVertex( pos, globalIndex );
        }
      }

      GeometryType elementType( (eltype == simplex) ? 
                                    GeometryType::simplex : 
                                    GeometryType::cube, dimworld );

      const int nFaces = (eltype == simplex) ? dimworld+1 : 2*dimworld;
      for( int n = 0; n < dgf_.nofelements; ++n )
      {
        factory_.insertElement( elementType, dgf_.elements[ n ] );
        for( int face = 0; face <nFaces; ++face )
        {
          typedef DuneGridFormatParser::facemap_t::key_type Key;
          typedef DuneGridFormatParser::facemap_t::iterator Iterator;

          const Key key = ElementFaceUtil::generateFace( dimworld, dgf_.elements[ n ], face );
          const Iterator it = dgf_.facemap.find( key );
          if( it != dgf_.facemap.end() )
            factory_.insertBoundary( n, face, it->second.first );
        }
      }

      dgf::ProjectionBlock projectionBlock( file, dimworld );
      const DuneBoundaryProjection< dimworld > *projection
        = projectionBlock.defaultProjection< dimworld >();

      if( projection != 0 )
        factory_.insertBoundaryProjection( *projection );

      const size_t numBoundaryProjections = projectionBlock.numBoundaryProjections();
      for( size_t i = 0; i < numBoundaryProjections; ++i )
      {
        GeometryType type( (eltype == simplex) ? 
                               GeometryType::simplex : 
                               GeometryType::cube, 
                            dimworld-1);

        const std::vector< unsigned int > &vertices = projectionBlock.boundaryFace( i );
        const DuneBoundaryProjection< dimworld > *projection
          = projectionBlock.boundaryProjection< dimworld >( i );
        factory_.insertBoundaryProjection( type, vertices, projection );
      }
    }

    typedef dgf::PeriodicFaceTransformationBlock::AffineTransformation Transformation;
    dgf::PeriodicFaceTransformationBlock trafoBlock( file, dimworld );
    const int size = trafoBlock.numTransformations();
    for( int k = 0; k < size; ++k )
    {
      const Transformation &trafo = trafoBlock.transformation( k );

      typename GridFactory::WorldMatrix matrix;
      for( int i = 0; i < dimworld; ++i )
        for( int j = 0; j < dimworld; ++j )
          matrix[ i ][ j ] = trafo.matrix( i, j );

      typename GridFactory::WorldVector shift;
      for( int i = 0; i < dimworld; ++i )
        shift[ i ] = trafo.shift[ i ];

      factory_.insertFaceTransformation( matrix, shift );
    }

    if ( ! parameter.dumpFileName().empty() )
      grid_ = factory_.createGrid( dgf_.facemap.empty(), false, parameter.dumpFileName() );
    else 
      grid_ = factory_.createGrid( dgf_.facemap.empty(), true, filename );
    return true;
  }

  template <ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm>
  inline bool DGFGridFactory< ALUGrid< 3, 3, eltype, refinementtype, Comm > >
    ::generate( std::istream &file, MPICommunicatorType communicator, const std::string &filename )
  {
    return BaseType :: generateALUGrid( eltype, file, communicator, filename );
  }


  // ALUGrid 2d version 
  //-------------------

  template < class G > 
  inline bool DGFBaseFactory< G > :: 
    generateALU2dGrid( const ALUGridElementType eltype, 
                       std::istream &file, 
                       MPICommunicatorType communicator, 
                       const std::string &filename )
  {
    const int dimgrid = 2;
    const int dimworld = G :: dimensionworld ; 
    dgf_.element = (eltype == simplex) ? 
        DuneGridFormatParser::Simplex : DuneGridFormatParser::Cube ;
    dgf_.dimgrid = dimgrid;
    dgf_.dimw = dimworld;

    const bool isDGF = dgf_.isDuneGridFormat( file );
    file.seekg( 0 );
    if( !isDGF )
      return false;
   
    int rank = 0;
#if HAVE_MPI
    MPI_Comm_rank( communicator, &rank );
#endif

    // set verbosity of factory only for rank = 0 
    factory_.setVerbosity( (rank == 0) );

    // only print warnings of ALU2dGridParameterBlock on rank = 0 
    dgf::ALU2dGridParameterBlock parameter( file, (rank == 0) );

    factory_.setTolerance( parameter.tolerance() );

    if( !dgf_.readDuneGrid( file, dimgrid, dimworld ) )
      DUNE_THROW( InvalidStateException, "DGF file not recognized on second call." );

    for( int n = 0; n < dgf_.nofvtx; ++n )
    {
      FieldVector< double, dimworld > pos;
      for( int i = 0; i < dimworld; ++i )
        pos[ i ] = dgf_.vtx[ n ][ i ];
      factory_.insertVertex( pos );
    }

    GeometryType elementType( (eltype == simplex) ? 
                               GeometryType::simplex : 
                               GeometryType::cube, dimgrid );

    const int nFaces = (eltype == simplex) ? dimgrid+1 : 2*dimgrid;
    for( int n = 0; n < dgf_.nofelements; ++n )
    {
      factory_.insertElement( elementType, dgf_.elements[ n ] );
      for( int face = 0; face <nFaces; ++face )
      {
        typedef typename DuneGridFormatParser::facemap_t::key_type Key;
        typedef typename DuneGridFormatParser::facemap_t::iterator Iterator;

        const Key key = ElementFaceUtil::generateFace( dimgrid, dgf_.elements[ n ], face );
        const Iterator it = dgf_.facemap.find( key );
        if( it != dgf_.facemap.end() )
          factory_.insertBoundary( n, face, it->second.first );
      }
    }

    dgf::ProjectionBlock projectionBlock( file, dimworld );
    const DuneBoundaryProjection< dimworld > *projection
      = projectionBlock.defaultProjection< dimworld >();
    if( projection != 0 )
      factory_.insertBoundaryProjection( *projection );
    const size_t numBoundaryProjections = projectionBlock.numBoundaryProjections();
    for( size_t i = 0; i < numBoundaryProjections; ++i )
    {
      GeometryType type( (eltype == simplex) ? 
                         GeometryType::simplex : 
                         GeometryType::cube, dimgrid-1 );
      const std::vector< unsigned int > &vertices = projectionBlock.boundaryFace( i );
      const DuneBoundaryProjection< dimworld > *projection
        = projectionBlock.boundaryProjection< dimworld >( i );
      factory_.insertBoundaryProjection( type, vertices, projection );
    }

    typedef dgf::PeriodicFaceTransformationBlock::AffineTransformation Transformation;
    dgf::PeriodicFaceTransformationBlock trafoBlock( file, dimworld );
    const int size = trafoBlock.numTransformations();
    for( int k = 0; k < size; ++k )
    {
      const Transformation &trafo = trafoBlock.transformation( k );

      typename GridFactory::WorldMatrix matrix;
      for( int i = 0; i < dimworld; ++i )
        for( int j = 0; j < dimworld; ++j )
          matrix[ i ][ j ] = trafo.matrix( i, j );

      typename GridFactory::WorldVector shift;
      for( int i = 0; i < dimworld; ++i )
        shift[ i ] = trafo.shift[ i ];

      factory_.insertFaceTransformation( matrix, shift );
    }

    if ( ! parameter.dumpFileName().empty() && (rank == 0) )
      grid_ = factory_.createGrid( dgf_.facemap.empty(), false, parameter.dumpFileName() );
    else 
      grid_ = factory_.createGrid( dgf_.facemap.empty(), true, filename );
    return true;
  }

  template <int dimw, ALUGridElementType eltype, 
            ALUGridRefinementType refinementtype, class Comm >
  inline bool DGFGridFactory< ALUGrid< 2, dimw, eltype, refinementtype, Comm > >
    ::generate( std::istream &file, MPICommunicatorType communicator, const std::string &filename )
  {
    return BaseType :: generateALU2dGrid( eltype, file, communicator, filename );
  }

} // namespace Dune

#endif // else if HAVE_ALUGRID

#endif // #ifndef DUNE_ALUGRID_DGF_HH
