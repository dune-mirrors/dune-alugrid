#ifndef DUNE_ALUGRID_ENTITY_CC
#define DUNE_ALUGRID_ENTITY_CC

#if COMPILE_ALUGRID_INLINE == 0
#include <config.h>
#endif

#include "alu3dinclude.hh"
#include <dune/alugrid/3d/alugrid.hh>
#include "entity.hh"

#if COMPILE_ALUGRID_INLINE == 0
#include <dune/alugrid/3d/gridfactory.hh>
#endif
#include <dune/alugrid/common/geostorage.hh>

#if COMPILE_ALUGRID_INLINE
#define alu_inline inline
#else
#define alu_inline
#endif

namespace Dune {

  // --Entity
  template <int cd, int dim, class GridImp>
  ALU3dGridEntity<cd,dim,GridImp> ::
  ALU3dGridEntity() : seed_()
  {}

  // --Entity
  template <int cd, int dim, class GridImp>
  ALU3dGridEntity<cd,dim,GridImp> ::
  ALU3dGridEntity( const EntitySeed& seed )
    : seed_( seed )
  {
  }

  template<int cd, int dim, class GridImp>
  alu_inline void ALU3dGridEntity<cd,dim,GridImp> ::
  setEntity(const ALU3dGridEntity<cd,dim,GridImp> & org)
  {
    setElement( org.seed_ );
  }

  template<int cd, int dim, class GridImp>
  alu_inline void ALU3dGridEntity<cd,dim,GridImp> ::
  setElement(const HItemType & item)
  {
    setElement( item, item.level() );
  }

  template<int cd, int dim, class GridImp>
  alu_inline void ALU3dGridEntity<cd,dim,GridImp> ::
  setElement(const HItemType & item, const GridImp& grid)
  {
    setElement( item, GetLevel<GridImp,cd>::getLevel(grid,item) );
  }

  template<int cd, int dim, class GridImp>
  alu_inline void ALU3dGridEntity<cd,dim,GridImp> ::
  setElement(const HItemType & item, const int level, int twist ) //, int face )
  {
    setElement( EntitySeed( item, level, twist ) );
  }

  template<int cd, int dim, class GridImp>
  alu_inline void ALU3dGridEntity<cd,dim,GridImp> ::
  setElement(const EntitySeed& seed )
  {
    // copy seed
    seed_ = seed;

    // reset geometry information
    geo_.invalidate();
  }

  template<int cd, int dim, class GridImp>
  alu_inline void ALU3dGridEntity<cd,dim,GridImp> ::
  setGhost(const HBndSegType &ghost)
  {
    // this method only exists, that we don't have to specialize the
    // Iterators for each codim, this method should not be called otherwise
    // error
    DUNE_THROW(GridError,"This method should not be called!");
  }

  template<int cd, int dim, class GridImp>
  alu_inline PartitionType ALU3dGridEntity<cd,dim,GridImp> ::
  convertBndId(const HItemType & item) const
  {
    if(item.isGhost())
    {
      return GhostEntity;
    }
    else if(item.isBorder())
    {
      return BorderEntity;
    }
    else
    {
      alugrid_assert ( item.isInterior() );
      return InteriorEntity;
    }
  }

  template< int cd, int dim, class GridImp >
  alu_inline typename ALU3dGridEntity< cd, dim, GridImp >::Geometry
  ALU3dGridEntity< cd, dim, GridImp >::geometry () const
  {
    if( ! geo_.valid() )
      geo_.buildGeom( getItem(), seed_.twist() );
    return Geometry( geo_ );
  }

  /////////////////////////////////////////////////
  //
  //  --Entity0
  //  --Codim0Entity
  //
  /////////////////////////////////////////////////

  template<int dim, class GridImp>
  alu_inline ALU3dGridEntity<0,dim,GridImp> ::
  ALU3dGridEntity() : item_( 0 ), ghost_( 0 )
  {}

