#ifndef ALUGRID_SFC_H_INCLUDED
#define ALUGRID_SFC_H_INCLUDED

namespace ALUGridMETIS
{

template < class idxtype >
void CALL_spaceFillingCurve(const int myRank,  // my rank 
                            const idxtype nCells,  // number of cells 
                            const idxtype nPart,   // number of partitions 
                            const idxtype *weights, // vertex weights 
                            idxtype* part ) // new partitioning 
{
  // calculate mean load 
  double meanLoad = 0 ;
  for( idxtype i = 0 ; i<nCells; ++i )
  {
    meanLoad += weights[ i ];
  }

  // calculate average load 
  meanLoad /= double(nPart);

  int rank = 0;
  double load = 0;
  //const double meanThreshold = loadThreshold * meanLoad;
  //const double meanThreshold = 1.01 * meanLoad;
  const double meanThreshold = round( meanLoad );
  for( int i = 0; i < nCells; ++i )
  {
    double nextLoad = load + weights[ i ] ;
    if( load > 0 && ( nextLoad > meanThreshold ) )           
    {
      if( rank < (nPart - 1) ) ++rank ;
      load = 0;
    }

    part[ i ] = rank ;
    load += weights[ i ];
  }

  if( myRank == 0 ) 
  {
    // vector of loads 
    std::vector< double > loads( nPart, 0.0 );
    for( idxtype i = 0; i<nCells; ++i )
    {
      loads[ part[ i ] ] += weights[ i ];
    }

    std::cout << "Loads: " << std::endl;
    for( idxtype i = 0; i<nPart; ++i )
    {
      std::cout << "P[ " << i << " ] = " << loads[ i ] << std::endl;
    }
  }
} // end of simple sfc splitting 

} // end namespace ALUGridMETIS
#endif
