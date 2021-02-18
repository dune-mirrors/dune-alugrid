#if ! COMPILE_ALUGRID_INLINE

#ifndef alu_extern
#define alu_extern
#endif

namespace Dune
{
  // Instantiation -3-3
  alu_extern template class ALUGrid< 3, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridMPIComm >;
  alu_extern template class ALUGrid< 3, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridMPIComm >;
  alu_extern template class ALUGrid< 3, 3, Dune::simplex, Dune::conforming, Dune::ALUGridMPIComm >;

  alu_extern template class ALU3dGridFactory< ALUGrid< 3, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridMPIComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 3, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridMPIComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 3, 3, Dune::simplex, Dune::conforming, Dune::ALUGridMPIComm > >;

  // Instantiation -2-3
  alu_extern template class ALUGrid< 2, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridMPIComm >;
  alu_extern template class ALUGrid< 2, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridMPIComm >;
  alu_extern template class ALUGrid< 2, 3, Dune::simplex, Dune::conforming, Dune::ALUGridMPIComm >;

  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridMPIComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridMPIComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 3, Dune::simplex, Dune::conforming, Dune::ALUGridMPIComm > >;

  // Instantiation -2-2
  alu_extern template class ALUGrid< 2, 2, Dune::cube, Dune::nonconforming, Dune::ALUGridMPIComm >;
  alu_extern template class ALUGrid< 2, 2, Dune::simplex, Dune::nonconforming, Dune::ALUGridMPIComm >;
  alu_extern template class ALUGrid< 2, 2, Dune::simplex, Dune::conforming, Dune::ALUGridMPIComm >;

  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 2, Dune::cube, Dune::nonconforming, Dune::ALUGridMPIComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 2, Dune::simplex, Dune::nonconforming, Dune::ALUGridMPIComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 2, Dune::simplex, Dune::conforming, Dune::ALUGridMPIComm > >;


  // Instantiation -3-3
  alu_extern template class ALUGrid< 3, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridNoComm >;
  alu_extern template class ALUGrid< 3, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridNoComm >;
  alu_extern template class ALUGrid< 3, 3, Dune::simplex, Dune::conforming, Dune::ALUGridNoComm >;

  alu_extern template class ALU3dGridFactory< ALUGrid< 3, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridNoComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 3, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridNoComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 3, 3, Dune::simplex, Dune::conforming, Dune::ALUGridNoComm > >;

  // Instantiation -2-3
  alu_extern template class ALUGrid< 2, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridNoComm >;
  alu_extern template class ALUGrid< 2, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridNoComm >;
  alu_extern template class ALUGrid< 2, 3, Dune::simplex, Dune::conforming, Dune::ALUGridNoComm >;

  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 3, Dune::cube, Dune::nonconforming, Dune::ALUGridNoComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 3, Dune::simplex, Dune::nonconforming, Dune::ALUGridNoComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 3, Dune::simplex, Dune::conforming, Dune::ALUGridNoComm > >;

  // Instantiation -2-2
  alu_extern template class ALUGrid< 2, 2, Dune::cube, Dune::nonconforming, Dune::ALUGridNoComm >;
  alu_extern template class ALUGrid< 2, 2, Dune::simplex, Dune::nonconforming, Dune::ALUGridNoComm >;
  alu_extern template class ALUGrid< 2, 2, Dune::simplex, Dune::conforming, Dune::ALUGridNoComm >;

  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 2, Dune::cube, Dune::nonconforming, Dune::ALUGridNoComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 2, Dune::simplex, Dune::nonconforming, Dune::ALUGridNoComm > >;
  alu_extern template class ALU3dGridFactory< ALUGrid< 2, 2, Dune::simplex, Dune::conforming, Dune::ALUGridNoComm > >;



}
#endif // ! COMPILE_ALUGRID_INLINE
#undef alu_extern
