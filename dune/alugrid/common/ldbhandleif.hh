#ifndef DUNE_LDBHANDLEIF_HH
#define DUNE_LDBHANDLEIF_HH

/** @file
 *  @author Robert Kloefkorn
 *  @brief Describes the load balancing nterface class for 
 *         Dune::Grid */

#include <dune/common/bartonnackmanifcheck.hh>

namespace Dune
{

  /** @brief
     Interface class for stearing the load balancing process of a Dune::Grid.
     There are two ways to influence the load balancing: 
      - provide a complete partitioning via providing a rank number 
        for each given element, or 
      - provide user defined weight loads and use the grid's internal 
        load balancing algorithm. 
  */
  template <class LoadBalanceHandleImpl>
  class LoadBalanceHandleIF 
  {
  protected:  
    // one should not create an explicit instance of this inteface object
    LoadBalanceHandleIF() {} 

  public:
    /** \brief return true, if user defined partitioning available or not */
    bool userDefinedPartitioning () const 
    {
      CHECK_INTERFACE_IMPLEMENTATION((asImp().userDefinedPartitioning()));
      return asImp().userDefinedPartitioning();
    }

    /** \brief return true, if user defined load balancing weights are provided */
    bool userDefinedLoadWeights () const 
    { 
      CHECK_INTERFACE_IMPLEMENTATION((asImp().userDefinedPartitioning()));
      return asImp().userDefinedLoadWeights();
    }

    /** \brief returns true if user defined partitioning needs to be readjusted */
    bool repartition () 
    { 
      CHECK_INTERFACE_IMPLEMENTATION((asImp().repartition()));
      return asImp().repartition();
    }

    /** \brief return load weight of given element
     *  \note  this requires the method userDefinedLoadWeights to return true */
    template <class Entity>
    int loadWeight( const Entity &element ) const 
    { 
      CHECK_INTERFACE_IMPLEMENTATION((asImp().loadWeight(element)));
      return asImp().loadWeight( element );
    }

    /** \brief return destination (i.e. rank) where the given element should be moved to 
     *  \note  this needs the methods userDefinedPartitioning to return true
     */
    template <class Entity>
    int destination( const Entity &element ) const 
    { 
      CHECK_INTERFACE_IMPLEMENTATION((asImp().destination(element)));
      return asImp().destination( element );
    }

  private:
    //!  Barton-Nackman trick 
    LoadBalanceHandleImpl& asImp () {return static_cast<LoadBalanceHandleImpl &> (*this);}
    //!  Barton-Nackman trick 
    const LoadBalanceHandleImpl& asImp () const 
    {
      return static_cast<const LoadBalanceHandleImpl &>(*this);
    }
  }; // end class LoadBalanceDataHandleIF 

  struct LoadBalanceHandleWithReserveAndCompress {};

} // end namespace Dune 
#endif // #if DUNE_LDBHANDLEIF_HH
