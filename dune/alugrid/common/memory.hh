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
  class ALUMemoryProvider
  {
    enum { maxStackObjects = 256 };
    typedef ::ALUGrid::ALUGridFiniteStack< Object *, maxStackObjects > StackType;

    std::vector< StackType > objStackVector_;

    typedef ALUMemoryProvider < Object > MyType;

    StackType &objStack () 
    {
      alugrid_assert( thread() < int(objStackVector_.size()) );
      return objStackVector_[ thread() ]; 
    }

  public:
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
    static inline int maxThreads() {
#ifdef _OPENMP
      return omp_get_max_threads();
#elif HAVE_DUNE_FEM 
      return Fem :: ThreadManager :: maxThreads() ;
#else
      return 1;
#endif
    }

    typedef Object ObjectType;

    //! default constructor 
    ALUMemoryProvider() : objStackVector_( maxThreads() ) {}

    //! copy constructor 
    ALUMemoryProvider( const ALUMemoryProvider& org ) : objStackVector_( maxThreads() ) {}

    //! call deleteEntity 
    ~ALUMemoryProvider ();

    //! i.e. return pointer to Entity
    template <class FactoryType>
    ObjectType * getObject(const FactoryType &factory, int level);

    //! i.e. return pointer to Entity
    template <class FactoryType, class EntityImp>
    inline ObjectType * getEntityObject(const FactoryType& factory, int level , EntityImp * fakePtr ) 
    {
      if( objStack().empty() )
      {
        return ( new ObjectType(EntityImp(factory,level) )); 
      }
      else
      {
        return stackObject();
      }
    }

    //! return object, if created default constructor is used 
    ObjectType * getEmptyObject ();

    //! i.e. return pointer to Entity
    ObjectType * getObjectCopy(const ObjectType & org);

    //! free, move element to stack, returns NULL 
    void freeObject (ObjectType * obj);

  protected:
    inline ObjectType * stackObject() 
    {
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
  inline typename ALUMemoryProvider<Object>::ObjectType * 
  ALUMemoryProvider<Object>::getObject
  (const FactoryType &factory, int level )
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
  ALUMemoryProvider<Object>::getObjectCopy
  (const ObjectType & org )
  {
    if( objStack().empty() )
    {
      return ( new Object (org) ); 
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
    if( thread() == 0 ) 
    {
      const int threads = maxThreads();
      for( int i=0; i<threads; ++i) 
      {
        StackType& objStk = objStackVector_[ i ];
        while ( ! objStk.empty() )
        {
          ObjectType * obj = objStk.pop();
          delete obj;
        }
      }
    }
  }

  template <class Object>
  inline void ALUMemoryProvider<Object>::freeObject(Object * obj)
  {
    StackType& stk = objStack();
    if( stk.full() ) 
      delete obj;
    else 
      stk.push( obj );
  }

} // namespace Dune

#endif // #ifndef DUNE_ALU3DGRIDMEMORY_HH
