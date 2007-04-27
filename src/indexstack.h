// (c) Robert Kloefkorn 2004 - 2007 
#ifndef ALUGRIDINDEXSTACK_H_INCLUDED
#define ALUGRIDINDEXSTACK_H_INCLUDED

#include <assert.h>

namespace ALUGridSpace {
  
// using namespace std has always to be called inside the namespace
// ALUGridSpace 
#ifdef _ANSI_HEADER
  using namespace std;
  #include <stack>
#else
  #include <stack.h>
#endif

template<class T, int length>
class ALUGridFiniteStack {
public :
  // Makes empty stack
  ALUGridFiniteStack () : _f(0) {}

  // Returns true if the stack is empty
  bool empty () const { return _f <= 0; }

  // Returns true if the stack is full
  bool full () const { return (_f >= length); }

  // Puts a new object onto the stack
  void push (const T& t) { 
    assert( _f < length );
    _s[_f++] = t; }

  // Removes and returns the uppermost object from the stack
  T pop () { 
    assert( _f > 0 );
    return _s[--_f]; 
  }

  // Returns the uppermost object on the stack
  T top () const { 
    assert( _f > 0 );
    return _s[_f-1]; 
  }

  // stacksize
  int size () const { return _f; }

  // backup stack to ostream 
  void backup ( ostream & os ) const 
  {
    os.write( ((const char *) &_f ), sizeof(int) ) ;
    for(int i=0; i<size(); ++i)
    {
      os.write( ((const char *) &_s[i] ), sizeof(int) ) ;
    }
  }
   
  // restore stack from istream 
  void restore ( istream & is )  
  {
    is.read ( ((char *) &_f), sizeof(int) );
    assert( _f >= 0 );
    assert( _f < length );
    for(int i=0; i<size(); ++i)
    {
      is.read ( ((char *) &_s[i]), sizeof(int) );
    }
  }
    
private:
   T   _s[length]; // the stack 
   int _f;         // actual position in stack  
};


//******************************************************
//
//  ALUGridIndexStack providing indices via getIndex and freeIndex
//  indices that are freed, are put on a stack and get
//
//******************************************************
template <class T, int length> 
class ALUGridIndexStack 
{
  typedef ALUGridFiniteStack<T,length> StackType;
  typedef stack < StackType * > StackListType;
  
  StackListType fullStackList_;
  StackListType emptyStackList_;
  
  //typedef typename StackListType::Iterator DListIteratorType;
  StackType * stack_; 

  // current maxIndex 
  int maxIndex_; 
public:
  //! Constructor, create new ALUGridIndexStack
  ALUGridIndexStack(); 

  //! Destructor, deleting all stacks 
  inline ~ALUGridIndexStack (); 

  //! set index as maxIndex if index is bigger than maxIndex
  void checkAndSetMax(T index) { if(index > maxIndex_) maxIndex_ = index;  }
  
  //! set index as maxIndex
  void setMaxIndex(T index) { maxIndex_ = index; }

  //! returns the larges index used + 1, actually this is the size of the
  //! index set 
  int getMaxIndex() const { return maxIndex_;  }
  
  //! restore index from stack or create new index 
  T getIndex (); 

  //! store index on stack 
  void freeIndex(T index);

  //! test stack functionality
  void test ();

  // backup set to out stream 
  void backupIndexSet ( ostream & os ); 

  // restore from in stream 
  void restoreIndexSet ( istream & is );

  // all entries in vector with value true 
  // are inserted as holes 
  void generateHoles(const vector<bool> & isHole);

private:
  // no copy constructor allowed 
  ALUGridIndexStack( const ALUGridIndexStack<T,length> & s);
 
  // no assignment operator allowed 
  ALUGridIndexStack<T,length> & operator = ( const ALUGridIndexStack<T,length> & s);
  
  // clear all stored indices 
  void clearStack ();
};  // end class ALUGridIndexStack 

//****************************************************************
// Inline implementation 
// ***************************************************************
template <class T, int length>
inline ALUGridIndexStack<T,length>::ALUGridIndexStack()
  : stack_ ( new StackType () ) , maxIndex_ (0) {} 
  
template <class T, int length>
inline ALUGridIndexStack<T,length>::~ALUGridIndexStack () 
{
  if(stack_) delete stack_;
  stack_ = 0;

  while( !fullStackList_.empty() )
  {
    StackType * st = fullStackList_.top();
    fullStackList_.pop();
    delete st; 
  }
  while( !emptyStackList_.empty() )
  {
    StackType * st = emptyStackList_.top();
    emptyStackList_.pop();
    delete st; 
  }
}

template <class T, int length>
inline T ALUGridIndexStack<T,length>::getIndex () 
{
  if((*stack_).empty()) 
  {
    if( fullStackList_.empty() )
    {
      assert( fullStackList_.size() <= 0 );
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
inline void ALUGridIndexStack<T,length>::freeIndex ( T index ) 
{
  if((*stack_).full())
  {
    fullStackList_.push(  stack_ );
    if( emptyStackList_.empty() )
    {
      assert( emptyStackList_.size() <= 0 );
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
inline void ALUGridIndexStack<T,length>::test () 
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
inline void ALUGridIndexStack<T,length>::backupIndexSet ( ostream & os ) 
{
  // holes are not stored at the moment 
  // they are reconstructed when gitter is 
  // restored 
  os.write( ((const char *) &maxIndex_ ), sizeof(int) ) ;
  
  /*
  assert( stack_ ); 
  stack_->backup( os ); 
  
  int s = fullStackList_.size();
  os.write( ((const char *) &s ), sizeof(int) ) ;
  
  if ( s > 0 )
  {
    // make a copy of the stack, so we can empty it 
    // without changeing the state of our object 
    StackListType backupStack = fullStackList_;
    while ( ! backupStack.empty()) 
    {
      StackType * st = backupStack.top();
      assert( st );
      backupStack.pop();
      st->backup( os );
    }
  }
  */
  return ;
}

template <class T, int length>
inline void ALUGridIndexStack<T,length>::restoreIndexSet ( istream & is )
{
  is.read ( ((char *) &maxIndex_), sizeof(int) );
  // clear stack fro reconstruction of holes 
  clearStack ();

  /*
  assert( stack_ );
  stack_->restore( is ); 

  int s; 
  is.read ( ((char *) &s), sizeof(int) );

  // if full stacks have been backuped 
  if( s > 0 ) 
  {
    // create as many stacks as have been backuped
    for(int i=0; i<s; ++i) 
    {
      StackType * st = new StackType();
      assert( st );
      st->restore( is );
      fullStackList_.push( st );
    }
  }
  */
  return ;
}

template <class T, int length>
inline void ALUGridIndexStack<T,length>::clearStack () 
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
    fullStackList_.pop();
    if(st) delete st; 
  }
  return;
}

template <class T, int length>
inline void ALUGridIndexStack<T,length>::
generateHoles(const vector<bool> & isHole) 
{
  const int idxsize = isHole.size();
  assert( idxsize == maxIndex_ );
  // big indices are inserted first 
  for(int i=idxsize-1; i>=0; --i)
  {
    // all entries marked true will be inserted as free 
    if(isHole[i] == true) freeIndex(i);
  }
}
 
enum { lengthOfFiniteStack = 10000 };
typedef ALUGridIndexStack<int,lengthOfFiniteStack> IndexManagerType;

} // end namespace 
#endif
