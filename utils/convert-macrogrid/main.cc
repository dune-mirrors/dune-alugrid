#include <config.h>

#include <time.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <utility>

#include <dune/common/version.hh>
#include <dune/common/fvector.hh>
#include <dune/common/parallel/mpihelper.hh>

#include <dune/grid/io/file/dgfparser/parser.hh>
#include <dune/alugrid/3d/topology.hh>

#include <dune/alugrid/impl/binaryio.hh>
#include <dune/alugrid/impl/byteorder.hh>
#include <dune/alugrid/impl/macrofileheader.hh>
#include <dune/alugrid/impl/serial/serialize.h>
#include <dune/alugrid/impl/serial/gitter_sti.h>
#include <dune/alugrid/impl/serial/gitter_mgb.h>


#include "partition.hh"
#include "dgfparser.hh"


// readLegacyFormat
// ----------------

template< ElementRawID rawId >
void readLegacyFormat ( std::istream &input,
                        std::vector< Vertex > &vertices,
                        std::vector< Element< rawId > > &elements,
                        std::vector< BndSeg< rawId > > &bndSegs,
                        std::vector< Periodic< rawId > > &periodics )
{
  // Das alte Format sieht im wesentlichen so aus:
  //
  // <Anzahl der Knoten : int >     /* 1.Zeile der Datei
  // <x-Koordinate : float>  <y-Koo. : float>  <z-Koo. : float>
  // ...            /* f"ur den letzten Knoten
  // <Anzahl der Elemente : int>
  // <KnotenNr. 0: int> ... <KnotenNr. 7: int>  /* f"ur das erste Hexaederelement
  // ...            /* f"ur das letzte Hexaederelement
  // <Anzahl der Randfl"achen : int>
  // <Randtyp>  4  <KnotenNr. 0> ... <KnotenNr. 3>/* erste Randfl"ache
  // ...            /* letzte Randfl"ache
  // <Identifier f"ur den 0. Knoten : int>  /* Identifierliste ist im seriellen
  // ...            /* Verfahren oder beim Aufsetzen aus
  // <Identifier f"ur den letzten Knoten : int> /* einem Gitter optional, sonst muss
  //            /* jeder Vertex eine eigene Nummer haben

  std::cout << "Reading legacy " << (rawId == HEXA_RAW ? "hexahedra" : "tetrahedra") << " format..." << std::endl;

  int nv = 0;
  input >> nv;
  vertices.resize( nv );
  for( int i = 0; i < nv; ++i )
    input >> vertices[ i ].x[ 0 ] >> vertices[ i ].x[ 1 ] >> vertices[ i ].x[ 2 ];
  std::cout << "  - read " << vertices.size() << " vertices." << std::endl;

  int ne = 0;
  input >> ne;
  elements.resize( ne );
  for( int i = 0; i < ne; ++i )
  {
    for( int vx = 0; vx < Element< rawId >::numVertices; ++vx )
      input >> elements[ i ].vertices[ vx ];
  }
  std::cout << "  - read " << elements.size() << " elements." << std::endl;

  int temp_nb = 0;
  input >> temp_nb;
  bndSegs.reserve( temp_nb );
  periodics.reserve( temp_nb );
  for( int i = 0; i < temp_nb; ++i )
  {
    int n, bndid;
    input >> bndid >> n;
    if( n == BndSeg< rawId >::numVertices )
    {
      BndSeg< rawId > seg;
      seg.bndid = -bndid;
      for( int vx = 0; vx < BndSeg< rawId >::numVertices; ++vx )
        input >> seg.vertices[ vx ];
      bndSegs.push_back( seg );
    }
    else if( n == Periodic< rawId >::numVertices )
    {
      Periodic< rawId > seg;
      seg.bndid = -bndid;
      for( int vx = 0; vx < Periodic< rawId >::numVertices; ++vx )
        input >> seg.vertices[ vx ];
      periodics.push_back( seg );
    }
    else
    {
      std::cerr << "ERROR (fatal): Invalid number of vertices for boundary object: " << n << "." << std::endl;
      std::exit( 1 );
    }
  }
  std::cout << "  - read " << bndSegs.size() << " boundary segments." << std::endl;
  std::cout << "  - read " << periodics.size() << " periodic boundary segments." << std::endl;

  if( !input )
  {
    std::cerr << "ERROR (fatal): Unexpected end of file." << std::endl;
    std::exit( 1 );
  }

  for( int i = 0; i < nv; ++i )
  {
    int dummy;
    input >> vertices[ i ].id >> dummy;
  }

  if( !input )
  {
    std::cerr << "WARNING (ignored) No parallel identification applied due to incomplete (or non-existent) identifier list." << std::endl;
    for( int i = 0; i < nv; ++ i )
      vertices[ i ].id = i;
  }
}



