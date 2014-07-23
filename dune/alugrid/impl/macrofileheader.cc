#include <config.h>

#include <map>
#include <sstream>

#include <dune/alugrid/impl/byteorder.hh>
#include <dune/alugrid/impl/macrofileheader.hh>

namespace ALUGrid
{

  inline static bool fail ( bool verbose, const std::string &message )
  {
    if( verbose )
      std::cerr << "ERROR: " << message << std::endl;
    return false;
  }


  template< std::size_t n, class T >
  inline static bool parseOption ( const std::string &string, const char *(&strings)[ n ], T &value )
  {
    for( std::size_t i = 0; i < n; ++i )
    {
      if( string != strings[ i ] )
        continue;
      value = static_cast< T >( i );
      return true;
    }
    return false;
  }


  template< class T >
  inline static bool parseValue ( const std::string &string, T &value )
  {
    std::istringstream stream( string );

    // extract value
    stream >> value;
    if( !stream )
      return false;

    // make sure nothing else can be extracted
    char c;
    stream >> c;
    return !stream;
  }



  // Implementation of MacroFileHeader
  // ---------------------------------

  bool MacroFileHeader::setType ( const std::string &string )
  {
    return parseOption( string, stringType, type_ );
  }


  bool MacroFileHeader::setFormat ( const std::string &string )
  {
    return parseOption( string, stringFormat, format_ );
  }


  bool MacroFileHeader::setByteOrder ( const std::string &string )
  {
    return parseOption( string, stringByteOrder, byteOrder_ );
  }


  void MacroFileHeader::setSystemByteOrder ()
  {
    switch( ALUGrid::systemByteOrder() )
    {
    case ALUGrid::bigEndian:
      byteOrder_ = bigendian;
      break;

    case ALUGrid::littleEndian:
      byteOrder_ = littleendian;
      break;

    default:
      byteOrder_ = native;
      break;
    }
  }


  BinaryFormat MacroFileHeader::binaryFormat () const
  {
    switch( format() )
    {
    case binary:
      return rawBinary;

    case zbinary:
      return zlibCompressed;

    default:
      std::cerr << "ERROR: '" << toString( format() ) << "' is not a binary format." << std::endl;
      std::abort();
    }
  }


  bool MacroFileHeader::read ( const std::string &firstLine, bool verbose )
  {
    // check signature
    if( firstLine.substr( 0, 4 ) != "!ALU" )
      return fail( verbose, "ALUGrid signature (!ALU) not found." );

    // extract key/value pairs
    std::istringstream input( firstLine.substr( 4 ) );
    std::map< std::string, std::string > options;
    while( true )
    {
      std::string pair;
      input >> pair;
      if( !input )
        break;

      std::size_t pos = pair.find( '=' );
      if( pos == pair.npos )
        return fail( verbose, "Invalid key/value pair: '" + pair + "'." );

      const std::string key = pair.substr( 0, pos );
      const std::string value = pair.substr( pos+1 );
      if( !options.insert( std::make_pair( key, value ) ).second )
        return fail( verbose, "Duplicate key: '" + key + "'." );
    }

    MacroFileHeader header;

    std::map< std::string, std::string >::const_iterator pos = options.find( "version" );
    if( pos == options.end() )
      return fail( verbose, "Option 'version' missing." );
    if( !parseValue( pos->second, header.version_ ) )
      return fail( verbose, "Invalid 'version': '" + pos->second + "'." );
    if( header.version() > currentVersion )
      return fail( verbose, "File version too recent (" + pos->second + ")." );

    pos = options.find( "type" );
    if( pos == options.end() )
      return fail( verbose, "Option 'type' missing." );
    if( !header.setType( pos->second ) )
      return fail( verbose, "Invalid 'type': '" + pos->second + "'." );

    pos = options.find( "format" );
    if( pos == options.end() )
      return fail( verbose, "Option 'format' missing." );
    if( !header.setFormat( pos->second ) )
      return fail( verbose, "Invalid 'format': '" + pos->second + "'." );

    if( header.isBinary() )
    {
      pos = options.find( "byteorder" );
      if( (pos != options.end()) && !header.setByteOrder( pos->second ) )
        return fail( verbose, "Invalid 'byteorder': '" + pos->second + "'." );

      pos = options.find( "size" );
      if( pos == options.end() )
        return fail( verbose, "Option 'size' missing in binary format." );
      if( !parseValue( pos->second, header.size_ ) )
        return fail( verbose, "Invalid 'size': '" + pos->second + "'." );
    }

    *this = header;
    return true;
  }


  bool MacroFileHeader::read ( std::istream &in, bool verbose )
  {
    std::string firstLine;
    std::getline( in, firstLine );
    if( !in )
      fail( verbose, "Unable to extract first line." );
    return read( firstLine, verbose );
  }


  void MacroFileHeader::write ( std::ostream &out ) const
  {
    out << "!ALU";
    out << " version=" << version_;
    out << " type=" << toString( type() );
    out << " format=" << toString( format() );

    if( isBinary() )
    {
      out << " byteorder=" << toString( byteOrder() );
      out << " size=" << size();
    }

    out << std::endl;
  }


  const int MacroFileHeader::currentVersion;

  const char *MacroFileHeader::stringType[ 2 ] = { "tetrahedra", "hexahedra" };
  const char *MacroFileHeader::stringFormat[ 3 ] = { "ascii", "binary", "zbinary" };
  const char *MacroFileHeader::stringByteOrder[ 3 ] = { "native", "bigendian", "littleendian" };

} // namespace ALUGrid
