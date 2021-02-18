#if ! COMPILE_ALUGRID_INLINE

#ifndef alu_extern
#define alu_extern
#endif

namespace Dune
{
  // Instantiation - 2-2
  alu_extern template class ALU3dGridGeometry<0, 2, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridGeometry<0, 2, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;

  alu_extern template class ALU3dGridGeometry<1, 2, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridGeometry<1, 2, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;

  alu_extern template class ALU3dGridGeometry<2, 2, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridGeometry<2, 2, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;


  // Instantiation with MPI
  alu_extern template class ALU3dGridGeometry<0, 2, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridGeometry<0, 2, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridGeometry<1, 2, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridGeometry<1, 2, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridGeometry<2, 2, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridGeometry<2, 2, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

  // Instantiation -2-3
  alu_extern template class ALU3dGridGeometry<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridGeometry<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

  alu_extern template class ALU3dGridGeometry<1, 2, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridGeometry<1, 2, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

  alu_extern template class ALU3dGridGeometry<1, 3, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridGeometry<1, 3, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

  alu_extern template class ALU3dGridGeometry<2, 3, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridGeometry<2, 3, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

  // Instantiation with MPI
  alu_extern template class ALU3dGridGeometry<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridGeometry<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridGeometry<1, 2, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridGeometry<1, 2, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridGeometry<1, 3, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridGeometry<1, 3, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridGeometry<2, 3, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridGeometry<2, 3, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;


  // Instantiation  -3-3
  alu_extern template class ALU3dGridGeometry<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridGeometry<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

  alu_extern template class ALU3dGridGeometry<1, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridGeometry<1, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

  alu_extern template class ALU3dGridGeometry<2, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridGeometry<2, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

  alu_extern template class ALU3dGridGeometry<3, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridGeometry<3, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

  // Instantiation with MPI
  alu_extern template class ALU3dGridGeometry<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridGeometry<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridGeometry<1, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridGeometry<1, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridGeometry<2, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridGeometry<2, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridGeometry<3, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridGeometry<3, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;


}
#endif // COMPILE_ALUGRID_INLINE
#undef alu_extern