  template<int dim, class GridImp>
  alu_inline ALU3dGridEntity<0,dim,GridImp> ::
  ALU3dGridEntity( const EntitySeed& seed )
  {
    setElement( seed );
  }

  template<int dim, class GridImp>
  alu_inline ALU3dGridEntity<0,dim,GridImp> ::
  ALU3dGridEntity( const HElementType& element )
  {
    setElement( const_cast< HElementType& > (element ) );
  }

  template<int dim, class GridImp>
  alu_inline ALU3dGridEntity<0,dim,GridImp> ::
  ALU3dGridEntity( const HBndSegType& ghost )
  {
    setGhost( const_cast< HBndSegType& > (ghost) );
  }

  template< int dim, class GridImp >
  alu_inline typename ALU3dGridEntity< 0, dim, GridImp >::Geometry
  ALU3dGridEntity< 0, dim, GridImp >::geometry () const
  {
    alugrid_assert (item_ != 0);
    if( ! geo_.valid() )
      geo_.buildGeom( *item_ );
    return Geometry( geo_ );
  }

  template< int dim, class GridImp >
  alu_inline typename ALU3dGridEntity<0,dim,GridImp>::LocalGeometry
  ALU3dGridEntity< 0, dim, GridImp >::geometryInFather () const
  {
    alugrid_assert ( item_ );
    // this method should only be called if a father exists
    alugrid_assert ( item_->up() );

    // get child number
    const int child = item_->nChild();

    // if the rule of the farher is not refine_element, it has to be bisection
    // this can only be true for tetrahedral elements
    if( (GridImp::elementType == tetra) && (item_->up()->getrule() != ImplTraits::RefinementRules::refine_element_t) )
    {
      LocalGeometryImpl geom;
#if DUNE_VERSION_NEWER(DUNE_GRID,2,4)
      geom.buildGeomInFather( father().geometry(), geometry() );
#else
      geom.buildGeomInFather( father()->geometry(), geometry() );
#endif

      return LocalGeometry( geom );
    }
    else
    {
      typedef ALULocalGeometryStorage< GridImp, LocalGeometryImpl, 8 > GeometryInFatherStorage ;
      const GeometryInFatherStorage& geometryStorage =
        GeometryInFatherStorage :: storage( type(), true );
      // get geometryInFather storage from grid and return childs geom
      return LocalGeometry( geometryStorage[ child ] );
    }
  }

  //********* begin method subIndex ********************
  // partial specialisation of subIndex
  template <int dim, class IMPLElemType, ALU3dGridElementType type, class Comm, int codim>
  struct IndexWrapper {};

  // specialisation for vertices
  template <int dim, class IMPLElemType, ALU3dGridElementType type, class Comm>
  struct IndexWrapper<dim, IMPLElemType, type, Comm, 3>
  {
    typedef ElementTopologyMapping<type> ElemTopo;

    static int subIndex(const IMPLElemType &elem, int i)
    {
      return elem.myvertex( ElemTopo::dune2aluVertex(i) )->getIndex(); // element topo
    }
  };

  // specialisation for faces
  template <int dim, class IMPLElemType, ALU3dGridElementType type, class Comm>
  struct IndexWrapper<dim, IMPLElemType, type , Comm, 1>
  {
    static int subIndex(const IMPLElemType &elem, int i)
    {
      // is specialised for each element type and uses
      // the dune2aluFace mapping and also specialised for dim 2
      return (ALU3dGridFaceGetter< Comm >::getFace(elem,i))->getIndex();
    }
  };

  // specialisation for edges
  template <int dim, class IMPLElemType, ALU3dGridElementType type, class Comm>
  struct IndexWrapper<dim, IMPLElemType, type, Comm, 2>
  {
    typedef ElementTopologyMapping<type> ElemTopo;

