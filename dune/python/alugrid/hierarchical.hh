// SPDX-FileCopyrightText: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_PYTHON_ALUGRID_HIERARCHICAL_HH
#define DUNE_PYTHON_ALUGRID_HIERARCHICAL_HH

#include <type_traits>
#include <dune/python/pybind11/pybind11.h>
#include <dune/python/grid/capabilities.hh>
#include <dune/python/grid/factory.hh>
#include <dune/alugrid/grid.hh>

namespace Dune
{
  namespace Python
  {
    template< class Grid >
    inline static auto reader ( const pybind11::dict &args, PriorityTag< 40 > ) // is called in dune-grid/dune/python/grid/hierarchical.hh with > PriorityTag<42>
      -> std::enable_if_t< Capabilities::HasGridFactory< Grid >::value, std::shared_ptr< Grid > >
    {
      unsigned int defaultBndId = 1;
      if( args.contains( "defaultBndId" ) )
      {
        defaultBndId = args["defaultBndId"].template cast<unsigned int>();
      }
      GridFactory< Grid > factory;
      fillGridFactory( args, factory );
      return std::shared_ptr< Grid >( factory.createGrid(defaultBndId) );
    }
  } // namespace Python
} // namespace Dune

#endif
