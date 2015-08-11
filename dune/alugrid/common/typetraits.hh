#ifndef DUNE_ALUGRID_COMMON_TYPETRAITS_HH
#define DUNE_ALUGRID_COMMON_TYPETRAITS_HH

#include <type_traits>
#include <utility>

#include <dune/grid/common/datahandleif.hh>

namespace Dune
{

  // HasDataType
  // -----------

  template< class T >
  std::true_type __HasDataType ( const T &, typename T::DataType * = nullptr );

  std::false_type __HasDataType ( ... );

  template< class T >
  struct HasDataType
    : public decltype( __HasDataType( std::declval< T >() ) )
  {};



  // IsDataHandle
  // ------------

  template< class T, bool >
  struct __IsDataHandle;

  template< class T >
  struct __IsDataHandle< T, true >
    : public std::is_base_of< CommDataHandleIF< T, typename T::DataType >, T >
  {};

  template< class T >
  struct __IsDataHandle< T, false >
    : std::false_type
  {};

  template< class T, bool = HasDataType< T >::value >
  struct IsDataHandle
    : public __IsDataHandle< typename std::decay< T >::type, HasDataType< T >::value >
  {};

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_COMMON_TYPETRAITS_HH
