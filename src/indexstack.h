// (c) Robert Kloefkorn 2004 - 2005 
#ifndef INDEXSTACK_H_INCLUDED
#define INDEXSTACK_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <assert.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <stack>
#else
  #include <stack.h>
#endif

#ifdef _DUNE_USES_BSGRID_

template<class T, int length>
class FiniteStack {
public :
  // Makes empty stack
  FiniteStack () : _f(0) {}

  // Returns true if the stack is empty
  bool empty () const { return _f==0; }

  // Returns true if the stack is full
  bool full () const { return (_f >= length); }

  // Puts a new object onto the stack
  void push (const T& t) { _s[_f++] = t; }

  // Removes and returns the uppermost object from the stack
  T pop () { return _s[--_f]; }

  // Returns the uppermost object on the stack
  T top () const { return _s[_f-1]; }

  // stacksize
  int size () const { return _f; }

private:
   T   _s[length]; // the stack 
   int _f;         // actual position in stack  
};


//******************************************************
//
//  IndexStack providing indices via getIndex and freeIndex
//  indices that are freed, are put on a stack and get
//
//******************************************************
template <class T, int length> 
class IndexStack 
{
  typedef FiniteStack<T,length> StackType;
  typedef stack < StackType * > StackListType;
  
  StackListType fullStackList_;
  StackListType emptyStackList_;
  
  //typedef typename StackListType::Iterator DListIteratorType;
  StackType * stack_; 

  // current maxIndex 
  int maxIndex_; 
public:
  //! Constructor, create new IndexStack
  IndexStack() : maxIndex_ (0)
  {
    stack_ = new StackType ();
  }

  //! Destructor, deleting all stacks 
  ~IndexStack () 
  {
    if(stack_) 
    {
      delete stack_;
      stack_ = new StackType();
      assert(stack_);
    }

    while( !fullStackList_.empty() )
    {
      StackType * st = fullStackList_.top();
      if(st) delete st; 
      fullStackList_.pop();
    }
    while( !emptyStackList_.empty() )
    {
      StackType * st = emptyStackList_.top();
      if(st) delete st; 
      emptyStackList_.pop();
    }
  }

  //! set index as maxIndex if index is bigger than maxIndex
  void checkAndSetMax(T index)
  {
    if(index > maxIndex_) maxIndex_ = index;
  }
  
  //! set index as maxIndex
  void setMaxIndex(T index)
  {
    maxIndex_ = index;
  }

  //! set index as maxIndex
  int getMaxIndex() const
  {
    return maxIndex_;
  }
  
  //! restore index from stack or create new index 
  T getIndex () 
  {
    if((*stack_).empty()) 
    {
      if( fullStackList_.size() <= 0)
      {
        return maxIndex_++;
      }
      else 
      {
        emptyStackList_.push( stack_ );
        stack_ = fullStackList_.top();
        fullStackList_.pop();
      }
    }
    return (*stack_).pop();
  }

  //! store index on stack 
  void freeIndex(T index)
  {
    if(stack_->full())
    {
      fullStackList_.push(  stack_ );
      if(emptyStackList_.size() <= 0)
      {
        stack_ = new StackType (); 
      }
      else 
      {
        stack_ = emptyStackList_.top();
        emptyStackList_.pop();
      }
    }
    stack_->push(index); 
  }

  //! test stack funtcionality
  void test ()
  {
    T vec[2*length];

    for(int i=0; i<2*length; i++)
      vec[i] = getIndex();

    for(int i=0; i<2*length; i++)
      freeIndex(vec[i]);
    
    for(int i=0; i<2*length; i++)
      vec[i] = getIndex();
    
    for(int i=0; i<2*length; i++)
      printf(" index [%d] = %d \n",i,vec[i]);
  }

  // backup set to out stream 
  void backupIndexSet ( std::ostream & os )
  {
    // holes are not stored at the moment 
    os.write( ((const char *) &maxIndex_ ), sizeof(int) ) ;
    return ;
  }

  // restore from in stream 
  void restoreIndexSet ( std::istream & is )
  {
    is.read ( ((char *) &maxIndex_), sizeof(int) );
    clearStack ();

    return ;
  }
  
private:
  // no copy constructor allowed 
  IndexStack( const IndexStack<T,length> & s) : maxIndex_ (0) , stack_(0) {}
 
  // no assignment operator allowed 
  IndexStack<T,length> & operator = ( const IndexStack<T,length> & s) 
  {
    cout << "IndexStack::operator = () not allowed! in: " __FILE__ << " line:" << __LINE__ << "\n";
    abort();
    return *this; 
  }
  
  // clear all stored indices 
  void clearStack () 
  {
    if(stack_) 
    {
      delete stack_;
      stack_ = new StackType();
      assert(stack_);
    }

    while( !fullStackList_.empty() )
    {
      StackType * st = fullStackList_.top();
      if(st) delete st; 
      fullStackList_.pop();
    }
    return;
  }
};  // end class IndexStack 

#else 

//*********************************************************************
//
// Dummy implementation for the index stack, if index is not used 
//
//*********************************************************************
template <class T>
struct DummyIndexStack 
{
  //! set index as maxIndex if index is bigger than maxIndex
  inline void checkAndSetMax(T index)
  {
  }
  
  //! set index as maxIndex
  inline void setMaxIndex(T index)
  {
  }
  
  //! restore index from stack or create new index 
  T getIndex () 
  {
    return -1;
  }

  //! store index on stack 
  inline void freeIndex(T index)
  {
  }

  //! test stack funtcionality
  inline void test ()
  {
  }
};  // end class DummyIndexStack 

#endif 

#endif
