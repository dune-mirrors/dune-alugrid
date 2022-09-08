#ifndef ALU3DGRID_ENTITYKEY_HH
#define ALU3DGRID_ENTITYKEY_HH

#include "alu3dinclude.hh"

namespace Dune
{

// Forward declarations
template<int cd, class GridImp>
class ALU3dGridEntitySeed ;

template<int cd, int dim, class GridImp>
class ALU3dGridEntity;

//**********************************************************************
//
// --ALU3dGridEntitySeed
// --EntitySeed
//**********************************************************************
template< int codim, class GridImp >
class ALU3dGridEntitySeedBase
{
protected:
  typedef ALU3dGridEntitySeedBase< codim, GridImp > ThisType;
  enum { dim       = GridImp::dimension };
  enum { dimworld  = GridImp::dimensionworld };

  typedef typename GridImp::MPICommunicatorType Comm;

  typedef ALU3dImplTraits< GridImp::elementType, Comm > ImplTraits;
  typedef typename ImplTraits::template Codim<dim, codim>::ImplementationType ImplementationType;
  typedef typename ImplTraits::template Codim<dim, codim>::InterfaceType      HElementType;
  typedef typename ImplTraits::template Codim<dim, codim>::EntitySeedType     KeyType ;

  typedef typename ImplTraits::BNDFaceType BNDFaceType;
  typedef typename ImplTraits::HBndSegType HBndSegType;

  template <int cd, class Key>
  struct Bnd
  {
    static Key* toKey(const HBndSegType*)
    {
      return nullptr;
    }
    static HElementType* getItem(KeyType* key)
    {
      return static_cast< HElementType* > ( key );
    }
    static bool isGhost(KeyType*) { return false; }
    static BNDFaceType* ghost( KeyType*  ) { return nullptr; }
  };
  template <class Key>
  struct Bnd<0, Key>
  {
    static Key* toKey(const HBndSegType* ghostFace)
    {
      return static_cast< KeyType* > (const_cast< BNDFaceType* >( static_cast<const BNDFaceType*> (ghostFace)));
    }
    static HElementType* getItem(KeyType* key)
    {
      if( key )
      {
        if( key->isboundary() )
        {
          return ((static_cast< BNDFaceType* > ( key ))->getGhost().first);
        }
        else
        {
          // we cannot cast to HElement here, since only the implementation is derived
          // from hasFace
          return static_cast< HElementType * > (static_cast< ImplementationType* > (key));
        }
      }
      else
        return nullptr;
    }
    static bool isGhost(KeyType* key) { alugrid_assert ( key ); return key->isboundary(); }
    static BNDFaceType* ghost( KeyType* key ) { alugrid_assert ( key ); return (static_cast< BNDFaceType* > ( key )); }
  };
public:
  static const int defaultValue = -1 ;
  static const int defaultTwist = 0 ;

  enum { codimension = codim };

  //! type of Entity
  typedef typename GridImp::template Codim<codimension>::Entity   Entity;
  //! underlying EntityImplementation
  typedef typename GridImp::template Codim<codimension>::EntityImp  EntityImp;

  //! typedef of my type
  typedef ThisType ALU3dGridEntitySeedType;

  //! make type of entity pointer implementation available in derived classes
  typedef ALU3dGridEntitySeed<codimension,GridImp> EntitySeedImp;

  //! Destructor
  ~ALU3dGridEntitySeedBase()
  {
#ifdef ALUGRIDDEBUG
    // clear pointer
    clear();
#endif
  }

  //! Constructor for EntitySeed that points to an element
  ALU3dGridEntitySeedBase();

  //! Constructor for EntitySeed that points to an element
  ALU3dGridEntitySeedBase(const HElementType& item);

  //! Constructor for EntitySeed that points to an element
  ALU3dGridEntitySeedBase(const HElementType* item, const HBndSegType* ghostFace );

  //! Constructor for EntitySeed that points to an ghost
  ALU3dGridEntitySeedBase(const HBndSegType& ghostFace );

  /////////////////////////////////////////////////////////////
  //
  //  interface methods
  //
  /////////////////////////////////////////////////////////////
  //! copy constructor
  ALU3dGridEntitySeedBase(const ALU3dGridEntitySeedType & org);

  bool isValid () const { return bool( item_ ); }

  //! equality operator
  bool operator == (const ALU3dGridEntitySeedType& i) const
  {
    return equals( i );
  }

  //! inequality operator
  bool operator != (const ALU3dGridEntitySeedType& i) const
  {
    return ! equals( i );
  }

  //! assignment operator
  ThisType & operator = (const ThisType & org);

  //////////////////////////////////////////////////////
  //
  //  non-interface methods
  //
  //////////////////////////////////////////////////////
  //! equality
  bool equals (const ALU3dGridEntitySeedType& i) const;

  //! invalidate seed
  void clear()
  {
    item_ = nullptr;
  }

  //! get item from key
  HElementType* item() const { return Bnd<codim,KeyType>::getItem( item_ ); }

