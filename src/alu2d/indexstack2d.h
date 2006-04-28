// (c) Robert Kloefkorn 2004 - 2005 
#ifndef INDEXSTACK2D_H_INCLUDED
#define INDEXSTACK2D_H_INCLUDED

#include <assert.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <stack>
#else
  #include <stack.h>
#endif

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
//  IndexStack2d providing indices via getIndex and freeIndex
//  indices that are freed, are put on a stack and get
//
//******************************************************
template <class T, int length> 
class IndexStack2d 
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
  //! Constructor, create new IndexStack2d
  IndexStack2d(); 

  //! Destructor, deleting all stacks 
  inline ~IndexStack2d (); 

  //! set index as maxIndex if index is bigger than maxIndex
  void checkAndSetMax(T index) { if(index > maxIndex_) maxIndex_ = index;  }
  
  //! set index as maxIndex
  void setMaxIndex(T index) { maxIndex_ = index; }

  //! returns the larges index used + 1, actually this is the size of the
  //! index set 
  int getMaxIndex() const { return maxIndex_;  }

  int size() const {return getMaxIndex();}
  
  //! restore index from stack or create new index 
  T getIndex (); 

  //! store index on stack 
  void freeIndex(T index);

  //! test stack funtcionality
  void test ();

  //! get max. used index
  int usedindex ();

  // backup set to out stream 
  void backupIndexSet ( ostream & os ); 

  // restore from in stream 
  void restoreIndexSet ( istream & is );
private:
  // no copy constructor allowed 
  IndexStack2d( const IndexStack2d<T,length> & s);
 
  // no assignment operator allowed 
  IndexStack2d<T,length> & operator = ( const IndexStack2d<T,length> & s);
  
  // clear all stored indices 
  void clearStack ();
};  // end class IndexStack2d 

//****************************************************************
// Inline implementation 
// ***************************************************************
template <class T, int length>
inline IndexStack2d<T,length>::IndexStack2d()
  : stack_ ( new StackType () ) , maxIndex_ (0) {} 
  
template <class T, int length>
inline IndexStack2d<T,length>::~IndexStack2d () 
{
  assert(usedindex()==0);
  if(stack_) delete stack_;
  stack_ = 0;

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

template <class T, int length>
inline T IndexStack2d<T,length>::getIndex () 
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

template <class T, int length>
inline void IndexStack2d<T,length>::freeIndex ( T index ) 
{
  if((*stack_).full())
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
  (*stack_).push(index); 
}

template <class T, int length>
inline void IndexStack2d<T,length>::test () 
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
template <class T, int length>
  inline int IndexStack2d<T,length>::usedindex () 
{
  return maxIndex_-(fullStackList_.size()*length+stack_->size());
}

template <class T, int length>
inline void IndexStack2d<T,length>::backupIndexSet ( ostream & os ) 
{
  // holes are not stored at the moment 
  os.write( ((const char *) &maxIndex_ ), sizeof(int) ) ;
  return ;
}

template <class T, int length>
inline void IndexStack2d<T,length>::restoreIndexSet ( istream & is )
{
  is.read ( ((char *) &maxIndex_), sizeof(int) );
  clearStack ();

  return ;
}

template <class T, int length>
inline void IndexStack2d<T,length>::clearStack () 
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
#endif
