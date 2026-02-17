#ifndef DUNE_ALUGRID_HH
#define DUNE_ALUGRID_HH

#pragma GCC diagnostic push
#if defined(__GNUC__) && __GNUC__ >= 13
#pragma GCC diagnostic ignored "-Wdangling-reference"
#endif // #if defined(__GNUC__)

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

#include <dune/alugrid/dgf.hh>
#include <dune/alugrid/common/structuredgridfactory.hh>
#include <dune/alugrid/common/persistentcontainer.hh>
#include <dune/alugrid/common/backuprestore.hh>

#endif // else if HAVE_ALUGRID

#pragma GCC diagnostic pop
#endif // #ifndef DUNE_ALUGRID_HH