// readMacroGrid
// -------------

template< class stream_t, ElementRawID rawId >
void readMacroGrid ( stream_t &input,
                     std::vector< Vertex > &vertices,
                     std::vector< Element< rawId > > &elements,
                     std::vector< BndSeg< rawId > > &bndSegs,
                     std::vector< Periodic< rawId > > &periodics )
{
  std::cout << "Reading new " << (rawId == HEXA_RAW ? "hexahedra" : "tetrahedra") << " format..." << std::endl;

  int vertexListSize = 0;
  input >> vertexListSize;
  vertices.resize( vertexListSize );
  for( int i = 0; i < vertexListSize; ++i )
    input >> vertices[ i ].id >> vertices[ i ].x[ 0 ] >> vertices[ i ].x[ 1 ] >> vertices[ i ].x[ 2 ];
  std::cout << "  - read " << vertices.size() << " vertices." << std::endl;

  int elementListSize = 0;
  input >> elementListSize;
  elements.resize( elementListSize );
  for( int i = 0; i < elementListSize; ++i )
  {
    for( int j = 0; j < Element< rawId >::numVertices; ++j )
      input >> elements[ i ].vertices[ j ];
  }
  std::cout << "  - read " << elements.size() << " elements." << std::endl;

  int bndSegListSize = 0;
  int periodicListSize = 0;
  input >> periodicListSize >> bndSegListSize;
  periodics.resize( periodicListSize );
  for( int i = 0; i < periodicListSize; ++i )
  {
    for( int j = 0; j < Periodic< rawId >::numVertices; ++j )
      input >> periodics[ i ].vertices[ j ];
  }
  bndSegs.resize( bndSegListSize );
  for( int i = 0; i < bndSegListSize; ++i )
  {
    int bndid;
    input >> bndid ;
    int j = 0;
    if( bndid < 0 ) // exterior bnd
      bndSegs[ i ].bndid = -bndid ;
    else // interior bnd
    {
      bndSegs[ i ].vertices[ j++ ] = bndid ;
      bndSegs[ i ].bndid = closure ;
    }
    for( ; j < BndSeg< rawId >::numVertices; ++j )
      input >> bndSegs[ i ].vertices[ j ];
  }
  std::cout << "  - read " << bndSegs.size() << " boundary segments." << std::endl;
  std::cout << "  - read " << periodics.size() << " periodic boundary segments." << std::endl;
}



// writeMacroGrid
// --------------

