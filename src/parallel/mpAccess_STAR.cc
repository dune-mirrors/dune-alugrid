// (c) bernhard schupp 1997 - 1998
#if HAVE_STARMPI
#include "mpAccess_STAR.h"

extern "C" {
  extern int
         STAR_Allreduce(void * send_buff, void * recv_buff, int count,
                        MPI_Datatype dtype, MPI_Op op, MPI_Comm comm, int call_site_id);
  extern int
         STAR_Allgather(void * send_buff, int send_count, MPI_Datatype send_type,
                        void * recv_buff, int recv_count, MPI_Datatype recv_type,
                        MPI_Comm comm, int call_site_id);
  extern int
         STAR_Allgatherv(void * send_buff, int send_count, MPI_Datatype send_type,
                         void * recv_buff, int * recv_counts, int * disps,
                         MPI_Datatype recv_type, MPI_Comm comm, int call_site_id);
}

#ifndef NDEBUG
#define MY_INT_TEST int test =
#else
#define MY_INT_TEST
#endif

static int getNextCallSiteId() 
{
  static int STAR_nextCallSiteID = 0;
  return STAR_nextCallSiteID++;
}

struct CallSiteId 
{

};

// workarround for old member variable 
#define _mpiComm (getMPICommunicator(_mpiCommPtr))

// the last parameter is the `call_site_id`. Each call has to have its own unique
// call_site_id 

int MpAccessSTAR_MPI :: star_allgather (int * i, int si, int * o, int so) const {
    static const int call_site_id = getNextCallSiteId();
    return STAR_Allgather (i, si, MPI_INT, o, so, MPI_INT, _mpiComm, call_site_id) ;
}

int MpAccessSTAR_MPI :: star_allgather (char * i, int si, char * o, int so) const {
    static const int call_site_id = getNextCallSiteId();
    return STAR_Allgather (i, si, MPI_BYTE, o, so, MPI_BYTE, _mpiComm, call_site_id) ;
}

int MpAccessSTAR_MPI :: star_allgather (double * i, int si, double * o, int so) const {
    static const int call_site_id = getNextCallSiteId();
    return STAR_Allgather (i, si, MPI_DOUBLE, o, so, MPI_DOUBLE, _mpiComm, call_site_id) ;
}

