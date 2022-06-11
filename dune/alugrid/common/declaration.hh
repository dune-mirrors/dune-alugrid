#ifndef DUNE_ALUGRID_COMMON_DECLARATION_HH
#define DUNE_ALUGRID_COMMON_DECLARATION_HH

#define ALU3DGRID_PARALLEL HAVE_MPI

#include <dune/common/parallel/communication.hh>
#if ALU3DGRID_PARALLEL
#include <dune/common/parallel/mpicommunication.hh>
#endif // #if ALU3DGRID_PARALLEL


namespace Dune
{

  //! \brief basic element types for ALUGrid
  enum ALUGridElementType
  {
    simplex, //!< use only simplex elements (i.e., triangles or tetrahedra)
    cube     //!< use only cube elements (i.e., quadrilaterals or hexahedra)
  };

  //! \brief available refinement types for ALUGrid
  enum ALUGridRefinementType
  {
    conforming,   //!< use only conforming bisection refinement
    nonconforming //!< use non-conforming (red) refinement by default,
                  //!< conforming can be enabled by env flag (experimental).
  };

  //! \brief type of class for specialization of serial ALUGrid (No_Comm as communicator)
  struct ALUGridNoComm
  {
    No_Comm noComm_;
    ALUGridNoComm() : noComm_() {}
    ALUGridNoComm( const No_Comm& comm ) : noComm_( comm ) {}
#if ALU3DGRID_PARALLEL
    ALUGridNoComm( MPI_Comm comm ) : noComm_() {}
    operator MPI_Comm () const { return MPI_COMM_SELF; }
#endif
    operator No_Comm () const { return noComm_; }
  };

  //! \brief type of class for specialization of parallel ALUGrid (MPI_Comm as communicator)
  struct ALUGridMPIComm {
#if ALU3DGRID_PARALLEL
    MPI_Comm mpiComm_;
    ALUGridMPIComm() : mpiComm_( MPI_COMM_WORLD ) {}
    ALUGridMPIComm( MPI_Comm comm ) : mpiComm_( comm ) {}
    operator MPI_Comm () const { return mpiComm_; }
#endif
  } ;

  /**
   * \brief unstructured parallel implementation of the DUNE grid interface
   *
   * %ALUGrid implements the DUNE grid interface for 2D quadrilateral and 3D
   * hexahedral as well as 2D triangular and 3D tetrahedral meshes.
   * This grid can be locally adapted (non-conforming and conforming bisection)
   * and used in parallel computations using dynamic load balancing.
   *
   * \tparam  dim         dimension of the grid (2 or 3)
   * \tparam  dimworld    dimension of the surrounding space (dim <= dimworld <=3)
   * \tparam  elType      type of elements (Dune::simplex or Dune::cube)
   * \tparam  refineType  defaults to Dune::nonconforming, Dune::conforming is deprecated
   * \tparam  Comm        type of communicator (Dune::ALUGridMPIComm or Dune::ALUGridNoComm)
   *
   * \note For cube elements, only nonconforming refinement is available.
   * \note The template parameter Comm defaults to ALUGridMPIComm, if MPI is available.
   *       Otherwise it defaults to ALUGridNoComm.
   */
  template <int dim, int dimworld, ALUGridElementType elType,
            ALUGridRefinementType refineType = nonconforming,
            class Comm =
#if ALU3DGRID_PARALLEL
              ALUGridMPIComm
#else
              ALUGridNoComm
#endif
           >
  class ALUGrid;

  //- traits class for declaring base class for ALUGrid
  template <int dim, int dimw, ALUGridElementType elType, class Comm >
  struct ALUGridBaseGrid ;
}
#endif // #ifndef DUNE_ALUGRID_COMMON_DECLARATION_HH