template< class stream_t, ElementRawID rawId >
void writeMacroGrid ( stream_t &output,
                      const std::vector< Vertex > &vertices,
                      const std::vector< Element< rawId > > &elements,
                      const std::vector< BndSeg< rawId > > &bndSegs,
                      const std::vector< Periodic< rawId > > &periodics,
                      const int rank,
                      const int nPartition )
{
  const int elementListSize = elements.size();
  const int bndSegListSize = bndSegs.size();
  const int periodicListSize = periodics.size();

  const bool writeParallel = nPartition > 1 ;

  ALUGrid::StandardWhiteSpace_t ws;

  typedef std::set< int > partitionvx_t ;
  partitionvx_t partitionVertexIds;

  int partitionElements = elementListSize ;
  if( writeParallel )
  {
    // reset partitionElements for new counting
    partitionElements = 0;
    for( int i = 0; i < elementListSize; ++i )
    {
      // if element is not in current partition continue
      if( elements[ i ].rank != rank ) continue ;

      ++ partitionElements ;
      for( int j = 0; j < Element< rawId >::numVertices; ++j )
      {
        partitionVertexIds.insert( elements[ i ].vertices[ j ] );
      }
    }
  }

  const int vertexListSize = vertices.size();
  const int partitionVertexSize = writeParallel ? partitionVertexIds.size() : vertexListSize ;
  output << partitionVertexSize << std::endl;
  const typename std::set<int>::iterator pVxEnd = partitionVertexIds.end();
  for( int i = 0; i < vertexListSize; ++i )
  {
    // if vertex is not in partition list continue
    if( writeParallel && partitionVertexIds.find( vertices[ i ].id ) == pVxEnd ) continue ;
    // write vertex id and coordinates
    output << vertices[ i ].id << ws
           << vertices[ i ].x[ 0 ] << ws << vertices[ i ].x[ 1 ] << ws << vertices[ i ].x[ 2 ] << std::endl;
  }

  output << partitionElements << std::endl;
  for( int i = 0; i < elementListSize; ++i )
  {
    // if element is not in current partition continue
    if( elements[ i ].rank != rank ) continue ;

    output << elements[ i ].vertices[ 0 ];
    for( int j = 1; j < Element< rawId >::numVertices; ++j )
      output << ws << elements[ i ].vertices[ j ];
    output << std::endl;
  }

  int bndSegPartSize   = ( writeParallel ) ? 0 : bndSegListSize;
  int periodicPartSize = ( writeParallel ) ? 0 : periodicListSize;

  // count number of boundary segment for this partition
  if( writeParallel )
  {
    for( int i = 0; i < periodicListSize; ++i )
    {
      assert( periodics[ i ].element[ 0 ] == periodics[ i ].element[ 1 ] );
      if( elements[ periodics[ i ].element[ 0 ] ].rank == rank )
        ++ periodicPartSize ;
    }

    for( int i = 0; i < bndSegListSize; ++ i )
    {
      if( elements[ bndSegs[ i ].element ].rank == rank )
        ++ bndSegPartSize ;
    }
  }

  output << periodicPartSize << ws << bndSegPartSize << std::endl;
  for( int i = 0; i < periodicListSize; ++i )
  {
    if( writeParallel && elements[ periodics[ i ].element[ 0 ] ].rank != rank ) continue ;

    output << periodics[ i ].vertices[ 0 ];
    for( int j = 1; j < Periodic< rawId >::numVertices; ++j )
      output << ws << periodics[ i ].vertices[ j ];
    output << std::endl;
  }

  for( int i = 0; i < bndSegListSize; ++i )
  {
    if( writeParallel && elements[ bndSegs[ i ].element ].rank != rank ) continue ;

    if( bndSegs[ i ].bndid != closure )
      output << -bndSegs[ i ].bndid << ws;

    output << bndSegs[ i ].vertices[ 0 ];
    for( int j = 1; j < BndSeg< rawId >::numVertices; ++j )
      output << ws << bndSegs[ i ].vertices[ j ];
    output << std::endl;
  }

  if( writeParallel )
  {
    typedef std::vector< int > linkagePattern_t;
    typedef std::map< linkagePattern_t, int > linkagePatternMap_t;

    // create linkage patterns
    linkagePatternMap_t linkage ;

    typedef typename partitionvx_t :: iterator iterator ;
    const iterator end = partitionVertexIds.end();
    for( iterator it = partitionVertexIds.begin(); it != end; ++it )
    {
      std::vector< int > links ;
      vertices[ *it ].fillLinkage( links, rank );
      if( links.size() > 0 )
        linkage[ links ] = 1 ;
    }

    // write size of linkage patterns
    output << linkage.size() << std::endl;

    typedef linkagePatternMap_t :: iterator linkiterator ;
    const linkiterator linkend = linkage.end();
    int idx = 0;
    for( linkiterator it = linkage.begin(); it != linkend ; ++it, ++idx )
    {
      // store index of linkage entry in list
      (*it).second = idx ;
      const std::vector< int >& ranks = (*it).first ;
      const int rSize = ranks.size();
      output << rSize << ws ;
      for( int i=0; i<rSize; ++i )
        output <<  ranks[ i ] << ws ;
      output << std::endl;
    }

    const int hasElementLinkage = 1 ;
    output << hasElementLinkage << std::endl;

    idx = 0;
    for( iterator it = partitionVertexIds.begin(); it != end; ++it, ++idx )
    {
      std::vector< int > links ;
      const Vertex& vertex = vertices[ *it ];
      vertex.fillLinkage( links, rank );
      // if vertex has linkage we need to write the position
      if( links.size() > 0 )
      {
        output << idx << ws << linkage[ links ];
        if( hasElementLinkage )
        {
          const int elsize = vertex.elements.size();
          output << ws << elsize;
          typedef std::set<int> :: const_iterator iterator ;
          const iterator elend = vertex.elements.end();
          for( iterator it = vertex.elements.begin(); it != elend; ++it )
          {
            output << ws << (*it) ;
          }
        }
        output << std::endl;
      }
    }
    output << int(-1) << std::endl; // end marker for vertex position list

  }
  else
    output << int(0) << std::endl; // no linkage
}



