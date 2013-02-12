#ifndef ALUGRID_SFC_H_INCLUDED
#define ALUGRID_SFC_H_INCLUDED

#include <cmath>
#include <vector>

#include "mpAccess.h"

namespace ALUGridMETIS
{

  template < class idxtype >
  void calculatePartition( const int nPart, 
                           const double threshold,
                           const idxtype nCells, 
                           const idxtype* weights, 
                           idxtype* part ) 
  {
    double load = 0;
    idxtype rank = 0;
    const idxtype lastRank = nPart - 1 ;
    for( idxtype elem = 0; elem < nCells; ++elem )
    {
      double nextLoad = load + weights[ elem ] ;
      if( load > 0 && ( nextLoad > threshold ) )           
      {
        // only increase rank if not already the last rank 
        if( rank < lastRank ) 
        {
          ++rank ;
        }
        load = 0;
      }

      part[ elem ] = rank ;
      load += weights[ elem ];
    }
  }

  template< class idxtype >
  void CALL_spaceFillingCurveNoEdges(const ALUGrid::MpAccessGlobal& mpa, // communicator
                                     const idxtype nCells,   // number of cells 
                                     const idxtype *weights, // vertex weights 
                                     idxtype* part )         // new partitioning 
  {
    // get number of partitions 
    const int nPart = mpa.psize();

    // make sure that the number of cells in the graph is the same on each core 
    assert( nCells == mpa.gmax( nCells ) );

    // calculate mean load 
    double meanLoad = 0 ;
    for( idxtype i = 0 ; i<nCells; ++i )
    {
      meanLoad += weights[ i ];
    }

    // calculate average load 
    meanLoad /= double(nPart);

    // make sure that the mean load is the same on each proc 
    assert( std::abs( meanLoad - mpa.gmax( meanLoad ) ) < 1e-8 );

    // round the average load to get a threshold value 
    double meanThreshold = std::round ( meanLoad ) ;

    // vector of loads 
    std::vector< int > offsets( nPart+1, -1 );

    bool emptyProc = true ;
    bool readjust = false ;
    const idxtype lastRank = nPart - 1 ;
    offsets[ 0 ] = 0;
    while ( emptyProc ) 
    {
      // compute new partition 
      calculatePartition( nPart, meanThreshold, nCells, weights, part ); 

      offsets[ nPart ] = nCells;

      // the last element should belong to the last proc,
      // otherwise the partition is not balanced 
      if( part[ nCells-1 ] == lastRank )
      {
        emptyProc  = false ;

        // vector of loads 
        std::vector< double > loads( nPart, 0.0 );

        // check load balance 
        for( idxtype i = 0; i<nCells; ++i )
        {
          loads[ part[ i ] ] += weights[ i ];
        }

        double minLoad = loads[ 0 ];
        double maxLoad = loads[ 0 ];

        // check load balance 
        for( int i = 1; i < nPart; ++ i ) 
        {
          minLoad = std::min( minLoad, loads[ i ] );
          maxLoad = std::max( maxLoad, loads[ i ] );
        }

        // if unbalanced partition detected 
        // the situation is most likely that the last proc has 
        // to many elements assigned 
        if( maxLoad/meanThreshold > 1.02 || minLoad/meanThreshold < 0.97 ) 
        {
          const double upperBound = 1.01 * meanThreshold;
          // backward sweep 
          for( int i = nPart-1; i > 0; -- i ) 
          {
            while ( loads[ i ] > upperBound && offsets[ i ] < (offsets[ i+1 ] - 1) )
            {
              // put the firt element of this partition to the 
              // previous proc 
              --part[ offsets[ i ] ] ;
              loads[ i ]   -= weights[ offsets[ i ] ];
              loads[ i-1 ] += weights[ offsets[ i ] ];
              ++offsets[ i ];
              assert( part[ offsets[ i ] ] >= 0 );
            }
          }

          // forward sweep 
          for( int i = 0; i<nPart-1; ++i ) 
          {
            int lastIndex = (offsets[ i+1 ]-1);
            const int firstIndex = offsets[ i ];
            while ( loads[ i ] > upperBound && lastIndex > firstIndex )
            {
              // put the firt element of this partition to the 
              // previous proc 
              ++part[ lastIndex ] ;
              loads[ i ]   -= weights[ lastIndex ];
              loads[ i+1 ] += weights[ lastIndex ];
              --offsets[ i+1 ];
              assert( part[ lastIndex ] < nPart );
              // update lastIndex 
              lastIndex = (offsets[ i+1 ]-1);
            }
          }
        }
      }
      else 
      {
        readjust = true ;
        // lower threshold and do it again 
        meanThreshold *= 0.95;
      }
    }

#ifndef NDEBUG
    if( mpa.myrank() == 0 && readjust ) 
    {
      // vector of loads 
      std::vector< double > loads( nPart, 0.0 );
      for( idxtype i = 0; i<nCells; ++i )
      {
        loads[ part[ i ] ] += weights[ i ];
      }

      double minLoad = loads[ 0 ];
      double maxLoad = loads[ 0 ];

      // check load balance 
      for( int i = 1; i < nPart; ++ i ) 
      {
        minLoad = std::min( minLoad, loads[ i ] );
        maxLoad = std::max( maxLoad, loads[ i ] );
      }

      std::cout << "OrgLoad: mean = " << std::round( meanLoad ) << std::endl;
      std::cout << "Loads: mean = " << meanThreshold << std::endl;
      std::cout << "Loads: min = " << minLoad << "  max = " << maxLoad << std::endl;
      for( idxtype i = 0; i<nPart; ++i )
      {
        std::cout << "P[ " << i << " ] = " << loads[ i ] << std::endl;
      }
    }
#endif
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
