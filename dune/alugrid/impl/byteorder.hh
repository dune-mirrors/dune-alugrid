#ifndef DUNE_ALUGRID_IMPL_BYTEORDER_HH
#define DUNE_ALUGRID_IMPL_BYTEORDER_HH

#include <algorithm>
#include <cassert>

namespace ALUGrid
{

  // ByteOrder
  // ---------

  enum ByteOrder { bigEndian = 0x12345678, littleEndian = 0x78563412 };



  // SystemByteOrder
  // ---------------

  union SystemByteOrder
  {
    SystemByteOrder ()
    {
      byte[ 0 ] = 0x12;
      byte[ 1 ] = 0x34;
      byte[ 2 ] = 0x56;
      byte[ 3 ] = 0x78;
      assert( (integer == bigEndian) || (integer == littleEndian) );
    }

    operator unsigned int () const { return integer; }

  private:
    char byte[ 4 ];
    unsigned int integer;
  };

  inline static unsigned int systemByteOrder () { return SystemByteOrder(); }



  // bufferCopy
  // ----------

  template< ByteOrder byteOrder, class T >
  inline static void bufferCopy ( T *dest, const void *src, std::size_t n )
  {
    char *d = reinterpret_cast< char * >( dest );
    const char *s = static_cast< const char * >( src );
    if( byteOrder != systemByteOrder() )
    {
      for( std::size_t i = 0; i < n; ++i )
        std::reverse_copy( s + i*sizeof( T ), s + (i+1)*sizeof( T ), d + i*sizeof( T ) );
    }
    else
    {
      for( std::size_t i = 0; i < n; ++i )
        std::copy( s + i*sizeof( T ), s + (i+1)*sizeof( T ), d + i*sizeof( T ) );
    }
  }

  template< ByteOrder byteOrder, class T >
  inline static void bufferCopy ( void *dest, const T *src, std::size_t n )
  {
    char *d = static_cast< char * >( dest );
    const char *s = reinterpret_cast< const char * >( src );
    if( byteOrder != systemByteOrder() )
    {
      for( std::size_t i = 0; i < n; ++i )
        std::reverse_copy( s + i*sizeof( T ), s + (i+1)*sizeof( T ), d + i*sizeof( T ) );
    }
    else
    {
      for( std::size_t i = 0; i < n; ++i )
        std::copy( s + i*sizeof( T ), s + (i+1)*sizeof( T ), d + i*sizeof( T ) );
    }
  }

} // namespace ALUGrid

#endif // #ifndef DUNE_ALUGRID_IMPL_BYTEORDER_HH