// ProgramOptions
// --------------

struct ProgramOptions
{
  ElementRawID defaultRawId;
  std::string format;
  std::string byteOrder;
  int nPartition ;
  int partitionMethod ;

  ProgramOptions ()
    : defaultRawId( HEXA_RAW ), format( "ascii" ),
      byteOrder( "default" ), nPartition( 1 ), partitionMethod( 10 )
  {}
};

std::string rankFileName( const std::string& filename, const int rank, const int nPartition )
{
  if( nPartition <= 1 ) return filename ;

  std::stringstream str;
  str << filename << "." << rank;
  return str.str();
}



// writeBinaryFormat
// -----------------

template< ElementRawID rawId, class ObjectStream >
void writeBinaryFormat ( std::ostream &output, MacroFileHeader &header,
                         const std::vector< Vertex > &vertices,
                         const std::vector< Element< rawId > > &elements,
                         const std::vector< BndSeg< rawId > > &bndSegs,
                         const std::vector< Periodic< rawId > > &periodics,
                         const int rank,
                         const int nPartition )
{
  ObjectStream os;
  writeMacroGrid( os, vertices, elements, bndSegs, periodics, rank, nPartition );
  ALUGrid::writeHeaderAndBinary( output, os, header );
}



// writeNewFormat
// --------------

template< ElementRawID rawId >
void writeNewFormat ( const std::string& filename,
                      const ProgramOptions &options,
                      std::vector< Vertex > &vertices,
                      std::vector< Element< rawId > > &elements,
                      std::vector< BndSeg< rawId > > &bndSegs,
                      std::vector< Periodic< rawId > > &periodics )
{
  // partition might change the order of elements due to space filling curve ordering
  partition( vertices, elements, bndSegs, periodics, options.nPartition, options.partitionMethod );

  // compute vertex linkage
  computeLinkage( vertices, elements );

  for( int rank=0; rank<options.nPartition; ++ rank )
  {
    std::ofstream output( rankFileName( filename, rank, options.nPartition ) );
    if( !output )
    {
      std::cerr << "Unable to open output file: " << filename << "." << std::endl;
      std::abort();
    }
    MacroFileHeader header;
    header.setType( rawId == HEXA_RAW ? MacroFileHeader::hexahedra : MacroFileHeader::tetrahedra );
    header.setFormat( options.format );

    if( options.byteOrder != "default" )
      header.setByteOrder( options.byteOrder );
    else
      header.setSystemByteOrder();

    if( header.isBinary() )
    {
      switch( header.byteOrder() )
      {
      case MacroFileHeader::native:
        writeBinaryFormat< rawId, ALUGrid::ObjectStream >( output, header, vertices, elements, bndSegs, periodics, rank, options.nPartition );
        break ;

      case MacroFileHeader::bigendian:
        writeBinaryFormat< rawId, ALUGrid::BigEndianObjectStream >( output, header, vertices, elements, bndSegs, periodics, rank, options.nPartition );
        break ;

      case MacroFileHeader::littleendian:
        writeBinaryFormat< rawId, ALUGrid::LittleEndianObjectStream >( output, header, vertices, elements, bndSegs, periodics, rank, options.nPartition );
        break ;
      }
    }
    else
    {
      header.write( output );
      output << std::scientific << std::setprecision( 16 );
      writeMacroGrid( output, vertices, elements, bndSegs, periodics, rank, options.nPartition );
    }
  }
}



