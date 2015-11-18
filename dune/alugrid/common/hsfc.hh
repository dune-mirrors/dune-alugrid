#ifndef DUNE_ALU3DGRID_HSFC_HH
#define DUNE_ALU3DGRID_HSFC_HH

#include <string>
#include <sstream>
#include <fstream>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/collectivecommunication.hh>
#include <dune/common/parallel/mpicollectivecommunication.hh>

#ifndef DISABLE_ALUGRID_SFC_ORDERING
#define USE_ALUGRID_SFC_ORDERING 1
#else
#warning "ALUGRID_SFC_ORDERING disabled by DISABLE_ALUGRID_SFC_ORDERING"
#endif

// to disable Zoltans HSFC ordering of the macro elements define
// DISABLE_ZOLTAN_HSFC_ORDERING on the command line
#if HAVE_ZOLTAN && HAVE_MPI
#define USE_ZOLTAN_SFC_ORDERING 1
#endif

#include <dune/alugrid/impl/parallel/zcurve.hh>
#if HAVE_ZOLTAN
#include <dune/alugrid/impl/parallel/aluzoltan.hh>

extern "C" {
  extern double Zoltan_HSFC_InvHilbert3d (Zoltan_Struct *zz, double *coord);
  extern double Zoltan_HSFC_InvHilbert2d (Zoltan_Struct *zz, double *coord);
}
#endif

namespace ALUGridSFC {

#if USE_ZOLTAN_SFC_ORDERING
  template <class Coordinate>
  class ZoltanSpaceFillingCurveOrdering
  {
    // type of communicator
    typedef Dune :: CollectiveCommunication< typename Dune :: MPIHelper :: MPICommunicator >
        CollectiveCommunication ;

    // type of Zoltan HSFC ordering function
    typedef double zoltan_hsfc_inv_t(Zoltan_Struct *zz, double *coord);

    static const int dimension = Coordinate::dimension;

    Coordinate lower_;
    Coordinate length_;

    const zoltan_hsfc_inv_t* hsfcInv_;

    mutable Zoltan zz_;

  public:
    ZoltanSpaceFillingCurveOrdering( const Coordinate& lower,
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

      Coordinate center( 0 ) ;
      // scale center into [0,1]^d box which is needed by Zoltan_HSFC_InvHilbert{2,3}d
      for( int d=0; d<dimension; ++d )
        center[ d ] = (point[ d ] - lower_[ d ]) / length_[ d ];

      // return hsfc index in interval [0,1]
      return hsfcInv_( zz_.Get_C_Handle(), &center[ 0 ] );
    }

    // return unique hilbert index in interval [0,1] given an element's center
    double index( const Coordinate& point ) const
    {
      return hilbertIndex( point );
    }
  };
#endif // if HAVE_ZOLTAN

  template< class GridView >
  void printSpaceFillingCurve ( const GridView& view, std::string name = "sfc" )
  {
    typedef typename GridView :: template Codim< 0 > :: Iterator  Iterator ;
    typedef typename Iterator :: Entity :: Geometry :: GlobalCoordinate GlobalCoordinate ;

    std::vector< GlobalCoordinate > vertices;
    vertices.reserve( view.indexSet().size( 0 ) );

    const Iterator endit = view.template end< 0 > ();
    for(Iterator it = view.template begin< 0 > (); it != endit; ++ it )
    {
      GlobalCoordinate center = (*it).geometry().center();
      vertices.push_back( center );
    }

    std::stringstream gnufilename;
    gnufilename << name << ".gnu";
    if( view.grid().comm().size() > 1 )
      gnufilename << "." << view.grid().comm().rank();

    std::ofstream gnuFile ( gnufilename.str() );
    if( gnuFile )
    {
      for( size_t i=0; i<vertices.size(); ++i )
      {
        gnuFile << vertices[ i ] << std::endl;
      }
      gnuFile.close();
    }

    std::stringstream vtkfilename;
    vtkfilename << name << ".vtk";
    if( view.grid().comm().size() > 1 )
      vtkfilename << "." << view.grid().comm().rank();
    std::ofstream vtkFile ( vtkfilename.str() );

    if( vtkFile )
    {
      vtkFile << "# vtk DataFile Version 1.0" << std::endl;
      vtkFile << "Line representation of vtk" << std::endl;
      vtkFile << "ASCII" << std::endl;
      vtkFile << "DATASET POLYDATA" << std::endl;
      vtkFile << "POINTS "<< vertices.size() << " FLOAT" << std::endl;

      for( size_t i=0; i<vertices.size(); ++i )
      {
        vtkFile << vertices[ i ];
        for( int d=GlobalCoordinate::dimension; d<3; ++d )
          vtkFile << " 0";
        vtkFile << std::endl;
      }

      // lines, #lines, #entries
      vtkFile << "LINES " << vertices.size()-1 << " " << (vertices.size()-1)*3 << std::endl;

      for( size_t i=0; i<vertices.size()-1; ++i )
        vtkFile << "2 " << i << " " << i+1 << std::endl;

      vtkFile.close();
    }
  }

} // end namespace ALUGridSFC

namespace Dune {

  template <class Coordinate>
  class SpaceFillingCurveOrdering : public
#if USE_ZOLTAN_SFC_ORDERING
    ::ALUGridSFC::ZoltanSpaceFillingCurveOrdering< Coordinate >
#else
#warning "(Zoltan && MPI) not found, using ALUGrid's ZCurve ordering"
    ::ALUGridSFC::ZCurve< long int, Coordinate::dimension>
#endif
  {

#if USE_ZOLTAN_SFC_ORDERING
    typedef ::ALUGridSFC::ZoltanSpaceFillingCurveOrdering< Coordinate > BaseType;
#else
    typedef ::ALUGridSFC::ZCurve< long int, Coordinate::dimension> BaseType;
#endif

    // type of communicator
    typedef Dune :: CollectiveCommunication< typename MPIHelper :: MPICommunicator >
        CollectiveCommunication ;

  public:
    SpaceFillingCurveOrdering( const Coordinate& lower,
                               const Coordinate& upper,
                               const CollectiveCommunication& comm =
                               CollectiveCommunication( Dune::MPIHelper::getCommunicator() ) )
      : BaseType( lower, upper, comm )
    {
    }

    template <class OtherComm>
    SpaceFillingCurveOrdering( const Coordinate& lower,
                               const Coordinate& upper,
                               const OtherComm& otherComm )
      : BaseType( lower, upper,
                  otherComm.size() > 1 ? CollectiveCommunication( Dune::MPIHelper::getCommunicator() ) :
                                         CollectiveCommunication( Dune::MPIHelper::getLocalCommunicator() ) )
    {
    }

    // return unique hilbert index in interval [0,1] given an element's center
    double index( const Coordinate& point ) const
    {
      return double( BaseType :: index( point ) );
    }
  };

} // end namespace Dune

#undef USE_ZOLTAN_SFC_ORDERING
#endif // #ifndef DUNE_ALU3DGRID_HSFC_HH
