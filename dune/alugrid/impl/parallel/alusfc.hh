#ifndef ALUGRID_SFC_H_INCLUDED
#define ALUGRID_SFC_H_INCLUDED

#include <cmath>
#include <vector>

#include "mpAccess.h"

namespace ALUGridMETIS
{
  // Bresenham line drawing algorithm to partition the space filling curve
  template< class idxtype >
  void CALL_spaceFillingCurveNoEdges(const ALUGrid::MpAccessGlobal& mpa, // communicator
                                     const idxtype N,        // number of cells 
                                     const idxtype *weights, // vertex weights 
                                     idxtype* ranks )         // new partitioning 
  {
    // get number of partitions 
    const int numProcs = mpa.psize();

    long int sum = 0 ;
    for( int i = 0; i < N; ++i )
      sum += weights[ i ];

    int rank = 0;
    long int d = -sum / 2;
    for( int i = 0; i < N; ++i )
    {
      if( d >= sum / 2 )
      {
        ++rank;
        d -= sum;
      }
      ranks[ i ] = rank;
      d += numProcs*weights[ i ];
    }

    // make sure that the rank does not exceed the number of cores 
    assert( rank < numProcs );
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
