#ifndef MPACCESS_H_INCLUDED
#define MPACCESS_H_INCLUDED

#include <limits>
#include <map>
#include <set>
#include <vector>

#include "../serial/serialize.h"

namespace ALUGrid
{

  class MpAccessGlobal
  {
    public :
      struct MinMaxSum 
      {
        MinMaxSum ()
        : min( std::numeric_limits< double >::max() ),
          max( std::numeric_limits< double >::min() ),
          sum( 0 )
        {}

        explicit MinMaxSum( const double value )
        : min( value ), max( value ), sum( value )
        {}

        double min ; 
        double max ;
        double sum ;
      };

      typedef MinMaxSum  minmaxsum_t ;

      inline virtual ~MpAccessGlobal () ;
      virtual int psize () const = 0 ;
      virtual int myrank () const = 0 ;
      virtual int barrier () const = 0 ;
      virtual bool gmax (bool) const = 0 ;
      virtual int gmax (int) const = 0 ;
      virtual int gmin (int) const = 0 ;
      virtual int gsum (int) const = 0 ;
      virtual long gmax (long) const = 0 ;
      virtual long gmin (long) const = 0 ;
      virtual long gsum (long) const = 0 ;
      virtual double gmax (double) const = 0 ;
      virtual double gmin (double) const = 0 ;
      virtual double gsum (double) const = 0 ;
      virtual void gmax (double*,int,double*) const = 0 ;
      virtual void gmin (double*,int,double*) const = 0 ;
      virtual void gsum (double*,int,double*) const = 0 ;
      virtual void gmax (int*,int,int*) const = 0 ;
      virtual void gmin (int*,int,int*) const = 0 ;
      virtual void gsum (int*,int,int*) const = 0 ;
      virtual minmaxsum_t minmaxsum( double ) const = 0;
      virtual std::pair< double, double > gmax ( std::pair< double, double > ) const = 0;
      virtual std::pair< double, double > gmin ( std::pair< double, double > ) const = 0;
      virtual std::pair< double, double > gsum ( std::pair< double, double > ) const = 0;
      virtual void bcast(int*,int, int) const = 0 ;
      virtual void bcast(char*,int, int) const = 0 ;
      virtual void bcast(double*,int, int) const = 0 ;
      virtual int exscan( int ) const = 0; 
      virtual int scan( int ) const = 0; 
      virtual std::vector< int > gcollect ( int ) const = 0;
      virtual std::vector< double > gcollect ( double ) const = 0;
      virtual std::vector< std::vector< int > > gcollect ( const std::vector< int > & ) const = 0;
      virtual std::vector< std::vector< double > > gcollect ( const std::vector< double > & ) const = 0;
      virtual std::vector< ObjectStream > gcollect (const ObjectStream &, const std::vector< int > & ) const = 0;

      // default gcollect method that first needs to communicate the sizes of the buffers 
      // this method actually does two communications, one allgather and one allgatherv 
      virtual std::vector< ObjectStream > gcollect ( const ObjectStream &in ) const
      {
        // size of buffer 
        const int snum = in._wb - in._rb ;
        // get length vector 
        std::vector< int > length = gcollect( snum );

        // return gcollect operation 
        return gcollect( in, length ); 
      }
  } ;

  class MpAccessLocal : public MpAccessGlobal 
  {
    typedef std::map< int, int > linkage_t;
    typedef std::vector< int > vector_t;

    linkage_t _linkage ;
    vector_t  _dest ;
    public :
      class NonBlockingExchange 
      {
      protected:
        NonBlockingExchange () {}
      public:  
        class DataHandleIF 
        {
        protected:  
          DataHandleIF () {}
        public:
          virtual ~DataHandleIF () {}
          virtual void   pack( const int link, ObjectStream& os ) = 0 ;
          virtual void unpack( const int link, ObjectStream& os ) = 0 ;
          // should contain work that could be done between send and receive 
          virtual void meantimeWork () {}
        };

        virtual ~NonBlockingExchange () {}
        virtual void send ( const std::vector< ObjectStream > & ) = 0;
        virtual void send ( std::vector< ObjectStream > &, DataHandleIF& ) = 0;
        virtual std::vector< ObjectStream > receive() = 0;  
        virtual void receive( DataHandleIF& ) = 0;  
        virtual void exchange( DataHandleIF& ) = 0;  
        virtual void allToAll( DataHandleIF& ) = 0;  
      };

      inline virtual ~MpAccessLocal () ;
      void printLinkage ( std::ostream & ) const;
      inline void removeLinkage () ;
      inline int nlinks () const ;
      inline int link (int) const ;
      const std::vector< int > &dest () const{ return _dest ; }
      int insertRequestSymetric ( const std::set< int >& );
      int insertRequest ( const std::set< int >&  );
      // exchange data and return new vector of object streams 
      virtual std::vector< ObjectStream > exchange (const std::vector< ObjectStream > &) const = 0 ;
      virtual void exchange ( const std::vector< ObjectStream > &, NonBlockingExchange::DataHandleIF& ) const = 0 ;
      virtual void exchange ( NonBlockingExchange::DataHandleIF& ) const = 0 ;
      virtual void allToAll ( NonBlockingExchange::DataHandleIF& ) const = 0 ;

      // return handle for non-blocking exchange and already do send operation
      virtual NonBlockingExchange* nonBlockingExchange ( const int tag, 
                                                         const std::vector< ObjectStream > & ) const = 0;

      // return handle for non-blocking exchange 
      virtual NonBlockingExchange* nonBlockingExchange ( const int tag ) const = 0;
  } ;


          //
          //    #    #    #  #          #    #    #  ######
          //    #    ##   #  #          #    ##   #  #
          //    #    # #  #  #          #    # #  #  #####
          //    #    #  # #  #          #    #  # #  #
          //    #    #   ##  #          #    #   ##  #
          //    #    #    #  ######     #    #    #  ######
          //

  inline MpAccessGlobal :: ~MpAccessGlobal () {
  }

  inline MpAccessLocal :: ~MpAccessLocal () {
  }

  inline int MpAccessLocal :: link (int i) const {
    alugrid_assert (_linkage.end () != _linkage.find (i)) ;
    return (* _linkage.find (i)).second ;
  }

  inline int MpAccessLocal :: nlinks () const {
    return _linkage.size () ;
  }

  inline void MpAccessLocal :: removeLinkage () 
  {
    _linkage = linkage_t();
    _dest    = vector_t();
    return ;
  }

} // namespace ALUGrid

#endif // #ifndef MPACCESS_H_INCLUDED