    // return subIndex of given edge
    static int subIndex(const IMPLElemType &elem, int i)
    {
      if(dim == 3)
      {
        // get hedge1 corresponding to dune reference element and return number
        return elem.myhedge( ElemTopo::dune2aluEdge(i) )->getIndex();
      }
      else if (dim == 2)
      {
        if (type == tetra)
        {
           // We want vertices 1,2,3 in DUNE numbering for tetra and 0,1,2,3 for hexa
           ++i;
        }
        // get vertex corresponding to dune reference element and return number
        return elem.myvertex( ElemTopo::dune2aluVertex(i) )->getIndex();
      }
    }
  };

  // specialisation for elements
  template <int dim, class IMPLElemType, ALU3dGridElementType type, class Comm>
  struct IndexWrapper<dim, IMPLElemType, type, Comm, 0>
  {
    static int subIndex(const IMPLElemType &elem, int i) {
      // just return the elements index
      return elem.getIndex();
    }
  };

  template<int dim, class GridImp>
  template<int cc>
  alu_inline int ALU3dGridEntity<0,dim,GridImp> :: getSubIndex (int i) const
  {
    alugrid_assert (item_ != 0);
    typedef typename  ImplTraits::IMPLElementType IMPLElType;
    return IndexWrapper<GridImp::dimension, IMPLElType,GridImp::elementType, typename GridImp::MPICommunicatorType, cc>::subIndex ( *item_, i);
  }

  template<int dim, class GridImp>
  alu_inline int ALU3dGridEntity<0,dim,GridImp> :: subIndex (int i, unsigned int codim ) const
  {
    typedef ElementTopologyMapping<GridImp::elementType> ElemTopo;

    alugrid_assert (item_ != 0);
    switch (codim)
    {
      case 0:
        return this->getIndex();
      case 1:
        return (ALU3dGridFaceGetter< Comm >::getFace( *item_, i ))->getIndex();
      case 2:
          if(GridImp::dimension == 3)
          {
            // get hedge1 corresponding to dune reference element and return number
            return item_->myhedge( ElemTopo::dune2aluEdge(i) )->getIndex();
          }
          else if (GridImp::dimension == 2)
          {
            if (GridImp:: elementType == tetra)
            {
              // We want vertices 1,2,3 in DUNE numbering for tetra and 0,1,2,3 for hexa
              ++i;
            }
            // get myvertex corresponding to dune reference element and return number
            return item_->myvertex( ElemTopo::dune2aluVertex( i ) )->getIndex();
          }
      case 3:
        return item_->myvertex( ElemTopo::dune2aluVertex( i ) )->getIndex();
      default :
        alugrid_assert (false);
        abort();
    }
    return -1;
  }



  // SubEntitites
  // ------------

  template <class GridImp, int dim, int cd> struct SubEntities {};

  // specialisation for elements
  template <class GridImp, int dim>
  struct SubEntities<GridImp, dim, 0>
  {
    typedef ALU3dGridEntity<0,dim,GridImp> ElementType;
    typedef typename GridImp::template Codim< 0 >:: EntityPointer     EntityPointer;
    typedef typename GridImp::template Codim< 0 >:: EntityPointerImpl EntityPointerImpl;

    typedef typename ALU3dImplTraits< GridImp::elementType, typename GridImp::MPICommunicatorType >::IMPLElementType Item;

    typedef typename ElementType::template Codim< 0 >::Twist Twist;

    // note: The subentity number i is in dune numbering.
    static EntityPointer
    entity ( int level, const ElementType &entity, const Item &item, int i )
    {
      return EntityPointerImpl( entity );
    }

    static Twist twist ( const Item &item, int i ) { return Twist(); }
  };

