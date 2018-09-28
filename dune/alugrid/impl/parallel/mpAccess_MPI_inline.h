#include <iostream>

#include "mpAccess_MPI.h"
#include "../indexstack.h"

#ifdef ALUGRIDDEBUG
#define MY_INT_TEST int test =
#else
#define MY_INT_TEST
#endif

namespace ALUGrid
{

  static inline long int& allreduceCalled ()
  {
    static long int allreduce = 0;
    return allreduce;
  }

  inline void incrementAllreduceCalls()
  {
#ifdef ALUGRID_COUNT_GLOBALCOMM
    ++allreduceCalled();
#endif
  }

  static inline long int& allgatherCalled ()
  {
    static long int allgather = 0;
    return allgather;
  }

  inline void incrementAllgatherCalls()
  {
#ifdef ALUGRID_COUNT_GLOBALCOMM
    ++allgatherCalled();
#endif
  }

  inline MpAccessMPI::MinMaxSumIF* MpAccessMPI::copyMPIComm ( MPI_Comm mpicomm )
  {
    int wasInitialized = 0;
    // check if MPI was initialized, otherwise exit
    MPI_Initialized( &wasInitialized );
    if( ! wasInitialized )
    {
      std::cerr << "ERROR: MPI was not initialized at the beginning of the program."
                << "       Please add the corresponding call to MPI_Init, e.g. by adding `Dune::MPIHelper::instance( argc, argv );`"
                << "       as a first line of the main routine." << std::endl;
      std::abort();
    }

    // duplicate mpi communicator
    MY_INT_TEST MPI_Comm_dup ( mpicomm, &_mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return nullptr ; // return NULL pointer to initialize _mpimaxsum
  }

  inline int MpAccessMPI::getSize()
  {
    // get size from MPI
    int size = 0;
    MY_INT_TEST MPI_Comm_size ( _mpiComm, & size );
    alugrid_assert (test == MPI_SUCCESS);
    return size;
  }

  inline int MpAccessMPI::getRank()
  {
    // get rank from MPI
    int rank = -1;
    MY_INT_TEST MPI_Comm_rank ( _mpiComm, & rank );
    alugrid_assert (test == MPI_SUCCESS);
    return rank;
  }

  inline MpAccessMPI::MpAccessMPI ( MPI_Comm mpicomm )
   : _minmaxsum( copyMPIComm( mpicomm ) ),
     _psize( getSize() ), _myrank( getRank() )
  {
    initMinMaxSum();
  }



  inline MpAccessMPI::MpAccessMPI (const MpAccessMPI & a)
   : MpAccessLocal( a ),
     _minmaxsum( copyMPIComm( a._mpiComm ) ),
     _psize( getSize() ) , _myrank( getRank() )
  {
    initMinMaxSum();
  }

  inline MpAccessMPI::~MpAccessMPI ()
  {
    _minmaxsum.reset();

    int wasFinalized = -1;
    MPI_Finalized( &wasFinalized );
    if( !wasFinalized)
    {
      // free mpi communicator
      MY_INT_TEST MPI_Comm_free (&_mpiComm);
      alugrid_assert (test == MPI_SUCCESS);
    }
  }

  inline int MpAccessMPI::barrier () const {
    return MPI_SUCCESS == MPI_Barrier (_mpiComm) ? psize () : 0;
  }

  inline int MpAccessMPI::mpi_allgather (int * i, int si, int * o, int so) const {
    incrementAllgatherCalls();
    return MPI_Allgather (i, si, MPI_INT, o, so, MPI_INT, _mpiComm);
  }

  inline int MpAccessMPI::mpi_allgather (char * i, int si, char * o, int so) const {
    incrementAllgatherCalls();
    return MPI_Allgather (i, si, MPI_BYTE, o, so, MPI_BYTE, _mpiComm);
  }

  inline int MpAccessMPI::mpi_allgather (double * i, int si, double * o, int so) const {
    incrementAllgatherCalls();
    return MPI_Allgather (i, si, MPI_DOUBLE, o, so, MPI_DOUBLE, _mpiComm);
  }

  template < class A > std::vector< std::vector< A > >
  inline doGcollectV (const int np, const int me,
                      const std::vector< A > & in, MPI_Datatype mpiType, MPI_Comm comm)
  {
    int * rcounts = new int [np];
    int * displ = new int [np];
    alugrid_assert (rcounts);
    std::vector< std::vector< A > > res (np);
    {
      int ln = in.size ();
      {
        MY_INT_TEST MPI_Allgather (& ln, 1, MPI_INT, rcounts, 1, MPI_INT, comm);
        alugrid_assert (test == MPI_SUCCESS);
      }
      displ [0] = 0;
      {for (int j = 1; j < np; j ++) {
        displ [j] = displ [j-1] + rcounts [j-1];
      }}
      const int xSize = displ [np-1] + rcounts [np-1];
      A * x = new A [xSize];
      A * y = new A [ln];
      alugrid_assert (x && y);
      std::copy (in.begin(), in.end(), y);
      {
        MY_INT_TEST MPI_Allgatherv (y, ln, mpiType, x, rcounts, displ, mpiType, comm);
        alugrid_assert (test == MPI_SUCCESS);
      }
      delete [] y;
      y = 0;
      {for (int i = 0; i < np; i ++ ) {
        res [i].reserve (rcounts [i]);
        std::copy (x + displ [i], x + displ [i] + rcounts [i], back_inserter(res [i]));
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
    incrementAllreduceCalls();
    int j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_INT, MPI_MAX, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return j;
  }

  inline int MpAccessMPI::gmin (int i) const {
    incrementAllreduceCalls();
    int j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_INT, MPI_MIN, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return j;
  }

  inline int MpAccessMPI::gsum (int i) const {
    incrementAllreduceCalls();
    int j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_INT, MPI_SUM, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return j;
  }

  inline long MpAccessMPI::gmax (long i) const {
    incrementAllreduceCalls();
    long j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_LONG, MPI_MAX, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return j;
  }

  inline long MpAccessMPI::gmin (long i) const {
    incrementAllreduceCalls();
    long j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_LONG, MPI_MIN, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return j;
  }

  inline long MpAccessMPI::gsum (long i) const {
    incrementAllreduceCalls();
    long j;
    MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_LONG, MPI_SUM, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return j;
  }

  inline double MpAccessMPI::gmax (double a) const {
    incrementAllreduceCalls();
    double x;
    MY_INT_TEST MPI_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_MAX, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return x;
  }

  inline double MpAccessMPI::gmin (double a) const {
    incrementAllreduceCalls();
    double x;
    MY_INT_TEST MPI_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_MIN, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return x;
  }

  inline double MpAccessMPI::gsum (double a) const {
    incrementAllreduceCalls();
    double x;
    MY_INT_TEST MPI_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_SUM, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return x;
  }

  inline void MpAccessMPI::gmax (double* a,int size,double *x) const {
    incrementAllreduceCalls();
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_DOUBLE, MPI_MAX, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
  }

  inline void MpAccessMPI::gmin (double* a,int size,double *x) const {
    incrementAllreduceCalls();
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_DOUBLE, MPI_MIN, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
  }

  inline void MpAccessMPI::gsum (double* a,int size,double *x) const {
    incrementAllreduceCalls();
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_DOUBLE, MPI_SUM, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
  }

  inline void MpAccessMPI::gmax (int* a,int size,int *x) const {
    incrementAllreduceCalls();
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_INT, MPI_MAX, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
  }

  inline void MpAccessMPI::gmin (int* a,int size,int *x) const {
    incrementAllreduceCalls();
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_INT, MPI_MIN, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
  }

  inline void MpAccessMPI::gsum (int* a,int size,int *x) const {
    incrementAllreduceCalls();
    MY_INT_TEST MPI_Allreduce (a, x, size, MPI_INT, MPI_SUM, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
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
      int wasFinalized = -1;
      MPI_Finalized( &wasFinalized );
      if( !wasFinalized)
      {
        MPI_Op_free (&_op);
        MPI_Type_free(&_mpi_minmaxsum_t);
      }
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
    {
      _minmaxsum.reset( new MinMaxSumOp( *this ) );
    }
  }

  inline MpAccessMPI::minmaxsum_t  MpAccessMPI::minmaxsum( double value ) const
  {
    incrementAllreduceCalls();
    alugrid_assert ( _minmaxsum );
    return _minmaxsum->minmaxsum( value );
  }

  inline std::pair<double,double> MpAccessMPI::gmax (std::pair<double,double> p) const {
    incrementAllreduceCalls();
    double x[2];
    double a[2]={p.first,p.second};
    MY_INT_TEST MPI_Allreduce (a, x, 2, MPI_DOUBLE, MPI_MAX, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return std::pair<double,double>(x[0],x[1]);
  }

  inline std::pair<double,double> MpAccessMPI::gmin (std::pair<double,double> p) const {
    incrementAllreduceCalls();
    double x[2];
    double a[2]={p.first,p.second};
    MY_INT_TEST MPI_Allreduce (a, x, 2, MPI_DOUBLE, MPI_MIN, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
    return std::pair<double,double>(x[0],x[1]);
  }

  inline std::pair<double,double> MpAccessMPI::gsum (std::pair<double,double> p) const {
    incrementAllreduceCalls();
    double x[2];
    double a[2]={p.first,p.second};
    MY_INT_TEST MPI_Allreduce (a, x, 2, MPI_DOUBLE, MPI_SUM, _mpiComm);
    alugrid_assert (test == MPI_SUCCESS);
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

  inline void MpAccessMPI::bcast ( ObjectStream& inout, int root ) const
  {
    char* buffer = inout._buf;
    int size = inout.capacity();
    // size has to be the same on all processes for bcast operation
    alugrid_assert( size == gmax( size ) );
    bcast( buffer, size, root );
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
    mpi_allgather (&i, 1, &r[ 0 ], 1);
    return r;
  }

  inline std::vector< double > MpAccessMPI::gcollect (double a) const
  {
    std::vector< double > r (psize (),0.0);
    mpi_allgather (& a, 1, &r[ 0 ], 1);
    return r;
  }

  inline std::vector< std::vector< int > > MpAccessMPI::gcollect (const std::vector< int > & v) const {
    incrementAllgatherCalls();
    return doGcollectV (psize(), myrank(), v, MPI_INT, _mpiComm);
  }

  inline std::vector< std::vector< double > > MpAccessMPI::gcollect (const std::vector< double > & v) const {
    incrementAllgatherCalls();
    return doGcollectV (psize(), myrank(), v, MPI_DOUBLE, _mpiComm);
  }

  inline std::vector< ObjectStream > MpAccessMPI::
  gcollect (const ObjectStream & in, const std::vector<int>& len ) const
  {
    incrementAllgatherCalls();

    // number of processes
    const int np = psize ();

    // size of buffer
    const int snum = in._wb - in._rb;

    // check that size is correct
    alugrid_assert( snum == len [ myrank() ] );

    // create empty objects streams
    std::vector< ObjectStream > o (np);

#ifdef ALUGRIDDEBUG
    // check that given length values are correct
    {
      std::vector< int > checkLength = gcollect( snum );
      // check sizes
      alugrid_assert ( len.size() == checkLength.size() );
      if( ! equal( checkLength.begin(), checkLength.end(), len.begin() ) )
      {
        for(size_t i=0; i<checkLength.size(); ++i )
          std::cout << "comm " << checkLength[ i ] << "  stored " << len[ i ] << std::endl;
        abort();
      }
      // make sure the list is correct
      alugrid_assert ( equal( checkLength.begin(), checkLength.end(), len.begin() ) );
    }
#endif

    int * rcounts = new int [np];
    alugrid_assert (rcounts);
    std::copy (len.begin (), len.end (), rcounts);
    int * const displ = new int [np];
    alugrid_assert (displ);

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
      alugrid_assert (y);

      // gather all data
      MY_INT_TEST MPI_Allgatherv (in._buf + in._rb, snum, MPI_BYTE, y, rcounts, displ, MPI_BYTE, _mpiComm);
      alugrid_assert (test == MPI_SUCCESS);
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

    const int _sendLinks;
    const int _recvLinks;
    const int _tag;

    MPI_Request* _sendRequest;
    MPI_Request* _recvRequest;

    const bool _symmetric;
    bool _needToSend ;

    //static const bool oldBlockingMethod = true ;
    static const bool oldBlockingMethod = false ;

    // no copying
    NonBlockingExchangeMPI( const NonBlockingExchangeMPI& );

    // return vector of send requests for number of send links is positive
    MPI_Request* createSendRequest() const
    {
      return ( _sendLinks > 0 ) ? new MPI_Request [ _sendLinks ] : 0;
    }

    // return vector of recv requests when
    // number of recv links is positive and symmetric is true
    MPI_Request* createRecvRequest( const bool symmetric ) const
    {
      return ( _recvLinks > 0 && symmetric ) ? new MPI_Request [ _recvLinks ] : 0;
    }

  public:
    typedef MpAccessLocal::NonBlockingExchange::DataHandleIF DataHandleIF;

    NonBlockingExchangeMPI( const MpAccessMPI& mpAccess,
                            const int tag,
                            const bool symmetric = false )
      : _mpAccess( mpAccess ),
        _sendLinks( _mpAccess.sendLinks() ),
        _recvLinks( _mpAccess.recvLinks() ),
        _tag( tag ),
        _sendRequest( createSendRequest() ),
        _recvRequest( createRecvRequest( symmetric ) ),
        _symmetric( symmetric ),
        _needToSend( true )
    {
      // make sure every process has the same tag
      alugrid_assert ( tag == mpAccess.gmax( tag ) );
    }

    NonBlockingExchangeMPI( const MpAccessMPI& mpAccess,
                            const int tag,
                            const std::vector< ObjectStream > & in )
      : _mpAccess( mpAccess ),
        _sendLinks( _mpAccess.sendLinks() ),
        _recvLinks( _mpAccess.recvLinks() ),
        _tag( tag ),
        _sendRequest( createSendRequest() ),
        _recvRequest( createRecvRequest( false ) ),
        _symmetric( false ),
        _needToSend( false )
    {
      // make sure every process has the same tag
      alugrid_assert ( tag == mpAccess.gmax( tag ) );

      alugrid_assert ( _sendLinks == int( in.size() ) );
      sendImpl( in );
    }

    /////////////////////////////////////////
    //  interface methods
    /////////////////////////////////////////
    ~NonBlockingExchangeMPI()
    {
      if( _sendRequest )
      {
        delete [] _sendRequest;
        _sendRequest = 0;
      }

      if( _recvRequest )
      {
        delete [] _recvRequest;
        _recvRequest = 0;
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
      const std::vector< int >& sendDest = _mpAccess.sendDest();

      // send data
      for (int link = 0; link < _sendLinks; ++link)
      {
        sendLink( sendDest[ link ], _tag, osv[ link ], _sendRequest[ link ], comm );
      }

      // set send info
      _needToSend = false ;
    }

    // receive data without buffer given
    std::vector< ObjectStream > receiveImpl ()
    {
      // create vector of empty streams
      std::vector< ObjectStream > out ( _recvLinks );
      receiveImpl( out );
      return out;
    }

    // receive data implementation with given buffers
    void receiveImpl ( std::vector< ObjectStream >& out, DataHandleIF* dataHandle = 0)
    {
      // do nothing if number of links is zero
      if( (_recvLinks + _sendLinks) == 0 ) return;

      // get mpi communicator
      MPI_Comm comm = _mpAccess.communicator();

      // get vector with destinations
      const std::vector< int >& recvSource = _mpAccess.recvSource();

      // check whether out vector has more than one stream
      const bool useFirstStreamOnly = (out.size() == 1) ;
#ifdef ALUGRIDDEBUG
      {
        const int outSize = out.size();
        // check for all links messages
        for (int link = 0; link < outSize; ++link )
        {
          // contains no written data
          alugrid_assert ( out[ link ].notReceived() );
        }
      }
#endif

      // flag vector holding information about received links
      std::vector< bool > linkNotReceived( _recvLinks, true );

      // count noumber of received messages
      int numReceived = 0;
      while( numReceived < _recvLinks )
      {
        // check for all links messages
        for (int link = 0; link < _recvLinks; ++link )
        {
          // if message was not received yet, check again
          if( linkNotReceived[ link ] )
          {
            // get appropriate object stream
            ObjectStream& osRecv = useFirstStreamOnly ? out[ 0 ] : out[ link ];

            // check whether a message was completely received
            // if message was received the unpack data
            if( probeAndReceive( comm, recvSource[ link ], _tag, osRecv ) )
            {
              // if data handle was given do unpack
              if( dataHandle ) dataHandle->unpack( link, osRecv );

              // mark link as received
              linkNotReceived[ link ] = false ;

              // increase number of received messages
              ++ numReceived;
            }
          }
        }
      }

      // if send request exists, i.e. some messages have been sent
      if( _sendRequest )
      {
        // wait until all processes are done with receiving
        MY_INT_TEST MPI_Waitall ( _sendLinks, _sendRequest, MPI_STATUSES_IGNORE);
        alugrid_assert (test == MPI_SUCCESS);
      }
    }

    // receive data implementation with given buffers
    void unpackSymmetric( std::vector< ObjectStream >& out, DataHandleIF& dataHandle )
    {
      // do nothing if number of links is zero
      if( _recvLinks == 0 ) return;

      // flag vector holding information about received links
      std::vector< bool > linkNotReceived( _recvLinks, true );

      // count noumber of received messages
      int numReceived = 0;
      while( numReceived < _recvLinks )
      {
        // check for all links messages
        for (int link = 0; link < _recvLinks; ++link )
        {
          // if message was not received yet, check again
          if( linkNotReceived[ link ] )
          {
            int received = 0;
            assert( _recvRequest );
            MPI_Request& request = _recvRequest[ link ];
            // if receive of message is finished, unpack
            MPI_Test( &request, &received, MPI_STATUS_IGNORE );
            if( received )
            {
              // if data handle was given do unpack
              dataHandle.unpack( link, out[ link ] );

              // mark link as received
              linkNotReceived[ link ] = false ;
              // increase number of received messages
              ++ numReceived;
            }
          }
        }
      }

      // if send request exists, i.e. some messages have been sent
      if( _sendRequest )
      {
        // wait until all processes are done with receiving
        MY_INT_TEST MPI_Waitall ( _sendLinks, _sendRequest, MPI_STATUSES_IGNORE);
        alugrid_assert (test == MPI_SUCCESS);
      }
    }

    // receive data implementation with given buffers
    void send( std::vector< ObjectStream >& osSend,
               DataHandleIF& dataHandle )
    {
      std::vector< ObjectStream > osRecv;
      send( osSend, osRecv, dataHandle );
    }

    // receive data implementation with given buffers
    void send( std::vector< ObjectStream >& osSend,
               std::vector< ObjectStream >& osRecv,
               DataHandleIF& dataHandle )
    {
      if( oldBlockingMethod )
      {
        // send data
        for (int link = 0; link < _sendLinks; ++link)
        {
          // pack data
          dataHandle.pack( link, osSend[ link ] );
        }
      }

      if( _needToSend )
      {
        // get mpi communicator
        MPI_Comm comm = _mpAccess.communicator();

        // get vector with destinations
        const std::vector< int >& sendDest = _mpAccess.sendDest();

        // send data
        for (int link = 0; link < _sendLinks; ++link)
        {
          if( ! oldBlockingMethod )
          {
            // pack data
            dataHandle.pack( link, osSend[ link ] );
          }

          // send data
          sendLink( sendDest[ link ], _tag, osSend[ link ], _sendRequest[ link ], comm );
        }

        // set send info
        _needToSend = false ;
      }

      // resize receive buffer if in symmetric mode
      if( _symmetric )
      {
        // get mpi communicator
        MPI_Comm comm = _mpAccess.communicator();

        assert( _sendLinks == _recvLinks );
        osRecv.resize( _recvLinks );

        // get vector with destinations
        const std::vector< int >& recvSource = _mpAccess.recvSource();

        // send data
        for (int link = 0; link < _recvLinks; ++link)
        {
          // send data
          const int bufferSize = osSend[ link ].size();

          // post receive if in symmetric mode
          assert( _recvRequest );
          assert( &_recvRequest[ link ] );
          postReceive( recvSource[ link ], _tag, bufferSize, osRecv[ link ], _recvRequest[ link ], comm );
        }
      }
    }

    // receive data implementation with given buffers
    void receive( DataHandleIF& dataHandle )
    {
      if( oldBlockingMethod )
      {
        // create receive message buffers
        std::vector< ObjectStream > out( _recvLinks );

        // receive data
        receiveImpl( out );

        // do work that can be done between send and receive
        dataHandle.localComputation() ;

        for( int link=0; link<_recvLinks; ++link )
          dataHandle.unpack( link, out[ link ] );
      }
      else
      {
        // do work that can be done between send and receive
        dataHandle.localComputation() ;

        // create receive message buffers
        std::vector< ObjectStream > out( 1 );
        // receive data
        receiveImpl( out, &dataHandle );
      }
    }

    // receive data implementation with given buffers
    void exchange( DataHandleIF& dataHandle )
    {
      const int recvLinks = _mpAccess.recvLinks();
      // do nothing if number of links is zero
      if( (recvLinks + _sendLinks) == 0 ) return;

      // send message buffers, we need several because of the
      // non-blocking send routines, send might not be finished
      // when we start recieving
      std::vector< ObjectStream > osSend ;
      std::vector< ObjectStream > osRecv ;

      // if data was noy send yet, do it now
      if( _needToSend )
      {
        // resize message buffer vector
        osSend.resize( _sendLinks );

        // send data
        send( osSend, osRecv, dataHandle );
      }

      // now receive data
      if( _symmetric )
        unpackSymmetric( osRecv, dataHandle );
      else
        receive( dataHandle );
    }

  protected:
    int sendLink( const int dest, const int tag, const ObjectStream& os, MPI_Request& request, MPI_Comm& comm )
    {
      // get send buffer from object stream
      char* buffer     = os._buf + os._rb;
      // get number of bytes to send
      int   bufferSize = os._wb  - os._rb;

      MY_INT_TEST MPI_Isend ( buffer, bufferSize, MPI_BYTE, dest, tag, comm, &request );
      alugrid_assert (test == MPI_SUCCESS);

      return bufferSize ;
    }

    void postReceive( const int source, const int tag, const int bufferSize,
                      ObjectStream& os, MPI_Request& request, MPI_Comm& comm )
    {
      // reserve memory
      os.reserve( bufferSize );
      // reset read and write counter
      os.clear();

      // set wb of ObjectStream, this also sets notReceived to false
      os.seekp( bufferSize );

      /*
      int available = 0;
      MPI_Status status ;

      while( ! available )
      {
        available = 0;
        // check for any message with tag (nonblocking)
        MPI_Iprobe( source, tag, comm, &available, &status );
      }

      assert( available );

      // length of message
      int bSize = -1;

      // get length of message
      {
        MY_INT_TEST MPI_Get_count ( & status, MPI_BYTE, & bSize );
        alugrid_assert (test == MPI_SUCCESS);
      }

      if( bSize != bufferSize )
        std::cout << bSize << " " << bufferSize << std::endl;
      assert( bSize == bufferSize );
      */

      // MPI receive (non-blocking)
      {
        MY_INT_TEST MPI_Irecv ( os._buf, bufferSize, MPI_BYTE, source, tag, comm, & request);
        alugrid_assert (test == MPI_SUCCESS);
      }
    }

    // does receive operation for one link
    bool receivedMessage( MPI_Request& request, ObjectStream& os )
    {
      MPI_Status status ;

      // msg received, 0 or 1
      int received = 0;

      // check for any message with tag (nonblocking)
      MPI_Test( & request, &received, &status );

      return bool(received);
    }

    // does receive operation for one link
    bool probeAndReceive( MPI_Comm& comm,
                          const int source,
                          const int tag,
                          ObjectStream& osRecv )
    {
      // corresponding MPI status
      MPI_Status status;

      // msg available, 0 or 1
      // available does not mean already received
      int available = 0;

      // check for any message with tag (nonblocking)
      MPI_Iprobe( source, tag, comm, &available, &status );

      // receive message if available flag is true
      if( available )
      {
        // this should be the same, otherwise we got an error
        alugrid_assert ( source == status.MPI_SOURCE );

        // length of message
        int bufferSize = -1;

        // get length of message
        {
          MY_INT_TEST MPI_Get_count ( & status, MPI_BYTE, & bufferSize );
          alugrid_assert (test == MPI_SUCCESS);
        }

        // reserve memory
        osRecv.reserve( bufferSize );
        // reset read and write counter
        osRecv.clear();

        // MPI receive (blocking)
        {
          MY_INT_TEST MPI_Recv ( osRecv._buf, bufferSize, MPI_BYTE, status.MPI_SOURCE, tag, comm, & status);
          alugrid_assert (test == MPI_SUCCESS);
        }

        // set wb of ObjectStream, this also sets notReceived to false
        osRecv.seekp( bufferSize );

        return true ; // received
      }
      return false ;  // not yet received
    }
  };

  inline MpAccessMPI::NonBlockingExchange*
  MpAccessMPI::nonBlockingExchange( const int tag, const std::vector< ObjectStream > & in ) const
  {
    alugrid_assert ( tag > messagetag+1 );
    return new NonBlockingExchangeMPI( *this, tag, in );
  }

  inline MpAccessMPI::NonBlockingExchange*
  MpAccessMPI::nonBlockingExchange( const int tag ) const
  {
    alugrid_assert ( tag > messagetag+1 );
    return new NonBlockingExchangeMPI( *this, tag );
  }

  inline MpAccessMPI::NonBlockingExchange *MpAccessMPI::nonBlockingExchange () const
  {
    return new NonBlockingExchangeMPI( *this, getMessageTag() );
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

  // --exchangeSymmetric
  inline void MpAccessMPI::exchangeSymmetric ( NonBlockingExchange::DataHandleIF& handle ) const
  {
    // this only works with symmetric linkage
    alugrid_assert( symmetricLinkage() );
    // note: for the non-blocking exchange the message tag
    // should be different each time to avoid MPI problems
    NonBlockingExchangeMPI nonBlockingExchange( *this, getMessageTag() );
    nonBlockingExchange.exchange( handle );
  }

} // namespace ALUGrid
