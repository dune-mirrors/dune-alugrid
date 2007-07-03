// (c) bernhard schupp 1997 - 1998
#ifndef MPACCESS_MPI_H_INCLUDED
#define MPACCESS_MPI_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <assert.h>
#include <mpi.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
  #include <vector>
  #include <functional>
#else
  #include <iostream.h>
  #include <vector.h>
  #include <function.h>
#endif

#include "mpAccess.h"

class MpAccessMPI : public MpAccessLocal 
{
    MPI_Comm _mpiComm ;
    
    inline int mpi_allgather (int *, int , int *, int) const ;
    inline int mpi_allgather (char *, int, char *, int) const ;
    inline int mpi_allgather (double *, int, double *, int ) const ;
    public :
      inline  MpAccessMPI (MPI_Comm) ;
      inline  MpAccessMPI (const MpAccessMPI &) ;
      inline ~MpAccessMPI () ;
      inline int psize () const ;
      inline int myrank () const ;
      inline int barrier () const ;
      int gmax (int) const ;
      int gmin (int) const ;
      int gsum (int) const ;
      long gmax (long) const ;
      long gmin (long) const ;
      long gsum (long) const ;
      double gmax (double) const ;
      double gmin (double) const ;
      double gsum (double) const ;
      void gmax (double*,int,double*) const ;
      void gmin (double*,int,double*) const ;
      void gsum (double*,int,double*) const ;
      pair<double,double> gmax (pair<double,double>) const ;
      pair<double,double> gmin (pair<double,double>) const ;
      pair<double,double> gsum (pair<double,double>) const ;
      vector < int > gcollect (int) const ;
      vector < double > gcollect (double) const ;
      vector < vector < int > > gcollect (const vector < int > &) const ;
      vector < vector < double > > gcollect (const vector < double > &) const ;
      vector < ObjectStream > gcollect (const ObjectStream &) const ;
      vector < vector < int > > exchange (const vector < vector < int > > &) const ;
      vector < vector < double > > exchange (const vector < vector < double > > &) const ;
      vector < vector < char > > exchange (const vector < vector < char > > &) const ;
      
      vector < ObjectStream > exchange (const vector < ObjectStream > &) const ;

      // symectric exchange with same buffer size 
      void exchange (const vector < ObjectStream > & in,
                     vector< ObjectStream > & out) const;
        
} ;


	//
	//    #    #    #  #          #    #    #  ######
	//    #    ##   #  #          #    ##   #  #
	//    #    # #  #  #          #    # #  #  #####
	//    #    #  # #  #          #    #  # #  #
	//    #    #   ##  #          #    #   ##  #
	//    #    #    #  ######     #    #    #  ######
	//
#ifndef NDEBUG
#define MY_INT_TEST int test =
#else
#define MY_INT_TEST
#endif

#define USE_MPI_COMM_DUP

inline MpAccessMPI :: MpAccessMPI (MPI_Comm i) {
#ifdef USE_MPI_COMM_DUP
  MY_INT_TEST MPI_Comm_dup (i, &_mpiComm) ;
  assert (test == MPI_SUCCESS) ;
#else 
  _mpiComm = i;
#endif
  return ;
}

inline MpAccessMPI :: MpAccessMPI (const MpAccessMPI & a) {
#ifdef USE_MPI_COMM_DUP
  MY_INT_TEST MPI_Comm_dup (a._mpiComm, &_mpiComm) ;
  assert (test == MPI_SUCCESS) ;
#else 
  _mpiComm = a._mpiComm;
#endif
  return ;
}

inline MpAccessMPI :: ~MpAccessMPI () {
#ifdef USE_MPI_COMM_DUP
  MY_INT_TEST MPI_Comm_free (&_mpiComm) ;
  assert (test == MPI_SUCCESS) ;
#endif
  return ;
}

inline int MpAccessMPI :: psize () const {
  int i ;
  MY_INT_TEST MPI_Comm_size (_mpiComm, & i) ;
  assert (test == MPI_SUCCESS) ;
  return i ;
}

inline int MpAccessMPI :: myrank () const {
  int i ;
  MY_INT_TEST MPI_Comm_rank (_mpiComm, & i) ;
  assert (test == MPI_SUCCESS) ;
  return i ;
}

inline int MpAccessMPI :: barrier () const {
  return MPI_SUCCESS == MPI_Barrier (_mpiComm) ? psize () : 0 ;
}

inline int MpAccessMPI :: mpi_allgather (int * i, int si, int * o, int so) const {
  return MPI_Allgather (i, si, MPI_INT, o, so, MPI_INT, _mpiComm) ;
}

inline int MpAccessMPI :: mpi_allgather (char * i, int si, char * o, int so) const {
  return MPI_Allgather (i, si, MPI_BYTE, o, so, MPI_BYTE, _mpiComm) ;
}

inline int MpAccessMPI :: mpi_allgather (double * i, int si, double * o, int so) const {
  return MPI_Allgather (i, si, MPI_DOUBLE, o, so, MPI_DOUBLE, _mpiComm) ;
}

#undef USE_MPI_COMM_DUP
#endif
