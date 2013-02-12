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
      // initialize partitition 
      part[ i ] = 0;
    }

    // calculate average load 
    meanLoad /= double(nPart);

    // make sure that the mean load is the same on each proc 
    assert( std::abs( meanLoad - mpa.gmax( meanLoad ) ) < 1e-8 );

    // round the average load to get a threshold value 
    double threshold = meanLoad ;

    bool converged  = false ;
    bool readjust = false ;
    const idxtype lastRank = nPart - 1 ;
    int count = 0 ;
    int overallcount = 0 ;

    // vector of loads 
    std::vector< double > loads( nPart, 0.0 );

    double bestMaxDir = 1e308 ;
    double bestMinDir = 1e308 ;
    double bestThreshold = threshold ;
    double factor = 1.0;
    while ( ! converged ) 
    {
      // compute new partition 
      calculatePartition( nPart, threshold, nCells, weights, part ); 

      if( part[ nCells - 1 ] != lastRank )
      {
        threshold *= 0.975 ;
      }
      else 
      {
        std::fill( loads.begin(), loads.end(), 0.0 );

        // check load balance 
        for( idxtype i = 0; i<nCells; ++i )
        {
          loads[ part[ i ] ] += weights[ i ];
        }

        double minLd = loads[ 0 ];
        double maxLd = loads[ 0 ];

        // check load balance 
        for( int i = 1; i < nPart; ++ i ) 
        {
          minLd = std::min( minLd, loads[ i ] );
          maxLd = std::max( maxLd, loads[ i ] );
        }

        // if unbalanced partition detected 
        // the situation is most likely that the last proc has 
        // to many elements assigned 
        double minDir = 1.0 - minLd/ meanLoad ;
        double maxDir = 1.0 - meanLoad / maxLd ;
        assert( minDir >= 0.0 ); 
        assert( maxDir >= 0.0 );

        //if( mpa.myrank() == 0 ) 
        //  std::cout << minDir << " " << maxDir << "  |  "  << bestMinDir << " " << bestMaxDir << std::endl;
        if( minDir < 0.01 && maxDir < 0.01 )
        {
          // store best threshold 
          bestThreshold = threshold ;
          converged = true ;
          break ;
        }
        else 
        {
          // otherwise adjust threshold 
          converged = false ;

          //if( (minDir < bestMinDir) && (maxDir < bestMaxDir) )
          if( (minDir+maxDir) < (bestMinDir+bestMaxDir) && 
              std::min(minDir,maxDir) < std::min(bestMinDir, bestMaxDir) )
          {
            bestThreshold = threshold ;
            bestMinDir = minDir ;
            bestMaxDir = maxDir ;
            // we found a better result, count new 
            count = 0 ;
          }

          factor *= 0.9;

          if( maxDir > minDir ) 
          {
            threshold *= 1.0 + factor * std::min( 0.05, maxDir );
          }
          else
          {
            threshold *= 1.0 - factor * std::min( 0.05, minDir );
          }
          //if( mpa.myrank() == 0 )
          // std::cout << "New threshold = "  << threshold << std::endl;
        }
      }

      // if we did not improve result for 5 iterations then break
      if( count > 5 ) break ;
      ++ count ;
      // if we have more than 20 iterations also break 
      if( overallcount > 50 ) break ;
      ++ overallcount ;
    }

    if( ! converged ) 
    {
      // compute new partition 
      calculatePartition( nPart, bestThreshold, nCells, weights, part ); 
    }

    /*
    if( mpa.myrank() == 0 ) 
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
      std::cout << "Loads: mean = " << bestThreshold << std::endl;
      std::cout << "Loads: min = " << minLoad << "  max = " << maxLoad << std::endl;
      std::cout << "count = " << overallcount << std::endl;
      for( idxtype i = 0; i<nPart; ++i )
      {
        std::cout << "P[ " << i << " ] = " << loads[ i ] << std::endl;
      }
    }
    */
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