// convertLegacyFormat
// -------------------

template< ElementRawID rawId >
void convertLegacyFormat ( std::istream &input, const std::string& filename, const ProgramOptions &options )
{
  std::vector< Vertex > vertices;
  std::vector< Element< rawId > > elements;
  std::vector< BndSeg< rawId > > bndSegs;
  std::vector< Periodic< rawId > > periodics;

  readLegacyFormat( input, vertices, elements, bndSegs, periodics );
  writeNewFormat( filename, options, vertices, elements, bndSegs, periodics );
}



// readBinaryMacroGrid
// -------------------

template< ElementRawID rawId, class ObjectStream >
void readBinaryMacroGrid ( std::istream &input, const MacroFileHeader &header,
                           std::vector< Vertex > &vertices,
                           std::vector< Element< rawId > > &elements,
                           std::vector< BndSeg< rawId > > &bndSegs,
                           std::vector< Periodic< rawId > > &periodics )
{
  // read file to alugrid stream
  ObjectStream os;
  ALUGrid::readBinary( input, os, header );
  readMacroGrid( os, vertices, elements, bndSegs, periodics );
}

template< ElementRawID rawId >
void readBinaryMacroGrid ( std::istream &input, const MacroFileHeader &header,
                           std::vector< Vertex > &vertices,
                           std::vector< Element< rawId > > &elements,
                           std::vector< BndSeg< rawId > > &bndSegs,
                           std::vector< Periodic< rawId > > &periodics )
{
  switch( header.byteOrder() )
  {
  case MacroFileHeader::native:
    return readBinaryMacroGrid< rawId, ALUGrid::ObjectStream >( input, header, vertices, elements, bndSegs, periodics );

  case MacroFileHeader::bigendian:
    return readBinaryMacroGrid< rawId, ALUGrid::BigEndianObjectStream >( input, header, vertices, elements, bndSegs, periodics );

  case MacroFileHeader::littleendian:
    return readBinaryMacroGrid< rawId, ALUGrid::LittleEndianObjectStream >( input, header, vertices, elements, bndSegs, periodics );
  }
}



// convertNewFormat
// ----------------

template< ElementRawID rawId >
void convertNewFormat ( std::istream &input, const std::string& filename, const ProgramOptions &options, const MacroFileHeader &header )
{
  std::vector< Vertex > vertices;
  std::vector< Element< rawId > > elements;
  std::vector< BndSeg< rawId > > bndSegs;
  std::vector< Periodic< rawId > > periodics;

  if( header.isBinary() )
    readBinaryMacroGrid( input, header, vertices, elements, bndSegs, periodics );
  else
    readMacroGrid( input, vertices, elements, bndSegs, periodics );
  writeNewFormat( filename, options, vertices, elements, bndSegs, periodics );
}

void convertNewFormat ( std::istream &input, const std::string& filename, const ProgramOptions &options, const MacroFileHeader &header )
{
  switch( header.type() )
  {
  case MacroFileHeader::tetrahedra:
    return convertNewFormat< TETRA_RAW >( input, filename, options, header );

  case MacroFileHeader::hexahedra:
    return convertNewFormat< HEXA_RAW > ( input, filename, options, header );
  }
}



// readDGF
// -------

