#ifndef DUNE_ALUGRIDOBJECTFACTORY_HH
#define DUNE_ALUGRIDOBJECTFACTORY_HH

#include <dune/alugrid/common/memory.hh>

namespace Dune 
{
  template <class InterfaceType>
  struct MakeableInterfaceObject ;

  template <class GridImp>
  class ALUGridObjectFactory
  {
    template <class OF, int codim>
    class ALUGridEntityFactory;

    /////////////////////////////////////////////////////
    //
    //  partial specialization of method getNewEntity
    //
    /////////////////////////////////////////////////////
    template <class GridObjectFactory>
    class ALUGridEntityFactory<GridObjectFactory,0>
    {
    public:  
      enum { codim = 0 };
      typedef typename GridImp :: template Codim<codim> :: Entity Entity;
      typedef MakeableInterfaceObject<Entity> EntityObject;
      typedef typename EntityObject :: ImplementationType EntityImp;
      
      inline static EntityObject * 
      getNewEntity (const GridObjectFactory& factory, int level)
      {
        return factory.entityProvider_.getEntityObject( factory, level, (EntityImp *) 0);
      }

      inline static void freeEntity(const GridObjectFactory& factory, EntityObject * e )
      {
        factory.entityProvider_.freeObject( e );
      }
    };

    template <class GridObjectFactory>
    class ALUGridEntityFactory<GridObjectFactory,1>
    {
    public:  
      enum { codim = 1 };
      typedef typename GridImp :: template Codim<codim> :: Entity Entity;
      typedef MakeableInterfaceObject<Entity> EntityObject;
      typedef typename EntityObject :: ImplementationType EntityImp;
      
      inline static EntityObject * 
      getNewEntity (const GridObjectFactory& factory, int level)
      {
        return factory.faceProvider_.getEntityObject( factory, level, (EntityImp *) 0);
      }

      inline static void freeEntity(const GridObjectFactory& factory, EntityObject * e )
      {
        factory.faceProvider_.freeObject( e );
      }
    };

    template <class GridObjectFactory>
    class ALUGridEntityFactory<GridObjectFactory,2>
    {
    public:  
      enum { codim = 2 };
      typedef typename GridImp :: template Codim<codim> :: Entity Entity;
      typedef MakeableInterfaceObject<Entity> EntityObject;
      typedef typename EntityObject :: ImplementationType EntityImp;
      
      inline static EntityObject * 
      getNewEntity (const GridObjectFactory& factory, int level)
      {
        return factory.edgeProvider_.getEntityObject( factory, level, (EntityImp *) 0);
      }

      inline static void freeEntity(const GridObjectFactory& factory, EntityObject * e )
      {
        factory.edgeProvider_.freeObject( e );
      }
    };
    
    template <class GridObjectFactory>
    class ALUGridEntityFactory<GridObjectFactory,3>
    {
    public:  
      enum { codim = 3 };
      typedef typename GridImp :: template Codim<codim> :: Entity Entity;
      typedef MakeableInterfaceObject<Entity> EntityObject;
      typedef typename EntityObject :: ImplementationType EntityImp;
      
      inline static EntityObject * 
      getNewEntity (const GridObjectFactory& factory, int level)
      {
        return factory.vertexProvider_.getEntityObject( factory, level, (EntityImp *) 0);
      }

      inline static void freeEntity(const GridObjectFactory& factory, EntityObject * e )
      {
        factory.vertexProvider_.freeObject( e );
      }
    }; // end of ALUGridEntityFactory

    enum { vxCodim = GridImp :: dimension };
  public:  
    typedef GridImp GridType;
    typedef ALUGridObjectFactory FactoryType;

    typedef MakeableInterfaceObject<typename GridType :: Traits::template Codim<0>::Entity> EntityObject;
    typedef MakeableInterfaceObject<typename GridType :: Traits::template Codim<1>::Entity> FaceObject;
    typedef MakeableInterfaceObject<typename GridType :: Traits::template Codim<2>::Entity> EdgeObject;
    typedef MakeableInterfaceObject<typename GridType :: Traits::template Codim< vxCodim >::Entity> VertexObject;

    typedef typename GridType :: LeafIntersectionIteratorImp  LeafIntersectionIteratorImp ;
    typedef typename GridType :: LevelIntersectionIteratorImp LevelIntersectionIteratorImp ;

    // declare friendship 
    friend class ALUGridEntityFactory<FactoryType,0>;
    friend class ALUGridEntityFactory<FactoryType,1>;
    friend class ALUGridEntityFactory<FactoryType,2>;
    friend class ALUGridEntityFactory<FactoryType,3>;

  protected:  
    typedef ALUMemoryProvider< EntityObject > EntityProvider;
    typedef ALUMemoryProvider< FaceObject >   FaceProvider;
    typedef ALUMemoryProvider< EdgeObject >   EdgeProvider;
    typedef ALUMemoryProvider< VertexObject > VertexProvider;

    mutable EntityProvider   entityProvider_;
    mutable FaceProvider     faceProvider_;
    mutable EdgeProvider     edgeProvider_;
    mutable VertexProvider   vertexProvider_;
  
    typedef ALUMemoryProvider< LeafIntersectionIteratorImp > LeafIntersectionIteratorProviderType;
    typedef ALUMemoryProvider< LevelIntersectionIteratorImp >   LevelIntersectionIteratorProviderType;

    mutable LeafIntersectionIteratorProviderType leafInterItProvider_;
    mutable LevelIntersectionIteratorProviderType levelInterItProvider_;

    const GridType& grid_ ;

  private:  
    ALUGridObjectFactory( const ALUGridObjectFactory& other );

  public:
    const  GridType& grid() const { return grid_; }

    ALUGridObjectFactory( const GridType& grid ) : grid_( grid ) {}

    template <int codim> 
    inline MakeableInterfaceObject<typename GridType :: Traits::template Codim<codim>::Entity> * 
    getNewEntity ( int level = -1 ) const
    {
      return ALUGridEntityFactory<FactoryType,codim>::getNewEntity( *this, level);
    }

    template <int codim>
    inline void freeEntity (MakeableInterfaceObject<typename GridType :: Traits::template Codim<codim>::Entity> * en) const
    {
      ALUGridEntityFactory<FactoryType,codim>::freeEntity(*this, en);
    }
  
    LeafIntersectionIteratorImp& getIntersection( const int wLevel, const LeafIntersectionIteratorImp* ) const
    {
      return * (leafInterItProvider_.getObject( *this, wLevel )); 
    }

    LevelIntersectionIteratorImp& getIntersection(const int wLevel, const LevelIntersectionIteratorImp* ) const 
    {
      return * (levelInterItProvider_.getObject( *this, wLevel )); 
    }

    //! free intersection 
    void freeIntersection(LeafIntersectionIteratorImp  & it) const { leafInterItProvider_.freeObject( &it ); }
    void freeIntersection(LevelIntersectionIteratorImp & it) const { levelInterItProvider_.freeObject( &it ); }
  }; /// end class ALUGridObjectFactory

}  // end namespace Dune 
#endif
