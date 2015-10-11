#ifndef DUNE_ALU3DGRIDMEMORY_HH
#define DUNE_ALU3DGRIDMEMORY_HH

#include <dune/alugrid/common/alugrid_assert.hh>
#include <cstdlib>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#if HAVE_DUNE_FEM
#include <dune/fem/misc/threads/threadmanager.hh>
#endif

namespace ALUGrid
{

  template< class T, int length >
  class ALUGridFiniteStack;

  //! organize the memory management for entitys used by the NeighborIterator
  template <class Object>
  class ALUMemoryProviderSingleThread
  {
    enum { maxStackObjects = 256 };
    typedef ::ALUGrid::ALUGridFiniteStack< Object *, maxStackObjects > StackType;

    // stack to store object pointers
    StackType objStack_;

    // thread number
    int thread_;

    // return reference to object stack
    StackType &objStack () { return objStack_; }
  public:
    // type of object to be stored
    typedef Object ObjectType;

    // return thread number
    static inline int thread()
    {
#ifdef _OPENMP
      return omp_get_thread_num();
#elif HAVE_DUNE_FEM
      return Fem :: ThreadManager :: thread() ;
#else
      return 0;
#endif
    }

    // return maximal possible number of threads
    static inline int maxThreads()
    {
#ifdef _OPENMP
      return omp_get_max_threads();
#elif HAVE_DUNE_FEM
      return Fem :: ThreadManager :: maxThreads() ;
#else
      return 1;
#endif
    }

    //! default constructor
    ALUMemoryProviderSingleThread()
      : objStack_(), thread_( -1 )
    {}

    //! copy constructor
    ALUMemoryProviderSingleThread( const ALUMemoryProviderSingleThread& org )
      : objStack_(), thread_( org.thread_ )
    {}

    //! set thread number this memory provider works for
    void setThreadNumber( const int thread ) { thread_ = thread; }

    //! call deleteEntity
    ~ALUMemoryProviderSingleThread ();

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
      // make sure we operate on the correct thread
      alugrid_assert ( thread_ == thread() );
      // make sure stack is not empty
      alugrid_assert ( ! objStack().empty() );
      // finite stack does also return object on pop
      return objStack().pop();
    }
  };


  //************************************************************************
  //
  //  ALUMemoryProviderSingleThread implementation
  //
  //************************************************************************
  template <class Object> template <class FactoryType>
  inline typename ALUMemoryProviderSingleThread<Object>::ObjectType*
  ALUMemoryProviderSingleThread<Object>::
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
  inline typename ALUMemoryProviderSingleThread<Object>::ObjectType *
  ALUMemoryProviderSingleThread<Object>::getEmptyObject ()
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
  inline ALUMemoryProviderSingleThread<Object>::~ALUMemoryProviderSingleThread()
  {
    StackType& objStk = objStack();
    while ( ! objStk.empty() )
    {
      ObjectType * obj = objStk.pop();
      delete obj;
    }
  }

  template <class Object>
  inline void ALUMemoryProviderSingleThread<Object>::freeObject( Object * obj )
  {
    // make sure we operate on the correct thread
    alugrid_assert ( thread_ == thread() );
    StackType& stk = objStack();
    if( stk.full() )
      delete obj;
    else
      stk.push( obj );
  }

  //! organize the memory management for entitys used by the NeighborIterator
  template <class Object>
  class ALUMemoryProvider
  {
    typedef ALUMemoryProviderSingleThread < Object > MemoryProvider ;

    std::vector< MemoryProvider > memProviders_;

    MemoryProvider& memProvider( const unsigned int thread )
    {
      alugrid_assert( thread < memProviders_.size() );
      return memProviders_[ thread ];
    }

    void init ()
    {
      const int threads = maxThreads();
      for( int thread = 0; thread < threads; ++ thread )
      {
        memProviders_[ thread ].setThreadNumber( thread );
      }
    }

  public:
    // return thread number
    static inline int thread() { return MemoryProvider :: thread(); }

    // return maximal possible number of threads
    static inline int maxThreads() { return MemoryProvider :: maxThreads(); }

    // type of stored object
    typedef Object ObjectType;

    //! default constructor
    ALUMemoryProvider() : memProviders_( maxThreads() )
    {
      init();
    }

    //! copy constructor (don't copy memory providers)
    ALUMemoryProvider( const ALUMemoryProvider& org ) : memProviders_( maxThreads() )
    {
      init();
    }

    //! i.e. return pointer to Entity
    template <class FactoryType>
    ObjectType * getObject(const FactoryType &factory, int level)
    {
      return memProvider( thread() ).getObject( factory, level );
    }

    //! i.e. return pointer to Entity
    template <class FactoryType, class EntityImp>
    inline ObjectType * getEntityObject(const FactoryType& factory, int level , EntityImp * fakePtr )
    {
      return memProvider( thread() ).getEntityObject( factory, level, fakePtr );
    }

    //! return object, if created default constructor is used
    ObjectType * getEmptyObject () { return memProvider( thread() ).getEmptyObject(); }

    //! free, move element to stack, returns NULL
    void freeObject (ObjectType * obj) { memProvider( thread() ).freeObject( obj ); }
  };

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
      static MemoryPoolType pool;
      return pool;
    }

  public:
    // default constructor
    SharedPointer()
    {
      /*
      static bool first = true ;
      if( first )
      {
        std::cout << "Memory object = " << sizeof( ObjectType ) << " "
                  << sizeof( ReferenceCountedObjectType ) << std::endl;
        first = false;
      }
      */
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