  // specialisation for faces
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,1>
  {
    typedef ElementTopologyMapping<GridImp::elementType> Topo;
    typedef ALU3dGridEntity<0,dim,GridImp> ElementType;

    typedef typename GridImp::template Codim< 1 >:: EntitySeed        EntitySeed;
    typedef typename GridImp::template Codim< 1 >:: EntityPointer     EntityPointer;
    typedef typename GridImp::template Codim< 1 >:: EntityPointerImpl EntityPointerImpl;

    typedef typename ALU3dImplTraits< GridImp::elementType, typename GridImp::MPICommunicatorType >::IMPLElementType Item;

    typedef typename ElementType::template Codim< 1 >::Twist Twist;

    // note: The subentity number i is in dune numbering.
    static EntityPointer
    entity ( int level, const ElementType &entity, const Item &item, int i )
    {
      return EntityPointerImpl( EntitySeed(
               *ALU3dGridFaceGetter< typename GridImp::MPICommunicatorType >::getFace( item, i ),
               level, static_cast< int >( twist( item, i ) ) )
             );
    }

    static Twist twist ( const Item &item, int i )
    {
      return Twist( Topo::duneFaceTwist( i ) ) * Twist( item.twist( Topo::dune2aluFace( i ) ) );
    }
  };

  // specialisation for edges
  template <class GridImp>
  struct SubEntities<GridImp,3,2>
  {
    typedef ElementTopologyMapping<GridImp::elementType> Topo;
    typedef ALU3dGridEntity<0,3,GridImp>   ElementType;

    typedef typename GridImp::template Codim< 2 >:: EntitySeed         EntitySeed;
    typedef typename GridImp::template Codim< 2 >:: EntityPointer      EntityPointer;
    typedef typename GridImp::template Codim< 2 >:: EntityPointerImpl  EntityPointerImpl;

    typedef typename GridImp::ctype coordType;
    typedef typename GridImp :: ReferenceElementType ReferenceElementType;

    typedef typename ALU3dImplTraits< GridImp::elementType, typename GridImp::MPICommunicatorType >::IMPLElementType Item;
    typedef typename ALU3dImplTraits< GridImp::elementType, typename GridImp::MPICommunicatorType >::GEOFaceType     Face;
    //typedef typename ALU3dImplTraits< GridImp::elementType, typename GridImp::MPICommunicatorType >::GEOEdgeType GEOEdgeType;

    typedef typename ElementType::template Codim< 2 >::Twist Twist;

    // note: The subentity number i is in dune numbering.
    static EntityPointer
    entity ( int level, const ElementType &entity, const Item &item, int i )
    {
      typedef typename ALU3dImplTraits< GridImp::elementType, typename GridImp::MPICommunicatorType>::GEOEdgeType Edge;
      typedef typename ALU3dImplTraits< GridImp::elementType, typename GridImp::MPICommunicatorType>::GEOFaceType Face;

      ALUTwist< Topo::numVerticesPerFace, 2 > faceTwist( item.twist( Topo::duneEdgeMap( i ).first ) );
      const Face &face = *item.myhface( Topo::duneEdgeMap( i ).first );
      const int j = faceTwist.apply( Topo::duneEdgeMap( i ).second, 1 );

      const Edge &edge = *face.myhedge( j );
      const int twist = (int( !faceTwist.positive() )^face.twist( j ));

      return EntityPointerImpl( EntitySeed( edge, level, twist ) );
    }

    static Twist twist ( const Item &item, int i )
    {
      typedef typename ALU3dImplTraits< GridImp::elementType, typename GridImp::MPICommunicatorType>::GEOFaceType Face;

      ALUTwist< Topo::numVerticesPerFace, 2 > faceTwist( item.twist( Topo::duneEdgeMap( i ).first ) );
      const Face &face = *item.myhface( Topo::duneEdgeMap( i ).first );
      const int j = faceTwist.apply( Topo::duneEdgeMap( i ).second, 1 );

      return Twist( int( !faceTwist.positive() )^face.twist( j ) );
    }
  };

