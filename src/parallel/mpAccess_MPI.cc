// (c) bernhard schupp 1997 - 1998
#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <stdio.h>
#ifdef _ANSI_HEADER
  using namespace std;
  #include <utility>
  #include <algorithm>
  #include <iterator>
#else
  #include <pair.h>
  #include <algo.h>
  #include <iterator.h>
#endif

#include "mpAccess_MPI.h"

template < class A > vector < vector < A > > doGcollectV 
	(const vector < A > & in, MPI_Datatype mpiType, MPI_Comm comm) {
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

static vector < pair < char *, int > > doExchange (const vector < pair < char *, int > > & in, 
	MPI_Comm comm, const vector < int > & d) {
  assert (in.size() == d.size()) ;
  int nl = d.size () ;
  vector < pair < char *, int > > out (nl) ;
  {
    MPI_Request * req = new MPI_Request [nl] ; 
    assert (req) ;
    {
      for (int link = 0 ; link < nl ; link ++) {
	MY_INT_TEST MPI_Issend (in [link].first, in [link].second, MPI_BYTE, d [link], 123, comm, & req [link]) ;
	assert (test == MPI_SUCCESS) ;
      }
    }
    {
      for (int link = 0 ; link < nl ; link ++ ) {
	MPI_Status s ;
	int cnt ;
  {
	  MY_INT_TEST MPI_Probe (d [link], 123, comm, & s) ; 
	  assert (test == MPI_SUCCESS) ;
  }
  {
	  MY_INT_TEST MPI_Get_count ( & s, MPI_BYTE, & cnt ) ;
	  assert (test == MPI_SUCCESS) ;
  }
	char * lne = new char [cnt] ;
	assert (lne) ;
  {
	  MY_INT_TEST MPI_Recv (lne, cnt, MPI_BYTE, d [link], 123, comm, & s) ;
	  assert (test == MPI_SUCCESS) ;
  }
	out [link].first = lne ;
	out [link].second = cnt ;
      }
    }
    {
      MPI_Status * sta = new MPI_Status [nl] ;
      assert (sta) ;
      MY_INT_TEST MPI_Waitall (nl, req, sta) ;
      assert (test == MPI_SUCCESS) ;
      delete [] sta ;
    }
    delete [] req ;
  }
  return out ;
}

template < class A > 
vector < vector < A > > doExchange (const vector < vector < A > > & in,
	MPI_Datatype mpiType, MPI_Comm comm, const vector < int > & d) {
  assert (in.size() == d.size()) ;
  int nl = d.size () ;
  vector < vector < A > > out (nl) ;
  {
    A ** buf = new A * [nl] ;
    assert (buf) ;
    MPI_Request * req = new MPI_Request [nl] ;
    assert (req) ;
    {
      for (int link = 0 ; link < nl ; link ++) {
	int size = in [link].size() ;
	A * lne = new A [size] ;
	assert (lne) ;
	copy (in [link].begin (), in [link].end (), lne) ;
	buf [link] = lne ;
	MY_INT_TEST MPI_Issend (lne, size, mpiType, d [link], 123, comm, & req [link]) ;
	assert (test == MPI_SUCCESS) ;
      } 
    }
    {
      for (int link = 0 ; link < nl ; link ++ ) {
	MPI_Status s ;
	int cnt ;
  {
	  MY_INT_TEST MPI_Probe (d [link], 123, comm, & s) ;
	  assert (test == MPI_SUCCESS) ;
  }
  
  {
	  MY_INT_TEST MPI_Get_count ( & s, mpiType, & cnt ) ;
	  assert (test == MPI_SUCCESS) ;
  }
  
	A * lne = new A [cnt] ;
	assert (lne) ;
  {
	  MY_INT_TEST MPI_Recv (lne, cnt, mpiType, d [link], 123, comm, & s) ;
	  assert (test == MPI_SUCCESS) ;
  }
	copy (lne, lne + cnt, back_inserter (out[link])) ;
	delete [] lne ;
      } 
    }
    {
      MPI_Status * sta = new MPI_Status [nl] ;
      assert (sta) ;
#ifndef NDEBUG
      MY_INT_TEST
#endif
        MPI_Waitall (nl, req, sta) ;
      assert (test == MPI_SUCCESS) ;
      delete [] sta ;
    }
    {for (int i = 0 ; i < nl ; i ++) delete [] buf [i] ; }
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

vector < double > MpAccessMPI :: gcollect (double a) const {
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

vector < ObjectStream > MpAccessMPI :: gcollect (const ObjectStream & in) const {
  const int np = psize (), snum = in._wb - in._rb ;
  vector < ObjectStream > o (np) ;
  vector < int > len = gcollect (snum) ;
  int * rcounts = new int [np] ;
  assert (rcounts) ;
  copy (len.begin (), len.end (), rcounts) ;
  int * const displ = new int [np] ;
  assert (displ) ;
  displ [0] = 0 ;
  for (int j = 1 ; j < np ; j ++) { displ [j] = displ [j - 1] + rcounts [j - 1] ; }
  const int bufSize = displ [np - 1] + rcounts [np - 1] ;
  {    
    char * y = new char [bufSize] ;
    assert (y) ;
    MY_INT_TEST MPI_Allgatherv (in._buf + in._rb, snum, MPI_BYTE, y, rcounts, displ, MPI_BYTE, _mpiComm) ;
    assert (test == MPI_SUCCESS) ;
    {for (int i = 0 ; i < np ; i ++ ) {
      if (rcounts [i]) {
        o [i]._buf = (char *) realloc (o [i]._buf, (o [i]._len = rcounts [i])) ;
	if (o [i]._buf) {
          memcpy (o [i]._buf, y + displ [i], rcounts [i]) ;
	  o [i]._rb = 0 ;
          o [i]._wb = rcounts [i] ;
	} else {
	  perror ("in MpAccessMPI :: gcollect (const ObjectStream &)") ;
	  exit (1) ;
	}
      } else {
        free (o [i]._buf) ;
	o [i]._buf = 0 ;
	o [i]._len = o [i]._rb = o [i]._wb = 0 ;
      }
    }}
    delete [] y ;
  }
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

vector < ObjectStream > MpAccessMPI :: exchange (const vector < ObjectStream > & in) const {
  const int nl = nlinks () ;
  assert (static_cast<int> (in.size ()) == nlinks()) ;
  vector < ObjectStream > out (nlinks ()) ;
  vector < pair < char *, int > > v (nlinks ()) ;
  {for (int l = 0 ; l < nl ; l ++ ) {
    v [l].first = in [l]._buf + in [l]._rb ;
    v [l].second = in [l]._wb - in [l]._rb ;
  }}
  v = doExchange (v, _mpiComm, dest ()) ;
  {for (int l = 0 ; l < nl ; l ++ ) {
    if (v [l].second > 0) {
      out [l]._buf = (char *) realloc (out [l]._buf, out [l]._len += v [l].second) ;
      assert (out [l]._wb + v [l].second < out [l]._len) ;
      memcpy (out [l]._buf + out [l]._wb , v [l].first, v[l].second) ;
      out [l]._wb += v [l].second ;
    }
    delete [] v [l].first ;
  }}
  return out ;
}















