#ifndef ALUGRID_SFC_H_INCLUDED
#define ALUGRID_SFC_H_INCLUDED

#include <cmath>
#include <vector>

#include <dune/alugrid/impl/serial/gitter_sti.h>
#include <dune/alugrid/impl/parallel/mpAccess.h>
#include <dune/alugrid/impl/parallel/gitter_pll_ldb.h>

namespace ALUGridSFC
{
  // Bresenham line drawing algorithm to partition the space filling curve.
  // This requires the complete graph weights to be present on all cores.
  // See http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  template< class vertexmap_t, class connect_t, class vec_t >
  bool CALL_spaceFillingCurve(const ALUGrid::MpAccessGlobal& mpa, // communicator
                              const int numProcs,                 // number of partitions
                              vertexmap_t& vertexMap,             // the space filling curve
                              connect_t&   connect,               // connectivity set
                              vec_t& graphSizes,                  // graph sizes to be communicated
                              const bool clearMap )               // true if vertex entries should no be deleted
  {
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
    const int sizeOfVertexData = ALUGrid::LoadBalancer::GraphVertex::sizeOfData ;

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
        graphSizes[ destination ] += sizeOfVertexData ;

      // if the element currently belongs to me
      // then check the new destination
      if( source == me && destination != me )
      {
        // insert into linkage set as send rank
        connect.insert( ALUGrid::MpAccessLocal::sendRank( destination ) );
      }
      else if( source != me )
      {
        if( clearMap )
        {
          // mark element for delete
          (*it).second = -1 ;
        }

        if( destination == me )
        {
          // insert into linkage set (receive ranks have negative numbers), see MpAccessLocal
          connect.insert( ALUGrid::MpAccessLocal::recvRank( source ) );
        }
      }
    }

    if( clearMap )
    {
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
    }