   // specialisation for vertices in 2d
  template <class GridImp>
  struct SubEntities<GridImp, 2, 2>
  {
    typedef ElementTopologyMapping<GridImp::elementType> Topo;
    typedef ALU3dGridEntity<0, 2, GridImp>          ElementType;
    typedef typename GridImp::ctype coordType;

    typedef typename GridImp::template Codim< 2 >:: EntitySeed        EntitySeed;
    typedef typename GridImp::template Codim< 2 >:: EntityPointer     EntityPointer;
    typedef typename GridImp::template Codim< 2 >:: EntityPointerImpl EntityPointerImpl;

    typedef typename ALU3dImplTraits<GridImp::elementType, typename GridImp::MPICommunicatorType>::IMPLElementType Item;

    typedef typename GridImp::template Codim< 2 >::Twist Twist;

    static EntityPointerImpl
    entity (const int level, const ElementType & entity, const Item& item, int i)
    {
      if( GridImp::elementType == tetra )
      {
        // we want vertices 1,2,3 (in DUNE numbering) for tetra and 0,1,2,3 for hexa
        ++i;
      }
      return
        EntityPointerImpl( EntitySeed( *item.myvertex( Topo::dune2aluVertex(i) ), level )); // element topo
    }

    static Twist twist ( const Item &item, int i ) { return Twist(); }
  };

  // specialisation for vertices
  template <class GridImp, int dim>
  struct SubEntities<GridImp,dim,3>
  {
    typedef ElementTopologyMapping<GridImp::elementType> Topo;
    typedef ALU3dGridEntity<0,dim,GridImp> ElementType;

    typedef typename GridImp::template Codim< 3 >:: EntitySeed        EntitySeed;
    typedef typename GridImp::template Codim< 3 >:: EntityPointer     EntityPointer;
    typedef typename GridImp::template Codim< 3 >:: EntityPointerImpl EntityPointerImpl;

    typedef typename ALU3dImplTraits< GridImp::elementType, typename GridImp::MPICommunicatorType >::IMPLElementType Item;

    typedef typename GridImp::template Codim< 3 >::Twist Twist;

    // note: The subentity number i is in dune numbering.
    static EntityPointer
    entity ( int level, const ElementType &entity, const Item &item, int i )
    {
      return EntityPointerImpl( EntitySeed( *item.myvertex(Topo::dune2aluVertex(i)), level ) );
    }

    static Twist twist ( const Item &item, int i ) { return Twist(); }
  };


  template<int dim, class GridImp>
  template<int cc>
  typename ALU3dGridEntity<0,dim,GridImp>::template Codim<cc>:: EntityPointer
  ALU3dGridEntity<0,dim,GridImp> :: subEntity (int i) const
  {
    return SubEntities<GridImp,dim,cc>::entity(level(), *this, *item_, i);
  }


  template< int dim, class GridImp >
  template< int codim >
  typename ALU3dGridEntity< 0, dim, GridImp >::template Codim< codim >::Twist
  ALU3dGridEntity< 0, dim, GridImp >::twist ( int i ) const
  {
    return SubEntities< GridImp, dim, codim >::twist( *item_, i );
  }


  template<int dim, class GridImp>
  typename ALU3dGridEntity<0,dim,GridImp> :: EntityPointer
  ALU3dGridEntity<0,dim,GridImp> :: father() const
  {
    typedef typename GridImp :: template Codim< 0 > :: EntityPointerImpl  EntityPointerImpl;
    HElementType* up = item_->up();
    if( ! up )
    {
      std::cerr << "ALU3dGridEntity<0," << dim << "," << dimworld << "> :: father() : no father of entity globalid = " << getIndex() << "\n";
      return EntityPointerImpl( static_cast<HElementType &> (*item_) );
    }

    if( isGhost () )
    {
      return EntityPointerImpl( static_cast<const HBndSegType &> (*(getGhost().up())));
    }

    return EntityPointerImpl( static_cast<HElementType &> ( *up ));
  }

