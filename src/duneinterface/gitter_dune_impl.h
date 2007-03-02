#ifndef GITTER_DUNE_IMPL_H_INCLUDED
#define GITTER_DUNE_IMPL_H_INCLUDED

#include "gitter_impl.h"
#include "lock.h"

template < class A > class PureElementAccessIterator : public AccessIterator <A> 
{
  public :
    Refcount ref ;
    virtual IteratorSTI < A > * pureElementIterator (const A *) const = 0 ;
    virtual IteratorSTI < A > * pureElementIterator (const IteratorSTI < A > *) const = 0 ;
  public :

  // the only difference to AccessIterator is, that in the Constructors of this class 
  // the method pureElementIterator is called instead of iterator, which gives an iterator 
  // that doesn't iterator over periodic elements 

  class Handle : public AccessIterator <A> :: Handle {
    public :
      inline Handle (AccessIterator < A > &) ;
      inline Handle (const PureElementAccessIterator < A > :: Handle &) ;
      inline Handle (); 
  } ;

  protected :
    PureElementAccessIterator () {}
    virtual ~PureElementAccessIterator () { assert (!ref) ; }
} ;

template < class A > class PureElementLeafIterator; 

class GitterDuneBasis :  public virtual GitterBasis 
{
  enum IndexType { no_index = 0 , hierarchic_index = 1, leaf_index = 3 };
protected:
  inline void goDownHelement( helement_STI & el , vector<bool> & idxcheck);

  AdaptRestrictProlongType * _arp;

  // call preCoarsening and postRefinement of arp
  virtual int preCoarsening  (Gitter::helement_STI &);
  virtual int postRefinement (Gitter::helement_STI &);
  
  // call preCoarsening and postRefinement of arp
  virtual int preCoarsening  (Gitter::hbndseg_STI &);
  virtual int postRefinement (Gitter::hbndseg_STI &);

  virtual void setAdaptRestrictProlongOp ( AdaptRestrictProlongType & arp );
  virtual void removeAdaptRestrictProlongOp ();

  // maxlevel of the grid 
  int maxlevel_;

  friend class PureElementLeafIterator < Gitter :: helement_STI >;
  // return leafIterator using pureElement Iterators 
  virtual IteratorSTI < Gitter :: helement_STI > * leafIterator (const Gitter :: helement_STI *) = 0 ;
  virtual IteratorSTI < Gitter :: helement_STI > * leafIterator (const IteratorSTI < Gitter :: helement_STI > *) = 0 ;

public:
  GitterDuneBasis() : _arp(0) , maxlevel_(0) {}
  
  virtual void backupIndices  (ostream & out);
  virtual void restoreIndices (istream & in );

  // write status of grid  
  virtual void duneBackup  (const char*) ; 

  // read status of grid 
  virtual void duneRestore (const char*) ;
  // Constructor getting macro file name 
 
  // refine all leaf elements 
  bool refine ();

  // coarse all leaf elements if possible 
  void coarse ();

  // done call notify and loadBalancer  
  bool duneAdapt (AdaptRestrictProlongType & arp);
  
private:  
  /*
  // communication of data
  void ALUcomm (
    GatherScatterType & vertexData ,
    GatherScatterType & edgeData,
    GatherScatterType & faceData ,
    GatherScatterType & elementData ,
    const CommunicationType commType);

  // communication of interior data
  void doInteriorGhostComm(
    vector< ObjectStream > & osvec ,
    GatherScatterType & vertexData ,
    GatherScatterType & edgeData,
    GatherScatterType & faceData,
    GatherScatterType & elementData ,
    const CommunicationType commType );
    */
};

class GitterDuneImpl : public GitterBasisImpl , public GitterDuneBasis 
{
  // return LeafIterator which only iterates over elements 
  virtual IteratorSTI < Gitter :: helement_STI > * leafIterator (const Gitter :: helement_STI *) ;
  virtual IteratorSTI < Gitter :: helement_STI > * leafIterator (const IteratorSTI < Gitter :: helement_STI > *) ;

  friend class PureElementLeafIterator < Gitter :: helement_STI > ;
public:

  //! constructor creating grid from macro grid file 
  inline GitterDuneImpl (const char *filename) : 
    GitterBasisImpl ( filename ) {}
  
  //! constructor creating empty grid 
  inline GitterDuneImpl () : GitterBasisImpl () {}
};


// this LeafIterator only iterates over elements, i.e. tetra,hexa 
template < class A > class PureElementLeafIterator : public MyAlloc {
  GitterDuneBasis * _grd ;
  IteratorSTI < A > * _w ;
  const A * _a ;
  void * operator new (size_t) { return 0 ; }
  void operator delete (void *) {}
  inline PureElementLeafIterator () ;
  public :
    inline PureElementLeafIterator (GitterDuneBasis &) ;
    inline PureElementLeafIterator (const PureElementLeafIterator < A > & ) ;
    inline ~PureElementLeafIterator () ;
    inline IteratorSTI < A > * operator -> () const ;
    inline IteratorSTI < A > & operator * () const ;
} ;

//
//    #    #    #  #          #    #    #  ######
//    #    ##   #  #          #    ##   #  #
//    #    # #  #  #          #    # #  #  #####
//    #    #  # #  #          #    #  # #  #
//    #    #   ##  #          #    #   ##  #
//    #    #    #  ######     #    #    #  ######
//

template < class A > inline PureElementAccessIterator < A > :: Handle :: 
Handle (AccessIterator < A > & f) 
 : AccessIterator < A > :: Handle () 
{
  this->removeObj();
  
  this->_fac = &f;
  this->_fac->ref ++ ;

  assert( this->_w == 0 );
  // this is the difference to the normal AccessIterator, we insert
  // pureElementIterator, all other things are the same   
  this->_w = this->_fac->pureElementIterator(this->_a) ;
  return ;
} 

template < class A > inline PureElementAccessIterator < A > :: Handle :: 
Handle (const PureElementAccessIterator < A > :: Handle & p) 
  : AccessIterator < A > :: Handle (p)
{
} 

template < class A > inline PureElementAccessIterator < A > :: Handle :: Handle () 
  : AccessIterator < A > :: Handle () {} 

template < class A > PureElementLeafIterator < A > :: PureElementLeafIterator () : _grd (0), _w (0) {
  return ;
}


// new LEafIterator which only iterates over elements 
template < class A > inline PureElementLeafIterator < A > :: 
PureElementLeafIterator (GitterDuneBasis & g) : _grd (&g), _w (0) , _a(0) {
  _w = _grd->leafIterator (_a) ;
  return ;
}

template < class A > inline PureElementLeafIterator < A > :: PureElementLeafIterator (const PureElementLeafIterator < A > & x) : _grd (x._grd), _w (0) {
  _w = _grd->leafIterator (x._w) ;
  return ;
}
  
template < class A > inline PureElementLeafIterator < A > :: ~PureElementLeafIterator () {
  if(_w) delete _w ;
  return ;
}
  
template < class A > inline IteratorSTI < A > * PureElementLeafIterator < A > :: operator -> () const {
  return _w ;
}
  
template < class A > inline IteratorSTI < A > & PureElementLeafIterator < A > :: operator * () const {
  return * _w ;
}
#endif
