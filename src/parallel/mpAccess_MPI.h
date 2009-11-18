// (c) bernhard schupp 1997 - 1998
#ifndef MPACCESS_MPI_H_INCLUDED
#define MPACCESS_MPI_H_INCLUDED

#include "mpAccess.h"

class MpAccessMPI : public MpAccessLocal 
{
public:
  class CommIF 
  {
  protected: 
    CommIF () {}
  public:
    virtual ~CommIF() {}
  };

  template <class MPICommunicator>
  class Comm : public CommIF
  {
    // no copying or assigning
    Comm( const Comm& );
    Comm& operator= (const Comm& );
    // we don't want MPI types here to avoid include of mpi.h 
    mutable MPICommunicator _mpiComm;
  public:  
    Comm( MPICommunicator );
    operator MPICommunicator () const { return _mpiComm; }
  };

protected:  
  // class holding the MPI communicator 
  CommIF* _mpiCommPtr;
  // number of processors
  int _psize; 
  // my processor number  
  int _myrank;

  int mpi_allgather (int *, int , int *, int) const ;
  int mpi_allgather (char *, int, char *, int) const ;
  int mpi_allgather (double *, int, double *, int ) const ;
public :
  template <class MPICommunicator>  
  inline MpAccessMPI (MPICommunicator i) 
    : _mpiCommPtr( (void *) new Comm<MPICommunicator> ( i ) ), 
      _psize( 0 ), _myrank( -1 )
  {
    initialize(); 
  }

  MpAccessMPI (const MpAccessMPI &) ;
  ~MpAccessMPI () ;
protected:  
  void initialize () ;
public:  
  inline int psize () const ;
  inline int myrank () const ;
  int barrier () const ;
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
    
  // return address of MPI communicator (dirty hack, but what can we do)
  void* communicator() { return _mpiCommPtr; }
} ;


//
//    #    #    #  #          #    #    #  ######
//    #    ##   #  #          #    ##   #  #
//    #    # #  #  #          #    # #  #  #####
//    #    #  # #  #          #    #  # #  #
//    #    #   ##  #          #    #   ##  #
//    #    #    #  ######     #    #    #  ######
//
inline int MpAccessMPI :: psize () const 
{
  assert( _psize > 0 );
  //return _psize;
  return _psize;
}

inline int MpAccessMPI :: myrank () const 
{
  assert( _myrank != -1 );
  return _myrank;
}
#endif
