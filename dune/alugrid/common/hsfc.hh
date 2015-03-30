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

    static const int dimension = Coordinate::dimension;

    Coordinate lower_;
    Coordinate length_;

    mutable Zoltan zz_;

  public:
    SpaceFillingCurveOrdering( const Coordinate& lower,
                               const Coordinate& upper,
                               const CollectiveCommunication& comm =
                               CollectiveCommunication( Dune::MPIHelper::getCommunicator() ) )
      : lower_( lower ),
        length_( upper ),
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

      if( dimension == 3 )
      {
        return Zoltan_HSFC_InvHilbert3d(zz_.Get_C_Handle(), &center[ 0 ] );
      }
      else if ( dimension == 2 )
      {
        std::cout << "2 dim version of hsfc" << std::endl;
        return Zoltan_HSFC_InvHilbert2d(zz_.Get_C_Handle(), &center[ 0 ] );
      }
      else
      {
        DUNE_THROW(NotImplemented,"Zoltan_HSFC_InvHilbert" << dimension << "d not available!");
        return 0.0;
      }
    }
  };

} // end namespace Dune

#endif // #ifdef USE_ZOLTAN_HSFC_ORDERING

#endif // #ifndef DUNE_ALU3DGRID_HSFC_HH
