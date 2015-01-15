#ifndef ALUGRID_DGFPARSER_HH_INCLUDED
#define ALUGRID_DGFPARSER_HH_INCLUDED

#include <dune/grid/io/file/dgfparser/parser.hh>
#include <dune/alugrid/common/alugrid_assert.hh>
#include <dune/alugrid/common/declaration.hh>

// DGFParser
// ---------

class DGFParser
  : public Dune::DuneGridFormatParser
{
  typedef Dune::DuneGridFormatParser Base;

public:
  typedef Base::facemap_t facemap_t;

  DGFParser ( const Dune::ALUGridElementType type,
              const int dimgrid = 3,
              const int dimworld = 3 )
    : Base( 0, 1 )
  {
    alugrid_assert( type == Dune::cube || type == Dune::simplex );
    Base::element = (type == Dune::cube) ? DGFParser::Cube : DGFParser::Simplex;
    Base::dimgrid = dimgrid;
    Base::dimw = dimworld;
  }

  static bool isDuneGridFormat ( std::istream &input )
  {
    const std::streampos pos = input.tellg();
    const bool isDGF = Base::isDuneGridFormat( input );
    input.clear();
    input.seekg( pos );
    return isDGF;
  }

  void setOrientation ( int use1, int use2 ) { Base::setOrientation( use1, use2 ); }

  int numVertices () const { return Base::nofvtx; }
  const std::vector< double > &vertex ( int i ) const { return Base::vtx[ i ]; }

  int numElements () const { return this->nofelements; }
  const std::vector< unsigned int > &element ( int i ) const { return Base::elements[ i ]; }

  const facemap_t &facemap () const { return Base::facemap; }

  bool isCubeGrid() const { return Base::element == DGFParser::Cube; }
};

#endif
