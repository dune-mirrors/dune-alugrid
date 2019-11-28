#ifndef ALUGRID_PARALLEL_DGF_HH
#define ALUGRID_PARALLEL_DGF_HH

#include <dune/alugrid/common/structuredgridfactory.hh>

#include <dune/alugrid/dgf.hh>
#include <dune/alugrid/common/structuredgridfactory.hh>

namespace Dune
{
  template <class Grid>
  struct CreateParallelGrid
  {
    static GridPtr< Grid > create( const std::string& filename )
    {
      //std::cout << "Reading the grid onto a single processor" << std::endl;
      return GridPtr< Grid >( filename );
    }
  };

  template < int dim, int dimworld, ALUGridRefinementType refineType, class Comm >
  class CreateParallelGrid< ALUGrid< dim, dimworld, Dune::cube, refineType, Comm > >
  {
    typedef ALUGrid< dim, dimworld, Dune::cube, refineType, Comm > Grid ;

  public:
    static GridPtr< Grid > create( const std::string& filename )
    {
      typedef StructuredGridFactory< Grid > SGF;
      return GridPtr< Grid >( SGF :: createCubeGrid( filename ).release() ) ;
    }
  };
}

#endif
