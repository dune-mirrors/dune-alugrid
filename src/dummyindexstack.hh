#ifndef __BSGRID_DUMMYINDEXSTACK_HH__
#define __BSGRID_DUMMYINDEXSTACK_HH__

//******************************************************
//
// Dummy implementation if the Dune IndexStack is not avaiable.
//
//******************************************************
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
};  // end class IndexStack 

#endif
