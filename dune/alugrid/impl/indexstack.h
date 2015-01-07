// (c) Robert Kloefkorn 2004 - 2007
#ifndef ALUGRIDINDEXSTACK_H_INCLUDED
#define ALUGRIDINDEXSTACK_H_INCLUDED

#include <algorithm>
#include <dune/alugrid/common/alugrid_assert.hh>
#include <iostream>
#include <stack>
#include <vector>

#include "byteorder.hh"

namespace ALUGrid
{
  static const bool byteOrderIsBigEndian = systemByteOrder() == bigEndian ? true : false;

  class RestoreInfo
  {
  public:
    // return byte order (0 = little endian, 1 = big endian)
    static char systemByteOrder ()
    {
      return char(byteOrderIsBigEndian);
    }

    // return byte order as a string
    static const char* byteOrderString()
    {
      static const char* bigEndian = "BigEndian";
      static const char* littleEndian = "LittleEndian";
      return systemByteOrder() ? bigEndian : littleEndian ;
    }

  protected:
    enum{ nCodims = 4 };

    std::vector< bool > isHole_[ nCodims ];
    const bool toggleByteOrder_;
    std::vector< char > buffer_ ;

  public:
    RestoreInfo( const char byteOrder )
      : toggleByteOrder_( systemByteOrder() != byteOrder )
    {
      for( int i=0; i<nCodims; ++i )
        isHole_[ i ].clear();
    }

    size_t size() const { return nCodims; }

    std::vector< bool >& operator() ( const size_t codim )
    {
      alugrid_assert ( codim < size() );
      return isHole_[ codim ];
    }

    // returns true if the byte order needs a change
    bool toggleByteOrder () const { return toggleByteOrder_; }

    //! change byte order of buff
    void changeByteOrder( char* buff, const size_t size )
    {
      if( buffer_.size() < size )
        buffer_.resize( size );

      // copy char buffer
      for( size_t i=0; i<size; ++i )
        buffer_[ i ] = buff[ i ];

      // change byte order
      for( size_t i=0; i<size; ++i )
        buff[ i ] = buffer_[ size - i - 1 ];
    }
  };


  // using namespace std has always to be called inside the namespace
  // ALUGridSpace

  template<class T, int length>
  class ALUGridFiniteStack
  {
  public :
    // Makes empty stack
    ALUGridFiniteStack () : _f(0) {}

    // Returns true if the stack is empty
    bool empty () const { return _f <= 0; }

    // Returns true if the stack is full
    bool full () const { return (_f >= length); }

    // clear stack
    void clear() { _f = 0; }

    // Puts a new object onto the stack
    void push (const T& t)
    {
      alugrid_assert ( _f < length );
      _s[_f++] = t;
    }

    // Removes and returns the uppermost object from the stack
    T pop () {
      alugrid_assert ( _f > 0 );
      return _s[--_f];
    }

    // Returns the uppermost object on the stack
    T top () const {
      alugrid_assert ( _f > 0 );
      return _s[_f-1];
    }

    // stacksize
    int size () const { return _f; }

    // backup stack to ostream
    void backup ( std::ostream & os ) const
    {
      os.write( ((const char *) &_f ), sizeof(int) ) ;
      for(int i=0; i<size(); ++i)
      {
        os.write( ((const char *) &_s[i] ), sizeof(int) ) ;
      }
    }