template< class stream_t, ElementRawID rawId >
void readDGF ( stream_t &input,
               std::vector< Vertex > &vertices,
               std::vector< Element< rawId > > &elements,
               std::vector< BndSeg< rawId > > &bndSegs,
               std::vector< Periodic< rawId > > &periodics )
{
  DGFParser dgf( rawId == HEXA_RAW ? Dune::cube : Dune::simplex );

  if( !dgf.readDuneGrid( input, 3, 3 ) )
  {
    std::cerr << "ERROR: Invalid DGF file." << std::endl;
    std::exit( 1 );
  }

  if( rawId == TETRA_RAW )
    dgf.setOrientation( 2, 3 );

  vertices.resize( dgf.numVertices() );
  for( int i = 0; i < dgf.numVertices(); ++i )
  {
    vertices[ i ].id = i;
    for( int j=0; j<3; ++j )
      vertices[ i ].x[ j ] = dgf.vertex( i )[ j ];
  }

  typedef Dune::ElementTopologyMapping< rawId == HEXA_RAW ? Dune::hexa : Dune::tetra > DuneTopologyMapping;
  elements.resize( dgf.numElements() );
  bndSegs.reserve( dgf.facemap().size() );
  for( int i = 0; i < dgf.numElements(); ++i )
  {
    if( int( dgf.element( i ).size() ) != Element< rawId >::numVertices )
    {
      std::cerr << "ERROR: Invalid element constructed by DGF parser (" << dgf.element( i ).size() << " vertices)." << std::endl;
      std::exit( 1 );
    }
    for( int j = 0; j < Element< rawId >::numVertices; ++j )
      elements[ i ].vertices[ DuneTopologyMapping::dune2aluVertex( j ) ] = dgf.element( i )[ j ];
    for( int j = 0; j < Dune::ElementFaceUtil::nofFaces( 3, dgf.element( i ) ); ++j )
    {
      const DGFParser::facemap_t::const_iterator pos = dgf.facemap().find( Dune::ElementFaceUtil::generateFace( 3, dgf.element( i ), j ) );
      if( pos != dgf.facemap().end() )
      {
        const int jalu = DuneTopologyMapping::generic2aluFace( j );
        BndSeg< rawId > bndSeg;
        bndSeg.bndid = pos->second.first;
        for( int k = 0; k < BndSeg< rawId >::numVertices; ++k )
          bndSeg.vertices[ k ] = elements[ i ].vertices[ DuneTopologyMapping::faceVertex( jalu, k ) ];
        bndSegs.push_back( bndSeg );
      }
    }
  }
}



// convertDGF
// ----------
template< ElementRawID rawId >
void convertDGF ( std::istream &input, const std::string& filename, const ProgramOptions &options )
{
  std::vector< Vertex > vertices;
  std::vector< Element< rawId > > elements;
  std::vector< BndSeg< rawId > > bndSegs;
  std::vector< Periodic< rawId > > periodics;

  readDGF( input, vertices, elements, bndSegs, periodics );
  writeNewFormat( filename, options, vertices, elements, bndSegs, periodics );
}

void convertDGF ( std::istream &input, const std::string& filename, const ProgramOptions &options )
{
  const clock_t start = clock();

  if( options.defaultRawId == HEXA_RAW )
    convertDGF< HEXA_RAW > ( input, filename, options );
  else
    convertDGF< TETRA_RAW >( input, filename, options );

  std::cout << "INFO: Conversion of DUNE grid format used " << (double( clock () - start ) / double( CLOCKS_PER_SEC )) << " s." << std::endl;
}



// convert
// -------

void convert ( std::istream &input, const std::string& filename, const ProgramOptions &options )
{
  const clock_t start = clock();

  std::string firstline;
  std::getline( input, firstline );
  if( firstline[ 0 ] == char( '!' ) )
  {
    if( firstline.substr( 1, 3 ) == "ALU" )
      convertNewFormat( input, filename, options, MacroFileHeader( firstline ) );
    else if( (firstline.find( "Tetrahedra" ) != firstline.npos) || (firstline.find( "Tetraeder" ) != firstline.npos) )
      convertLegacyFormat< TETRA_RAW >( input, filename, options );
    else if( (firstline.find( "Hexahedra" ) != firstline.npos) || (firstline.find( "Hexaeder" ) != firstline.npos) )
      convertLegacyFormat< HEXA_RAW >( input, filename, options );
    else
    {
      std::cerr << "ERROR: Unknown comment to file format (" << firstline << ")." << std::endl;
      std::exit( 1 );
    }
  }
  else
  {
    std::cerr << "WARNING: No identifier for file format found. Trying to read as legacy "
              << (options.defaultRawId == HEXA_RAW ? "hexahedral" : "tetrahedral") << " grid." << std::endl;
    if( options.defaultRawId == HEXA_RAW )
      convertLegacyFormat< HEXA_RAW >( input, filename, options );
    else
      convertLegacyFormat< TETRA_RAW >( input, filename, options );
  }

  std::cout << "INFO: Conversion of macro grid format used " << (double( clock () - start ) / double( CLOCKS_PER_SEC )) << " s." << std::endl;
}



