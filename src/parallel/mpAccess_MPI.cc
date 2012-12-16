#include "mpAccess_MPI.h"

#ifndef NDEBUG
#define MY_INT_TEST int test =
#else
#define MY_INT_TEST
#endif

MPI_Comm getMPICommunicator(const MpAccessMPI :: CommIF* mpiCommPtr) 
{
  typedef MpAccessMPI :: Comm< MPI_Comm > MyComm;
  return static_cast<const MyComm&> (*mpiCommPtr);
}

template <>
MpAccessMPI :: Comm< MPI_Comm > :: Comm( MPI_Comm mpicomm ) 
{
  // duplicate mpi communicator 
  MY_INT_TEST MPI_Comm_dup ( mpicomm, &_mpiComm) ;
  assert (test == MPI_SUCCESS) ;
}

template <>
MpAccessMPI :: Comm< MPI_Comm > :: ~Comm( ) 
{
  // free mpi communicator 
  MY_INT_TEST MPI_Comm_free (&_mpiComm) ;
  assert (test == MPI_SUCCESS) ;
}

// workarround for old member variable 
#define _mpiComm (getMPICommunicator(_mpiCommPtr))

int MpAccessMPI :: getSize()
{
  // get size from MPI 
  int size = 0; 
  MY_INT_TEST MPI_Comm_size ( _mpiComm, & size );
  assert (test == MPI_SUCCESS) ;
  return size;
}

int MpAccessMPI :: getRank()
{
  // get rank from MPI 
  int rank = -1;
  MY_INT_TEST MPI_Comm_rank ( _mpiComm, & rank );
  assert (test == MPI_SUCCESS) ;
  return rank;
}

MpAccessMPI :: MpAccessMPI (const MpAccessMPI & a)
: _mpiCommPtr( a._mpiCommPtr->clone() ),
  _minmaxsum( 0 ),
  _psize( getSize() ) , _myrank( getRank() )
{
  initMinMaxSum();
}

MpAccessMPI :: ~MpAccessMPI ()
{
  delete _minmaxsum;
  delete _mpiCommPtr;
  _mpiCommPtr = 0;
}

int MpAccessMPI :: barrier () const {
    return MPI_SUCCESS == MPI_Barrier (_mpiComm) ? psize () : 0 ;
}

int MpAccessMPI :: mpi_allgather (int * i, int si, int * o, int so) const {
    return MPI_Allgather (i, si, MPI_INT, o, so, MPI_INT, _mpiComm) ;
}

int MpAccessMPI :: mpi_allgather (char * i, int si, char * o, int so) const {
    return MPI_Allgather (i, si, MPI_BYTE, o, so, MPI_BYTE, _mpiComm) ;
}

int MpAccessMPI :: mpi_allgather (double * i, int si, double * o, int so) const {
    return MPI_Allgather (i, si, MPI_DOUBLE, o, so, MPI_DOUBLE, _mpiComm) ;
}

template < class A > vector < vector < A > > 
doGcollectV (const vector < A > & in, MPI_Datatype mpiType, MPI_Comm comm) 
{
  int np, me, test ;

  test = MPI_Comm_rank (comm, & me) ;
  if (test != MPI_SUCCESS)
  {
    cerr << "Unable to obtain rank in MPI communicator." << endl;
    abort();
  }

  test = MPI_Comm_size (comm, & np) ;
  if (test != MPI_SUCCESS)
  {
    cerr << "Unable to obtain size of MPI communicator." << endl;
    abort();
  }

  int * rcounts = new int [np] ;
  int * displ = new int [np] ;
  assert (rcounts) ;
  vector < vector < A > > res (np) ;
  {
    int ln = in.size () ;
    MY_INT_TEST MPI_Allgather (& ln, 1, MPI_INT, rcounts, 1, MPI_INT, comm) ;
    assert (test == MPI_SUCCESS) ;
    displ [0] = 0 ;
    {for (int j = 1 ; j < np ; j ++) {
      displ [j] = displ [j-1] + rcounts [j-1];
    }}
    const int xSize = displ [np-1] + rcounts [np-1] ;
    A * x = new A [xSize] ;
    A * y = new A [ln] ;
    assert (x && y) ;
    copy (in.begin(), in.end(), y) ;
    test = MPI_Allgatherv (y, ln, mpiType, x, rcounts, displ, mpiType, comm) ;
    delete [] y ;
    y = 0 ;
    assert (test == MPI_SUCCESS) ;
    {for (int i = 0 ; i < np ; i ++ ) {
      res [i].reserve (rcounts [i]) ;
      copy (x + displ [i], x + displ [i] + rcounts [i], back_inserter(res [i])) ;
    }}
    delete [] x ;
  }
  delete [] displ ;
  delete [] rcounts ;
  return res ;
}

