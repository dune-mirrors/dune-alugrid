#ifndef ALUGRID_PARALLEL_DGF_HH
#define ALUGRID_PARALLEL_DGF_HH

#include <dune/grid/io/file/dgfparser/dgfs.hh>
#include <dune/alugrid/dgf.hh>

#include <dune/alugrid/common/structuredgridfactory.hh>

namespace Dune 
{
  template <class Grid> 
  class CreateParallelGrid ;  

  template < int dim, int dimworld, ALUGridElementType eltype, 
             ALUGridRefinementType refineType, class Comm > 
  class CreateParallelGrid< ALUGrid< dim, dimworld, eltype, refineType, Comm > >
  {
    typedef ALUGrid< dim, dimworld, eltype, refineType, Comm > Grid ;

  public:  
    static GridPtr< Grid > create( const std::string& filename ) 
    {
      // this only works for Cartesian grid using DGF's IntervalBlock
      if( eltype == simplex ) 
        return GridPtr< Grid >( filename );

#if ! HAVE_ALUGRID
      typedef StructuredGridFactory< Grid > SGF;
      return SGF :: createCubeGrid( filename );
#else 
      return GridPtr< Grid > (filename);
#endif // if ! HAVE_ALUGRID 
    }
  };
}

#endif
