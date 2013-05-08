#ifndef ALUGRID_SFC_H_INCLUDED
#define ALUGRID_SFC_H_INCLUDED

#include <cmath>
#include <vector>

#include "mpAccess.h"

namespace ALUGridMETIS
{
  // Bresenham line drawing algorithm to partition the space filling curve
  template< class vertexmap_t, class connect_t, class vec_t >
  bool CALL_spaceFillingCurve(const ALUGrid::MpAccessGlobal& mpa, // communicator
                              vertexmap_t& vertexMap,             // the space filling curve
                              connect_t&   connect,               // connectivity set
                              vec_t& graphSizes )                 // graph sizes to be communicated 
  {
    // get number of partitions 
    const int numProcs = mpa.psize();
    // my rank 
    const int me = mpa.myrank();

    // clear connectivity set 
    connect.clear();

    typedef typename vertexmap_t :: iterator   iterator ;
    const iterator vertexEnd = vertexMap.end();
    long int sum = 0 ;
    // compute sum at first 
    for( iterator it = vertexMap.begin(); it != vertexEnd; ++ it ) 
    {
      sum += (*it).first.weight();
    }

    const bool graphSizeCalculation = graphSizes.size() > 0 ;

    int destination = 0;
    long int d = -sum ;
    for( iterator it = vertexMap.begin(); it != vertexEnd; ++ it ) 
    {
      // increase destination if neccessary 
      if( d >= sum )
      {
        ++destination;
        d -= 2 * sum;
      }

      // get current rank
      const int source = (*it).second ;

      // set new rank information 
      (*it).second = destination ;
      // add weight 
      d += (2 * numProcs) * ((*it).first.weight());

      // add communication sizes of graph
      if( graphSizeCalculation ) 
        graphSizes[ destination ] += ALUGrid::LoadBalancer::GraphVertex::sizeOfData ;

      // if the element currently belongs to me
      // then check the new destination 
      if( source == me && destination != me )
      {
        // insert into linkage set as send rank  
        connect.insert( ALUGrid::MpAccessLocal::sendRank( destination ) );
      }
      else if( source != me ) 
      {
#ifndef STORE_LINKAGE_IN_VERTICES
        // mark element for delete 
        (*it).second = -1 ;
#endif
        if( destination == me )
        {
          // insert into linkage set (receive ranks have negative numbers), see MpAccessLocal 
          connect.insert( ALUGrid::MpAccessLocal::recvRank( source ) );
        }
      }
    }

#ifndef STORE_LINKAGE_IN_VERTICES
    // erase elements that are not further needed to save memory 
    for (iterator it = vertexMap.begin (); it != vertexEnd; )
    {
      // if element does neither belong to me not will belong to me, erase it 
      if( (*it).second < 0 )
      {
        vertexMap.erase( it++ );
      }
      else
        ++ it;
    }
#endif

    alugrid_assert ( destination < numProcs );
    // return true if partitioning is ok, should never be false 
    return (destination < numProcs);
  } // end of simple sfc splitting without edges 

} // namespace ALUGridMETIS

#endif // #ifndef ALUGRID_SFC_H_INCLUDED
