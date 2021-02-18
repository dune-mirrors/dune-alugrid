#if ! COMPILE_ALUGRID_INLINE

#ifndef alu_extern
#define alu_extern
#endif

namespace Dune {
  // Instantiation - 2-2
  //template class ALU3dGrid<2, 2, hexa, ALUGridNoComm >;
  //template class ALU3dGrid<2, 2, tetra, ALUGridNoComm >;

  // Instantiation
 alu_extern template class ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
 alu_extern template class ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;

 alu_extern template class ALU3dGridEntity<1, 2, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
 alu_extern template class ALU3dGridEntity<1, 2, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;

 alu_extern template class ALU3dGridEntity<2, 2, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
 alu_extern template class ALU3dGridEntity<2, 2, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;


  alu_extern template ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > > :: subEntity< 0 >( int ) const;
  alu_extern template ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > > :: subEntity< 0 >( int ) const;

  alu_extern template ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > > :: subEntity< 1 >( int ) const;
  alu_extern template ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > > :: subEntity< 1 >( int ) const;

  alu_extern template ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > > :: subEntity< 2 >( int ) const;
  alu_extern template ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > > :: subEntity< 2 >( int ) const;

  // Instantiation
  //template class ALU3dGrid<2, 2, hexa, ALUGridMPIComm >;
  //template class ALU3dGrid<2, 2, tetra, ALUGridMPIComm >;

  // Instantiation with MPI
 alu_extern template class ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
 alu_extern template class ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

 alu_extern template class ALU3dGridEntity<1, 2, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
 alu_extern template class ALU3dGridEntity<1, 2, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

 alu_extern template class ALU3dGridEntity<2, 2, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
 alu_extern template class ALU3dGridEntity<2, 2, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

  alu_extern template ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;
  alu_extern template ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;

  alu_extern template ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;
  alu_extern template ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;

  alu_extern template ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;
  alu_extern template ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;

  // Instantiation
  //template class ALU3dGrid<2, 3, hexa, ALUGridNoComm >;
  //template class ALU3dGrid<2, 3, tetra, ALUGridNoComm >;

  // Instantiation - 2-3
 alu_extern template class ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
 alu_extern template class ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

 alu_extern template class ALU3dGridEntity<1, 2, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
 alu_extern template class ALU3dGridEntity<1, 2, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

 alu_extern template class ALU3dGridEntity<2, 2, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
 alu_extern template class ALU3dGridEntity<2, 2, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

  alu_extern template ALU3dGrid< 2, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > > :: subEntity< 0 >( int ) const;
  alu_extern template ALU3dGrid< 2, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > > :: subEntity< 0 >( int ) const;

  alu_extern template ALU3dGrid< 2, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > > :: subEntity< 1 >( int ) const;
  alu_extern template ALU3dGrid< 2, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > > :: subEntity< 1 >( int ) const;

  alu_extern template ALU3dGrid< 2, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > > :: subEntity< 2 >( int ) const;
  alu_extern template ALU3dGrid< 2, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > > :: subEntity< 2 >( int ) const;

  // Instantiation
  //template class ALU3dGrid<2, 3, hexa, ALUGridMPIComm >;
  //template class ALU3dGrid<2, 3, tetra, ALUGridMPIComm >;

  // Instantiation with MPI
 alu_extern template class ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
 alu_extern template class ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

 alu_extern template class ALU3dGridEntity<1, 2, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
 alu_extern template class ALU3dGridEntity<1, 2, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

 alu_extern template class ALU3dGridEntity<2, 2, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
 alu_extern template class ALU3dGridEntity<2, 2, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

  alu_extern template ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;
  alu_extern template ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;

  alu_extern template ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;
  alu_extern template ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;

  alu_extern template ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;
  alu_extern template ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 2, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;

  // Instantiation  - 3-3
  //template class ALU3dGrid<3, 3, hexa, ALUGridNoComm >;
  //template class ALU3dGrid<3, 3, tetra, ALUGridNoComm >;

  // Instantiation
 alu_extern template class ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
 alu_extern template class ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

 alu_extern template class ALU3dGridEntity<1, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
 alu_extern template class ALU3dGridEntity<1, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

 alu_extern template class ALU3dGridEntity<2, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
 alu_extern template class ALU3dGridEntity<2, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

 alu_extern template class ALU3dGridEntity<3, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
 alu_extern template class ALU3dGridEntity<3, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

  alu_extern template ALU3dGrid< 3, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > > :: subEntity< 0 >( int ) const;
  alu_extern template ALU3dGrid< 3, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > > :: subEntity< 0 >( int ) const;

  alu_extern template ALU3dGrid< 3, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > > :: subEntity< 1 >( int ) const;
  alu_extern template ALU3dGrid< 3, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > > :: subEntity< 1 >( int ) const;

  alu_extern template ALU3dGrid< 3, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > > :: subEntity< 2 >( int ) const;
  alu_extern template ALU3dGrid< 3, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > > :: subEntity< 2 >( int ) const;

  alu_extern template ALU3dGrid< 3, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 3 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > > :: subEntity< 3 >( int ) const;
  alu_extern template ALU3dGrid< 3, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 3 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > > :: subEntity< 3 >( int ) const;

  // Instantiation
  //template class ALU3dGrid<3, 3, hexa, ALUGridMPIComm >;
  //template class ALU3dGrid<3, 3, tetra, ALUGridMPIComm >;

  // Instantiation with MPI
 alu_extern template class ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
 alu_extern template class ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

 alu_extern template class ALU3dGridEntity<1, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
 alu_extern template class ALU3dGridEntity<1, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

 alu_extern template class ALU3dGridEntity<2, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
 alu_extern template class ALU3dGridEntity<2, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

 alu_extern template class ALU3dGridEntity<3, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
 alu_extern template class ALU3dGridEntity<3, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

  alu_extern template ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;
  alu_extern template ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 0 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;

  alu_extern template ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;
  alu_extern template ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 1 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;

  alu_extern template ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;
  alu_extern template ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 2 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;

  alu_extern template ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 3 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > > :: subEntity< 3 >( int ) const;
  alu_extern template ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 3 > :: Entity
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > > :: subEntity< 3 >( int ) const;

  //alu_extern template ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 0 > :: LocalGeometry
  //  ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > > :: geometryInFather() ;
}
#endif
#undef alu_extern
