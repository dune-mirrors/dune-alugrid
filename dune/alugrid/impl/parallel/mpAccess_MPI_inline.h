#include <iostream>

#include "mpAccess_MPI.h"
#include "../indexstack.h"

#ifndef NDEBUG
#define MY_INT_TEST int test =
#else
#define MY_INT_TEST
#endif

namespace ALUGrid
{
  inline MpAccessMPI::MinMaxSumIF* MpAccessMPI::copyMPIComm ( MPI_Comm mpicomm ) 
  {
    // duplicate mpi communicator 
    MY_INT_TEST MPI_Comm_dup ( mpicomm, &_mpiComm);
    assert (test == MPI_SUCCESS);
    return 0 ; // return NULL pointer to initialize _mpimaxsum
  }

  inline int MpAccessMPI::getSize()
  {
    // get size from MPI 
    int size = 0; 
    MY_INT_TEST MPI_Comm_size ( _mpiComm, & size );
    assert (test == MPI_SUCCESS);
    return size;
  }

  inline int MpAccessMPI::getRank()
  {
    // get rank from MPI 
    int rank = -1;
    MY_INT_TEST MPI_Comm_rank ( _mpiComm, & rank );
    assert (test == MPI_SUCCESS);
    return rank;
  }

  inline MpAccessMPI::MpAccessMPI ( MPI_Comm mpicomm )
   : _minmaxsum( copyMPIComm( mpicomm ) ),
     _psize( getSize() ), _myrank( getRank() )
  {
    initMinMaxSum();
  }



  inline MpAccessMPI::MpAccessMPI (const MpAccessMPI & a)
   : _minmaxsum( copyMPIComm( a._mpiComm ) ),
     _psize( getSize() ) , _myrank( getRank() )
  {
    initMinMaxSum();
  }

  inline MpAccessMPI::~MpAccessMPI ()
  {
    if( _minmaxsum ) 
    {
      delete _minmaxsum;
      _minmaxsum = 0 ;
    }

    // free mpi communicator 
    MY_INT_TEST MPI_Comm_free (&_mpiComm);
    assert (test == MPI_SUCCESS);
  }

  inline int MpAccessMPI::barrier () const {
      return MPI_SUCCESS == MPI_Barrier (_mpiComm) ? psize () : 0;
  }

  inline int MpAccessMPI::mpi_allgather (int * i, int si, int * o, int so) const {
      return MPI_Allgather (i, si, MPI_INT, o, so, MPI_INT, _mpiComm);
  }

  inline int MpAccessMPI::mpi_allgather (char * i, int si, char * o, int so) const {
      return MPI_Allgather (i, si, MPI_BYTE, o, so, MPI_BYTE, _mpiComm);
  }

  inline int MpAccessMPI::mpi_allgather (double * i, int si, double * o, int so) const {
      return MPI_Allgather (i, si, MPI_DOUBLE, o, so, MPI_DOUBLE, _mpiComm);
  }

  template < class A > std::vector< std::vector< A > > 
  inline doGcollectV (const std::vector< A > & in, MPI_Datatype mpiType, MPI_Comm comm) 
  {
    int np, me, test;

    test = MPI_Comm_rank (comm, & me);
    if (test != MPI_SUCCESS)
    {
      std::cerr << "ERROR (fatal): Unable to obtain rank in MPI communicator." << std::endl;
      abort();
    }

    test = MPI_Comm_size (comm, & np);
    if (test != MPI_SUCCESS)
    {
      std::cerr << "ERROR (fatal): Unable to obtain size of MPI communicator." << std::endl;
      abort();
    }

    int * rcounts = new int [np];
    int * displ = new int [np];
    assert (rcounts);
    std::vector< std::vector< A > > res (np);
    {
      int ln = in.size ();
      MY_INT_TEST MPI_Allgather (& ln, 1, MPI_INT, rcounts, 1, MPI_INT, comm);
      assert (test == MPI_SUCCESS);
      displ [0] = 0;
      {for (int j = 1; j < np; j ++) {
        displ [j] = displ [j-1] + rcounts [j-1];
      }}
      const int xSize = displ [np-1] + rcounts [np-1];
      A * x = new A [xSize];
      A * y = new A [ln];
      assert (x && y);
      copy (in.begin(), in.end(), y);
      test = MPI_Allgatherv (y, ln, mpiType, x, rcounts, displ, mpiType, comm);
      delete [] y;
      y = 0;
      assert (test == MPI_SUCCESS);
      {for (int i = 0; i < np; i ++ ) {
        res [i].reserve (rcounts [i]);
        copy (x + displ [i], x + displ [i] + rcounts [i], back_inserter(res [i]));
      }}
      delete [] x;
    }
    delete [] displ;
    delete [] rcounts;
    return res;
  }

