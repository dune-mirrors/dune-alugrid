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
        if( rank < lastRank ) ++rank ;
        load = 0;
      }

      part[ elem ] = rank ;
      load += weights[ elem ];
    }
  }

  template <class idxtype>
  bool checkPartitioning( const int nPart,
                          const double threshold,
                          const double meanLoad,
                          const int nCells,
                          const idxtype* weights,
                          idxtype* part,
                          std::vector<double>& loads, 
                          double& minLd, double& maxLd )
  {
    // compute new partition 
    calculatePartition( nPart, threshold, nCells, weights, part ); 

    std::fill( loads.begin(), loads.end(), 0.0 );

    // check load balance 
    for( idxtype i = 0; i<nCells; ++i )
    {
      loads[ part[ i ] ] += weights[ i ];
    }

    minLd = loads[ 0 ];
    maxLd = loads[ 0 ];

    // check load balance 
    for( int i = 1; i < nPart; ++ i ) 
    {
      minLd = std::min( minLd, loads[ i ] );
      maxLd = std::max( maxLd, loads[ i ] );
    }

    // if unbalanced partition detected 
    // the situation is most likely that the last proc has 
    // to many elements assigned 
    double minDir = 1.0 - minLd / meanLoad ;
    double maxDir = 1.0 - meanLoad / maxLd ;

    assert( minDir >= 0.0 ); 
    assert( maxDir >= 0.0 );

    if( minDir < 0.01 && maxDir < 0.01 )
      return true ;
    else 
      return false ;
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

    bool converged  = false ;
    int count = 0 ;

    // vector of loads 
    std::vector< double > loads( nPart, 0.0 );

    double bestSumMinMax = 1e308 ;
    double bestMinMax = 1e308 ;
    double bestThreshold = meanLoad ;

    double lastSumMinMax = 1e308 ;
    double lastMinMax = 1e308 ;
    double lastThreshold = meanLoad ;
    double threshold = meanLoad ;

    {
      double minLd = 0;
      double maxLd = 0;

      // compute new partition 
      if( checkPartitioning( nPart, threshold, meanLoad, 
                             nCells, weights, part, loads, minLd, maxLd ) )
      {
        converged = true ;
        bestThreshold = threshold ;
        //std::cout << "Converged " << std::endl;
      } 

      // if unbalanced partition detected 
      // the situation is most likely that the last proc has 
      // to many elements assigned 
      double minDir = 1.0 - minLd / meanLoad ;
      double maxDir = 1.0 - meanLoad / maxLd ;

      assert( minDir >= 0.0 ); 
      assert( maxDir >= 0.0 );

      // if( minDir > maxDir ) sign = -1.0;

      const double sumMinMax = minDir + maxDir;
      const double minMax    = std::max( minDir, maxDir);

      if( sumMinMax < bestSumMinMax && minMax < bestMinMax ) 
      {
        count = 0;
        bestThreshold = threshold ;
        bestMinMax    = minMax ;
        bestSumMinMax = sumMinMax ;
      }
    }

    double factor = 0.2;
    double sign = 1.0 ;
    threshold = ( 1.0 + factor ) * meanLoad ;

    //if( mpa.myrank() == 0 )
    //  std::cout << "Start serach " << meanLoad << " " << threshold << std::endl;

    while ( ! converged ) 
    {
      double minLd = 0;
      double maxLd = 0;

      // compute new partition 
      if( checkPartitioning( nPart, threshold, meanLoad, 
                             nCells, weights, part, loads, minLd, maxLd ) )
      {
        converged = true ;
        bestThreshold = threshold ;
        // std::cout << "Converged " << std::endl;
        break ;
      } 

      // if unbalanced partition detected 
      // the situation is most likely that the last proc has 
      // to many elements assigned 
      double minDir = 1.0 - minLd / meanLoad ;
      double maxDir = 1.0 - meanLoad / maxLd ;

      assert( minDir >= 0.0 ); 
      assert( maxDir >= 0.0 );

      const double sumMinMax = minDir + maxDir;
      const double minMax    = std::max( minDir, maxDir);

      if( sumMinMax < bestSumMinMax && minMax < bestMinMax ) 
      {
        //count = 0;
        bestThreshold = threshold ;
        bestMinMax    = minMax ;
        bestSumMinMax = sumMinMax ;
      }

      if( sumMinMax < lastSumMinMax && minMax < lastMinMax ) 
      {
        //count = 0;
        lastThreshold = threshold ;
        lastMinMax    = minMax ;
        lastSumMinMax = sumMinMax ;
      }

      factor -= 0.005 ;
      threshold = ( 1.0 + sign * factor ) * meanLoad ;

      /*
      if( mpa.myrank() == 0 ) 
      {
        std::cout << " thres =  " << threshold << " " << bestThreshold << std::endl;
        std::cout << " minMax = " << minMax << " sum = " << sumMinMax ;
        std::cout << " bestMinMax = " << bestMinMax << "  bestSum " << bestSumMinMax << std::endl;
      }
      */

      if( factor < 0 ) 
      {
        if( sign > 0 ) 
        {
          sign = -1.0; 
          factor = 0.2;
        }
        else 
          break ; 
      }
      ++ count ;
      // if( count > 25 ) break ;
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

      std::cout << "OrgLoad: mean = " << meanLoad << std::endl;
      std::cout << "Loads: mean = " << bestThreshold << std::endl;
      std::cout << "Loads: min = " << minLoad << "  max = " << maxLoad << std::endl;
      std::cout << "count = " << count << std::endl;
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
