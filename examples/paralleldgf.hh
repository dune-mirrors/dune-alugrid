#ifndef ALUGRID_PARALLEL_DGF_HH
#define ALUGRID_PARALLEL_DGF_HH

#include <dune/grid/io/file/dgfparser/dgfs.hh>

#if ! HAVE_ALUGRID
#include <dune/alugrid/dgf.hh>
#include <dune/alugrid/common/structuredgridfactory.hh>
#endif

namespace Dune 
{
  template <class Grid> 
  struct CreateParallelGrid   
  {
    static GridPtr< Grid > create( const std::string& filename ) 
    {
      std::cout << "Reading the grid onto a single processor" << std::endl;
      return GridPtr< Grid >( filename );
    }
  };

#if ! HAVE_ALUGRID
  template < ALUGridRefinementType refineType, class Comm > 
  class CreateParallelGrid< ALUGrid< 3,3, Dune::cube, refineType, Comm > >
  {
    typedef ALUGrid< 3,3 , Dune::cube, refineType, Comm > Grid ;

  public:  
    static GridPtr< Grid > create( const std::string& filename ) 
    {
      typedef StructuredGridFactory< Grid > SGF;
      return SGF :: createCubeGrid( filename );
    }
  };
#endif // if ! HAVE_ALUGRID 
}

#endif
