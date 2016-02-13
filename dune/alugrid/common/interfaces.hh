#ifndef DUNE_ALUGRID_INTERFACES_HH
#define DUNE_ALUGRID_INTERFACES_HH

#include <type_traits>

/** @file
  @author Robert Kloefkorn
  @brief Provides a Interfaces for detection of specific behavior
*/

namespace Dune {

  //! Tagging interface to indicate that Grid provides typedef ObjectStreamType
  struct HasObjectStream {};

  //! Helper template (implicit specialisation if GridImp exports an object
  //! stream
  template <bool hasStream, class GridImp, class DefaultImp>
  struct GridObjectStreamOrDefaultHelper {
    typedef typename GridImp::InStreamType     InStreamType;
    typedef typename GridImp::OutStreamType    OutStreamType;
  };

  //! Helper template (explicit specialisation if GridImp doesn't export an
  //! object stream -> DefaultImplementation is exported)
  template <class GridImp, class DefaultImp>
  struct GridObjectStreamOrDefaultHelper<false, GridImp, DefaultImp> {
    typedef DefaultImp InStreamType;
    typedef DefaultImp OutStreamType;
  };

  //! Template to choose right Object stream type for a given class
  template <class GridImp, class DefaultImp>
  struct GridObjectStreamOrDefault
  {
    typedef GridObjectStreamOrDefaultHelper<
                std::is_base_of< HasObjectStream, GridImp >::value,
                GridImp,
                DefaultImp> GridObjectStreamTraits;

    typedef typename GridObjectStreamTraits :: InStreamType   InStreamType;  //  read  stream
    typedef typename GridObjectStreamTraits :: OutStreamType  OutStreamType; //  write stream
  };

  //! Tagging interface to indicate that class is of Type DofManager
  struct IsDofManager {};

  //! Tagging interface to indicate that Grid has HierarchicIndexSet
  struct HasHierarchicIndexSet {};

} // end namespace Dune
#endif
