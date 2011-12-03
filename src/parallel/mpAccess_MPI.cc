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
  _psize( getSize() ) , _myrank( getRank() )
{
}

MpAccessMPI :: ~MpAccessMPI ()
{
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
  assert (test == MPI_SUCCESS) ;
  test = MPI_Comm_size (comm, & np) ;       
  assert (test == MPI_SUCCESS) ;
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
      for (int link = 0 ; link < nl ; link ++) 
      {
        int size = in [link].size() ;
        A * lne = new A [size] ;
        assert (lne) ;
        copy (in [link].begin (), in [link].end (), lne) ;
        buf [link] = lne ;
        MY_INT_TEST MPI_Issend (lne, size, mpiType, d [link], messagetag, comm, & req [link]) ;
        assert (test == MPI_SUCCESS) ;
      } 
    }
    
    {
      for (int link = 0 ; link < nl ; link ++ ) 
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
      for (int i = 0 ; i < nl ; i ++) delete [] buf [i] ; 
    }
    delete [] buf ;
    delete [] req ;
  }
  return out ;
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

vector < int > MpAccessMPI :: gcollect (int i) const {
  vector < int > r (psize (), 0L) ;
  int * v = new int [psize ()] ;
  mpi_allgather (&i, 1, v, 1) ;
  copy (v, v + psize (), r.begin ()) ;
  delete [] v ;
  return r ;
}

vector < double > MpAccessMPI :: gcollect (double a) const 
{
  vector < double > r (psize (),0.0) ;
  double * v = new double [psize ()] ;
  mpi_allgather (& a, 1, v, 1) ;
  copy (v, v + psize (), r.begin ()) ;
  delete [] v;
  return r ;
}

vector < vector < int > > MpAccessMPI :: gcollect (const vector < int > & v) const {
  return doGcollectV (v, MPI_INT, _mpiComm) ;
}

vector < vector < double > > MpAccessMPI :: gcollect (const vector < double > & v) const {
  return doGcollectV (v, MPI_DOUBLE, _mpiComm) ;
}

vector < ObjectStream > MpAccessMPI :: gcollect (const ObjectStream & in) const 
{
  // number of processes 
  const int np = psize (); 
  
  // size of buffer 
  const int snum = in._wb - in._rb ;
  
  // create empty objects streams 
  vector < ObjectStream > o (np) ;
  
  // collect sizes for all processors 
  vector < int > len = gcollect (snum) ;

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
  const int bufSize = displ [np - 1] + rcounts [np - 1] ;
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
        o [i]. write2Stream( y + displ [i], rcounts [i] );
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
  const MpAccessMPI :: CommIF* _mpiCommPtr;
  const vector< int >& _dest;

  const int _nLinks; 
  const int _tag;

  MPI_Request* _request;

  // create an unique tag for the communication 
  static int getTag() 
  {
    static int tagCounter = 0;
    ++ tagCounter;
    int tag = tagCounter ;

    // avoid overflow 
    if( tag < 0 ) 
    {
      tag = 0;
      tagCounter = 0;
    }
    return tag;
  }
public:
  NonBlockingExchangeMPI( const MpAccessMPI :: CommIF* comm,
                          const vector< int >& dest,
                          const int tag )
    : _mpiCommPtr( comm ),
      _dest( dest ),
      _nLinks( _dest.size() ),
      _tag( tag ),
      _request( ( _nLinks > 0 ) ? new MPI_Request [ _nLinks ] : 0)
  {
  }

  NonBlockingExchangeMPI( const MpAccessMPI :: CommIF* comm,
                          const vector< int >& dest,
                          const int tag,
                          const vector< ObjectStream > & in ) 
    : _mpiCommPtr( comm ),
      _dest( dest ),
      _nLinks( _dest.size() ),
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
  void receive( vector< ObjectStream > & out ) { receiveImpl( out ); }  
  vector< ObjectStream > receive() { return receiveImpl(); }

  //////////////////////////////////////////
  // implementation 
  //////////////////////////////////////////

  // send data implementation 
  void sendImpl( const vector< ObjectStream > & osv ) 
  {
    MPI_Comm comm = _mpiComm ;

    // send data 
    for (int link = 0 ; link < _nLinks ; ++link) 
    {
      // get send buffer from object stream 
      char* buff = osv[ link ]._buf + osv[ link ]._rb ;
      // get number of bytes to send 
      int   cnt  = osv[ link ]._wb  - osv[ link ]._rb ; 

      MY_INT_TEST MPI_Issend ( buff, cnt, MPI_BYTE, _dest[link], _tag, comm, & _request[ link ] ) ;
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
    MPI_Comm comm = _mpiComm ;

    // receive  data 
    for (int link = 0 ; link < _nLinks ; ++link ) 
    {
      MPI_Status s ;

      pair< char*, int > buff( (char *) 0, -1 );

      {
        MY_INT_TEST MPI_Probe ( _dest[link], _tag, comm, & s) ; 
        assert (test == MPI_SUCCESS) ;
      }
      {
        MY_INT_TEST MPI_Get_count ( & s, MPI_BYTE, & buff.second ) ;
        assert (test == MPI_SUCCESS) ;
      }

      assert( buff.second >= 0 );

      // use alloc from objects stream because this is the 
      // buffer of the object stream
      buff.first = ObjectStream :: allocateBuffer( buff.second );
      
      {
        MY_INT_TEST MPI_Recv ( buff.first, buff.second, MPI_BYTE, _dest[ link ], _tag, comm, & s) ;
        assert (test == MPI_SUCCESS) ;
      }

      // copy buffer and count to object stream 
      // this will only set the pointer in ObjectStream 
      out[ link ] = buff ;
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
};

MpAccessMPI :: NonBlockingExchange*
MpAccessMPI :: nonBlockingExchange( const int tag, const vector < ObjectStream > & in ) const 
{
  assert( tag > messagetag+1 );
  return new NonBlockingExchangeMPI( _mpiCommPtr, dest(), tag, in );
}

MpAccessMPI :: NonBlockingExchange*
MpAccessMPI :: nonBlockingExchange( const int tag ) const 
{
  assert( tag > messagetag+1 );
  return new NonBlockingExchangeMPI( _mpiCommPtr, dest(), tag );
}

// --exchange
vector < ObjectStream > MpAccessMPI :: exchange (const vector < ObjectStream > & in) const 
{
  NonBlockingExchangeMPI nonBlockingExchange( _mpiCommPtr, dest(), messagetag+1, in );
  return nonBlockingExchange.receiveImpl();
}

#undef _mpiComm
