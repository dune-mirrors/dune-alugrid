#ifndef DUNE_ALU_BNDPROJECTION_HH
#define DUNE_ALU_BNDPROJECTION_HH

namespace Dune {

  //! \brief ALUGrid boundary projection implementation 
  //!  DuneBndProjection has to fulfil the DuneBoundaryProjection interface 
  template <class GridImp, class ctype = double > 
  class ALUGridBoundaryProjection 
    : public GridImp :: ALUGridVertexProjectionType
  {
    typedef GridImp GridType;
    // type of double coordinate vector 
    typedef ctype coord_t[ 3 ];
  protected:

    //! reference to boundary projection implementation 
    const GridType& grid_;
  public: 
    //! type of boundary projection 
    typedef typename GridType :: DuneBoundaryProjectionType DuneBoundaryProjectionType;

    //! type of coordinate vector 
    typedef typename DuneBoundaryProjectionType :: CoordinateType CoordinateType;

    //! constructor storing reference to boundary projection implementation 
    ALUGridBoundaryProjection(const GridType& grid) 
      : grid_( grid ) 
    {
    }

    //! (old) method projection vertices defaults to segment 0
    int operator () (const coord_t &orig, 
                     coord_t &prj) const 
    {
      return this->operator()( orig, 0, prj);
    }

    //! projection operator  
    int operator () (const coord_t &orig, 
                     const int segmentIndex,
                     coord_t &prj) const 
    {
      // get boundary projection 
      const DuneBoundaryProjectionType* bndPrj = 
        grid_.boundaryProjection( segmentIndex );

      // if pointer is zero we do nothing, i.e. identity mapping
      if( bndPrj ) 
      {
        // call projection operator 
        reinterpret_cast<CoordinateType &> (* (&prj[0])) = 
          (*bndPrj)( reinterpret_cast<const CoordinateType &> (* (&orig[0])) );
      }

      // return 1 for success 
      return 1;
    }
  }; 

} // end namespace Dune 
#endif
