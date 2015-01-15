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

}

namespace Dune
{
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

} // namespace Dune

#endif // #ifndef DUNE_ALU3DGRIDMEMORY_HH