template < class A > vector < vector < A > > 
doGcollectV_STAR (const vector < A > & in, MPI_Datatype mpiType, MPI_Comm comm) 
{
  static const int call_site_id = getNextCallSiteId();
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
    MY_INT_TEST STAR_Allgather (& ln, 1, MPI_INT, rcounts, 1, MPI_INT, comm, call_site_id) ;
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
    test = STAR_Allgatherv (y, ln, mpiType, x, rcounts, displ, mpiType, comm, call_site_id) ;
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

int MpAccessSTAR_MPI :: gmax (int i) const {
  static const int call_site_id = getNextCallSiteId();
  int j ;
  MY_INT_TEST STAR_Allreduce (&i, &j, 1, MPI_INT, MPI_MAX, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

int MpAccessSTAR_MPI :: gmin (int i) const {
  static const int call_site_id = getNextCallSiteId();
  int j ;
  MY_INT_TEST STAR_Allreduce (&i, &j, 1, MPI_INT, MPI_MIN, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

int MpAccessSTAR_MPI :: gsum (int i) const {
  static const int call_site_id = getNextCallSiteId();
  int j ;
  MY_INT_TEST STAR_Allreduce (&i, &j, 1, MPI_INT, MPI_SUM, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

long MpAccessSTAR_MPI :: gmax (long i) const {
  static const int call_site_id = getNextCallSiteId();
  long j ;
  MY_INT_TEST STAR_Allreduce (&i, &j, 1, MPI_LONG, MPI_MAX, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

long MpAccessSTAR_MPI :: gmin (long i) const {
  static const int call_site_id = getNextCallSiteId();
  long j ;
  MY_INT_TEST STAR_Allreduce (&i, &j, 1, MPI_LONG, MPI_MIN, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

long MpAccessSTAR_MPI :: gsum (long i) const {
  static const int call_site_id = getNextCallSiteId();
  long j ;
  MY_INT_TEST STAR_Allreduce (&i, &j, 1, MPI_LONG, MPI_SUM, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return j ;
}

double MpAccessSTAR_MPI :: gmax (double a) const {
  static const int call_site_id = getNextCallSiteId();
  double x ;
  MY_INT_TEST STAR_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_MAX, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return x ;
}

double MpAccessSTAR_MPI :: gmin (double a) const {
  static const int call_site_id = getNextCallSiteId();
  double x ;
  MY_INT_TEST STAR_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_MIN, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return x ;
}

double MpAccessSTAR_MPI :: gsum (double a) const {
  static const int call_site_id = getNextCallSiteId();
  double x ;
  MY_INT_TEST STAR_Allreduce (&a, &x, 1, MPI_DOUBLE, MPI_SUM, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return x ;
}

void MpAccessSTAR_MPI :: gmax (double* a,int size,double *x) const {
  static const int call_site_id = getNextCallSiteId();
  MY_INT_TEST STAR_Allreduce (a, x, size, MPI_DOUBLE, MPI_MAX, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
}

void MpAccessSTAR_MPI :: gmin (double* a,int size,double *x) const {
  static const int call_site_id = getNextCallSiteId();
  MY_INT_TEST STAR_Allreduce (a, x, size, MPI_DOUBLE, MPI_MIN, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
}

void MpAccessSTAR_MPI :: gsum (double* a,int size,double *x) const {
  static const int call_site_id = getNextCallSiteId();
  MY_INT_TEST STAR_Allreduce (a, x, size, MPI_DOUBLE, MPI_SUM, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
}

pair<double,double> MpAccessSTAR_MPI :: gmax (pair<double,double> p) const {
  static const int call_site_id = getNextCallSiteId();
  double x[2] ;
  double a[2]={p.first,p.second};
  MY_INT_TEST STAR_Allreduce (a, x, 2, MPI_DOUBLE, MPI_MAX, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return pair<double,double>(x[0],x[1]) ;
}

pair<double,double> MpAccessSTAR_MPI :: gmin (pair<double,double> p) const {
  static const int call_site_id = getNextCallSiteId();
  double x[2] ;
  double a[2]={p.first,p.second};
  MY_INT_TEST STAR_Allreduce (a, x, 2, MPI_DOUBLE, MPI_MIN, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return pair<double,double>(x[0],x[1]) ;
}

pair<double,double> MpAccessSTAR_MPI :: gsum (pair<double,double> p) const {
  static const int call_site_id = getNextCallSiteId();
  double x[2] ;
  double a[2]={p.first,p.second};
  MY_INT_TEST STAR_Allreduce (a, x, 2, MPI_DOUBLE, MPI_SUM, _mpiComm, call_site_id) ;
  assert (test == MPI_SUCCESS) ;
  return pair<double,double>(x[0],x[1]) ;
}

vector < int > MpAccessSTAR_MPI :: gcollect (int i) const {
  vector < int > r (psize (), 0L) ;
  int * v = new int [psize ()] ;
  star_allgather (&i, 1, v, 1) ;
  copy (v, v + psize (), r.begin ()) ;
  delete [] v ;
  return r ;
}

vector < double > MpAccessSTAR_MPI :: gcollect (double a) const 
{
  vector < double > r (psize (),0.0) ;
  double * v = new double [psize ()] ;
  star_allgather (& a, 1, v, 1) ;
  copy (v, v + psize (), r.begin ()) ;
  delete [] v;
  return r ;
}

vector < vector < int > > MpAccessSTAR_MPI :: gcollect (const vector < int > & v) const {
  return doGcollectV_STAR (v, MPI_INT, _mpiComm) ;
}

vector < vector < double > > MpAccessSTAR_MPI :: gcollect (const vector < double > & v) const {
  return doGcollectV_STAR (v, MPI_DOUBLE, _mpiComm) ;
}

vector < ObjectStream > MpAccessSTAR_MPI :: gcollect (const ObjectStream & in) const 
{
  static const int call_site_id = getNextCallSiteId();
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
    MY_INT_TEST STAR_Allgatherv (in._buf + in._rb, snum, MPI_BYTE, y, rcounts, displ, MPI_BYTE, _mpiComm, call_site_id) ;
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

#undef _mpiComm
#endif