template < class A > 
vector < vector < A > > doExchange (const vector < vector < A > > & in,
                                    MPI_Datatype mpiType, 
                                    MPI_Comm comm, 
                                    const vector < int > & d) 
{
  const int messagetag = MpAccessMPI :: messagetag ;
  assert (in.size() == d.size()) ;
  int nl = d.size () ;
  vector < vector < A > > out (nl) ;
  {
    A ** buf = new A * [nl] ;
    assert (buf) ;
    MPI_Request * req = new MPI_Request [nl] ;
    assert (req) ;
    {
      for (int link = 0 ; link < nl ; ++ link) 
      {
        int size = in [link].size() ;
        A * lne = new A [size] ;
        assert (lne) ;
        copy (in [link].begin (), in [link].end (), lne) ;
        buf [link] = lne ;
        MY_INT_TEST MPI_Isend (lne, size, mpiType, d [link], messagetag, comm, & req [link]) ;
        assert (test == MPI_SUCCESS) ;
      } 
    }
    
    {
      for (int link = 0 ; link < nl ; ++ link) 
      {
        MPI_Status s ;
        int cnt ;
        {
          MY_INT_TEST MPI_Probe (d [link], messagetag, comm, & s) ;
          assert (test == MPI_SUCCESS) ;
        }
        
        {
          MY_INT_TEST MPI_Get_count ( & s, mpiType, & cnt ) ;
          assert (test == MPI_SUCCESS) ;
        }
        
        A * lne = new A [cnt] ;
        assert (lne) ;
        {
          MY_INT_TEST MPI_Recv (lne, cnt, mpiType, d [link], messagetag, comm, & s) ;
          assert (test == MPI_SUCCESS) ;
        }
        copy (lne, lne + cnt, back_inserter (out[link])) ;
        delete [] lne ;
      } 
    }

    {
      MPI_Status * sta = new MPI_Status [nl] ;
      assert (sta) ;
      MY_INT_TEST MPI_Waitall (nl, req, sta) ;
      assert (test == MPI_SUCCESS) ;
      delete [] sta ;
    }
    {
      for (int i = 0 ; i < nl ; ++i) delete [] buf [i] ; 
    }
    delete [] buf ;
    delete [] req ;
  }
  return out ;
}

bool MpAccessMPI :: gmax (bool i) const 
{
  int j = int( i );
  // call int method 
  const int ret = gmax( j );
  return (ret == 1) ? true : false ;
}

