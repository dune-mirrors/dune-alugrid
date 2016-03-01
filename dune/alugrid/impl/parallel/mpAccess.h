#ifndef MPACCESS_H_INCLUDED
#define MPACCESS_H_INCLUDED

#include <limits>
#include <map>
#include <set>
#include <vector>

#include "../serial/serialize.h"

namespace ALUGrid
{

  class MpAccessGlobal
  {
    public :
      struct MinMaxSum
      {
        MinMaxSum ()
        : min( std::numeric_limits< double >::max() ),
          max( std::numeric_limits< double >::min() ),
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
      virtual std::pair< double, double > gmax ( std::pair< double, double > ) const = 0;
      virtual std::pair< double, double > gmin ( std::pair< double, double > ) const = 0;
      virtual std::pair< double, double > gsum ( std::pair< double, double > ) const = 0;
      virtual void bcast(int*,int, int) const = 0 ;
      virtual void bcast(char*,int, int) const = 0 ;
      virtual void bcast(double*,int, int) const = 0 ;
      virtual void bcast( ObjectStream&, int ) const = 0 ;
      virtual int exscan( int ) const = 0;
      virtual int scan( int ) const = 0;
      virtual std::vector< int > gcollect ( int ) const = 0;
      virtual std::vector< double > gcollect ( double ) const = 0;
      virtual std::vector< std::vector< int > > gcollect ( const std::vector< int > & ) const = 0;
      virtual std::vector< std::vector< double > > gcollect ( const std::vector< double > & ) const = 0;
      virtual std::vector< ObjectStream > gcollect (const ObjectStream &, const std::vector< int > & ) const = 0;

      // default gcollect method that first needs to communicate the sizes of the buffers
      // this method actually does two communications, one allgather and one allgatherv
      virtual std::vector< ObjectStream > gcollect ( const ObjectStream &in ) const
      {
        // size of buffer
        const int snum = in._wb - in._rb ;
        // get length vector
        std::vector< int > length = gcollect( snum );

        // return gcollect operation
        return gcollect( in, length );
      }
  } ;

  // class fulfilling the MpAccessGlobal communicator interface
  // for serial usage (ie for usage of the partitioners in serial)
  class MpAccessSerial : public MpAccessGlobal
  {
  public:
    MpAccessSerial() {}
    virtual ~MpAccessSerial () {}

    // this features is only available in the newer version
    typedef MpAccessGlobal :: minmaxsum_t  minmaxsum_t;

    virtual int psize() const { return 1; }
    int myrank () const { return 0; }

    virtual int barrier () const { return psize(); }
    virtual bool gmax (bool value) const { return value; }
    virtual int gmax (int value) const { return value; }
    virtual int gmin ( int value ) const { return value; }
    virtual int gsum ( int value ) const { return value; }
    virtual long gmax ( long value ) const { return value; }
    virtual long gmin ( long value ) const { return value; }
    virtual long gsum ( long value ) const { return value; }
    virtual double gmax ( double value ) const { return value; }
    virtual double gmin ( double value ) const { return value; }
    virtual double gsum ( double value ) const { return value; }
    virtual void gmax (double* in, int length, double* out) const { std::copy(in, in+length, out ); }
    virtual void gmin (double* in, int length, double* out) const { std::copy(in, in+length, out ); }
    virtual void gsum (double* in, int length, double* out) const { std::copy(in, in+length, out ); }
    virtual void gmax (int*,int,int*) const {  }
    virtual void gmin (int*,int,int*) const {  }
    virtual void gsum (int*,int,int*) const {  }
    virtual minmaxsum_t minmaxsum( double value ) const { return minmaxsum_t( value ); }
    virtual std::pair<double,double> gmax (std::pair<double,double> value ) const { return value; }
    virtual std::pair<double,double> gmin (std::pair<double,double> value ) const { return value; }
    virtual std::pair<double,double> gsum (std::pair<double,double> value ) const { return value; }
    virtual void bcast(int*,int, int) const { }
    virtual void bcast(char*,int, int) const { }
    virtual void bcast(double*,int, int) const { }
    virtual void bcast ( ObjectStream &, int ) const {}
    virtual int exscan( int ) const { return 0; }
    virtual int scan( int value ) const { return value; }
    virtual std::vector < int > gcollect ( int value ) const { return std::vector<int> (psize(), value); }
    virtual std::vector < double > gcollect ( double value ) const { return std::vector<double> (psize(), value); }
    virtual std::vector < std::vector < int > > gcollect (const std::vector < int > & value) const
    {
      return std::vector < std::vector < int > > (psize(), value);
    }
    virtual std::vector < ObjectStream > gcollect (const ObjectStream &, const std::vector<int>& ) const
    {
      return std::vector < ObjectStream > (psize());
    }
    virtual std::vector < std::vector < double > > gcollect (const std::vector < double > & value) const
    {
      return std::vector < std::vector < double > > (psize(), value);
    }
    virtual std::vector < ObjectStream > gcollect (const ObjectStream &os) const { return std::vector < ObjectStream >(psize(),os); }
  };


