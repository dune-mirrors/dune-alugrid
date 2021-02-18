#if ! COMPILE_ALUGRID_INLINE

#ifndef alu_extern
#define alu_extern
#endif
namespace Dune {

  // Instantiation 3-3

  // Instantiation without MPI
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLeafIterator< 3, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLevelIterator< 3, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;

  // Instantiation

  // Instantiation with MPI
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLeafIterator< 3, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLevelIterator< 3, All_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, All_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Interior_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, InteriorBorder_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Overlap_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, OverlapFront_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 3, Ghost_Partition, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;

  // Instantiation 2-3

  // Instantiation without MPI
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;


  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;

  // Instantiation

  // Instantiation with MPI
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;


  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;


  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;

  // Instantiation 2-2

  // Instantiation without MPI
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;

  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;


  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;
  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;

  // Instantiation

  // Instantiation with MPI
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLeafIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 0, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 1, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, All_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Interior_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, InteriorBorder_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Overlap_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, OverlapFront_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIterator< 2, Ghost_Partition, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridIntersectionIterator< const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridLevelIntersectionIterator< const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;
  alu_extern template class ALU3dGridHierarchicIterator< const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;

}
#endif // ! COMPILE_ALUGRID_INLINE
#undef alu_extern
