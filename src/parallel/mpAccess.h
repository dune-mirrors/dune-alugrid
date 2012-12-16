#ifndef MPACCESS_H_INCLUDED
#define MPACCESS_H_INCLUDED

#include "serialize.h"

class MpAccessGlobal {
  public :
    class CommIF
    {
    protected:
      CommIF () {}
    public:
      virtual ~CommIF() {}
      virtual CommIF* clone () const = 0;
    };

    struct MinMaxSum 
    {
      MinMaxSum() 
        : min( numeric_limits< double > :: max() ),
          max( numeric_limits< double > :: min() ),
          sum( 0 )
      {}

      explicit MinMaxSum( const double value ) 
        : min( value ), max( value ), sum( value )
      {}

      double min ; 
      double max ;
      double sum ;
    };

    typedef MinMaxSum  minmaxsum_t ;

    inline virtual ~MpAccessGlobal () ;
    virtual int psize () const = 0 ;
    virtual int myrank () const = 0 ;
    virtual int barrier () const = 0 ;
    virtual bool gmax (bool) const = 0 ;
    virtual int gmax (int) const = 0 ;
    virtual int gmin (int) const = 0 ;
    virtual int gsum (int) const = 0 ;
    virtual long gmax (long) const = 0 ;
    virtual long gmin (long) const = 0 ;
    virtual long gsum (long) const = 0 ;
    virtual double gmax (double) const = 0 ;
    virtual double gmin (double) const = 0 ;
    virtual double gsum (double) const = 0 ;
    virtual void gmax (double*,int,double*) const = 0 ;
    virtual void gmin (double*,int,double*) const = 0 ;
    virtual void gsum (double*,int,double*) const = 0 ;
    virtual void gmax (int*,int,int*) const = 0 ;
    virtual void gmin (int*,int,int*) const = 0 ;
    virtual void gsum (int*,int,int*) const = 0 ;
    virtual minmaxsum_t minmaxsum( double ) const = 0;
    virtual pair<double,double> gmax (pair<double,double>) const = 0 ;
    virtual pair<double,double> gmin (pair<double,double>) const = 0 ;
    virtual pair<double,double> gsum (pair<double,double>) const = 0 ;
    virtual void bcast(int*,int, int) const = 0 ;
    virtual void bcast(char*,int, int) const = 0 ;
    virtual void bcast(double*,int, int) const = 0 ;
    virtual int exscan( int ) const = 0; 
    virtual int scan( int ) const = 0; 
    virtual vector < int > gcollect (int) const = 0 ;
    virtual vector < double > gcollect (double) const = 0 ;
    virtual vector < vector < int > > gcollect (const vector < int > &) const = 0 ;
    virtual vector < vector < double > > gcollect (const vector < double > &) const = 0 ;
    virtual vector < ObjectStream > gcollect (const ObjectStream &, const vector<int>& ) const = 0 ;

    // default gcollect method that first needs to communicate the sizes of the buffers 
    // this method actually does two communications, one allgather and one allgatherv 
    virtual vector < ObjectStream > gcollect (const ObjectStream &in) const
    {
      // size of buffer 
      const int snum = in._wb - in._rb ;
      // get length vector 
      vector< int > length = gcollect( snum );

      // return gcollect operation 
      return gcollect( in, length ); 
    }

    //! return address of communicator (not optimal but avoid explicit MPI types here)
    virtual const CommIF* communicator() const = 0;
} ;

class MpAccessLocal : public MpAccessGlobal 
{
  map < int, int, less < int > > _linkage ;
  vector< int > _dest ;
  public :
    class NonBlockingExchange 
    {
    protected:
      NonBlockingExchange () {}
    public:  
      virtual ~NonBlockingExchange () {}
      virtual void send( const vector< ObjectStream >& ) = 0;  
      virtual vector < ObjectStream > receive() = 0;  
    };

    inline virtual ~MpAccessLocal () ;
    void printLinkage (ostream &) const ;
    inline void removeLinkage () ;
    inline int nlinks () const ;
    inline int link (int) const ;
    const vector < int >& dest () const{ return _dest ; }
    int insertRequestSymetric (set < int, less < int > >) ;
    virtual vector < vector < int > > exchange (const vector < vector < int > > &) const = 0 ;
    virtual vector < vector < double > > exchange (const vector < vector < double > > &) const = 0 ;
    virtual vector < vector < char > > exchange (const vector < vector < char > > &) const = 0 ;
    // exchange data and return new vector of object streams 
    virtual vector < ObjectStream > exchange (const vector < ObjectStream > &) const = 0 ;

    // return handle for non-blocking exchange and already do send operation
    virtual NonBlockingExchange* nonBlockingExchange ( const int tag, 
                                                       const vector < ObjectStream > & ) const = 0;

    // return handle for non-blocking exchange 
    virtual NonBlockingExchange* nonBlockingExchange ( const int tag ) const = 0;
} ;


	//
	//    #    #    #  #          #    #    #  ######
	//    #    ##   #  #          #    ##   #  #
	//    #    # #  #  #          #    # #  #  #####
	//    #    #  # #  #          #    #  # #  #
	//    #    #   ##  #          #    #   ##  #
	//    #    #    #  ######     #    #    #  ######
	//

inline MpAccessGlobal :: ~MpAccessGlobal () {
}

inline MpAccessLocal :: ~MpAccessLocal () {
}

inline int MpAccessLocal :: link (int i) const {
  assert (_linkage.end () != _linkage.find (i)) ;
  return (* _linkage.find (i)).second ;
}

inline int MpAccessLocal :: nlinks () const {
  return _linkage.size () ;
}

inline void MpAccessLocal :: removeLinkage () 
{
  _linkage.clear();
  _dest.clear();
  return ;
}

#endif
