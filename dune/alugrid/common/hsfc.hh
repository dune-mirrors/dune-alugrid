#ifndef DUNE_ALU3DGRID_HSFC_HH
#define DUNE_ALU3DGRID_HSFC_HH

#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#include <dune/common/exceptions.hh>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/communication.hh>
#include <dune/common/parallel/mpicommunication.hh>

#include <dune/alugrid/impl/parallel/zcurve.hh>
#if HAVE_ZOLTAN
#include <dune/alugrid/impl/parallel/aluzoltan.hh>

extern "C" {
  extern double Zoltan_HSFC_InvHilbert3d (Zoltan_Struct *zz, double *coord);
  extern double Zoltan_HSFC_InvHilbert2d (Zoltan_Struct *zz, double *coord);
}
#endif

namespace ALUGridSFC {

  template <class Coordinate>
  class ZoltanSpaceFillingCurveOrdering
  {
    // type of communicator
    typedef Dune :: Communication< typename Dune :: MPIHelper :: MPICommunicator >
        Communication ;

#if ! HAVE_ZOLTAN
    typedef void                      Zoltan_Struct;
    typedef Communication   Zoltan;
#endif

    // type of Zoltan HSFC ordering function
    typedef double zoltan_hsfc_inv_t(Zoltan_Struct *zz, double *coord);

    static const int dimension = Coordinate::dimension;

    Coordinate lower_;
    Coordinate length_;

    zoltan_hsfc_inv_t* hsfcInv_;

    mutable Zoltan zz_;

  public:
    ZoltanSpaceFillingCurveOrdering( const Coordinate& lower,
                                     const Coordinate& upper,
                                     const Communication& comm =
                                     Communication( Dune::MPIHelper::getCommunicator() ) )
      : lower_( lower ),
        length_( upper ),
#if HAVE_ZOLTAN
        hsfcInv_( dimension == 3 ? Zoltan_HSFC_InvHilbert3d : Zoltan_HSFC_InvHilbert2d ),
#else
        hsfcInv_( 0 ),
#endif
        zz_( comm )
    {
      // compute length
      length_ -= lower_;
    }

    // return unique hilbert index in interval [0,1] given an element's center
    double hilbertIndex( const Coordinate& point ) const
    {
#if HAVE_ZOLTAN
      assert( point.size() == (unsigned int)dimension );

      Coordinate center( 0 ) ;
      // scale center into [0,1]^d box which is needed by Zoltan_HSFC_InvHilbert{2,3}d
      for( int d=0; d<dimension; ++d )
        center[ d ] = (point[ d ] - lower_[ d ]) / length_[ d ];

      // return hsfc index in interval [0,1]
      return hsfcInv_( zz_.Get_C_Handle(), &center[ 0 ] );
#else
      DUNE_THROW(Dune::SystemError,"Zoltan not found, cannot use Zoltan's Hilbert curve");
      return 0.0;
#endif
    }

    // return unique hilbert index in interval [0,1] given an element's center
    double index( const Coordinate& point ) const
    {
      return hilbertIndex( point );
    }
  };

  template< class GridView >
  void printSpaceFillingCurve ( const GridView& view, std::string name = "sfc", const bool vtk = false  )
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

    if( vtk )
    {
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
  }

} // end namespace ALUGridSFC

namespace Dune {

  template <class Coordinate>
  class SpaceFillingCurveOrdering
  {
    typedef ::ALUGridSFC::ZCurve< long int, Coordinate::dimension>      ZCurveOrderingType;
    typedef ::ALUGridSFC::ZoltanSpaceFillingCurveOrdering< Coordinate > HilbertOrderingType;

    // type of communicator
    typedef Dune :: Communication< typename MPIHelper :: MPICommunicator >
        Communication ;
  public:
    enum CurveType { ZCurve, Hilbert, None };

#if HAVE_ZOLTAN
    static const CurveType DefaultCurve = Hilbert ;
#else
    static const CurveType DefaultCurve = ZCurve ;
#endif

  protected:
    ZCurveOrderingType   zCurve_;
    HilbertOrderingType  hilbert_;

    const CurveType curveType_;

  public:
    SpaceFillingCurveOrdering( const CurveType& curveType,
                               const Coordinate& lower,
                               const Coordinate& upper,
                               const Communication& comm =
                               Communication( Dune::MPIHelper::getCommunicator() ) )
      : zCurve_ ( lower, upper, comm )
      , hilbert_( lower, upper, comm )
      , curveType_( curveType )
    {
    }

    template <class OtherComm>
    SpaceFillingCurveOrdering( const CurveType& curveType,
                               const Coordinate& lower,
                               const Coordinate& upper,
                               const OtherComm& otherComm )
      : zCurve_ ( lower, upper,
                  otherComm.size() > 1 ? Communication( Dune::MPIHelper::getCommunicator() ) :
                                         Communication( Dune::MPIHelper::getLocalCommunicator() ) )
      , hilbert_( lower, upper,
                  otherComm.size() > 1 ? Communication( Dune::MPIHelper::getCommunicator() ) :
                                         Communication( Dune::MPIHelper::getLocalCommunicator() ) )
      , curveType_( curveType )
    {
    }

    // return unique hilbert index in interval [0,1] given an element's center
    double index( const Coordinate& point ) const
    {
      if( curveType_ == ZCurve )
      {
        return double( zCurve_.index( point ) );
      }
      else if ( curveType_ == Hilbert )
      {
        return double( hilbert_.index( point ) );
      }
      else
      {
        DUNE_THROW(NotImplemented,"Wrong space filling curve ordering selected");
        return 0.0;
      }
    }
  };

} // end namespace Dune

#endif // #ifndef DUNE_ALU3DGRID_HSFC_HH