  inline bool MpAccessMPI::gmax (bool i) const 
  {
    int j = int( i );
    // call int method 
    const int ret = gmax( j );
    return (ret == 1) ? true : false;
  }

  inline int MpAccessMPI::gmax (int i) const {
    int j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_INT, MPI_MAX, _mpiComm);
    assert (test == MPI_SUCCESS);
    return j;
  }

  inline int MpAccessMPI::gmin (int i) const {
    int j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_INT, MPI_MIN, _mpiComm);
    assert (test == MPI_SUCCESS);
    return j;
  }

  inline int MpAccessMPI::gsum (int i) const {
    int j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_INT, MPI_SUM, _mpiComm);
    assert (test == MPI_SUCCESS);
    return j;
  }

  inline long MpAccessMPI::gmax (long i) const {
    long j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_LONG, MPI_MAX, _mpiComm);
    assert (test == MPI_SUCCESS);
    return j;
  }

  inline long MpAccessMPI::gmin (long i) const {
    long j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_LONG, MPI_MIN, _mpiComm);
    assert (test == MPI_SUCCESS);
    return j;
  }

  inline long MpAccessMPI::gsum (long i) const {
    long j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_LONG, MPI_SUM, _mpiComm);
    assert (test == MPI_SUCCESS);
    return j;
  }

  inline double MpAccessMPI::gmax (double a) const {
    double x;
    MY_INT_TEST MPI_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_MAX, _mpiComm);
    assert (test == MPI_SUCCESS);
    return x;
  }

  inline double MpAccessMPI::gmin (double a) const {
    double x;
    MY_INT_TEST MPI_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_MIN, _mpiComm);
    assert (test == MPI_SUCCESS);
    return x;
  }

  inline double MpAccessMPI::gsum (double a) const {
    double x;
    MY_INT_TEST MPI_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_SUM, _mpiComm);
    assert (test == MPI_SUCCESS);
    return x;
  }

  inline void MpAccessMPI::gmax (double* a,int size,double *x) const {
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_DOUBLE, MPI_MAX, _mpiComm);
    assert (test == MPI_SUCCESS);
  }

  inline void MpAccessMPI::gmin (double* a,int size,double *x) const {
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_DOUBLE, MPI_MIN, _mpiComm);
    assert (test == MPI_SUCCESS);
  }

  inline void MpAccessMPI::gsum (double* a,int size,double *x) const {
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_DOUBLE, MPI_SUM, _mpiComm);
    assert (test == MPI_SUCCESS);
  }

  inline void MpAccessMPI::gmax (int* a,int size,int *x) const {
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_INT, MPI_MAX, _mpiComm);
    assert (test == MPI_SUCCESS);
  }

  inline void MpAccessMPI::gmin (int* a,int size,int *x) const {
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_INT, MPI_MIN, _mpiComm);
    assert (test == MPI_SUCCESS);
  }

  inline void MpAccessMPI::gsum (int* a,int size,int *x) const {
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_INT, MPI_SUM, _mpiComm);
    assert (test == MPI_SUCCESS);
  }

  struct MinMaxSumOp : public MpAccessMPI::MinMaxSumIF 
  {
    typedef MpAccessMPI::minmaxsum_t  minmaxsum_t;

    MinMaxSumOp ( const MpAccessMPI& mpAccess ) 
      : _mpAccess( mpAccess )
    {
      MPI_Type_contiguous (3, MPI_DOUBLE, &_mpi_minmaxsum_t);
      MPI_Type_commit (&_mpi_minmaxsum_t);
      MPI_Op_create((MPI_User_function *)&min_max_sum, 0, &_op);
    }

    ~MinMaxSumOp() 
    {
      MPI_Op_free (&_op);
      MPI_Type_free(&_mpi_minmaxsum_t);
    }

    const MpAccessMPI& _mpAccess;

    MPI_Op _op; 
    MPI_Datatype _mpi_minmaxsum_t;

    static void 
    min_max_sum( minmaxsum_t* in, minmaxsum_t* inout, 
                 int* len, MPI_Datatype* datatype ) 
    {
      const int size = *len;
      for( int i = 0; i < size; ++i )
      {
        // min 
        inout[ i ].min = std::min( in[ i ].min, inout[ i ].min );
        // max 
        inout[ i ].max = std::max( in[ i ].max, inout[ i ].max );
        // sum 
        inout[ i ].sum += in[ i ].sum;
      }
    } 

    minmaxsum_t minmaxsum( double value ) const 
    {
      // get mpi communicator
      MPI_Comm comm = _mpAccess.communicator();

      // create send buf 
      minmaxsum_t sendbuf( value ); 

      // create recv buf 
      minmaxsum_t result( value ); 

      MPI_Allreduce( &sendbuf, &result, 1, _mpi_minmaxsum_t, _op, comm );
      return result;
    }
  };

  inline void MpAccessMPI::initMinMaxSum() 
  {
    if( ! _minmaxsum ) 
      _minmaxsum = new MinMaxSumOp( *this );
  }

  inline MpAccessMPI::minmaxsum_t  MpAccessMPI::minmaxsum( double value ) const
  {
    assert( _minmaxsum );
    return _minmaxsum->minmaxsum( value );
  }

  inline std::pair<double,double> MpAccessMPI::gmax (std::pair<double,double> p) const {
    double x[2];
    double a[2]={p.first,p.second};
    MY_INT_TEST MPI_Allreduce (a, x, 2, MPI_DOUBLE, MPI_MAX, _mpiComm);
    assert (test == MPI_SUCCESS);
    return std::pair<double,double>(x[0],x[1]);
  }

  inline std::pair<double,double> MpAccessMPI::gmin (std::pair<double,double> p) const {
    double x[2];
    double a[2]={p.first,p.second};
    MY_INT_TEST MPI_Allreduce (a, x, 2, MPI_DOUBLE, MPI_MIN, _mpiComm);
    assert (test == MPI_SUCCESS);
    return std::pair<double,double>(x[0],x[1]);
  }

  inline std::pair<double,double> MpAccessMPI::gsum (std::pair<double,double> p) const {
    double x[2];
    double a[2]={p.first,p.second};
    MY_INT_TEST MPI_Allreduce (a, x, 2, MPI_DOUBLE, MPI_SUM, _mpiComm);
    assert (test == MPI_SUCCESS);
    return std::pair<double,double>(x[0],x[1]);
  }

  inline void MpAccessMPI::bcast (int* buff, int length, int root ) const 
  {
    MPI_Bcast(buff, length, MPI_INT, root, _mpiComm);
  }

  inline void MpAccessMPI::bcast (char* buff, int length, int root ) const 
  {
    MPI_Bcast(buff, length, MPI_BYTE, root, _mpiComm);
  }

  inline void MpAccessMPI::bcast (double* buff, int length, int root ) const 
  {
    MPI_Bcast(buff, length, MPI_DOUBLE, root, _mpiComm);
  }

  inline int MpAccessMPI::exscan( int myvalue ) const 
  {
    int sum = myvalue;
    MPI_Exscan(&myvalue, &sum, 1, MPI_INT, MPI_SUM, _mpiComm);
    return sum;
  }

  inline int MpAccessMPI::scan( int myvalue ) const 
  {
    int sum = myvalue;
    MPI_Scan(&myvalue, &sum, 1, MPI_INT, MPI_SUM, _mpiComm);
    return sum;
  }

  inline std::vector< int > MpAccessMPI::gcollect (int i) const {
    std::vector< int > r (psize (), 0L);
    mpi_allgather (&i, 1, &r[ 0], 1);
    return r;
  }

  inline std::vector< double > MpAccessMPI::gcollect (double a) const 
  {
    std::vector< double > r (psize (),0.0);
    mpi_allgather (& a, 1, &r[ 0 ], 1);
    return r;
  }

  inline std::vector< std::vector< int > > MpAccessMPI::gcollect (const std::vector< int > & v) const {
    return doGcollectV (v, MPI_INT, _mpiComm);
  }

  inline std::vector< std::vector< double > > MpAccessMPI::gcollect (const std::vector< double > & v) const {
    return doGcollectV (v, MPI_DOUBLE, _mpiComm);
  }

  inline std::vector< ObjectStream > MpAccessMPI::
  gcollect (const ObjectStream & in, const std::vector<int>& len ) const 
  {
    // number of processes 
    const int np = psize (); 
    
    // size of buffer 
    const int snum = in._wb - in._rb;
    
    // create empty objects streams 
    std::vector< ObjectStream > o (np);

#ifndef NDEBUG 
    // check that given length values are correct 
    {
      std::vector< int > checkLength = gcollect( snum );
      // check sizes 
      assert( len.size() == checkLength.size() );
      if( ! equal( checkLength.begin(), checkLength.end(), len.begin() ) )
      {
        for(size_t i=0; i<checkLength.size(); ++i ) 
          std::cout << "comm " << checkLength[ i ] << "  stored " << len[ i ] << std::endl;
        abort();
      }
      // make sure the list is correct 
      assert( equal( checkLength.begin(), checkLength.end(), len.begin() ) );
    }
#endif
    
    int * rcounts = new int [np];
    assert (rcounts);
    copy (len.begin (), len.end (), rcounts);
    int * const displ = new int [np];
    assert (displ);
    
    // set offsets 
    displ [0] = 0;
    for (int j = 1; j < np; ++j ) 
    { 
      displ [j] = displ [j - 1] + rcounts [j - 1]; 
    }
    
    // overall buffer size 
    const size_t bufSize = displ [np - 1] + rcounts [np - 1];
    {    
      // allocate buffer 
      char * y = ObjectStream::allocateBuffer(bufSize);
      assert (y);
      
      // gather all data 
      MY_INT_TEST MPI_Allgatherv (in._buf + in._rb, snum, MPI_BYTE, y, rcounts, displ, MPI_BYTE, _mpiComm);
      assert (test == MPI_SUCCESS);
      // copy data to object streams 
      for (int i = 0; i < np; ++ i ) 
      {
        // write data to stream 
        if( rcounts [i] )
        {
          o [i].write( y + displ [i], rcounts [i] );
        }
      }

      // delete buffer 
      ObjectStream::freeBuffer( y );
    }

    // delete helper functions 
    delete [] displ;
    delete [] rcounts;
    
    return o;
  }

  //////////////////////////////////////////////////////////////////////////
  // non-blocking communication object 
  // this class is defined here since it contains MPI information 
  //////////////////////////////////////////////////////////////////////////
  class NonBlockingExchangeMPI : public MpAccessLocal::NonBlockingExchange
  {
    const MpAccessMPI& _mpAccess;

    const int _nLinks; 
    const int _tag;

    MPI_Request* _request;

    bool _needToSend ;

    // no copying 
    NonBlockingExchangeMPI( const NonBlockingExchangeMPI& );

  public:
    typedef MpAccessLocal::NonBlockingExchange::DataHandleIF DataHandleIF;

    NonBlockingExchangeMPI( const MpAccessMPI& mpAccess,
                            const int tag )
      : _mpAccess( mpAccess ),
        _nLinks( _mpAccess.nlinks() ),
        _tag( tag ),
        _request( ( _nLinks > 0 ) ? new MPI_Request [ _nLinks ] : 0),
        _needToSend( true )
    {
      // make sure every process has the same tag 
      assert( tag == mpAccess.gmax( tag ) );
    }

    NonBlockingExchangeMPI( const MpAccessMPI& mpAccess,
                            const int tag,
                            const std::vector< ObjectStream > & in ) 
      : _mpAccess( mpAccess ),
        _nLinks( _mpAccess.nlinks() ),
        _tag( tag ),
        _request( ( _nLinks > 0 ) ? new MPI_Request [ _nLinks ] : 0),
        _needToSend( false )
    {
      // make sure every process has the same tag 
      assert( tag == mpAccess.gmax( tag ) );

      assert( _nLinks == int( in.size() ) );
      sendImpl( in ); 
    }

    // constructor for intializing all-to-all non-blocking communication
    NonBlockingExchangeMPI( const MpAccessMPI& mpAccess,
                            const int tag,
                            const int maxMesg )
      : _mpAccess( mpAccess ),
        _nLinks( maxMesg ),
        _tag( tag ),
        _request( ( _nLinks > 0 ) ? new MPI_Request [ _nLinks ] : 0),
        _needToSend( false )
    {
      // make sure every process has the same tag 
      assert( tag == mpAccess.gmax( tag ) );
    }

    /////////////////////////////////////////
    //  interface methods 
    /////////////////////////////////////////
    ~NonBlockingExchangeMPI() 
    {
      if( _request ) 
      {
        delete [] _request;
        _request = 0;
      }
    }

    // virtual methods 
    void send( const std::vector< ObjectStream > & in ) { sendImpl( in ); }
    std::vector< ObjectStream > receive() { return receiveImpl(); }

    //////////////////////////////////////////
    // implementation 
    //////////////////////////////////////////

    // send data implementation 
    void sendImpl( const std::vector< ObjectStream > & osv ) 
    {
      // get mpi communicator
      MPI_Comm comm = _mpAccess.communicator();

      // get vector with destinations 
      const std::vector< int >& dest = _mpAccess.dest();

      // send data 
      for (int link = 0; link < _nLinks; ++link) 
      {
        sendLink( link, dest[ link ], _tag, osv[ link ], comm );
      }

      // set send info 
      _needToSend = false ;
    }

    // receive data without buffer given 
    std::vector< ObjectStream > receiveImpl () 
    {
      // create vector of empty streams 
      std::vector< ObjectStream > out ( _nLinks );
      receiveImpl( out );
      return out; 
    }

    // receive data implementation with given buffers 
    void receiveImpl ( std::vector< ObjectStream >& out ) 
    {
      // do nothing if number of links is zero 
      if( _nLinks == 0 ) return; 

      // get mpi communicator
      MPI_Comm comm = _mpAccess.communicator();

      // get vector with destinations 
      const std::vector< int >& dest = _mpAccess.dest();

#ifndef NDEBUG
      // check for all links messages 
      for (int link = 0; link < _nLinks; ++link ) 
      {
        // contains no written data
        assert( out[ link ].notReceived() ); 
      }
#endif

      // count noumber of received messages 
      int numReceived = 0;
      while( numReceived < _nLinks ) 
      {
        // check for all links messages 
        for (int link = 0; link < _nLinks; ++link ) 
        {
          ObjectStream& osRecv = out[ link ];
          // if nothing was received for this link yet
          if( osRecv.notReceived() ) 
          {
            // checks for message and if received also fills osRecv
            if( checkAndReceive( comm, dest[ link ], _tag, osRecv ) )
            {
              // increase number of received messages 
              ++ numReceived;
            }
          }
        }
      }
      
      // wait until all processes are done with receiving
      {
        MPI_Status * sta = new MPI_Status [ _nLinks ];
        assert( sta );
        assert( _request );
        MY_INT_TEST MPI_Waitall ( _nLinks, _request, sta);
        assert (test == MPI_SUCCESS);
        delete [] sta;
      }
    }

    // receive data implementation with given buffers 
    void send( std::vector< ObjectStream >& osSend, 
               DataHandleIF& dataHandle )
    {
      if( _needToSend ) 
      {
        // get mpi communicator
        MPI_Comm comm = _mpAccess.communicator();

        // get vector with destinations 
        const std::vector< int >& dest = _mpAccess.dest();

        // send data 
        for (int link = 0; link < _nLinks; ++link) 
        {
          // pack data 
          dataHandle.pack( link, osSend[ link ] );

          // send data 
          sendLink( link, dest[ link ], _tag, osSend[ link ], comm );
        }

        // set send info 
        _needToSend = false ;
      }
    }

    // receive data implementation with given buffers 
    void receive( DataHandleIF& dataHandle )
    {
      // do work that can be done between send and receive 
      dataHandle.meantimeWork() ;

      // get mpi communicator
      MPI_Comm comm = _mpAccess.communicator();

      // get vector with destinations 
      const std::vector< int >& dest = _mpAccess.dest();

      // get received vector 
      std::vector< bool > linkReceived( _nLinks, false );

      // receive message buffer 
      ObjectStream osRecv ;

      // count number of received messages 
      int numReceived = 0;
      while( numReceived < _nLinks ) 
      {
        // check for all links messages 
        for (int link = 0; link < _nLinks; ++link ) 
        {
          if( ! linkReceived[ link ] ) 
          {
            // checks for message and if received also fills osRecv
            if( checkAndReceive( comm, dest[ link ], _tag, osRecv ) )
            {
              // unpack data 
              dataHandle.unpack( link, osRecv );

              // increase number of received messages 
              ++ numReceived;

              // store received information 
              linkReceived[ link ] = true ;
            }
          }
        }
      }
      
      // Is this really needed?
      // wait until all processes are done with receiving
      {
        MPI_Status * sta = new MPI_Status [ _nLinks ];
        assert( sta );
        assert( _request );
        MY_INT_TEST MPI_Waitall ( _nLinks, _request, sta);
        assert (test == MPI_SUCCESS);
        delete [] sta;
      }
    }

    // receive data implementation with given buffers 
    void exchange( DataHandleIF& dataHandle )
    {
      // do nothing if number of links is zero 
      if( _nLinks == 0 ) return; 

      // send message buffers, we need several because of the 
      // non-blocking send routines, send might not be finished 
      // when we start recieving
      std::vector< ObjectStream > osSend ;

      // if data was noy send yet, do it now 
      if( _needToSend ) 
      {
        // resize message buffer vector 
        osSend.resize( _nLinks );

        // send data 
        send( osSend, dataHandle );
      }

      // now receive data 
      receive( dataHandle );
    }

    // all-to-all communication using the pack and unpack of the data handle 
    void allToAll( DataHandleIF& dataHandle )
    {
      // get mpi communicator
      MPI_Comm comm = _mpAccess.communicator();

      // create send and recv buffers 
      std::vector< ObjectStream > sendBuffers( _nLinks );
      std::vector< ObjectStream > recvBuffers( _nLinks );

      ALUGridFiniteStack< int, 1024 > sendLinks; 

      // don't push 0 to sendLinks because this is used for the first message
      for( int l=_nLinks-1; l > 0 ; --l )
      {
        sendLinks.push( l );
      }

      assert( _nLinks > 0 );

      // pack own data 
      dataHandle.pack( 0, sendBuffers[ 0 ] );

      int tag = _tag ;

      const int me = _mpAccess.myrank(); 
      const int np = _mpAccess.psize();
      const int totalMesg = np-1 ;

      // ring structure receive from previous and send to next 
      const int dest   = (me == totalMesg) ? 0    : me + 1; 
      const int source = (me == 0)    ? totalMesg : me - 1; 

      // send first message 
      sendLink( 0, dest, tag, sendBuffers[ 0 ], comm );

      int totalRecv = 0 ;
      while( totalRecv < totalMesg )
      {
        const int nLinks = ( (totalRecv + _nLinks) <= totalMesg ) ? _nLinks : totalMesg - totalRecv ;
        // get received vector 
        std::vector< bool > linkReceived( _nLinks, false );

        int received = 0 ;
        while( received < nLinks ) 
        {
          for( int l=0; l<nLinks; ++ l )
          {
            if( ! linkReceived[ l ] ) 
            {
              // checks for message and if received also fills osRecv
              if( checkAndReceive( comm, source, tag+l, recvBuffers[ l ] ) )
              {
                // get link number and send data to next process 
                int link = sendLinks.pop();
                sendBuffers[ link ].clear();
                sendBuffers[ link ].writeStream( recvBuffers[ l ] );
                sendLink( link, dest, _tag+link, sendBuffers[ link ], comm );

                // unpack data 
                dataHandle.unpack( l, recvBuffers[ l ] );
                // reset read-write position
                recvBuffers[ l ].clear();

                ++received ;
                linkReceived[ l ] = true ;
              }
            }
          }
        }

        // Is this really needed?
        // wait until all messages have been send and received 
        {
          MPI_Status * sta = new MPI_Status [ nLinks ];
          assert( sta );
          assert( _request );
          MY_INT_TEST MPI_Waitall ( nLinks, _request, sta);
          assert (test == MPI_SUCCESS);
          delete [] sta;
        }

        sendLinks.clear();
        for( int l=_nLinks-1; l >= 0 ; --l )
        {
          sendLinks.push( l );
        }

        // increase msg tags 
        tag += nLinks ;
      }
    }

  protected:  
    void sendLink( const int link, const int dest, const int tag, const ObjectStream& os, MPI_Comm& comm ) 
    {
      // get send buffer from object stream 
      char* buffer     = os._buf + os._rb;
      // get number of bytes to send 
      int   bufferSize = os._wb  - os._rb; 

      MY_INT_TEST MPI_Isend ( buffer, bufferSize, MPI_BYTE, dest, tag, comm, &_request[ link ] );
      assert (test == MPI_SUCCESS);
    }

    // does receive operation for one link 
    bool checkAndReceive( MPI_Comm& comm, 
                          const int source, 
                          const int tag,
                          ObjectStream& osRecv )
    {
      // corresponding MPI status 
      MPI_Status status;

      // received, 0 or 1 
      int received = 0;

      // check for any message with tag (nonblocking)
      MPI_Iprobe( source, tag, comm, &received, &status ); 

      // receive message of received flag is true 
      if( received ) 
      {
        // this should be the same, otherwise we got an error
        assert( source == status.MPI_SOURCE );

        // length of message 
        int bufferSize = -1;

        // get length of message 
        {
          MY_INT_TEST MPI_Get_count ( & status, MPI_BYTE, & bufferSize );
          assert (test == MPI_SUCCESS);
        }

        // reserve memory 
        osRecv.reserve( bufferSize );
        // reset read and write counter
        osRecv.clear();

        // MPI receive 
        {
          MY_INT_TEST MPI_Recv ( osRecv._buf, bufferSize, MPI_BYTE, status.MPI_SOURCE, tag, comm, & status);
          assert (test == MPI_SUCCESS);
        }

        // set wb of ObjectStream  
        osRecv.seekp( bufferSize );

        return true ; // received 
      }
      return false ;  // not yet received 
    }
  };

  inline MpAccessMPI::NonBlockingExchange*
  MpAccessMPI::nonBlockingExchange( const int tag, const std::vector< ObjectStream > & in ) const 
  {
    assert( tag > messagetag+1 );
    return new NonBlockingExchangeMPI( *this, tag, in );
  }

  inline MpAccessMPI::NonBlockingExchange*
  MpAccessMPI::nonBlockingExchange( const int tag ) const 
  {
    assert( tag > messagetag+1 );
    return new NonBlockingExchangeMPI( *this, tag );
  }

  // --exchange
  inline std::vector< ObjectStream > MpAccessMPI::exchange (const std::vector< ObjectStream > & in) const 
  {
    // note: for the non-blocking exchange the message tag 
    // should be different each time to avoid MPI problems 
    NonBlockingExchangeMPI nonBlockingExchange( *this, getMessageTag(), in );
    return nonBlockingExchange.receiveImpl();
  }

  // --exchange
  inline void MpAccessMPI::exchange ( const std::vector< ObjectStream > & in, NonBlockingExchange::DataHandleIF& handle ) const 
  {
    // note: for the non-blocking exchange the message tag 
    // should be different each time to avoid MPI problems 
    NonBlockingExchangeMPI nonBlockingExchange( *this, getMessageTag(), in );
    nonBlockingExchange.exchange( handle );
  }

  // --exchange
  inline void MpAccessMPI::exchange ( NonBlockingExchange::DataHandleIF& handle ) const 
  {
    // note: for the non-blocking exchange the message tag 
    // should be different each time to avoid MPI problems 
    NonBlockingExchangeMPI nonBlockingExchange( *this, getMessageTag() );
    nonBlockingExchange.exchange( handle );
  }

  // --all-to-all
  inline void MpAccessMPI::allToAll ( NonBlockingExchange::DataHandleIF& handle ) const 
  {
    // note: for the non-blocking exchange the message tag 
    // should be different each time to avoid MPI problems 
    NonBlockingExchangeMPI nonBlockingExchange( *this, getMessageTag(), psize() );
    nonBlockingExchange.allToAll( handle );
  }

} // namespace ALUGrid
