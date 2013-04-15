#ifndef ALUGRID_SFC_H_INCLUDED
#define ALUGRID_SFC_H_INCLUDED

#include <cmath>
#include <vector>

#include "mpAccess.h"

namespace ALUGridMETIS
{
  // Bresenham line drawing algorithm to partition the space filling curve
  template< class idxtype >
  bool CALL_spaceFillingCurveNoEdges(const ALUGrid::MpAccessGlobal& mpa, // communicator
                                     const idxtype N,                    // number of cells 
                                     const idxtype *weights,             // vertex weights 
                                     idxtype* ranks )                    // new partitioning 
  {
    // get number of partitions 
    const int numProcs = mpa.psize();

    long int sum = 0 ;
    for( int i = 0; i < N; ++i )
      sum += weights[ i ];

    int rank = 0;
    long int d = -sum ;
    for( int i = 0; i < N; ++i )
    {
      if( d >= sum )
      {
        ++rank;
        d -= 2 * sum;
      }
      ranks[ i ] = rank;
      d += (2 * numProcs) * weights[ i ];
    }

    /*
    std::vector< int > els( numProcs, 0 );
    std::vector< int > load( numProcs, 0 );
    for( std::size_t i = 0; i < N; ++i )
    {
      ++els[ ranks[ i ] ];
      load[ ranks[ i ] ] += weights[ i ];
      // std::cout << ranks[ i ] << "  ";
    }

    //for( int i=0 ; i<numProcs; ++i ) 
    //  std::cout << "load[ " << i << " ] = " << load[ i ] << std::endl;
    */

    alugrid_assert ( rank < numProcs );
    // return true if partitioning is ok, should never be false 
    return (rank < numProcs);
  } // end of simple sfc splitting without edges 

  template < class idxtype >
  void CALL_spaceFillingCurve(const ALUGrid::MpAccessGlobal& mpa, // communicator
                              const idxtype nCells,   // number of cells 
                              const idxtype *weights, // vertex weights 
                              idxtype* part )         // new partitioning 
  {
    // TODO, also use edge info 
    CALL_spaceFillingCurveNoEdges( mpa, nCells, weights, part ); 
  } // end of simple sfc splitting 

} // namespace ALUGridMETIS

#endif // #ifndef ALUGRID_SFC_H_INCLUDED