  // MpAccessLocal is an implementation of MpAccessGlobal but also adding a
  // point-to-point interface
  class MpAccessLocal : public MpAccessGlobal
  {
    typedef std::map< int, int > linkage_t;
    typedef std::vector< int >   vector_t;

    linkage_t  _sendLinkage ;
    linkage_t  _recvLinkage ;
    linkage_t* _currentRecvLinkage;

    vector_t   _sendDest ;
    vector_t   _recvDest ;
    vector_t*  _currentRecvDest;

    void computeDestinations( const linkage_t&, vector_t& );

    public :
      class NonBlockingExchange
      {
      protected:
        NonBlockingExchange () {}
      public:
        class DataHandleIF
        {
        protected:
          DataHandleIF () {}
        public:
          virtual ~DataHandleIF () {}
          virtual void   pack( const int link, ObjectStream& os ) = 0 ;
          virtual void unpack( const int link, ObjectStream& os ) = 0 ;
          // should contain work that could be done between send and receive
          virtual void localComputation () {}
        };

        virtual ~NonBlockingExchange () {}
        virtual void send ( const std::vector< ObjectStream > & ) = 0;
        virtual void send ( std::vector< ObjectStream > &, DataHandleIF& ) = 0;
        virtual std::vector< ObjectStream > receive() = 0;
        virtual void receive( DataHandleIF& ) = 0;
        virtual void exchange( DataHandleIF& ) = 0;
      };

      inline MpAccessLocal () ;
      inline MpAccessLocal (const MpAccessLocal& ) ;
      inline virtual ~MpAccessLocal () ;
      void printLinkage ( std::ostream & ) const;
      inline void removeLinkage () ;
      inline bool symmetricLinkage () const ;
      inline int nlinks () const ;
      inline int sendLinks () const ;
      inline int recvLinks () const ;
      // use assert here, since this part also affects some communications methods in dune-fem
      inline int link (int l ) const { assert( symmetricLinkage() ); return sendLink( l ); }
      inline int sendLink (int) const ;
      inline int recvLink (int) const ;
      // use assert here, since this part also affects some communications methods in dune-fem
      const std::vector< int > &dest () const{ assert( symmetricLinkage() ); return sendDest(); }
      const std::vector< int > &sendDest   () const ;
      const std::vector< int > &recvSource () const ;

      // send ranks are not converted
      static int sendRank( const int rank ) { return rank; }

      // recv ranks are converted to a negative number
      static int recvRank( const int rank ) { return -rank-1; }

      // deprecated method, don't use anymore
      int insertRequestSymetric ( const std::set< int >& req ) { return insertRequestSymmetric( req ); }

      // insert non-symmetric linkage, i.e. send and recv links differ
      int insertRequestNonSymmetric ( const std::set< int >& );

      // insert symmetric linkage, i.e. send and recv links are the same, no global communication
      int insertRequestSymmetric ( const std::set< int >& );

