#ifndef ALUGRID_INSERTGRID_HH
#define ALUGRID_INSERTGRID_HH

// include serial part of ALUGrid
#include <dune/alugrid/common/declaration.hh>
#include <dune/alugrid/common/alugrid_assert.hh>
#include <dune/alugrid/3d/alu3dinclude.hh>

#include <dune/grid/io/file/dgfparser/parser.hh>

// DGFParser
// ---------
#include "../../../utils/convert-macrogrid/dgfparser.hh"


void insertGrid( DGFParser& dgf, ALUGrid::GitterDuneImpl* grid )
{
  double extraPoint[ 3] = {3,3,3};

  ALUGrid::Gitter::Geometric::BuilderIF* builder =
   dynamic_cast< ALUGrid::Gitter::Geometric::BuilderIF* >( &grid->container() );

  ALUGrid :: MacroGridBuilder mgb ( *builder, (ALUGrid::ProjectVertex *) 0);
  if( !dgf.isCubeGrid() )
    mgb.InsertUniqueVertex( extraPoint[0], extraPoint[1], extraPoint[2], 0 );

  const int nVx = dgf.numVertices();
  for( int i=0; i<nVx; ++i )
  {
    if( !dgf.isCubeGrid() )
      mgb.InsertUniqueVertex( dgf.vertex( i )[0], dgf.vertex( i )[1], 0, i+1 );
    else
    {
      mgb.InsertUniqueVertex( dgf.vertex( i )[0], dgf.vertex( i )[1], 0, 2*i );
      mgb.InsertUniqueVertex( dgf.vertex( i )[0], dgf.vertex( i )[1], 1, 2*i+1);
    }
  }

  const size_t elemSize = dgf.numElements();
  for( size_t el = 0; el<elemSize; ++el )
  {
    if( dgf.isCubeGrid() )
    {
      //typedef Dune::ElementTopologyMapping< Dune::hexa > ElementTopologyMappingType;
      int element[ 8 ];
      //  const unsigned int j = ElementTopologyMappingType::dune2aluVertex( i );
        element[ 0 ] = 2*dgf.element( el )[ 0 ];
        element[ 1 ] = 2*dgf.element( el )[ 1 ];
        element[ 2 ] = 2*dgf.element( el )[ 3 ];
        element[ 3 ] = 2*dgf.element( el )[ 2 ];
        element[ 4 ] = 2*dgf.element( el )[ 0 ]+1;
        element[ 5 ] = 2*dgf.element( el )[ 1 ]+1;
        element[ 6 ] = 2*dgf.element( el )[ 3 ]+1;
        element[ 7 ] = 2*dgf.element( el )[ 2 ]+1;

      mgb.InsertUniqueHexa( element );
    }
    else
    {
     // typedef Dune::ElementTopologyMapping< Dune::tetra > ElementTopologyMappingType;
      int element[ 4 ];
      //element[ ElementTopologyMappingType::dune2aluVertex( 0 ) ] = 0; // the fake vertex
      //const unsigned int j = ElementTopologyMappingType::dune2aluVertex( i );
      element[ 0 ] = 0;
      element[ 1 ] = dgf.element( el )[ 0 ]+1;
      element[ 3 ] = dgf.element( el )[ 1 ]+1;
      element[ 2 ] = dgf.element( el )[ 2 ]+1;

      int dimension = grid->dimension();
      ALU3DSPACE SimplexTypeFlag simplexTypeFlag( int(dimension == 3 ? (el % 2) : 0), 0 );
      mgb.InsertUniqueTetra( element, simplexTypeFlag );
    }
  }

  // TODO insert boundary faces
}

#endif
