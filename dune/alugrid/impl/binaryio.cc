#include <config.h>

#include <cstdlib>

#include <dune/alugrid/impl/binaryio.hh>

#if HAVE_ZLIB
#include <zlib.h>
#endif // #if HAVE_ZLIB

namespace ALUGrid
{

  // readBinary
  // ----------

  void readBinary ( std::istream &stream, void *data, uint64_t size, BinaryFormat format )
  {
    if( format == rawBinary )
      stream.read( static_cast< char * >( data ), size );
    else if( format == zlibCompressed )
    {
#if HAVE_ZLIB
      // initialize zlib inflate algorithm
      z_stream zinfo;
      zinfo.zalloc = Z_NULL;
      zinfo.zfree = Z_NULL;
      zinfo.opaque = Z_NULL;
      zinfo.avail_in = 0;
      zinfo.next_in = Z_NULL;
      if( inflateInit( &zinfo ) != Z_OK )
      {
        std::cerr << "ERROR: Unable to initialize zlib inflate algorithm." << std::endl;
        stream.setstate( std::ios_base::failbit );
        return;
      }

      // set output data
      zinfo.avail_out = size;
      zinfo.next_out = static_cast< uint8_t * >( data );

      // inflate data from istream
      const std::size_t bufferSize = 1 << 18;
      void *bufferAddress = std::malloc( bufferSize );
      for( int status = Z_OK; status != Z_STREAM_END; )
      {
        zinfo.next_in = static_cast< Bytef * >( bufferAddress );
        zinfo.avail_in = stream.readsome( static_cast< char * >( bufferAddress ), bufferSize );
        if( !stream )
          break;
        status = inflate( &zinfo, Z_NO_FLUSH );
        if( (status != Z_OK) && (status != Z_STREAM_END) )
        {
          std::cerr << "ERROR: Error reading zlib compressed binary data (" << zError( status ) << ")." << std::endl;
          stream.setstate( std::ios_base::failbit );
          break;
        }
      }


      // return additional bytes in buffer by seeking
      if( stream )
      {
        std::streampos pos = stream.tellg();
        pos -= zinfo.avail_in ;
        stream.seekg( pos );
      }

      // finalize inflate algorithm
      inflateEnd( &zinfo );
      std::free( bufferAddress );
#else // #if HAVE_ZLIB
      std::cerr << "ERROR: zlib compression requires zlib." << std::endl;
      stream.setstate( std::ios_base::failbit );
#endif // #else // #if HAVE_ZLIB
    }
    else
    {
      std::cerr << "ERROR: Invalid binary format." << std::endl;
      stream.setstate( std::ios_base::failbit );
    }
  }



  // writeBinary
  // -----------

  void writeBinary ( std::ostream &stream, const void *data, uint64_t size, BinaryFormat format )
  {
    if( format == rawBinary )
      stream.write( static_cast< const char * >( data ), size );
    else if( format == zlibCompressed )
    {
#if HAVE_ZLIB
      // initialize zlib deflate algorithm
      z_stream zinfo;
      zinfo.zalloc = Z_NULL;
      zinfo.zfree = Z_NULL;
      zinfo.opaque = Z_NULL;
      if( deflateInit( &zinfo, Z_DEFAULT_COMPRESSION ) != Z_OK )
      {
        std::cerr << "ERROR: Unable to initialize zlib deflate algorithm." << std::endl;
        stream.setstate( std::ios_base::failbit );
        return;
      }

      // set input data
      zinfo.avail_in = size;
      zinfo.next_in = static_cast< Bytef * >( const_cast< void * >( data ) );

      // deflate data to ostream
      const std::size_t bufferSize = 1 << 18;
      void *bufferAddress = std::malloc( bufferSize );
      for( int status = Z_OK; status != Z_STREAM_END; )
      {
        zinfo.avail_out = bufferSize;
        zinfo.next_out = static_cast< Bytef * >( bufferAddress );
        status = deflate( &zinfo, Z_FINISH );
        if( (status != Z_OK) && (status != Z_STREAM_END) )
        {
          std::cerr << "ERROR: Error writing zlib compressed binary data (" << zError( status ) << ")." << std::endl;
          stream.setstate( std::ios_base::failbit );
          break;
        }
        if( !stream.write( static_cast< const char * >( bufferAddress ), bufferSize - zinfo.avail_out ) )
          break;
      }

      // finalize deflate algorithm
      deflateEnd( &zinfo );
      std::free( bufferAddress );
#else // #if HAVE_ZLIB
      std::cerr << "ERROR: zlib compression requires zlib." << std::endl;
      stream.setstate( std::ios_base::failbit );
#endif // #else // #if HAVE_ZLIB
    }
    else
    {
      std::cerr << "ERROR: Invalid binary format." << std::endl;
      stream.setstate( std::ios_base::failbit );
    }
  }

} // namespace ALUGrid
