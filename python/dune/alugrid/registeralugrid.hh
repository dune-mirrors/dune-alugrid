#pragma once

#include <string>
#include <sstream>
#include <dune/common/tupleutility.hh>

#include <dune/alugrid/dgf.hh>
#include <dune/alugrid/grid.hh>
#include <dune/python/grid/hierarchical.hh>

template <int dim, int dimworld, Dune::ALUGridElementType eltype>
void registerALUGrid(pybind11::module module)
#ifdef INCLUDE_REGALUGRID_INLINE
{
  // add commonly used ALUGrid variants
  using pybind11::operator""_a;
  std::string eltypestr = std::string(eltype == Dune::simplex ? "simplex" : "cube");
  std::string modname = std::string("_alugrid_" + std::to_string(dim) + std::to_string(dimworld) + "_" + eltypestr);
  std::string descr("Precompiled ");
  descr += modname;
  pybind11::module cls0 = module.def_submodule( modname.c_str(), descr.c_str());
  {
    using DuneType = Dune::ALUGrid< dim, dimworld, eltype >;
    std::string gridTypeName;
    {
      std::stringstream gridStr;
      gridStr << "Dune::ALUGrid< " << dim << ", " << dimworld << " , Dune::" << eltypestr << " >";
      gridTypeName = gridStr.str();
    }

    auto cls = Dune::Python::insertClass< DuneType, std::shared_ptr<DuneType> >( cls0, "HierarchicalGrid",pybind11::dynamic_attr(),
        Dune::Python::GenerateTypeName(gridTypeName),
        Dune::Python::IncludeFiles{"dune/alugrid/dgf.hh","dune/alugrid/grid.hh","dune/python/grid/hierarchical.hh"}).first;
    Dune::Python::registerHierarchicalGrid( cls0, cls );
  }
}
#else
;
#endif