    alugrid_assert ( destination < numProcs );
    // return true if partitioning is ok, should never be false
    return (destination < numProcs);
  } // end of simple sfc splitting without edges

  template <class vec_t>
  void shiftElementCuts( const int me, const int pSize, const int nElem, vec_t& elementCuts )
  {
    typedef typename vec_t :: value_type value_type ;

    // only do this if number of procs is smaller then number of elements
    if( pSize <= nElem )
    {
      bool emptyPart = false ;
      for( int i=1; i<pSize; ++i )
      {
        // check for empty partition
        if( elementCuts[ i-1 ] == elementCuts[ i ] )
        {
          emptyPart = true ;
          break ;
        }
      }

      int count = 0 ;
      // assign at least one element to each proc
      while( emptyPart )
      {
        emptyPart = false ;
        for( int i=1; i<pSize; ++i )
        {
          value_type& elemCut = elementCuts[ i ];
          if( elementCuts[ i-1 ] >= elemCut && elemCut < nElem )
          {
            emptyPart = true ;
            // assign at least one element
            ++elemCut ;
          }
        }

        for( int i=pSize-1; i>0; --i )
        {
          value_type& elemCut = elementCuts[ i-1 ];
          if( elementCuts[ i ] <= elemCut && elemCut > 0 )
          {
            emptyPart = true ;
            // assign at least one element
            --elemCut ;
          }
        }

        // only allow for pSize iterations
        ++ count ;
        if( count > pSize ) break ;
      }
    }
  } // shiftElementCuts

  // Partitioning of the space filling curve after Burstedde, Wilcox, and Ghattas, p4est, 2011.
  // This method needs two global communications, one allgather and one allreduce
  template< class vertexmap_t, class connect_t, class vec_t >
  bool CALL_parallelSpaceFillingCurve(const ALUGrid::MpAccessGlobal& mpa, // communicator
                                      const int numProcs,                 // number of partitions
                                      vertexmap_t& vertexMap,             // the space filling curve
                                      connect_t&   connect,               // connectivity set
                                      vec_t& elementCuts )                // element cuts in sfc
  {
    // my rank
    const int me   = mpa.myrank();

    /*
    if( me == 0 )
    {
      static int called = 0;
      std::cout << "ALUsfc repartitioning call " << called++ << std::endl;
    }
    */

    // number of procs
    const int pSize = mpa.psize();

    typedef typename vertexmap_t :: iterator   iterator ;
    const iterator vertexEnd = vertexMap.end();

    int maxIdx = -1;
    int sum = 0 ;
    // compute my sum of
    for( iterator it = vertexMap.begin(); it != vertexEnd; ++ it )
    {
      const int weight = (*it).first.weight();
      const int index = (*it).first.index();
      sum += weight;
      maxIdx = std::max( index, maxIdx );
    }

    const int myCut = (maxIdx >= 0 ) ? maxIdx + 1 : -1;

    int nMax = 0 ;
    // if element cuts have not been computed, compute current cuts
    if( elementCuts.size() == 0 )
    {
      elementCuts = mpa.gcollect( myCut );
      for( int i=0; i<pSize; ++ i)
        nMax = std::max( nMax, elementCuts[ i ] );
    }
    else
    {
      // number of macro elements corresponds to
      // last element cut if cuts have been computed
      nMax = elementCuts[ pSize-1 ];
    }

    // number of macro elements corresponds to last element cut
    const int nElem = nMax ;

    long int Wme   = 0;
    long int Wnext = 0;
    long int Wlast = 0;

    {
      // allgather W[0,...,p]
      std::vector< int > Wlocal = mpa.gcollect( sum );

      // accumulate weights
      for( int i=0; i<pSize; ++i )
        Wlast += Wlocal[ i ];

      // get cumulative weight for me
      for( int i=0; i<me; ++i )
        Wme += Wlocal[ i ];

      // get sum for me+1
      Wnext = Wme + sum;
    }

    // compute the average weight
    const double averageWeight = double(Wlast) / double(pSize);

    // clear connectivity set
    connect.clear();

    int pLow  = pSize ;
    int pHigh = 0;
    std::set< int > S;
    for( int pstar=1; pstar<=pSize; ++pstar )
    {
      const long int pStarAver = std::floor( averageWeight * double(pstar) );
      if( Wme < pStarAver && pStarAver <= Wnext )
      {
        S.insert( pstar );
      }
    }

    typedef std::set< int > :: iterator setiterator ;
    const setiterator end = S.end();
    for( setiterator it = S.begin(); it != end; ++ it )
    {
      pLow  = std::min( *it, pLow  );
      pHigh = std::max( *it, pHigh );
    }

    // cuts of partitioning
    std::vector< int > cuts( pSize, 0 );

    if( pLow <= pHigh )
    {
      for( int pstar = pLow; pstar<= pHigh; ++pstar )
      {
        int minI = std::numeric_limits< int > :: max();
        const long int pStarAver = std::floor( averageWeight * double(pstar) );
        // use cummulative weight
        long int weight = Wme;
        for( iterator it = vertexMap.begin(); it != vertexEnd; ++ it )
        {
          // accumulate weight
          weight += (*it).first.weight();

          if( weight >= pStarAver )
          {
            minI = std::min( int(minI), (*it).first.index()+1 );
            break ;
          }
        }

        if( minI < std::numeric_limits< int > :: max() )
          cuts[ pstar-1 ] = minI;
      }
    }

    std::vector< int > oldCuts( elementCuts );

    elementCuts.clear();
    elementCuts.resize( pSize, 0 );

    // communicate cuts
    mpa.gmax( &cuts[ 0 ], pSize, &elementCuts[ 0 ] );

    // make sure that every process has at least one element
    shiftElementCuts( me, pSize, nElem, elementCuts );

    // get start and end element
    const int wStart = (me == 0) ? 0 : elementCuts[ me-1 ];
    const int wEnd   = elementCuts[ me ];

    // get source connectivity
    for( int source = 0, lastStart = 0 ; source < pSize; ++source )
    {
      // we wont receive from ourselfs
      if( oldCuts[ source ] < 0 ) continue ;

      const int w1 = std::max( wStart, lastStart );
      const int w2 = std::min( wEnd  , oldCuts[ source ] );

      if( source != me && (w1 >= wStart && w2 <= wEnd) && w2 > w1 )
      {
        // insert source connectivity
        connect.insert( ALUGrid::MpAccessLocal::recvRank( source ) );
      }

      // update last interval start
      lastStart = oldCuts[ source ];
    }

    int destination = 0;
    for( iterator it = vertexMap.begin(); it != vertexEnd; ++ it )
    {
      // increment destination number to correct interval
      while( elementCuts[ destination ] <= (*it).first.index() &&
             destination < pSize-1 )
      {
        ++destination;
      }

      // set new destination
      (*it).second = destination ;

      // element currently belongs to me
      // so set destination to connectivity if different
      if( destination != me )
      {
        // insert into linkage set as send rank
        connect.insert( ALUGrid::MpAccessLocal::sendRank( destination ) );
      }
    }

    //if( me == 0 )
    //  for( int i=0; i<pSize; ++ i)
    //    std::cout << elementCuts[ i ] << std::endl;


    // check whether the element cuts have changed
    {
      typedef std::vector<int>::iterator iterator ;
      const iterator end = elementCuts.end();
      for( iterator newCut = elementCuts.begin(), oldCut = oldCuts.begin();
           newCut != end; ++newCut, ++oldCut )
      {
        // if cuts changed return true
        if( *newCut != *oldCut ) return true ;
      }
    }

    // no change
    return false;
  } // end of simple sfc splitting without edges
} // namespace ALUGridMETIS

#endif // #ifndef ALUGRID_SFC_H_INCLUDED
