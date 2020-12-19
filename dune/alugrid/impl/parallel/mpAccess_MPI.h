#ifndef MPACCESS_MPI_H_INCLUDED
#define MPACCESS_MPI_H_INCLUDED

#include <memory>
#include <dune/common/parallel/communication.hh>

#include "mpAccess.h"

// the following implementation is only available in case MPI is available
#if HAVE_MPI

extern "C" {
// the message passing interface (MPI) headers for C
#include <mpi.h>
}

namespace ALUGrid
{
  class MpAccessMPI
  : public MpAccessLocal
  {
  public:
    // type of min,max, and sum structure
    typedef MpAccessLocal::minmaxsum_t  minmaxsum_t;

    class MinMaxSumIF
    {
    protected:
        MinMaxSumIF () {}
    public:
      virtual ~MinMaxSumIF() {}
      virtual minmaxsum_t  minmaxsum( double ) const = 0;
    };

    // non blocking exchange handler
    typedef MpAccessLocal::NonBlockingExchange  NonBlockingExchange;

    // MPI communication tag
    enum { messagetag = 123 };

    // conversion operator to MPI_Comm
    MPI_Comm communicator () const { return _mpiComm; }

  protected:
    // the MPI communicator
    MPI_Comm _mpiComm ;
    // pointer to minmaxsum communication
    std::unique_ptr< MinMaxSumIF > _minmaxsum;
    // number of processors
    const int _psize;
    // my processor number
    const int _myrank;

    int mpi_allgather (int *, int , int *, int) const;
    int mpi_allgather (char *, int, char *, int) const;
    int mpi_allgather (double *, int, double *, int ) const;

    void initMinMaxSum();
  public :
    // constructor taking MPI_Comm
    explicit MpAccessMPI ( MPI_Comm mpicomm ) ;

    // constructor taking Dune::No_Comm and creating MpAccessMPI with MPI_COMM_SELF
    explicit MpAccessMPI ( const Dune::No_Comm& )
      : MpAccessMPI( MPI_COMM_SELF )
    {}

    // copy constructor
    MpAccessMPI (const MpAccessMPI &);
    // destructor
    ~MpAccessMPI ();
  protected:
    MinMaxSumIF* copyMPIComm( MPI_Comm mpicomm );
    int getSize ();
    int getRank ();
    // return new tag number for the exchange messages
    static int getMessageTag( const unsigned int icrement )
    {
      static int tag = messagetag + 2 ;
      // increase tag counter
      const int retTag = tag;
      tag += icrement ;
      // the MPI standard guaratees only up to 2^15-1
      // this needs to be revised for the all-to-all communication
      if( tag < 0 ) // >= 32767 )
      {
        // reset tag to initial value
        tag = messagetag + 2 ;
      }
      return retTag;
    }

    // return new tag number for the exchange messages
    static int getMessageTag()
    {
      return getMessageTag( 1 );
    }

  public:
    inline int psize () const;
    inline int myrank () const;
    int barrier () const;
    bool gmax (bool) const;
    int gmax (int) const;
    int gmin (int) const;
    int gsum (int) const;
    long gmax (long) const;
    long gmin (long) const;
    long gsum (long) const;
    double gmax (double) const;
    double gmin (double) const;
    double gsum (double) const;
    void gmax (double*,int,double*) const;
    void gmin (double*,int,double*) const;
    void gsum (double*,int,double*) const;
    void gmax (int*,int,int*) const;
    void gmin (int*,int,int*) const;
    void gsum (int*,int,int*) const;
    minmaxsum_t minmaxsum( double ) const;
    std::pair<double,double> gmax (std::pair<double,double>) const;
    std::pair<double,double> gmin (std::pair<double,double>) const;
    std::pair<double,double> gsum (std::pair<double,double>) const;
    void bcast(int*, int, int ) const;
    void bcast(char*, int, int ) const;
    void bcast(double*, int, int ) const;
    void bcast( ObjectStream&, int ) const;
    int exscan ( int ) const;
    int scan ( int ) const;

    using MpAccessLocal::gcollect;

    std::vector< int > gcollect (int) const;
    std::vector< double > gcollect (double) const;
    std::vector< std::vector< int > > gcollect (const std::vector< int > &) const;
    std::vector< std::vector< double > > gcollect (const std::vector< double > &) const;
    std::vector< ObjectStream > gcollect (const ObjectStream &, const std::vector<int>& ) const;

    std::vector< ObjectStream > exchange (const std::vector< ObjectStream > &) const;

    // exchange object stream and then unpack one-by-one as received
    void exchange ( const std::vector< ObjectStream > &, NonBlockingExchange::DataHandleIF& ) const;

    // exchange object stream and immediately unpack, when data was received
    void exchange ( NonBlockingExchange::DataHandleIF& ) const;

    // exchange object stream and immediately unpack, when data was received
    void exchangeSymmetric ( NonBlockingExchange::DataHandleIF& ) const;

    // return handle for non-blocking exchange and already do send operation
    NonBlockingExchange* nonBlockingExchange( const int tag, const std::vector< ObjectStream > & ) const;
    // return handle for non-blocking exchange
    NonBlockingExchange* nonBlockingExchange( const int tag ) const;
    // return handle for non-blocking exchange
    NonBlockingExchange *nonBlockingExchange () const;
  };

  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //
  inline int MpAccessMPI::psize () const
  {
    alugrid_assert ( _psize > 0 );
    return _psize;
  }

  inline int MpAccessMPI::myrank () const
  {
    alugrid_assert ( _myrank != -1 );
    return _myrank;
  }

} // namespace ALUGrid

// include inline implementation
#include <dune/alugrid/impl/parallel/mpAccess_MPI_inline.h>
#endif // #if HAVE_MPI

#endif // #ifndef MPACCESS_MPI_H_INCLUDED