int MpAccessMPI :: gmax (int i) const {
  int j ;
  MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_INT, MPI_MAX, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

int MpAccessMPI :: gmin (int i) const {
  int j ;
  MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_INT, MPI_MIN, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

int MpAccessMPI :: gsum (int i) const {
  int j ;
  MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_INT, MPI_SUM, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

long MpAccessMPI :: gmax (long i) const {
  long j ;
  MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_LONG, MPI_MAX, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

long MpAccessMPI :: gmin (long i) const {
  long j ;
  MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_LONG, MPI_MIN, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

long MpAccessMPI :: gsum (long i) const {
  long j ;
  MY_INT_TEST MPI_Allreduce (&i, &j, 1, MPI_LONG, MPI_SUM, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

double MpAccessMPI :: gmax (double a) const {
  double x ;
  MY_INT_TEST MPI_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_MAX, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return x ;
}

double MpAccessMPI :: gmin (double a) const {
  double x ;
  MY_INT_TEST MPI_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_MIN, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return x ;
}

double MpAccessMPI :: gsum (double a) const {
  double x ;
  MY_INT_TEST MPI_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_SUM, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return x ;
}

void MpAccessMPI :: gmax (double* a,int size,double *x) const {
  MY_INT_TEST MPI_Allreduce (a, x, size, MPI_DOUBLE, MPI_MAX, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
}

void MpAccessMPI :: gmin (double* a,int size,double *x) const {
  MY_INT_TEST MPI_Allreduce (a, x, size, MPI_DOUBLE, MPI_MIN, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
}

void MpAccessMPI :: gsum (double* a,int size,double *x) const {
  MY_INT_TEST MPI_Allreduce (a, x, size, MPI_DOUBLE, MPI_SUM, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
}

void MpAccessMPI :: gmax (int* a,int size,int *x) const {
  MY_INT_TEST MPI_Allreduce (a, x, size, MPI_INT, MPI_MAX, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
}

void MpAccessMPI :: gmin (int* a,int size,int *x) const {
  MY_INT_TEST MPI_Allreduce (a, x, size, MPI_INT, MPI_MIN, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
}

void MpAccessMPI :: gsum (int* a,int size,int *x) const {
  MY_INT_TEST MPI_Allreduce (a, x, size, MPI_INT, MPI_SUM, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
}

struct MinMaxSumOp : public MpAccessMPI :: MinMaxSumIF 
{
  typedef MpAccessMPI :: minmaxsum_t  minmaxsum_t;

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

  const MpAccessMPI& _mpAccess ;

  MPI_Op _op; 
  MPI_Datatype _mpi_minmaxsum_t;

  static void 
  min_max_sum( minmaxsum_t* in, minmaxsum_t* inout, 
               int* len, MPI_Datatype* datatype ) 
  {
    const int size = *len ;
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
    const MpAccessMPI :: CommIF* _mpiCommPtr = _mpAccess.mpiCommPtr();
    // get mpi communicator (use define, see above)
    MPI_Comm comm = _mpiComm ;

    // create send buf 
    minmaxsum_t sendbuf( value ) ; 

    // create recv buf 
    minmaxsum_t result( value ); 

    MPI_Allreduce( &sendbuf, &result, 1, _mpi_minmaxsum_t, _op, comm );
    return result;
  }
};

void MpAccessMPI :: initMinMaxSum() 
{
  if( ! _minmaxsum ) 
    _minmaxsum = new MinMaxSumOp( *this );
}

MpAccessMPI :: minmaxsum_t  MpAccessMPI :: minmaxsum( double value ) const
{
  assert( _minmaxsum );
  return _minmaxsum->minmaxsum( value );
}

pair<double,double> MpAccessMPI :: gmax (pair<double,double> p) const {
  double x[2] ;
  double a[2]={p.first,p.second};
  MY_INT_TEST MPI_Allreduce (a, x, 2, MPI_DOUBLE, MPI_MAX, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return pair<double,double>(x[0],x[1]) ;
}

pair<double,double> MpAccessMPI :: gmin (pair<double,double> p) const {
  double x[2] ;
  double a[2]={p.first,p.second};
  MY_INT_TEST MPI_Allreduce (a, x, 2, MPI_DOUBLE, MPI_MIN, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return pair<double,double>(x[0],x[1]) ;
}

pair<double,double> MpAccessMPI :: gsum (pair<double,double> p) const {
  double x[2] ;
  double a[2]={p.first,p.second};
  MY_INT_TEST MPI_Allreduce (a, x, 2, MPI_DOUBLE, MPI_SUM, _mpiComm) ;
  assert (test == MPI_SUCCESS) ;
  return pair<double,double>(x[0],x[1]) ;
}

void MpAccessMPI :: bcast (int* buff, int length, int root ) const 
{
  MPI_Bcast(buff, length, MPI_INT, root, _mpiComm) ;
}

void MpAccessMPI :: bcast (char* buff, int length, int root ) const 
{
  MPI_Bcast(buff, length, MPI_BYTE, root, _mpiComm) ;
}

void MpAccessMPI :: bcast (double* buff, int length, int root ) const 
{
  MPI_Bcast(buff, length, MPI_DOUBLE, root, _mpiComm) ;
}

int MpAccessMPI :: exscan( int myvalue ) const 
{
  int sum = myvalue ;
  MPI_Exscan(&myvalue, &sum, 1, MPI_INT, MPI_SUM, _mpiComm) ;
  return sum;
}

int MpAccessMPI :: scan( int myvalue ) const 
{
  int sum = myvalue ;
  MPI_Scan(&myvalue, &sum, 1, MPI_INT, MPI_SUM, _mpiComm) ;
  return sum;
}

vector < int > MpAccessMPI :: gcollect (int i) const {
  vector < int > r (psize (), 0L) ;
  mpi_allgather (&i, 1, &r[ 0], 1) ;
  return r ;
}

vector < double > MpAccessMPI :: gcollect (double a) const 
{
  vector < double > r (psize (),0.0) ;
  mpi_allgather (& a, 1, &r[ 0 ], 1) ;
  return r ;
}

vector < vector < int > > MpAccessMPI :: gcollect (const vector < int > & v) const {
  return doGcollectV (v, MPI_INT, _mpiComm) ;
}

vector < vector < double > > MpAccessMPI :: gcollect (const vector < double > & v) const {
  return doGcollectV (v, MPI_DOUBLE, _mpiComm) ;
}

vector < ObjectStream > MpAccessMPI :: 
gcollect (const ObjectStream & in, const vector<int>& len ) const 
{
  // number of processes 
  const int np = psize (); 
  
  // size of buffer 
  const int snum = in._wb - in._rb ;
  
  // create empty objects streams 
  vector < ObjectStream > o (np) ;

#ifndef NDEBUG 
  // check that given length values are correct 
  {
    vector< int > checkLength = gcollect( snum );
    // check sizes 
    assert( len.size() == checkLength.size() );
    // make sure the list is correct 
    assert( equal( checkLength.begin(), checkLength.end(), len.begin() ) );
    /*
    if( ! equal( checkLength.begin(), checkLength.end(), len.begin() ) )
    {
      for(size_t i=0; i<checkLength.size(); ++i ) 
      {
        cout << "comm " << checkLength[ i ] << "  stored " << len[ i ] << endl;
      }
      abort();
    }
    */
  }
#endif
  
  int * rcounts = new int [np] ;
  assert (rcounts) ;
  copy (len.begin (), len.end (), rcounts) ;
  int * const displ = new int [np] ;
  assert (displ) ;
  
  // set offsets 
  displ [0] = 0 ;
  for (int j = 1 ; j < np ; ++j ) 
  { 
    displ [j] = displ [j - 1] + rcounts [j - 1] ; 
  }
  
  // overall buffer size 
  const size_t bufSize = displ [np - 1] + rcounts [np - 1] ;
  {    
    // allocate buffer 
    char * y = ObjectStream :: allocateBuffer(bufSize);
    assert (y) ;
    
    // gather all data 
    MY_INT_TEST MPI_Allgatherv (in._buf + in._rb, snum, MPI_BYTE, y, rcounts, displ, MPI_BYTE, _mpiComm) ;
    assert (test == MPI_SUCCESS) ;
    // copy data to object streams 
    for (int i = 0 ; i < np ; ++ i ) 
    {
      // write data to stream 
      if( rcounts [i] )
      {
        o [i].write( y + displ [i], rcounts [i] );
      }
    }

    // delete buffer 
    ObjectStream :: freeBuffer( y );
  }

  // delete helper functions 
  delete [] displ ;
  delete [] rcounts ;
  
  return o ;
}

vector < vector < int > > MpAccessMPI :: exchange (const vector < vector < int > > & in) const {
  assert (static_cast<int> (in.size ()) == nlinks ()) ;
  return doExchange (in, MPI_INT, _mpiComm, dest ()) ;
}

vector < vector < double > > MpAccessMPI :: exchange (const vector < vector < double > > & in) const {
  assert (static_cast<int> (in.size ()) == nlinks ()) ;
  return doExchange (in, MPI_DOUBLE, _mpiComm, dest ()) ;
}

vector < vector < char > > MpAccessMPI :: exchange (const vector < vector < char > > & in) const {
  assert (static_cast<int> (in.size ()) == nlinks ()) ;
  return doExchange (in, MPI_BYTE, _mpiComm, dest ()) ;
}

//////////////////////////////////////////////////////////////////////////
// non-blocking communication object 
// this class is defined here since it contains MPI information 
//////////////////////////////////////////////////////////////////////////
class NonBlockingExchangeMPI : public MpAccessLocal :: NonBlockingExchange
{
  const MpAccessMPI& _mpAccess;

  const int _nLinks; 
  const int _tag;

  MPI_Request* _request;

public:
  NonBlockingExchangeMPI( const MpAccessMPI& mpAccess,
                          const int tag )
    : _mpAccess( mpAccess ),
      _nLinks( _mpAccess.nlinks() ),
      _tag( tag ),
      _request( ( _nLinks > 0 ) ? new MPI_Request [ _nLinks ] : 0)
  {
  }

  NonBlockingExchangeMPI( const MpAccessMPI& mpAccess,
                          const int tag,
                          const vector< ObjectStream > & in ) 
    : _mpAccess( mpAccess ),
      _nLinks( _mpAccess.nlinks() ),
      _tag( tag ),
      _request( ( _nLinks > 0 ) ? new MPI_Request [ _nLinks ] : 0)
  {
    assert( _nLinks == int( in.size() ) );
    sendImpl( in ); 
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
  void send( const vector< ObjectStream > & in ) { sendImpl( in ); }
  vector< ObjectStream > receive() { return receiveImpl(); }

  //////////////////////////////////////////
  // implementation 
  //////////////////////////////////////////

  // send data implementation 
  void sendImpl( const vector< ObjectStream > & osv ) 
  {
    const MpAccessMPI :: CommIF* _mpiCommPtr = _mpAccess.mpiCommPtr();
    // get mpi communicator (use define, see above)
    MPI_Comm comm = _mpiComm ;

    // get vector with destinations 
    const vector< int >& dest = _mpAccess.dest();

    // send data 
    for (int link = 0 ; link < _nLinks ; ++link) 
    {
      // get send buffer from object stream 
      char* buffer     = osv[ link ]._buf + osv[ link ]._rb ;
      // get number of bytes to send 
      int   bufferSize = osv[ link ]._wb  - osv[ link ]._rb ; 

      MY_INT_TEST MPI_Isend ( buffer, bufferSize, MPI_BYTE, dest[ link ], _tag, comm, & _request[ link ] ) ;
      assert (test == MPI_SUCCESS) ;
    }
  }

  // receive data without buffer given 
  vector< ObjectStream > receiveImpl () 
  {
    // create vector of empty streams 
    vector < ObjectStream > out ( _nLinks ) ;
    receiveImpl( out );
    return out ; 
  }


  // receive data implementation with given buffers 
  void receiveImpl ( vector< ObjectStream >& out ) 
  {
    const MpAccessMPI :: CommIF* _mpiCommPtr = _mpAccess.mpiCommPtr();
    // get mpi communicator (use define, see above)
    MPI_Comm comm = _mpiComm ;

    // get vector with destinations 
    const vector< int >& dest = _mpAccess.dest();

#ifndef NDEBUG
    // check for all links messages 
    for (int link = 0 ; link < _nLinks ; ++link ) 
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
      for (int link = 0 ; link < _nLinks ; ++link ) 
      {
        ObjectStream& objStream = out[ link ];
        if( objStream.notReceived() ) 
        {
          // corresponding MPI status 
          MPI_Status status ;

          // received, 0 or 1 
          int received = 0 ;

          // check for any message with tag (nonblocking)
          MPI_Iprobe( dest[ link ], _tag, comm, &received, &status ) ; 

          // receive message of received flag is true 
          if( received ) 
          {
            // this should be the same, otherwise we got an error
            assert( dest[ link ] == status.MPI_SOURCE );

            // receive message for link 
            bufferpair_t buff = receiveLink( comm, status ) ;

#ifndef NDEBUG
            const int length = buff.second ;
#endif
            // copy to buffers (buff is reset to (0,0))
            // also sets objStream.notReceived() to false 
            objStream = buff ;

            // make sure buffer match 
            assert( length == objStream.size() );

            // increase number of received messages 
            ++ numReceived ;
          }
        }
      }
    }
    
    // wait until all processes are done with receiving
    {
      MPI_Status * sta = new MPI_Status [ _nLinks ] ;
      assert (sta) ;
      MY_INT_TEST MPI_Waitall ( _nLinks, _request, sta) ;
      assert (test == MPI_SUCCESS) ;
      delete [] sta ;
    }
  }
protected:  
  typedef std::pair< char*, int > bufferpair_t;
  // does receive operation for one link 
  bufferpair_t  receiveLink( MPI_Comm& comm, 
                             MPI_Status& status ) 
  {
    // length of message 
    int bufferSize = -1;

    // get length of message 
    {
      MY_INT_TEST MPI_Get_count ( & status, MPI_BYTE, & bufferSize ) ;
      assert (test == MPI_SUCCESS) ;
    }

    assert( bufferSize >= 0 );

    // use alloc from objects stream because this is the 
    // buffer of the object stream
    char* buffer = ObjectStream :: allocateBuffer( bufferSize );
    
    // MPI receive 
    {
      MY_INT_TEST MPI_Recv ( buffer, bufferSize, MPI_BYTE, status.MPI_SOURCE, _tag, comm, & status) ;
      assert (test == MPI_SUCCESS) ;
    }

    // copy buffer and count to object stream 
    // this will only set the pointer in ObjectStream 
    return bufferpair_t( buffer, bufferSize );
  }

};

MpAccessMPI :: NonBlockingExchange*
MpAccessMPI :: nonBlockingExchange( const int tag, const vector < ObjectStream > & in ) const 
{
  assert( tag > messagetag+1 );
  return new NonBlockingExchangeMPI( *this, tag, in );
}

MpAccessMPI :: NonBlockingExchange*
MpAccessMPI :: nonBlockingExchange( const int tag ) const 
{
  assert( tag > messagetag+1 );
  return new NonBlockingExchangeMPI( *this, tag );
}

// --exchange
vector < ObjectStream > MpAccessMPI :: exchange (const vector < ObjectStream > & in) const 
{
  NonBlockingExchangeMPI nonBlockingExchange( *this, messagetag+1, in );
  return nonBlockingExchange.receiveImpl();
}
#undef _mpiComm
