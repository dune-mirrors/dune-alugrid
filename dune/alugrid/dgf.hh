#ifndef DUNE_ALUGRID_DGF_HH
#define DUNE_ALUGRID_DGF_HH

#include <type_traits>

#if HAVE_ALUGRID
#include <dune/alugrid/grid.hh>
#include <dune/grid/io/file/dgfparser/dgfalu.hh>
#else

// include grid first to avoid includes from dune-grid/grid/alugrid
#include <dune/alugrid/grid.hh>

#include <dune/grid/common/intersection.hh>
#include <dune/grid/io/file/dgfparser/dgfparser.hh>
#include <dune/grid/io/file/dgfparser/parser.hh>
#include <dune/grid/io/file/dgfparser/blocks/projection.hh>


namespace Dune
{

  namespace
  {

    // GlobalVertexIndexBlock
    // ----------------------

    class GlobalVertexIndexBlock
    : public dgf::BasicBlock
    {
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
      : factory_(comm),
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
      const typename Intersection::Entity & entity = intersection.inside();

      const int face = intersection.indexInInside();

      const auto& refElem =
        ReferenceElements< double, dimension >::general( entity.type() );
      int corners = refElem.size( face, 1, dimension );
      std :: vector< unsigned int > bound( corners );
      for( int i=0; i < corners; ++i )
      {
        const int k =  refElem.subEntity( face, 1, i, dimension );
        bound[ i ] = factory_.insertionIndex( entity.template subEntity< dimension >( k ) );
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
      const typename Intersection::Entity & entity = intersection.inside();

      const int face = intersection.indexInInside();

      const auto& refElem =
        ReferenceElements< double, dimension >::general( entity.type() );
      int corners = refElem.size( face, 1, dimension );
      std :: vector< unsigned int > bound( corners );
      for( int i=0; i < corners; ++i )
      {
        const int k =  refElem.subEntity( face, 1, i, dimension );
        bound[ i ] = factory_.insertionIndex( entity.template subEntity< dimension >( k ) );
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
                          const ALUGridRefinementType refinementtype,
                          std::istream &file,
                          MPICommunicatorType communicator,
                          const std::string &filename );


    static Grid* callDirectly( const std::string& gridname,
                               const int rank,
                               const char *filename,
                               MPICommunicatorType communicator )
    {
      typedef typename Grid::MPICommunicatorType  GridCommunicatorType;
      static const bool isSameComm = std::is_same< GridCommunicatorType, MPICommunicatorType >::value;
      return callDirectlyImpl( gridname, rank, filename, communicator, std::integral_constant< bool, isSameComm >() );
    }

    static Grid* callDirectlyImpl( const std::string& gridname,
                                   const int rank,
                                   const char *filename,
                                   MPICommunicatorType communicator,
                                   std::false_type )
    {
      // for rank 0 we also check the normal file name
      if( rank == 0 )
      {
        if( fileExists( filename ) )
          return new Grid( filename );

        // only throw this exception on rank 0 because
        // for the other ranks we can still create empty grids
        DUNE_THROW( GridError, "Unable to create " << gridname << " from '"
                    << filename << "'." );
      }
      // return empty grid on all other processes
      return new Grid();
    }

    static Grid* callDirectlyImpl( const std::string& gridname,
                                   const int rank,
                                   const char *filename,
                                   MPICommunicatorType communicator,
                                   std::true_type )
    {
      if constexpr ( !std::is_same< MPICommunicatorType, No_Comm >::value )
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
    static int rank( MPICommunicatorType mpiComm )
    {
      int rank = 0;
#if HAVE_MPI
      MPI_Comm_rank( mpiComm, &rank );
#endif
      return rank;
    }
    static int size( MPICommunicatorType mpiComm )
    {
      int size = 1;
#if HAVE_MPI
      MPI_Comm_size( mpiComm, &size );
#endif
      return size;
    }
    Grid *grid_;
    GridFactory factory_;
    DuneGridFormatParser dgf_;
  };

  template <int dim, int dimw, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm >
  struct DGFGridFactory< ALUGrid< dim, dimw, eltype, refinementtype, Comm > > :
    public DGFBaseFactory< ALUGrid< dim, dimw, eltype, refinementtype, Comm > >
  {
    typedef ALUGrid< dim, dimw, eltype, refinementtype, Comm > DGFGridType;
    typedef DGFBaseFactory< DGFGridType > BaseType;
    typedef typename BaseType :: MPICommunicatorType MPICommunicatorType;
  protected:
    using BaseType :: grid_;
    using BaseType :: callDirectly;
  public:
    explicit DGFGridFactory ( std::istream &input,
                              MPICommunicatorType mpiComm )
      : DGFGridFactory( input, Comm(mpiComm) )
    {}

    explicit DGFGridFactory ( const std::string &filename,
                              MPICommunicatorType mpiComm )
      : DGFGridFactory( filename, Comm(mpiComm) )
    {}

    DGFGridFactory ( std::istream &input,
                     Comm comm = Comm() ) // casts from and to MPI_Comm
      : BaseType( MPICommunicatorType(comm) )
    {
      input.clear();
      input.seekg( 0 );
      if( !input )
        DUNE_THROW( DGFException, "Error resetting input stream." );
      generate( input, comm );
    }

    explicit DGFGridFactory ( const std::string &filename,
                              Comm comm = Comm() ) // casts from and to MPI_Comm
      : BaseType( MPICommunicatorType(comm) )
    {
      std::ifstream input( filename.c_str() );
      bool fileFound = input.is_open() ;
      if( fileFound )
        fileFound = generate( input, comm, filename );

      if( ! fileFound )
      {
        std::stringstream gridname;
        gridname << "ALUGrid< " << dim << ", " << dimw << ", eltype, ref, comm >";
        grid_ = callDirectly( gridname.str(), this->rank( comm ), filename.c_str(), comm );
      }
    }

  protected:
    bool generate( std::istream &file, MPICommunicatorType comm, const std::string &filename = "" );
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

  } //end namespace dgf


  template < class G >
  inline bool DGFBaseFactory< G > ::
  generateALUGrid( const ALUGridElementType eltype,
                   const ALUGridRefinementType refinementtype,
                   std::istream &file, MPICommunicatorType communicator,
                   const std::string &filename )
  {
    typedef G DGFGridType ;

    const int dimworld = DGFGridType :: dimensionworld ;
    const int dimgrid  = DGFGridType :: dimension;
    dgf_.element = ( eltype == simplex) ?
                        DuneGridFormatParser::Simplex :
                        DuneGridFormatParser::Cube ;
    dgf_.dimgrid = dimgrid;
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
        if ( !newfile )
        {
          std::cout << "prozess " << rank << " failed to open file " << newfilename << " ... abort" << std::endl;
          DUNE_THROW( InvalidStateException, "parallel DGF file could not opend" );
        }
        assert( newfile );
        return generateALUGrid(eltype, refinementtype, newfile, communicator, filename);
      }
    }

    const GeometryType elementType = (eltype == simplex) ?
            GeometryTypes::simplex(dimgrid) : GeometryTypes::cube(dimgrid);
    const GeometryType faceType = (eltype == simplex) ?
            GeometryTypes::simplex(dimgrid-1) : GeometryTypes::cube(dimgrid-1);

    GlobalVertexIndexBlock vertexIndex( file );
    const bool globalVertexIndexFound = vertexIndex.isactive();
    if( rank == 0 || globalVertexIndexFound )
    {
      if( !dgf_.readDuneGrid( file, dimgrid, dimworld ) )
        DUNE_THROW( InvalidStateException, "DGF file not recognized on second call." );

      if( eltype == simplex )
      {
        if(dimgrid == 3)
          dgf_.setOrientation( 2, 3 );
        else
          dgf_.setRefinement( 0, 1);
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

      const int nFaces = (eltype == simplex) ? dimgrid+1 : 2*dimgrid;
      for( int n = 0; n < dgf_.nofelements; ++n )
      {
        factory_.insertElement( elementType, dgf_.elements[ n ] );
        for( int face = 0; face <nFaces; ++face )
        {
          typedef DuneGridFormatParser::facemap_t::key_type Key;
          typedef DuneGridFormatParser::facemap_t::iterator Iterator;

          const Key key = ElementFaceUtil::generateFace( dimgrid, dgf_.elements[ n ], face );
          const Iterator it = dgf_.facemap.find( key );
          if( it != dgf_.facemap.end() )
            factory_.insertBoundary( n, face, it->second.first );
        }
      }

    } // end rank == 0 || globalVertexIndexBlock

    dgf::ProjectionBlock projectionBlock( file, dimworld );
    const DuneBoundaryProjection< dimworld > *projection
      = projectionBlock.defaultProjection< dimworld >();

    //There is currently only the possibility to insert one
    //surface OR a global BOUNDARY projection
    //This is done via a second argument bool
    //that defaults to dimgrid != dimworld
    if( projection )
      factory_.insertBoundaryProjection( *projection );

    if( rank == 0 || globalVertexIndexFound )
    {
      const size_t numBoundaryProjections = projectionBlock.numBoundaryProjections();
      for( size_t i = 0; i < numBoundaryProjections; ++i )
      {
        const std::vector< unsigned int > &vertices = projectionBlock.boundaryFace( i );
        const DuneBoundaryProjection< dimworld > *projection
          = projectionBlock.boundaryProjection< dimworld >( i );
        factory_.insertBoundaryProjection( faceType, vertices, projection );
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
    } // end rank == 0 || globalVertexIndexBlock

    int addMissingBoundariesLocal = (dgf_.nofelements > 0) && dgf_.facemap.empty();
    int addMissingBoundariesGlobal = addMissingBoundariesLocal;
#if ALU3DGRID_PARALLEL
    MPI_Allreduce( &addMissingBoundariesLocal, &addMissingBoundariesGlobal, 1, MPI_INT, MPI_MAX, communicator );
#endif

    // pass longest edge marking (default is off)
    if( ( refinementtype == conforming ) && parameter.markLongestEdge() )
      factory_.setLongestEdgeFlag();

    if( !parameter.dumpFileName().empty() )
      grid_ = std::unique_ptr<Grid>(factory_.createGrid( addMissingBoundariesGlobal, false, parameter.dumpFileName() )).release() ;
    else
      grid_ = std::unique_ptr<Grid>(factory_.createGrid( addMissingBoundariesGlobal, true, filename )).release();
    return true;
  }

  template <int dim, int dimw, ALUGridElementType eltype, ALUGridRefinementType refinementtype, class Comm>
  inline bool DGFGridFactory< ALUGrid< dim, dimw, eltype, refinementtype, Comm > >
    ::generate( std::istream &file, MPICommunicatorType communicator, const std::string &filename )
  {
    return BaseType :: generateALUGrid( eltype, refinementtype, file, communicator, filename );
  }



} // namespace Dune

#endif // else if HAVE_ALUGRID

#endif // #ifndef DUNE_ALUGRID_DGF_HH
