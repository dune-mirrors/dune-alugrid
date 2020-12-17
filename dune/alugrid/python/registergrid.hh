#ifndef DUNE_ALUGRID_PYTHON_REGISTERGRID_HH
#define DUNE_ALUGRID_PYTHON_REGISTERGRID_HH


#if USING_DUNE_PYTHON
#include <dune/common/visibility.hh>

#include <dune/alugrid/grid.hh>

#include <dune/python/grid/hierarchical.hh>
#include <dune/python/common/typeregistry.hh>
#include <dune/python/pybind11/pybind11.h>

namespace Dune
{
  namespace Python
  {

    template <int dim, int dimworld, ALUGridElementType elType, ALUGridRefinementType refineType>
    inline void registerAluGrid( pybind11::module module )
    {
      using pybind11::operator""_a;
      pybind11::module cls0 = module;
      {
        std::string elTypeStr = (elType == Dune::cube) ? "Dune::cube" : "Dune::simplex";
        std::string refineTypeStr = (refineType == Dune::nonconforming) ? "Dune::nonconforming" : "Dune::conforming";
        using DuneType = Dune::ALUGrid< dim, dimworld, elType, refineType >;
        std::string typeName = "Dune::ALUGrid< " + std::to_string(dim) + ", " + std::to_string(dimworld) + ", " +
          elTypeStr + ", " + refineTypeStr + " >";
        auto cls = Dune::Python::insertClass< DuneType >( cls0, "HierarchicalGrid",
                  Dune::Python::GenerateTypeName( typeName.c_str() ),
                  Dune::Python::IncludeFiles{"dune/alugrid/dgf.hh","dune/alugrid/grid.hh","dune/python/grid/hierarchical.hh"}).first;
        Dune::Python::registerHierarchicalGrid( cls0, cls );
      }

    }

  } // end namespace Python

} // end namespace Dune

#endif // USING_DUNE_PYTHON
#endif
