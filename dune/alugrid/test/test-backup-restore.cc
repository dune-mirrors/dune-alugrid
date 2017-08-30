#include <config.h>

#include <cstddef>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <dune/common/parallel/mpihelper.hh>

#include <dune/grid/common/exceptions.hh>

#include <dune/alugrid/dgf.hh>
#include <dune/alugrid/grid.hh>


// dgfUnitCube
// -----------

inline static std::string dgfUnitCube ( int dimWorld, int cells )
{
  std::string dgf = "DGF\nINTERVAL\n";
  for( int i = 0; i < dimWorld; ++i )
    dgf += " 0";
  dgf += "\n";
  for( int i = 0; i < dimWorld; ++i )
    dgf += " 1";
  dgf += "\n";
  for( int i = 0; i < dimWorld; ++i )
    dgf += (" " + std::to_string( cells ));
  dgf += "\n#\n";
  return dgf;
}



// backup
// ------

template< class Grid >
std::pair< std::string, std::size_t > backup ()
{
  std::istringstream input( dgfUnitCube( Grid::dimensionworld, 2 ) );
  Dune::GridPtr< Grid > grid( input );

  grid->loadBalance();
  grid->globalRefine( 4 );

  std::ostringstream output;
  Dune::BackupRestoreFacility< Grid >::backup( *grid, output );
  return std::make_pair( output.str(), grid->leafGridView().size( 0 ) );
}



// restore
// -------

template< class Grid >
std::size_t restore ( const std::string &checkpoint )
{
  std::istringstream input( checkpoint );
  std::unique_ptr< Grid > grid( Dune::BackupRestoreFacility< Grid >::restore( input ) );
  return grid->leafGridView().size( 0 );
}



// check
// -----

template< class Grid >
void check ()
{
  auto backupData = backup< Grid >();
  auto restoreData = restore< Grid >( backupData.first );
  if( restoreData != backupData.second )
    DUNE_THROW( Dune::GridError, "Grid has been restored incorrectly" );
}



// main
// ----

int main ( int argc, char *argv[] )
try
{
  Dune::MPIHelper::instance( argc, argv );

  check< Dune::ALUGrid< 2, 2, Dune::cube, Dune::nonconforming > >();
  check< Dune::ALUGrid< 2, 2, Dune::simplex, Dune::nonconforming > >();
  check< Dune::ALUGrid< 2, 2, Dune::simplex, Dune::conforming > >();

  check< Dune::ALUGrid< 3, 3, Dune::cube, Dune::nonconforming > >();
  check< Dune::ALUGrid< 3, 3, Dune::simplex, Dune::nonconforming > >();
  check< Dune::ALUGrid< 3, 3, Dune::simplex, Dune::conforming > >();

  return 0;
}
catch( Dune::Exception &e )
{
  std::cerr << e << std::endl;
  return 1;
}