  // Adaptation methods
  template<int dim, class GridImp>
  bool ALU3dGridEntity<0,dim,GridImp> :: mark ( const int ref, const bool conformingRefinement ) const
  {
    alugrid_assert (item_ != 0);

    // do not allow to mark ghost cells or non-leaf cells
    // this will lead to unpredictable results errors
    if( isGhost() || ! isLeaf() ) return false ;

    // mark for coarsening
    if(ref < 0)
    {
      // don't mark macro elements for coarsening ;)
      if(level() <= 0) return false;

      item_->request(coarse_element_t);
      return true;
    }

    // mark for refinement
    if(ref > 0)
    {
      // for tetrahedral elements check whether to use bisection
      if( GridImp :: elementType == tetra &&
          conformingRefinement )
      {
        item_->request( bisect_element_t );
      }
      else
      {
        item_->request( refine_element_t );
      }
      return true;
    }

    // mark for none
    item_->request( nosplit_element_t );
    return true;
  }

  // return mark of entity
  template<int dim, class GridImp>
  alu_inline int ALU3dGridEntity<0,dim,GridImp> :: getMark () const
  {
    alugrid_assert (item_ != 0);

    const MarkRuleType rule = (*item_).requestrule();

    if(rule == coarse_element_t) return -1;
    else if(rule == nosplit_element_t ) return 0;
    else
    {
      // rule == refine_element_t is not true for bisection
      // since we have different refinement rules in this case
      return 1;
    }
  }


  template<int dim, class GridImp>
  bool ALU3dGridEntity<0,dim,GridImp> :: hasBoundaryIntersections () const
  {
    // on ghost elements return false
    if( isGhost() ) return false;

    enum { numFaces = dim == 3 ? EntityCount<GridImp::elementType>::numFaces : (elementType == tetra ? 3 : 4) };
    typedef typename ImplTraits::HasFaceType HasFaceType;
    typedef typename ImplTraits::GEOFaceType GEOFaceType;

    alugrid_assert ( item_ );
    for(int i=0; i<numFaces; ++i)
    {
      const GEOFaceType &face = *ALU3dGridFaceGetter< Comm >::getFace( *item_, i );

      // don't count internal boundaries as boundary
      if( face.isBorder() ) continue ;

      // check both
      const HasFaceType * outerElement = face.nb.front().first;
      // if we got our own element, get other side
      // the above is a bad test, as it does not take into account
      // the situation of conforming refinement 
      // it is better to directly test the face twist
      if(item_ == outerElement)
     // if( item_->twist(ElementTopologyMapping< elementType >::dune2aluFace(i) ) < 0 )
      {
        outerElement = face.nb.rear().first;
      }

      alugrid_assert ( outerElement );
      if( outerElement->isboundary() ) return true;
    }
    return false;
  }

#if COMPILE_ALUGRID_LIB
  // Instantiation - 2-2
  template class ALU3dGrid<2, 2, hexa, ALUGridNoComm >;
  template class ALU3dGrid<2, 2, tetra, ALUGridNoComm >;

  // Instantiation
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;

  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;

  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;

  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > >;

  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > > :: subEntity< 0 >( int ) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > > :: subEntity< 0 >( int ) const;

  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > > :: subEntity< 1 >( int ) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > > :: subEntity< 1 >( int ) const;

  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > > :: subEntity< 2 >( int ) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > > :: subEntity< 2 >( int ) const;

  template ALU3dGrid< 2, 2, tetra, ALUGridNoComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, tetra, ALUGridNoComm > > :: subEntity< 3 >( int ) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridNoComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, hexa, ALUGridNoComm > > :: subEntity< 3 >( int ) const;

  // Instantiation
  template class ALU3dGrid<2, 2, hexa, ALUGridMPIComm >;
  template class ALU3dGrid<2, 2, tetra, ALUGridMPIComm >;

  // Instantiation with MPI
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > >;

  template ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;

