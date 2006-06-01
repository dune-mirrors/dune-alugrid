#ifndef GITTER_DUNE_IMPL_H_INCLUDED
#define GITTER_DUNE_IMPL_H_INCLUDED

#include "gitter_impl.h"
#include "lock.h"

enum IndexType { no_index = 0 , hierarchic_index = 1, leaf_index = 3 };

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
protected:
  void backupIndices  (ostream & out);
  void restoreIndices (istream & in );

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
  
  // write status of grid  
  virtual void duneBackup  (const char*) ; 

  // read status of grid 
  virtual void duneRestore (const char*) ;
  // Constructor getting macro file name 
  
  bool refine ();

  void coarse ();

//  void c1() { std::cout << "GitterDuneBasis::c1() \n"; }

  // done call notify and loadBalancer  
  bool duneAdapt (AdaptRestrictProlongType & arp);
};

class GitterDuneImpl : public GitterBasisImpl , public GitterDuneBasis 
{
  // return LeafIterator which only iterates over elements 
  virtual IteratorSTI < Gitter :: helement_STI > * leafIterator (const Gitter :: helement_STI *) ;
  virtual IteratorSTI < Gitter :: helement_STI > * leafIterator (const IteratorSTI < Gitter :: helement_STI > *) ;

  friend class PureElementLeafIterator < Gitter :: helement_STI > ;
public:

  /*
  class ObjectsDune : public Objects 
  {
    typedef Objects :: TetraEmpty TetraEmpty;
    class TetraDune : public TetraEmpty  
    {
      protected:
        typedef TetraEmpty :: innervertex_t innervertex_t;
        typedef TetraEmpty :: inneredge_t   inneredge_t;
        typedef TetraEmpty :: innerface_t   innerface_t;

      public:  
        inline TetraDune(myhface3_t *f1,int t1, myhface3_t *f2,int t2
              ,myhface3_t *f3,int t3,myhface3_t *f4 ,int t4, Gitter * grd) 
          : TetraEmpty(f1,t1,f2,t2,f3,t3,f4,t4,grd) , 
        _determinant(6.0 * quadraturTetra3D < VolumeCalc > (LinearMapping (
                myvertex(0)->Point(), myvertex(1)->Point(),
                myvertex(2)->Point(),
                myvertex(3)->Point())).integrate1 (0.0))
        {
          cout << "Calc el determinant = " <<  _determinant << "\n";
        }
        
        inline double determinant () const { return _determinant; }

      protected: 
        double _determinant; 
        friend class TetraTop < TetraDune > ;
    };
    public:  
    typedef TetraTop < TetraDune > tetra_IMPL;  
  };
  typedef ObjectsDune Objects;
  */
  
  inline GitterDuneImpl (const char *filename) : GitterBasisImpl ( filename ) {}
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
  this->_fac = &f;
  this->_fac->ref ++ ;

  // this is the difference to the normal AccessIterator, we insert
  // pureElementIterator, all other things are the same   
  this->_w = this->_fac->pureElementIterator(this->_a) ;
  return ;
} 

template < class A > inline PureElementAccessIterator < A > :: Handle :: 
Handle (const PureElementAccessIterator < A > :: Handle & p) 
  : AccessIterator < A > :: Handle ()
{
  this->_fac = p._fac; 
  if( this->_fac ) this->_fac->ref ++; 
  this->_w = p._w->clone(); 
  return ;
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