    // restore stack from istream
    void restore ( std::istream & is )
    {
      is.read ( ((char *) &_f), sizeof(int) );
      alugrid_assert ( _f >= 0 );
      alugrid_assert ( _f < length );
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
    typedef std::stack< StackType * > StackListType;

    StackListType fullStackList_;
    StackListType emptyStackList_;

    //typedef typename StackListType::Iterator DListIteratorType;
    StackType * stack_;

    // current maxIndex
    int maxIndex_;
    int dim_;
  public:
    //! Constructor, create new ALUGridIndexStack
    ALUGridIndexStack();

    //! Destructor, deleting all stacks
    inline ~ALUGridIndexStack ();

    //! set dimension of grid
    void setDimension ( const int dim );
    //! return dimension of grid
    int dimension () const;

    //! set index as maxIndex if index is bigger than maxIndex
    void checkAndSetMax(T index) { if(index > maxIndex_) maxIndex_ = index;  }

    //! set index as maxIndex
    void setMaxIndex(T index) { maxIndex_ = index; }

    //! returns the larges index used + 1, actually this is the size of the
    //! index set
    int getMaxIndex() const {  return maxIndex_;  }

    //! restore index from stack or create new index
    T getIndex ();

    //! store index on stack
    void freeIndex(T index);

    //! test stack functionality
    void test ();

    // backup set to out stream
    template <class ostream_t>
    void backupIndexSet ( ostream_t & os );

    // restore from in stream
    template <class istream_t>
    void restoreIndexSet ( istream_t & is, RestoreInfo& restoreInfo );

    // all entries in vector with value true
    // are inserted as holes
    void generateHoles(const std::vector<bool> & isHole);

    // remove all indices that are not used (if possible)
    void compress ();

    // return size of used memory in bytes
    size_t memUsage () const ;

  private:
    //! push index to stack
    inline void pushIndex(T index);

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
    : stack_ ( new StackType () ) , maxIndex_ (0), dim_(-1) {}

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
  inline void ALUGridIndexStack<T,length>::setDimension( const int dim )
  {
    dim_ = dim;
  }

  template <class T, int length>
  inline int ALUGridIndexStack<T,length>::dimension() const
  {
    alugrid_assert( dim_ == 2 || dim_ == 3 );
    return dim_;
  }

  template <class T, int length>
  inline T ALUGridIndexStack<T,length>::getIndex ()
  {
    if((*stack_).empty())
    {
      if( fullStackList_.empty() )
      {
        alugrid_assert ( fullStackList_.size() <= 0 );
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
    if(index == (maxIndex_ -1))
    {
      --maxIndex_;
      return ;
    }
    else
    {
      pushIndex(index);
    }
  }


  template <class T, int length>
  inline void ALUGridIndexStack<T,length>::pushIndex( T index )
  {
    if((*stack_).full())
    {
      fullStackList_.push( stack_ );
      if( emptyStackList_.empty() )
      {
        alugrid_assert ( emptyStackList_.size() <= 0 );
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
  template <class ostream_t>
  inline void ALUGridIndexStack<T,length>::backupIndexSet ( ostream_t & os )
  {
    // holes are not stored at the moment
    // they are reconstructed when gitter is
    // restored
    os.write( ((const char *) &maxIndex_ ), sizeof(int) ) ;

    return ;
  }

  template <class T, int length>
  template <class istream_t>
  inline void ALUGridIndexStack<T,length>::
  restoreIndexSet ( istream_t & is, RestoreInfo& restoreInfo)
  {
    // read maxIndex from stream
    is.read ( ((char *) &maxIndex_), sizeof(int) );

    // adjust byte order if necessary
    if( restoreInfo.toggleByteOrder() )
      restoreInfo.changeByteOrder( ((char *) &maxIndex_), sizeof(int) );

    // clear stack fro reconstruction of holes
    clearStack ();

    return ;
  }

  template <class T, int length>
  inline void ALUGridIndexStack<T,length>::clearStack ()
  {
    if(stack_)
    {
      delete stack_;
      stack_ = new StackType();
      alugrid_assert (stack_);
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
  generateHoles(const std::vector<bool> & isHole)
  {
    const int idxsize = isHole.size();
    alugrid_assert ( idxsize == maxIndex_ );
    // big indices are inserted first
    for(int i=idxsize-1; i>=0; --i)
    {
      // all entries marked true will be pushed to stack
      // to create the exact index manager status from before
      if(isHole[i] == true) pushIndex(i);
    }
  }

  template <class T, int length>
  inline void ALUGridIndexStack<T,length>::
  compress()
  {
    std::vector<int> tmpStorage;

    if( stack_ )
    {
      // StackType is of type FiniteStack
      StackType& stack = *stack_;

      // reserve memory for tmpStorage
      tmpStorage.reserve( stack.size() );

      // copy all values to the temporary storage
      while( ! stack.empty() )
      {
        tmpStorage.push_back( stack.pop() );
      }
      delete stack_; stack_ = 0;
    }

    while( !fullStackList_.empty() )
    {
      StackType * st = fullStackList_.top();
      fullStackList_.pop();
      // if stack is available
      if( st )
      {
        // StackType is of type FiniteStack
        StackType& stack = *st;

        // reserve memory for tmpStorage
        tmpStorage.reserve( tmpStorage.size() + stack.size() );

        // copy all values to the temporary storage
        while( ! stack.empty() )
        {
          tmpStorage.push_back( stack.pop() );
        }
        delete st;
      }
    }

    // sort so that the larges values is at the end
    // this sort is necessary to really free indices
    std::sort( tmpStorage.begin(), tmpStorage.end() );

    // now free all indices again, freeIndex
    // does remove the maxIndex in case of freed index is equal
    stack_ = new StackType();
    alugrid_assert ( stack_ );
    // until tmpStorage is not empty, freeIndices
    while( ! tmpStorage.empty () )
    {
      // free index
      freeIndex( tmpStorage.back() );
      // remove index from tmpStorage
      tmpStorage.pop_back();
    }
  }

  template <class T, int length>
  inline size_t ALUGridIndexStack<T,length>::
  memUsage () const
  {
    size_t mySize = sizeof(ALUGridIndexStack<T,length>);
    size_t stackSize = sizeof(StackType);
    if(stack_) mySize += stackSize;
    mySize += stackSize * fullStackList_.size();
    mySize += stackSize * emptyStackList_.size();
    return mySize;
  }

  // define index stack tpye for all grids
  enum { lengthOfFiniteStack = 262144 }; // 2^18
  typedef ALUGridIndexStack<int,lengthOfFiniteStack> IndexManagerType;

} // namespace ALUGrid

#endif // #ifndef ALUGRIDINDEXSTACK_H_INCLUDED
