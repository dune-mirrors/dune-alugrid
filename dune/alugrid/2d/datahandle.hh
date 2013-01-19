#ifndef DUNE_ALU2DGRIDDATAHANDLE_HH
#define DUNE_ALU2DGRIDDATAHANDLE_HH

#include <iostream>

#include <dune/grid/common/adaptcallback.hh>

#include <dune/alugrid/2d/alu2dinclude.hh>

namespace ALU2DGrid
{

  // AdaptRestrictProlong2dImpl
  // --------------------------

  template< class GridType, class AdaptDataHandle >
  class AdaptRestrictProlong2dImpl
  : public AdaptRestrictProlong2d ALU2DDIMWORLD( GridType::dimensionworld, GridType::elementType )
  {
    GridType & grid_;
    typedef Dune :: MakeableInterfaceObject<typename GridType::template Codim<0>::Entity> EntityType;
    typedef typename EntityType :: ImplementationType RealEntityType;
    typedef typename Dune::ALU2dImplTraits< GridType::dimensionworld, GridType::elementType >::HElementType HElementType ;
    
    EntityType & reFather_;
    EntityType & reSon_;
    RealEntityType & realFather_;
    RealEntityType & realSon_;

    AdaptDataHandle &rp_;

    int maxlevel_;

  public:
    //! Constructor
    AdaptRestrictProlong2dImpl ( GridType &grid,
                                 EntityType &f, RealEntityType &rf,
                                 EntityType &s, RealEntityType &rs,
                                 AdaptDataHandle &rp )
     : grid_(grid)
      , reFather_(f)
      , reSon_(s)
      , realFather_(rf) 
      , realSon_(rs) 
      , rp_(rp) 
      , maxlevel_(-1) 
    {
    }

    virtual ~AdaptRestrictProlong2dImpl () 
    {}

    //! restrict data , elem is always the father 
    int preCoarsening ( HElementType &father )
    {
      maxlevel_ = std::max( maxlevel_, father.level() );
      //father.resetRefinedTag();
      realFather_.setElement( father );
      rp_.preCoarsening( reFather_ );

      return 0;
    }

    //! prolong data, elem is the father  
    int postRefinement ( HElementType &father )
    {
      maxlevel_ = std::max( maxlevel_, father.level()+1 );
      //father.resetRefinedTag();
      realFather_.setElement( father );
      rp_.postRefinement( reFather_ );

      return 0;
    }

    int maxLevel () const { return maxlevel_; }
  };

} // namespace ALU2DGrid

#endif // #ifndef DUNE_ALU2DGRIDDATAHANDLE_HH