  //! return iterior item
  HElementType* interior() const
  {
    alugrid_assert ( ! isGhost() );
    return static_cast< HElementType * > (static_cast< ImplementationType* > (item_));
  }

  //! methods for ghosts
  bool isGhost() const { return Bnd<codim,KeyType>::isGhost( item_ ); }
  BNDFaceType* ghost() const
  {
    alugrid_assert ( isGhost() );
    return Bnd<codim,KeyType>::ghost( item_ );
  }

  KeyType* toKey(const HElementType* item)
  {
    return static_cast< KeyType* > (const_cast< ImplementationType* > (static_cast<const ImplementationType* > (item)));
  }

  void set(const HElementType& item)
  {
    item_ = toKey( &item );
  }

  KeyType* toKey( const HBndSegType* ghostFace )
  {
    return Bnd<codim,KeyType>::toKey( ghostFace );
  }

  void set(const HBndSegType& ghostFace)
  {
    item_ = toKey( &ghostFace );
  }

  int level () const
  {
    alugrid_assert( isValid() );
    return item()->level();
  }

  int twist () const { return defaultTwist; }

protected:
  // pointer to item
  mutable KeyType* item_;
};

template<int cd, class GridImp>
class ALU3dGridEntitySeed :
public ALU3dGridEntitySeedBase<cd,GridImp>
{
  typedef ALU3dGridEntitySeedBase<cd,GridImp> BaseType;

  typedef ALU3dGridEntitySeed <cd,GridImp> ThisType;
  enum { dim       = GridImp::dimension };
  enum { dimworld  = GridImp::dimensionworld };

  typedef typename GridImp::MPICommunicatorType Comm;

  typedef ALU3dImplTraits< GridImp::elementType, Comm > ImplTraits;
  typedef typename ImplTraits::template Codim<dim, cd>::ImplementationType ImplementationType;
  typedef typename ImplTraits::template Codim<dim, cd>::InterfaceType HElementType;

  typedef typename ImplTraits::BNDFaceType BNDFaceType;
  typedef ALU3dGridEntity<cd,dim,GridImp> ALU3dGridEntityType;

public:
  using BaseType :: defaultValue ;
  using BaseType :: defaultTwist ;

  //! type of Entity
  typedef typename GridImp::template Codim<cd>::Entity Entity;

  //! typedef of my type
  typedef ALU3dGridEntitySeed<cd,GridImp> ALU3dGridEntitySeedType;

  //! Constructor for EntitySeed that points to an element
  //! this constructor should only be called by codim=0 entity keys
  ALU3dGridEntitySeed(const ImplementationType & item) = delete;

  //! Constructor for EntitySeed that points to an element
  ALU3dGridEntitySeed(const HElementType & item,
                      const int level,
                      const int twist = defaultTwist
                    );

  //! Constructor for EntitySeed that points to an element
  ALU3dGridEntitySeed()
    : BaseType(), level_(defaultValue), twist_(defaultTwist)
  {}

  //! Constructor for EntitySeed that points to given entity
  ALU3dGridEntitySeed(const ALU3dGridEntityType& entity)
    : ALU3dGridEntitySeedBase<cd,GridImp> (entity.getItem()),
      level_(entity.level()),
      twist_(defaultTwist)
  {}

  //! copy constructor
  ALU3dGridEntitySeed(const ALU3dGridEntitySeedType & org);

  //! assignment operator
  ThisType & operator = (const ThisType & org);

  //! clear the key data structure
  void clear();

  //! set element and level
  void set(const HElementType & item, const int level )
  {
    BaseType :: set( item );
    level_ = level ;
  }

  //! return level
  int level () const { return level_ ; }
  //! return twist
  int twist () const { return twist_ ; }

  using BaseType :: set ;

  bool operator == (const ALU3dGridEntitySeedType& i) const
  {
    return equals( i );
  }

  bool operator != (const ALU3dGridEntitySeedType& i) const
  {
    return ! equals( i );
  }

  //! equality, calls BaseType equals
  bool equals (const ALU3dGridEntitySeedType& key) const
  {
    // only compare the item pointer, this is the real key
    return BaseType :: equals( key ) && (level() == key.level());
  }

protected:
  // level of entity
  int level_;
  // twist of face, for codim 1 only
  int twist_;
};

//! ALUGridEntitySeed points to an entity
//! this class is the specialisation for codim 0,
//! it has exactly the same functionality as the ALU3dGridEntitySeedBase
template<class GridImp>
class ALU3dGridEntitySeed<0,GridImp> :
public ALU3dGridEntitySeedBase<0,GridImp>
{
protected:
  typedef ALU3dGridEntitySeedBase<0,GridImp> BaseType;

  enum { cd = 0 };
  typedef ALU3dGridEntitySeed <cd,GridImp> ThisType;
  enum { dim       = GridImp::dimension };
  enum { dimworld  = GridImp::dimensionworld };

  typedef typename GridImp::MPICommunicatorType Comm;

  typedef ALU3dImplTraits< GridImp::elementType, Comm > ImplTraits;
  typedef typename ImplTraits::template Codim<dim, cd>::ImplementationType ImplementationType;
  typedef typename ImplTraits::template Codim<dim, cd>::InterfaceType      HElementType;

  typedef typename ImplTraits::BNDFaceType BNDFaceType;
  typedef typename ImplTraits::HBndSegType HBndSegType;

  typedef ALU3dGridEntity< 0,dim,GridImp> ALU3dGridEntityType ;

public:
  using BaseType :: defaultValue ;
  using BaseType :: defaultTwist ;

  //! type of Entity
  typedef typename GridImp::template Codim<cd>::Entity Entity;

  //! typedef of my type
  typedef ThisType ALU3dGridEntitySeedType;

  //! Constructor for EntitySeed that points to an element
  ALU3dGridEntitySeed() : BaseType() {}

  //! Constructor for EntitySeed that points to an interior element
  ALU3dGridEntitySeed(const HElementType& item)
    : ALU3dGridEntitySeedBase<cd,GridImp> (item) {}

  //! Constructor for EntitySeed that points to an interior element
  ALU3dGridEntitySeed(const HElementType& item, int , int )
    : ALU3dGridEntitySeedBase<cd,GridImp> (item) {}

  //! Constructor for EntitySeed that points to an ghost
  ALU3dGridEntitySeed(const HBndSegType& ghostFace )
    : ALU3dGridEntitySeedBase<cd,GridImp> ( ghostFace ) {}

  //! copy constructor
  ALU3dGridEntitySeed(const ALU3dGridEntitySeedType & org)
    : ALU3dGridEntitySeedBase<cd,GridImp> (org)
  {
  }

  //! assignment operator
  ThisType & operator = (const ThisType & org)
  {
    BaseType :: operator = (org);
    return *this;
  }
};


//! print alugrid entity key to std::stream
template <int cd, class GridImp>
inline std :: ostream &operator<< ( std :: ostream &out,
                                    const ALU3dGridEntitySeed<cd,GridImp>& key)
{
  out << key.item() << " " << key.level() << " " << key.twist() << " " << key.face();
  return out;
}


//*******************************************************************
//
//  Implementation
//
//*******************************************************************
template<int codim, class GridImp >
inline ALU3dGridEntitySeedBase<codim,GridImp> ::
ALU3dGridEntitySeedBase()
  : item_( 0 )
{
}

template<int codim, class GridImp >
inline ALU3dGridEntitySeedBase<codim,GridImp> ::
ALU3dGridEntitySeedBase(const HElementType &item)
  : item_( toKey(&item) )
{
}

template<int codim, class GridImp >
inline ALU3dGridEntitySeedBase<codim,GridImp> ::
ALU3dGridEntitySeedBase(const HBndSegType& ghostFace )
  : item_( toKey(&ghostFace) )
{
}

template<int codim, class GridImp >
inline ALU3dGridEntitySeedBase<codim,GridImp> ::
ALU3dGridEntitySeedBase(const ALU3dGridEntitySeedType & org)
  : item_(org.item_)
{
}

template<int codim, class GridImp >
inline ALU3dGridEntitySeedBase<codim,GridImp> &
ALU3dGridEntitySeedBase<codim,GridImp> ::
operator = (const ALU3dGridEntitySeedType & org)
{
  item_  = org.item_;
  return *this;
}

template<int codim, class GridImp >
inline bool ALU3dGridEntitySeedBase<codim,GridImp>::
equals (const ALU3dGridEntitySeedBase<codim,GridImp>& i) const
{
  // check equality of underlying items
  return (item_ == i.item_);
}

///////////////////////////////////////////////////////////////////
//
//  specialisation for higher codims
//
///////////////////////////////////////////////////////////////////

template<int codim, class GridImp >
inline ALU3dGridEntitySeed<codim,GridImp> ::
ALU3dGridEntitySeed(const HElementType &item,
                   const int level,
                   const int twist )
  : ALU3dGridEntitySeedBase<codim,GridImp> (item)
  , level_(level)
  , twist_ (twist)
{
}

template<int codim, class GridImp >
inline ALU3dGridEntitySeed<codim,GridImp> ::
ALU3dGridEntitySeed(const ALU3dGridEntitySeedType & org)
  : ALU3dGridEntitySeedBase<codim,GridImp>(org)
  , level_(org.level_)
  , twist_(org.twist_)
{
}

template<int codim, class GridImp >
inline ALU3dGridEntitySeed<codim,GridImp> &
ALU3dGridEntitySeed<codim,GridImp>::
operator = (const ALU3dGridEntitySeedType & org)
{
  // docu and cleanup
  BaseType :: operator = ( org );

  // clone other stuff
  level_ = org.level_;
  twist_ = org.twist_;
  return *this;
}

template<int codim, class GridImp >
inline void
ALU3dGridEntitySeed<codim,GridImp>::clear ()
{
  BaseType :: clear();
  level_ = defaultValue ;
  twist_ = defaultTwist ;
}

} // end namespace Dune
#endif
