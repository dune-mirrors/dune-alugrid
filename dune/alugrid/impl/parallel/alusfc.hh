#ifndef ALUGRID_SFC_H_INCLUDED
#define ALUGRID_SFC_H_INCLUDED

#include <cmath>
#include <vector>

namespace ALUGridMETIS
{

  template< class idxtype >
  void CALL_spaceFillingCurveNoEdges(const int myRank,       // my rank 
                                     const idxtype nCells,   // number of cells 
                                     const idxtype nPart,    // number of partitions 
                                     const idxtype *weights, // vertex weights 
                                     idxtype* part )         // new partitioning 
  {
    // calculate mean load 
    double meanLoad = 0 ;
    for( idxtype i = 0 ; i<nCells; ++i )
    {
      meanLoad += weights[ i ];
    }

    // calculate average load 
    meanLoad /= double(nPart);

    // round the average load to get a threshold value 
    double meanThreshold = std::round( meanLoad );

    bool emptyProc = true ;
    bool readjust = false ;
    while ( emptyProc ) 
    {
      double load = 0;
      idxtype rank = 0;
      const idxtype lastRank = nPart - 1 ;
      for( idxtype i = 0; i < nCells; ++i )
      {
        double nextLoad = load + weights[ i ] ;
        if( load > 0 && ( nextLoad > meanThreshold ) )           
        {
          // only increase rank if not already the last rank 
          if( rank < lastRank ) ++rank ;
          load = 0;
        }

        part[ i ] = rank ;
        load += weights[ i ];
      }

      // the last element should belong to the last proc,
      // otherwise the partition is not balanced 
      if( part[ nCells-1 ] == lastRank )
      {
        emptyProc = false ;
      }
      else 
      {
        readjust = true ;
        // lower threshold and do it again 
        meanThreshold *= 0.95;
      }
    }

    /*
    if( myRank == 0 && readjust ) 
    {
      // vector of loads 
      std::vector< double > loads( nPart, 0.0 );
      for( idxtype i = 0; i<nCells; ++i )
      {
        loads[ part[ i ] ] += weights[ i ];
      }

      std::cout << "OrgLoad: mean = " << std::round( meanLoad ) << std::endl;
      std::cout << "Loads: mean = " << meanThreshold << std::endl;
      for( idxtype i = 0; i<nPart; ++i )
      {
        std::cout << "P[ " << i << " ] = " << loads[ i ] << std::endl;
      }
    }
    */
  } // end of simple sfc splitting without edges 

  template < class idxtype >
  void CALL_spaceFillingCurve(const int myRank,       // my rank 
                              const idxtype nCells,   // number of cells 
                              const idxtype nPart,    // number of partitions 
                              const idxtype *weights, // vertex weights 
                              idxtype* part )         // new partitioning 
  {
    // TODO, also use edge info 
    CALL_spaceFillingCurveNoEdges(myRank, nCells, nPart, weights, part ); 
  } // end of simple sfc splitting 

} // namespace ALUGridMETIS

#endif // #ifndef ALUGRID_SFC_H_INCLUDED
