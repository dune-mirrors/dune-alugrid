#ifndef DUNE_ALU3DGRIDMEMORY_HH
#define DUNE_ALU3DGRIDMEMORY_HH

#include <dune/alugrid/common/alugrid_assert.hh>
#include <cstdlib>
#include <vector>

namespace ALUGrid
{

  template< class T, int length >
  class ALUGridFiniteStack;

  //! organize the memory management for entitys used by the NeighborIterator
  template <class Object>
  class ALUMemoryProvider
  {
    enum { maxStackObjects = 256 };
    typedef ::ALUGrid::ALUGridFiniteStack< Object *, maxStackObjects > StackType;

    // stack to store object pointers
    StackType objStack_;

    // return reference to object stack
    StackType &objStack () { return objStack_; }
  public:
    // type of object to be stored
    typedef Object ObjectType;

    //! default constructor
    ALUMemoryProvider()
      : objStack_()
    {}

    //! copy constructor
    ALUMemoryProvider( const ALUMemoryProvider& org )
      : objStack_()
    {}

    //! call deleteEntity
    ~ALUMemoryProvider ();

    //! i.e. return pointer to Entity
    template <class FactoryType>
    ObjectType * getObject(const FactoryType &factory, int level);

    //! i.e. return pointer to Entity
    template <class FactoryType, class EntityImp>
    inline ObjectType * getEntityObject(const FactoryType& factory, int level, EntityImp* )
    {
      if( objStack().empty() )
      {
        return new ObjectType( EntityImp(factory,level) );
      }
      else
      {
        return stackObject();
      }
    }

    //! return object, if created default constructor is used
    ObjectType* getEmptyObject ();

    //! free, move element to stack, returns NULL
    void freeObject (ObjectType * obj);

  protected:
    inline ObjectType * stackObject()
    {
      // make sure stack is not empty
      alugrid_assert ( ! objStack().empty() );
      // finite stack does also return object on pop
      return objStack().pop();
    }
  };


  //************************************************************************
  //
  //  ALUMemoryProvider implementation
  //
  //************************************************************************
  template <class Object> template <class FactoryType>
  inline typename ALUMemoryProvider<Object>::ObjectType*
  ALUMemoryProvider<Object>::
  getObject( const FactoryType &factory, int level )
  {
    if( objStack().empty() )
    {
      return ( new Object (factory, level) );
    }
    else
    {
      return stackObject();
    }
  }

  template <class Object>
  inline typename ALUMemoryProvider<Object>::ObjectType *
  ALUMemoryProvider<Object>::getEmptyObject ()
  {
    if( objStack().empty() )
    {
      return new Object () ;
    }
    else
    {
      return stackObject();
    }
  }

  template <class Object>
  inline ALUMemoryProvider<Object>::~ALUMemoryProvider()
  {
    StackType& objStk = objStack();
    while ( ! objStk.empty() )
    {
      ObjectType * obj = objStk.pop();
      delete obj;
    }
  }

  template <class Object>
  inline void ALUMemoryProvider<Object>::freeObject( Object * obj )
  {
    // make sure we operate on the correct thread
    StackType& stk = objStack();
    if( stk.full() )
      delete obj;
    else
      stk.push( obj );
  }

  template <class ObjectImp>
  class ReferenceCountedObject
  {
  protected:
    // type of object to be reference counted
    typedef ObjectImp    ObjectType;

    // object (e.g. geometry impl or intersection impl)
    ObjectType object_;

    unsigned int& refCount() { return object_.refCount_; }
    const unsigned int& refCount() const { return object_.refCount_; }

  public:
    //! reset status and reference count
    void reset()
    {
      // reset reference counter
      refCount() = 1;

      // reset status of object
      object_.invalidate();
    }

    //! increase reference count
    void operator ++ () { ++ refCount(); }

    //! decrease reference count
    void operator -- () { alugrid_assert ( refCount() > 0 ); --refCount(); }

    //! return true if object has no references anymore
    bool operator ! () const { return refCount() == 0; }

    //! return true if there exists more then on reference
    bool unique () const { return refCount() == 1 ; }

    const ObjectType& object() const { return object_; }
          ObjectType& object()       { return object_; }
  };

  template <class ObjectImp>
  class SharedPointer
  {
  protected:
    typedef ObjectImp  ObjectType;
    typedef ReferenceCountedObject< ObjectType >              ReferenceCountedObjectType;
    typedef ALUMemoryProvider< ReferenceCountedObjectType >   MemoryPoolType;

    static MemoryPoolType& memoryPool()
    {
      static thread_local MemoryPoolType pool;
      return pool;
    }

  public:
    // default constructor
    SharedPointer()
    {
      getObject();
    }

    // copy contructor making shallow copy
    SharedPointer( const SharedPointer& other )
    {
      assign( other );
    }

    // destructor clearing pointer
    ~SharedPointer()
    {
      removeObject();
    }

    void getObject()
    {
      ptr_ = memoryPool().getEmptyObject();
      ptr().reset();
    }

    void assign( const SharedPointer& other )
    {
      // copy pointer
      ptr_ = other.ptr_;

      // increase reference counter
      ++ ptr();
    }

    void removeObject()
    {
      // decrease reference counter
      -- ptr();

      // if reference count is zero free the object
      if( ! ptr() )
      {
        memoryPool().freeObject( ptr_ );
      }

      // reset pointer
      ptr_ = nullptr;
    }

    void invalidate()
    {
      // if pointer is unique, invalidate status
      if( ptr().unique() )
      {
        ptr().object().invalidate();
      }
      else
      {
        // if pointer is used elsewhere remove the pointer
        // and get new object
        removeObject();
        getObject();
      }
    }

    SharedPointer& operator = ( const SharedPointer& other )
    {
      if( ptr_ != other.ptr_ )
      {
        removeObject();
        assign( other );
      }
      return *this;
    }

    operator bool () const { return bool( ptr_ ); }

    bool operator == (const SharedPointer& other ) const { return ptr_ == other.ptr_; }

    bool unique () const { return ptr().unique(); }

    // dereferencing
          ObjectType& operator* ()       { return ptr().object(); }
    const ObjectType& operator* () const { return ptr().object(); }

  protected:
    ReferenceCountedObjectType& ptr() { alugrid_assert( ptr_ ); return *ptr_; }
    const ReferenceCountedObjectType& ptr() const { alugrid_assert( ptr_ ); return *ptr_; }

    ReferenceCountedObjectType* ptr_;
  };

} // namespace ALUGrid

#endif // #ifndef DUNE_ALU3DGRIDMEMORY_HH