      // insert symmetric linkage, i.e. send and recv links are the same,
      // perform global communication to check linkage
      int insertRequestSymmetricGlobalComm ( const std::set< int >& );

      // exchange data and return new vector of object streams
      virtual std::vector< ObjectStream > exchange (const std::vector< ObjectStream > &) const = 0 ;
      virtual void exchange ( const std::vector< ObjectStream > &, NonBlockingExchange::DataHandleIF& ) const = 0 ;
      virtual void exchange ( NonBlockingExchange::DataHandleIF& ) const = 0 ;
      virtual void exchangeSymmetric ( NonBlockingExchange::DataHandleIF& dh ) const { exchange( dh ); }

      // return handle for non-blocking exchange and already do send operation
      virtual NonBlockingExchange* nonBlockingExchange ( const int tag,
                                                         const std::vector< ObjectStream > & ) const = 0;

      // return handle for non-blocking exchange
      virtual NonBlockingExchange* nonBlockingExchange ( const int tag ) const = 0;
      // return handle for non-blocking exchange
      virtual NonBlockingExchange *nonBlockingExchange () const = 0;
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

  inline MpAccessLocal :: MpAccessLocal ()
    : _sendLinkage(), _recvLinkage(), _currentRecvLinkage( &_sendLinkage ),
      _sendDest(), _recvDest(), _currentRecvDest( &_sendDest )
  {
  }

  inline MpAccessLocal :: MpAccessLocal (const MpAccessLocal& other)
    : _sendLinkage( other._sendLinkage ),
      _recvLinkage( other._recvLinkage ),
      _currentRecvLinkage( other.symmetricLinkage() ? &_sendLinkage : &_recvLinkage ),
      _sendDest( other._sendDest ),
      _recvDest( other._recvDest ),
      _currentRecvDest( other.symmetricLinkage() ? &_sendDest : &_recvDest )
  {
  }

  inline MpAccessLocal :: ~MpAccessLocal () {
  }

  inline int MpAccessLocal :: sendLink (int i) const
  {
    alugrid_assert (_sendLinkage.end () != _sendLinkage.find (i)) ;
    return (* _sendLinkage.find (i)).second ;
  }

  inline int MpAccessLocal::recvLink ( int i ) const
  {
    alugrid_assert( _currentRecvLinkage );
    const linkage_t::const_iterator pos = _currentRecvLinkage->find( i );
    alugrid_assert( pos != _currentRecvLinkage->end() );
    return pos->second;
  }

  inline const std::vector<int>& MpAccessLocal :: sendDest() const
  {
    return _sendDest ;
  }

  inline const std::vector<int>& MpAccessLocal :: recvSource () const
  {
    alugrid_assert( _currentRecvDest );
    return *_currentRecvDest;
  }

  inline bool MpAccessLocal :: symmetricLinkage() const
  {
    // if _currentRecvLinkage points to sendLinkage we have a symmetric situation
    return (&_sendLinkage == _currentRecvLinkage);
  }

  // only use this for symmetric situations
  inline int MpAccessLocal :: nlinks () const {
    // use assert here, since this part also affects some communications methods in dune-fem
    assert( symmetricLinkage () );
    return sendLinks();
  }

  inline int MpAccessLocal :: sendLinks () const {
    return _sendLinkage.size();
  }

  inline int MpAccessLocal :: recvLinks () const {
    alugrid_assert( _currentRecvLinkage );
    return _currentRecvLinkage->size();
  }

  inline void MpAccessLocal :: removeLinkage ()
  {
    linkage_t().swap( _sendLinkage );
    linkage_t().swap( _recvLinkage );
    _currentRecvLinkage = & _sendLinkage ;

    vector_t().swap( _sendDest );
    vector_t().swap( _recvDest );
    _currentRecvDest = &_sendDest ;
  }

} // namespace ALUGrid

#endif // #ifndef MPACCESS_H_INCLUDED
