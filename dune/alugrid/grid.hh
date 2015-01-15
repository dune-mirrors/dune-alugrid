#ifndef DUNE_ALUGRID_HH
#define DUNE_ALUGRID_HH

// only include this code, if HAVE_ALUGRID is true
#if HAVE_ALUGRID
#ifndef DUNE_ALUGRID_HH_INCLUDED
#define DUNE_ALUGRID_HH_INCLUDED
#undef DUNE_ALUGRID_HH
#endif
#warning "Using old ALUGrid version from dune-grid"
#include <dune/grid/alugrid.hh>
#else

#include <dune/alugrid/common/declaration.hh>

#include <dune/alugrid/3d/alugrid.hh>
#include <dune/alugrid/3d/gridfactory.hh>

// 2d version
// will now be included in 3d
//#include <dune/alugrid/2d/alugrid.hh>
//#include <dune/alugrid/2d/gridfactory.hh>

#include <dune/alugrid/common/persistentcontainer.hh>
#include <dune/alugrid/common/backuprestore.hh>

#endif // else if HAVE_ALUGRID

#endif // #ifndef DUNE_ALUGRID_HH
