#ifndef DUNE_ALUGRID_COMMON_TYPETRAITS_HH
#define DUNE_ALUGRID_COMMON_TYPETRAITS_HH

#include <type_traits>
#include <utility>

#include <dune/grid/common/datahandleif.hh>

namespace Dune
{

  // IsDataHandle
  // -----------

  template< class Impl, class Data >
  std::true_type __IsDataHandle ( const CommDataHandleIF< Impl, Data > & );

  std::false_type __IsDataHandle ( ... );

  template< class T >
  struct IsDataHandle
    : public decltype( __IsDataHandle( std::declval< T >() ) )
  {};

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_COMMON_TYPETRAITS_HH
