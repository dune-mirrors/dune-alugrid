#ifndef DUNE_ALUGRID_IMPL_BINARYIO_HH
#define DUNE_ALUGRID_IMPL_BINARYIO_HH

#include <cstddef>
#include <iostream>
#include <cstdint>

#include <dune/alugrid/impl/serial/serialize.h>

namespace ALUGrid
{

  enum BinaryFormat { rawBinary, zlibCompressed };

  void readBinary ( std::istream &stream,  void *data, uint64_t size, BinaryFormat format );
  void writeBinary ( std::ostream &stream, const void *data, uint64_t size, BinaryFormat format );

  template <class Traits>
  inline void readBinary ( std::istream &stream, BasicObjectStream< Traits >& data )
  {
    // read size of object stream
    uint64_t size = data.size();
    stream.read( (char *) &size, sizeof(uint64_t) );

    // reserve memory
    data.reserve( size );
    data.clear();

    // read binary data from zlib
    readBinary( stream, data.raw(), size, zlibCompressed );

    // set wb counter in ObjectStream
    data.seekp( size );
  }

  template <class Traits>
  inline void writeBinary ( std::ostream &stream, const BasicObjectStream< Traits >& data )
  {
    uint64_t size = data.size();
    stream.write( (char *) &size, sizeof( uint64_t ) );

    writeBinary( stream, data.raw(), size, zlibCompressed );
  }

  template <class Traits, class MacroFileHeader>
  inline void readBinary ( std::istream &stream,  BasicObjectStream< Traits >& data, const MacroFileHeader& header )
  {
    // reserve memory, size is determined by header
    data.reserve( header.size() );
    data.clear();

    // read binary data from zlib
    readBinary( stream, data.raw(), header.size(), header.binaryFormat() );

    if( !stream )
    {
      std::cerr << "ERROR (fatal): Unable to read binary input." << std::endl;
      std::abort();
    }

    // set wb counter in ObjectStream
    data.seekp( header.size() );
  }

  template <class Traits, class MacroFileHeader>
  inline void writeHeaderAndBinary ( std::ostream &stream, const BasicObjectStream< Traits >& data, MacroFileHeader& header )
  {
    // get data size and store in header
    header.setSize( data.size() );
    // write header and then data
    header.write( stream );

    // write binary data
    writeBinary( stream, data.raw(), header.size(), header.binaryFormat() );

    if( !stream )
    {
      std::cerr << "ERROR: Unable to write binary output." << std::endl;
      std::abort();
    }
  }

} // namespace ALUGrid

#endif // #ifndef DUNE_ALUGRID_IMPL_BINARYIO_HH
