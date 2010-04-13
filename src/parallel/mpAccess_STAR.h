// (c) bernhard schupp 1997 - 1998
#ifndef MPACCESS_STARMPI_H_INCLUDED
#define MPACCESS_STARMPI_H_INCLUDED

#include "mpAccess_MPI.h"

class MpAccessSTAR_MPI : public MpAccessMPI 
{
  typedef MpAccessMPI BaseType;
protected:  
  using BaseType :: _mpiCommPtr;
  using BaseType :: _psize;
  using BaseType :: _myrank;

  int star_allgather (int *, int , int *, int) const ;
  int star_allgather (char *, int, char *, int) const ;
  int star_allgather (double *, int, double *, int ) const ;
public :
  // constructor taking MPI_Comm 
  // to avoid MPI types here this is a template constructor 
  template <class MPICommunicator>  
  inline MpAccessSTAR_MPI (MPICommunicator mpicomm ) 
    : BaseType( mpicomm )
  {
  }

  // copy constructor 
  inline MpAccessSTAR_MPI (const MpAccessSTAR_MPI &a ) 
    : BaseType( a ) 
  {}

public:  
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
} ;
#endif
