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
      typedef typename Grid :: ctype ctype ;
      // type of communicator 
      typedef Dune :: CollectiveCommunication< typename MPIHelper :: MPICommunicator >
        CollectiveCommunication ;

      CollectiveCommunication comm( MPIHelper :: getCommunicator() );
      const int myrank = comm.rank();

      typedef SGrid< dim, dimworld, ctype > SGridType ;
      // only work for the new ALUGrid version 
      // if creation of SGrid fails the DGF file does not contain a proper
      // IntervalBlock, and thus we cannot create the grid parallel, 
      // we will use the standard technique 
      bool sgridCreated = true ;
      array<int, dim> dims;
      FieldVector<ctype, dimworld> lowerLeft ( 0 ); 
      FieldVector<ctype, dimworld> upperRight( 0 ); 
      if( myrank == 0 ) 
      {
        GridPtr< SGridType > sPtr;
        try 
        { 
          sPtr = GridPtr< SGridType >( filename );
        }
        catch ( DGFException & e ) 
        {
          sgridCreated = false ;
          std::cout << "Caught DGFException on creation of SGrid, trying default DGF method!" << std::endl;
        }
        if( sgridCreated ) 
        { 
          SGridType& sgrid = *sPtr ;
          dims = sgrid.dims( 0 );
          lowerLeft  = sgrid.lowerLeft();
          upperRight = sgrid.upperRight();
        }
      }

      // get global min to be on the same path
      sgridCreated = comm.min( sgridCreated );
      if( ! sgridCreated ) 
      {
        // use traditional method
        return GridPtr< Grid >( filename );
      }
      else 
      { 
        // broadcast array values 
        comm.broadcast( &dims[ 0 ], dim, 0 );
        comm.broadcast( &lowerLeft [ 0 ], dim, 0 );
        comm.broadcast( &upperRight[ 0 ], dim, 0 );
      }

      typedef StructuredGridFactory< Grid > SGF;
      return GridPtr< Grid > ( SGF :: createCubeGrid( lowerLeft, upperRight, dims ) );
#else 
      return GridPtr< Grid > (filename);
#endif // if ! HAVE_ALUGRID 
    }
  };
}

#endif
