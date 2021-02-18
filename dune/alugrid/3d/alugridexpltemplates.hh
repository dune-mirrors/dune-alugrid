#ifndef DUNE_ALUGRID_EXPLICT_TEMPLATE_INST_HH
#define DUNE_ALUGRID_EXPLICT_TEMPLATE_INST_HH

#if ! COMPILE_ALUGRID_INLINE


#ifndef alu_extern
#define alu_extern
#endif

namespace Dune {

// conforming
alu_extern template class ALUGrid<2, 2, Dune::simplex, Dune::conforming, Dune::ALUGridMPIComm >;
alu_extern template class ALUGrid<2, 2, Dune::simplex, Dune::conforming, Dune::ALUGridNoComm >;

alu_extern template class ALUGrid<2, 3, Dune::simplex, Dune::conforming, Dune::ALUGridMPIComm >;
alu_extern template class ALUGrid<2, 3, Dune::simplex, Dune::conforming, Dune::ALUGridNoComm >;

alu_extern template class ALUGrid<3, 3, Dune::simplex, Dune::conforming, Dune::ALUGridMPIComm >;
alu_extern template class ALUGrid<3, 3, Dune::simplex, Dune::conforming, Dune::ALUGridNoComm >;

// simplex
alu_extern template class ALUGrid<2, 2, Dune::simplex, Dune::nonconforming, Dune::ALUGridMPIComm >;
alu_extern template class ALUGrid<2, 2, Dune::simplex, Dune::nonconforming, Dune::ALUGridNoComm >;

alu_extern template class ALUGrid<2, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridMPIComm >;
alu_extern template class ALUGrid<2, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridNoComm >;

alu_extern template class ALUGrid<3, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridMPIComm >;
alu_extern template class ALUGrid<3, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridNoComm >;

// cube
alu_extern template class ALUGrid<2, 2, Dune::cube, Dune::nonconforming, Dune::ALUGridMPIComm >;
alu_extern template class ALUGrid<2, 2, Dune::cube, Dune::nonconforming, Dune::ALUGridNoComm >;

alu_extern template class ALUGrid<2, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridMPIComm >;
alu_extern template class ALUGrid<2, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridNoComm >;

alu_extern template class ALUGrid<3, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridMPIComm >;
alu_extern template class ALUGrid<3, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridNoComm >;


} // end namespace Dune
#endif

#undef alu_extern

#endif
