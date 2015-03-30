#ifndef DUNE_ALU3DGRID_HSFC_HH
#define DUNE_ALU3DGRID_HSFC_HH

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/collectivecommunication.hh>
#include <dune/common/parallel/mpicollectivecommunication.hh>

// to disable Zoltans HSFC ordering of the macro elements define
// DISABLE_ZOLTAN_HSFC_ORDERING on the command line
#if HAVE_ZOLTAN && HAVE_MPI
#ifndef DISABLE_ZOLTAN_HSFC_ORDERING
#define USE_ZOLTAN_HSFC_ORDERING
#else
#warning "ZOLTAN_HSFC_ORDERING disabled by DISABLE_ZOLTAN_HSFC_ORDERING"
#endif
#endif

#ifdef USE_ZOLTAN_HSFC_ORDERING
#include <dune/alugrid/impl/parallel/aluzoltan.hh>

extern "C" {
  extern double Zoltan_HSFC_InvHilbert3d (Zoltan_Struct *zz, double *coord);
  extern double Zoltan_HSFC_InvHilbert2d (Zoltan_Struct *zz, double *coord);
}

namespace Dune {

  template <class Coordinate>
  class SpaceFillingCurveOrdering
  {
    // type of communicator
    typedef Dune :: CollectiveCommunication< typename MPIHelper :: MPICommunicator >
        CollectiveCommunication ;

    // type of Zoltan HSFC ordering function
    typedef double zoltan_hsfc_inv_t(Zoltan_Struct *zz, double *coord);

    static const int dimension = Coordinate::dimension;

    Coordinate lower_;
    Coordinate length_;

    const zoltan_hsfc_inv_t* hsfcInv_;

    mutable Zoltan zz_;

  public:
    SpaceFillingCurveOrdering( const Coordinate& lower,
                               const Coordinate& upper,
                               const CollectiveCommunication& comm =
                               CollectiveCommunication( Dune::MPIHelper::getCommunicator() ) )
      : lower_( lower ),
        length_( upper ),
        hsfcInv_( dimension == 3 ? Zoltan_HSFC_InvHilbert3d : Zoltan_HSFC_InvHilbert2d ),
        zz_( comm )
    {
      // compute length
      length_ -= lower_;
    }

    // return unique hilbert index in interval [0,1] given an element's center
    double hilbertIndex( const Coordinate& point ) const
    {
      assert( point.size() == (unsigned int)dimension );

      Coordinate center ;
      // scale center into [0,1]^3 box which is needed by Zoltan_HSFC_InvHilbert3d
      for( int d=0; d<dimension; ++d )
        center[ d ] = (point[ d ] - lower_[ d ]) / length_[ d ];

      // return hsfc index in interval [0,1]
      return hsfcInv_( zz_.Get_C_Handle(), &center[ 0 ] );
    }
  };

} // end namespace Dune

#endif // #ifdef USE_ZOLTAN_HSFC_ORDERING

#endif // #ifndef DUNE_ALU3DGRID_HSFC_HH