  template ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;

  template ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;

  template ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, tetra, ALUGridMPIComm > > :: subEntity< 3 >( int ) const;
  template ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 2, hexa, ALUGridMPIComm > > :: subEntity< 3 >( int ) const;

      // Instantiation
  template class ALU3dGrid<2, 3, hexa, ALUGridNoComm >;
  template class ALU3dGrid<2, 3, tetra, ALUGridNoComm >;

  // Instantiation - 2-3
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > >;

  template ALU3dGrid< 2, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > > :: subEntity< 0 >( int ) const;
  template ALU3dGrid< 2, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > > :: subEntity< 0 >( int ) const;

  template ALU3dGrid< 2, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > > :: subEntity< 1 >( int ) const;
  template ALU3dGrid< 2, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > > :: subEntity< 1 >( int ) const;

  template ALU3dGrid< 2, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > > :: subEntity< 2 >( int ) const;
  template ALU3dGrid< 2, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > > :: subEntity< 2 >( int ) const;

  template ALU3dGrid< 2, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridNoComm > > :: subEntity< 3 >( int ) const;
  template ALU3dGrid< 2, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridNoComm > > :: subEntity< 3 >( int ) const;

  // Instantiation
  template class ALU3dGrid<2, 3, hexa, ALUGridMPIComm >;
  template class ALU3dGrid<2, 3, tetra, ALUGridMPIComm >;

  // Instantiation with MPI
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > >;

  template ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;
  template ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;

  template ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;
  template ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;

  template ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;
  template ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;

  template ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, tetra, ALUGridMPIComm > > :: subEntity< 3 >( int ) const;
  template ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 2, 3, hexa, ALUGridMPIComm > > :: subEntity< 3 >( int ) const;

      // Instantiation  - 3-3
  template class ALU3dGrid<3, 3, hexa, ALUGridNoComm >;
  template class ALU3dGrid<3, 3, tetra, ALUGridNoComm >;

  // Instantiation
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > >;
  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > >;

  template ALU3dGrid< 3, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > > :: subEntity< 0 >( int ) const;
  template ALU3dGrid< 3, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > > :: subEntity< 0 >( int ) const;

  template ALU3dGrid< 3, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > > :: subEntity< 1 >( int ) const;
  template ALU3dGrid< 3, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > > :: subEntity< 1 >( int ) const;

  template ALU3dGrid< 3, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > > :: subEntity< 2 >( int ) const;
  template ALU3dGrid< 3, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > > :: subEntity< 2 >( int ) const;

  template ALU3dGrid< 3, 3, tetra, ALUGridNoComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridNoComm > > :: subEntity< 3 >( int ) const;
  template ALU3dGrid< 3, 3, hexa, ALUGridNoComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridNoComm > > :: subEntity< 3 >( int ) const;

  // Instantiation
  template class ALU3dGrid<3, 3, hexa, ALUGridMPIComm >;
  template class ALU3dGrid<3, 3, tetra, ALUGridMPIComm >;

  // Instantiation with MPI
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<1, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<2, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > >;
  template class ALU3dGridEntity<3, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > >;

  template ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;
  template ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 0 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > > :: subEntity< 0 >( int ) const;

  template ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;
  template ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 1 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > > :: subEntity< 1 >( int ) const;

  template ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;
  template ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 2 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > > :: subEntity< 2 >( int ) const;

  template ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, tetra, ALUGridMPIComm > > :: subEntity< 3 >( int ) const;
  template ALU3dGrid< 3, 3, hexa, ALUGridMPIComm > :: Traits :: Codim< 3 > :: EntityPointer
    ALU3dGridEntity<0, 3, const ALU3dGrid< 3, 3, 3, 3, hexa, ALUGridMPIComm > > :: subEntity< 3 >( int ) const;

#endif // #if COMPILE_ALUGRID_LIB
}
#undef alu_inline
#endif
