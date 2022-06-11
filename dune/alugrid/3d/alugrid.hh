#ifndef DUNE_ALU3DGRID_ALUGRID_HH
#define DUNE_ALU3DGRID_ALUGRID_HH

#include <type_traits>

// 3d version
#include <dune/alugrid/common/capabilities.hh>
#include <dune/alugrid/3d/alu3dinclude.hh>
#include <dune/alugrid/3d/indexsets.hh>
#include <dune/alugrid/3d/iterator.hh>
#include <dune/alugrid/3d/entity.hh>
#include <dune/alugrid/3d/geometry.hh>
#include <dune/alugrid/3d/grid.hh>

/** @file
    @author Robert Kloefkorn
    @brief Provides base classes for ALUGrid
**/

namespace Dune
{

  template <class Comm>
  static const char* ALUGridParallelSerial()
  {
    return ( std::is_same< Comm, ALUGridNoComm >::value ) ? "serial" : "parallel";
  }

  template< int dim, int dimworld, ALUGridElementType elType, ALUGridRefinementType refineType, class Comm >
  class ALUGrid
  : public ALUGridBaseGrid< dim, dimworld, elType, Comm > :: BaseGrid
  {
    typedef ::ALUGrid::ALUGridExternalParameters ALUGridExternalParameters;

    // the cube version of ALUGrid only works with nonconforming refinement
    static_assert( elType == cube ? refineType == nonconforming : true, "cube only works with nonconforming refinement");

    typedef ALUGrid< dim, dimworld, elType, refineType, Comm > This;
    typedef typename ALUGridBaseGrid< dim, dimworld, elType, Comm > :: BaseGrid  BaseType;

    ALUGridRefinementType getRefinementType( const ALUGridRefinementType defaultRefType )
    {
      // for simplex grid we allow to dynamically switch to conforming refinement (experimental)
      if constexpr ( elType == Dune::simplex && refineType == nonconforming )
      {
        // if environment var has been set (mainly from Python side)
        if( getenv("ALUGRID_CONFORMING_REFINEMENT") )
        {
          const int conformingRefinement = atoi(getenv("ALUGRID_CONFORMING_REFINEMENT"));
          if( bool(ALUGridExternalParameters::verbosityLevel()) )
            std::cout << "ALUGrid: setting conforming refinement in constructor!" <<std::endl;
          return conformingRefinement ? Dune::conforming : Dune::nonconforming;
        }
      }

      return defaultRefType;
    }

   public:
    typedef typename BaseType::MPICommunicatorType MPICommunicatorType;

    //! type of boundary projection
    typedef typename BaseType :: ALUGridVertexProjectionPairType  ALUGridVertexProjectionPairType;

    enum { dimension=BaseType::dimension,  dimensionworld=BaseType::dimensionworld};
    static const ALUGridRefinementType refinementType = refineType;
    typedef typename BaseType::ctype ctype;
    typedef typename BaseType::GridFamily GridFamily;
    typedef typename GridFamily::Traits Traits;
    typedef typename BaseType::LocalIdSetImp LocalIdSetImp;
    typedef typename Traits :: GlobalIdSet GlobalIdSet;
    typedef typename Traits :: LocalIdSet LocalIdSet;
    typedef typename GridFamily :: LevelIndexSetImp  LevelIndexSetImp;
    typedef typename GridFamily :: LeafIndexSetImp  LeafIndexSetImp;
    typedef typename BaseType::LeafIteratorImp LeafIteratorImp;
    typedef typename Traits:: template Codim<0>::LeafIterator LeafIteratorType;
    typedef typename Traits:: template Codim<0>::LeafIterator LeafIterator;

    //! \brief constructor for creating ALUGrid from given macro grid file
    //! \param macroName  filename for macro grid in ALUGrid tetra format
    //! \param mpiComm    MPI Communicator (when HAVE_MPI == 1 then mpiComm is of
    //!                   type MPI_Comm and the default value is MPI_COMM_WORLD)
    //! \param bndProject global boundary projection pointer
    //! \param bndVector  pointer to vector holding boundary projection for
    //!                   each boundary segment.  ALUGrid takes ownership of
    //!                   this pointer and will delete it in the desctructor
    //! \param verb       Whether to write a notice about grid creation to
    //!                   stdout.
    ALUGrid(const std::string macroName,
            const MPICommunicatorType mpiComm = BaseType::defaultCommunicator(),
            const ALUGridVertexProjectionPairType& bndPrj = ALUGridVertexProjectionPairType(),
            const bool verb = true ) :
      BaseType(macroName, mpiComm, bndPrj, getRefinementType(refinementType) )
    {
      const bool verbose = verb && (this->comm().rank() == 0) && bool(ALUGridExternalParameters::verbosityLevel());
      if( verbose )
      {
        std::cout << "\nCreated " << ALUGridParallelSerial< Comm >() << " " << name() << nameSuffix( this->conformingRefinement() )
                  << " from macro grid file '" << macroName << "'. \n\n";
      }
    }

    static std::string name () { return std::string("ALUGrid"); }

    static std::string nameSuffix( const bool conformingRefinement = false )
    {
      std::string elt ( elType == cube ? "cube>" : "simplex>" );
      std::string ref ( conformingRefinement ? " (conforming)": "" );
      std::stringstream suffix;
      suffix << "<"<< dimension <<","<< dimensionworld <<"," << elt << ref;
      return suffix.str();
    }


    //! \brief constructor called from ALUGridFactory
    //! for creating ALUConformGrid from given macro grid file
    //! \param mpiComm MPI Communicator (when HAVE_MPI == 1 then mpiComm is of type MPI_Comm)
    //! \param bndProject global boundary projection pointer
    //! \param bndVector  pointer to vector holding boundary projection for each boundary segment
    //!  \note ALUGrid takes ownership of this pointer and will delete it in the desctructor
    //! \param macroName filename from which ALUGrid is being generated
    //! \param verb       Whether to write a notice about grid creation to
    //!                   stdout.
    ALUGrid(const MPICommunicatorType mpiComm,
            const ALUGridVertexProjectionPairType& bndPrj,
            const std::string macroName,
            const bool verb = true ) :
      BaseType("", mpiComm, bndPrj, getRefinementType(refinementType) )
    {
      const bool verbose = verb && (this->comm().rank() == 0) && bool(ALUGridExternalParameters::verbosityLevel());
      if( verbose )
      {
        std::cout << "\nCreated " << ALUGridParallelSerial< Comm >() << " " << name() << nameSuffix( this->conformingRefinement() );
        if( macroName.empty() )
          std::cout << " from input stream. \n";
        else
          std::cout << " from macro grid file '" << macroName << "'. \n";
        std::cout << std::endl;
      }
    }

    //! constructor creating empty grid, empty string creates empty grid
    ALUGrid(const MPICommunicatorType mpiComm = BaseType::defaultCommunicator()) :
      BaseType("", mpiComm, ALUGridVertexProjectionPairType(), getRefinementType(refinementType) )
    {
      if(this->comm().rank() == 0 && bool(ALUGridExternalParameters::verbosityLevel()) )
      {
        std::cout << "\nCreated empty " << ALUGridParallelSerial< Comm >() << " "
                  << name() << nameSuffix( this->conformingRefinement() ) << "." << std::endl << std::endl;
      }
    }

    // ALUGrid only typedefs
    typedef typename BaseType::HierarchicIteratorImp HierarchicIteratorImp;
    typedef typename BaseType::ObjectStreamType      ObjectStreamType;

    template< PartitionIteratorType pitype >
    struct Partition
    {
      typedef Dune::GridView< ALU3dLevelGridViewTraits< const This, pitype > > LevelGridView;
      typedef Dune::GridView< ALU3dLeafGridViewTraits< const This, pitype > > LeafGridView;
    };

    typedef typename Partition< All_Partition > :: LevelGridView LevelGridView;
    typedef typename Partition< All_Partition > :: LeafGridView LeafGridView;

    // old grid view methods
    template< PartitionIteratorType pitype >
    typename Partition< pitype >::LevelGridView levelView ( int level ) const { return levelGridView< pitype >( level ); }

    template< PartitionIteratorType pitype >
    typename Partition< pitype >::LeafGridView leafView () const { return leafGridView< pitype >(); }

    LevelGridView levelView ( int level ) const { return levelGridView( level ); }

    LeafGridView leafView () const { return leafGridView(); }

    // new grid view methods
    template< PartitionIteratorType pitype >
    typename Partition< pitype >::LevelGridView levelGridView ( int level ) const
    {
      typedef typename Partition< pitype >::LevelGridView LevelGridView;
      typedef typename LevelGridView::GridViewImp LevelGridViewImp;
      return LevelGridView( LevelGridViewImp( *this, level ) );
    }

    template< PartitionIteratorType pitype >
    typename Partition< pitype >::LeafGridView leafGridView () const
    {
      typedef typename Partition< pitype >::LeafGridView LeafGridView;
      typedef typename LeafGridView::GridViewImp LeafGridViewImp;
      return LeafGridView( LeafGridViewImp( *this ) );
    }

    LevelGridView levelGridView ( int level ) const
    {
      typedef typename LevelGridView::GridViewImp LevelGridViewImp;
      return LevelGridView( LevelGridViewImp( *this, level ) );
    }

    LeafGridView leafGridView () const
    {
      typedef typename LeafGridView::GridViewImp LeafGridViewImp;
      return LeafGridView( LeafGridViewImp( *this ) );
    }

  private:
    template< class > friend class ALU3dGridFactory;

    //! Copy constructor should not be used
    ALUGrid( const ALUGrid & g ); //  : BaseType(g) {}

    //! assignment operator should not be used
    This& operator = (const ALUGrid& g);
  };

  //template< int dim, int dimworld, ALUGridElementType elType, ALUGridRefinementType refineType, class Comm >
  //using ALUGrid = ALUGrid< dim, dimworld, elType, Comm >;

} //end  namespace Dune

//#undef alu_inline
#endif // #ifndef DUNE_ALU3DGRID_ALUGRID_HH