// main
// ----

int main ( int argc, char **argv )
{
  Dune::MPIHelper::instance( argc, argv );

  ProgramOptions options;

  for( int i = 1; i < argc; ++i )
  {
    if( argv[ i ][ 0 ] != '-' )
      continue;

    for( int j = 1; argv[ i ][ j ]; ++j )
    {
      switch( argv[ i ][ j ] )
      {
      case 'b':
        options.format = "binary";
        break;

      case '4':
        options.defaultRawId = TETRA_RAW;
        break;

      case '8':
        options.defaultRawId = HEXA_RAW;
        break;

      case 'f':
        if( i+1 >= argc )
        {
          std::cerr << "Missing argument to option -f." << std::endl;
          return 1;
        }
        options.format = argv[ i+1 ];
        std::copy( argv + (i+2), argv + argc, argv + i+1 );
        --argc;
        if( (options.format != "ascii") && (options.format != "binary") && (options.format != "zbinary") )
        {
          std::cerr << "Invalid format: " << options.format << "." << std::endl;
          return 1;
        }
        break;

      case 'p':
        if( i+1 >= argc )
        {
          std::cerr << "Missing argument to option -p." << std::endl;
          return 1;
        }
        options.nPartition = atoi( argv[ i+1 ] );
        std::copy( argv + (i+2), argv + argc, argv + i+1 );
        --argc;
        break ;

      case 'm':
        if( i+1 >= argc )
        {
          std::cerr << "Missing argument to option -m." << std::endl;
          return 1;
        }
        options.partitionMethod = atoi( argv[ i+1 ] );
        std::copy( argv + (i+2), argv + argc, argv + i+1 );
        --argc;
        break ;

      case 'o':
        if( i+1 >= argc )
        {
          std::cerr << "Missing argument to option -o." << std::endl;
          return 1;
        }
        options.byteOrder = argv[ i+1 ];
        std::copy( argv + (i+2), argv + argc, argv + i+1 );
        --argc;
        if( (options.byteOrder != "native") && (options.byteOrder != "bigendian") && (options.byteOrder != "littleendian") )
        {
          std::cerr << "Invalid byte order: " << options.byteOrder << "." << std::endl;
          return 1;
        }
        break;
      }
    }

    std::copy( argv + (i+1), argv + argc, argv + i );
    --i; --argc;
  }

  if( argc <= 2 )
  {
    typedef ALUGrid::LoadBalancer::DataBase DataBase;
    const std::string mth[ 3 ] = { DataBase::methodToString( DataBase::ALUGRID_SpaceFillingCurveSerial ),
                                   DataBase::methodToString( DataBase::METIS_PartGraphKway ),
                                   DataBase::methodToString( DataBase::METIS_PartGraphRecursive ) };

    std::cerr << "Usage: " << argv[ 0 ] << " [-b] [-4|-8] [-o <byteorder>] <input> <output>" << std::endl;
    std::cerr << "Flags: -4 : read tetrahedral grid, if not determined by input file" << std::endl;
    std::cerr << "       -8 : read hexahedral grid, if not determined by input file" << std::endl;
    std::cerr << "       -b : write binary output (alias for -f binary)" << std::endl;
    std::cerr << "       -f : select output format (one of 'ascii', 'binary', 'zbinary')" << std::endl;
    std::cerr << "       -p : number of partitions (default is 1)" << std::endl;
    std::cerr << "       -m : partitioning method (default is " << mth[ 0 ] << ")," << std::endl;
    std::cerr << "            also valid are " << mth[ 1 ] << " and " << mth[ 2 ] << std::endl;
    std::cerr << "       -o : select output byte order (one of 'native', 'bigendian', 'littleendian')" << std::endl;
    return 1;
  }

  std::ifstream input( argv[ 1 ] );
  if( !input )
  {
    std::cerr << "Unable to open input file: " << argv[ 1 ] << "." << std::endl;
    return 1;
  }

  std::string filename( argv[ 2 ] );
  if( DGFParser::isDuneGridFormat( input ) )
    convertDGF( input, filename, options );
  else
    convert( input, filename, options );
}
