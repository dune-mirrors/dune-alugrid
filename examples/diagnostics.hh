#ifndef DUNE_FEM_DG_RUNFILE_HH
#define DUNE_FEM_DG_RUNFILE_HH

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

#include <dune/common/timer.hh>

#include <dune/alugrid/impl/serial/myalloc.h>
#include <sys/time.h>
#include <sys/resource.h>

//#define PRINT_IDENTIFICATION_TIMES

#ifdef PRINT_IDENTIFICATION_TIMES
namespace ALUGrid {
  extern double identU2;
  extern double identU3;
  extern double identU4;

  extern double ldbTimerU2;
  extern double ldbTimerU3;
  extern double ldbTimerU4;
}
#endif

namespace Dune {

  template <class GridType>
  class Diagnostics
  {
    typedef typename GridType :: Traits :: Communication CommunicatorType;
    const CommunicatorType& comm_;
    const std::string runFileName_;
    const int writeDiagnostics_; // 0 don't, 1 only speedup file, 2 write all runfiles
                                 // 3 only write 0, others at end, 4 all files at end
    std::ostream* diagnosticsFile_;

    std::vector< double > times_ ;
    double elements_;
    double maxDofs_;
    size_t timesteps_;

    // write in milli seconds
    inline size_t inMS(const double t) const
    {
      return (size_t (t * 1e3));
    }

    void writeHeader(std::ostream& runfile)
    {
      // write header
      runfile << "# Time          ";
      runfile << "   dt         ";
      runfile << "  Elements   ";
      runfile << "        dg   ";
      runfile << "       ode      ";
      runfile << "  adapt      ";
      runfile << "     lb   ";
      runfile << "       all  ";
      runfile << "      rusage   ";
      runfile << "   alugrid  ";
      runfile << std::endl;
      runfile.flush();
    }

    std::string runFileName(const int rank) const
    {
      std::stringstream runfile;
      runfile << "./diagnostics." << rank;
      return runfile.str();
    }

    std::ostream* createDiagnostics( const int rank,
                                     const int writeId,
                                     const bool newStart )
    {
      // in case of no writing or only speedup table don't create runfile
      if( writeId <= 1 ) return 0;

      bool writeAtOnce = ( writeId > 2 );
      // when writeId == 2 then only for rank 0 write file every time step
      // this is for monitoring issues
      if( rank == 0 && writeId == 3 ) writeAtOnce = false ;

      if( writeAtOnce )
      {
        return new std::stringstream();
      }
      else
      {
        std::ofstream* file = new std::ofstream( runFileName_.c_str(), ( newStart ) ? std::ios::out : std::ios::app );
        if( ! file )
        {
          std::cerr << "Couldn't open run file <"<<runFileName_<<">, ciao!" << std::endl;
          abort();
        }
        return file;
      }
    }
  public:
    Diagnostics( const CommunicatorType& comm, const int writeDiagnostics )
      : comm_( comm )
      , runFileName_( runFileName( comm_.rank() ) )
      , writeDiagnostics_( writeDiagnostics )
      , diagnosticsFile_( createDiagnostics( comm_.rank(), writeDiagnostics_, false ) )
      , times_()
      , elements_( 0.0 )
      , maxDofs_( 0.0 )
      , timesteps_( 0 )
    {
      if( diagnosticsFile_ )
      {
        writeHeader( *diagnosticsFile_ );
      }
    }

    //! destructor
    ~Diagnostics()
    {
      delete diagnosticsFile_;
    }


  protected:
    template <class T>
    void writeVectors(std::ostream& file,
                      const std::string& descr,
                      const std::vector< T >& avgTimes,
                      const std::vector< T >& maxTimes,
                      const std::vector< T >& minTimes ) const
    {
      const size_t size = avgTimes.size();
      file.precision(6);
      file << std::scientific ;
      file << "#########################################" << std::endl ;
      file << "# Aver " << descr << std::endl ;
      for(size_t i=0; i<size-1; ++i)
      {
        file << avgTimes[ i ] << "  ";
      }
      file << std::endl;
      file << "# Max " << descr << std::endl ;
      for(size_t i=0; i<size-1; ++i)
      {
        file << maxTimes[ i ] << "  ";
      }
      file << std::endl;
      file << "# Min " << descr << std::endl ;
      for(size_t i=0; i<size-1; ++i)
      {
        file << minTimes[ i ] << "  ";
      }
      file << std::endl;
    }

#ifdef PRINT_IDENTIFICATION_TIMES
    void printIdentificationTimes() const
    {
      std::vector<double> times;

      // vertex linkage estimate
      times.push_back( ::ALUGrid::identU2 );
      // request linkage
      times.push_back( ::ALUGrid::identU3 );
      // identify
      times.push_back( ::ALUGrid::identU4 );

      // calculate partitioning
      times.push_back( ::ALUGrid::ldbTimerU2 );

      // load-balaning
      times.push_back( ::ALUGrid::ldbTimerU3 );
      // identification
      times.push_back( ::ALUGrid::ldbTimerU4 );

      const int timerSize = times.size();

      std::vector<double> maxTimes( times );
      std::vector<double> minTimes( times );

      // sum, max, and min for all procs
      comm_.max( &maxTimes[ 0 ], timerSize );
      comm_.min( &minTimes[ 0 ], timerSize );

      if( comm_.rank() == 0 )
      {
        for(int i=0; i<timerSize; ++i )
        {
          std::cout << "U" << i+2 << " max: " << maxTimes[ i ] << "  min: " << minTimes[ i ] << std::endl;
        }
      }
    }
#endif

  public:
    void flush() const
    {
      // if write is > 0 then create speedup file
      if( writeDiagnostics_ )
      {
#ifdef PRINT_IDENTIFICATION_TIMES
        printIdentificationTimes();
#endif

        std::vector< double > times( times_ );

        times.push_back( elements_ );
        const size_t size = times.size();
        std::vector< double > avgTimes ( times );
        std::vector< double > maxTimes ( times );
        std::vector< double > minTimes ( times );

        // sum, max, and min for all procs
        comm_.sum( &avgTimes[ 0 ], size );
        comm_.min( &minTimes[ 0 ], size );

        maxTimes.push_back( maxDofs_ );
        comm_.max( &maxTimes[ 0 ], maxTimes.size() );

        const double maxDofs = maxTimes.back();
        maxTimes.pop_back();

        if( comm_.rank() == 0 && timesteps_ > 0 )
        {
          const int maxThreads = 1;
          const double tasks   = comm_.size() * maxThreads ;

          const double ts_1 = 1.0 / double( timesteps_ );

          { // adjust elements to be the average element number
            size_t i = size - 1 ;
            avgTimes[ i ] *= ts_1;
            maxTimes[ i ] *= ts_1;
            minTimes[ i ] *= ts_1;
          }

          std::stringstream runfile;
          runfile << "./speedup." << comm_.size();
          std::ofstream file ( runfile.str().c_str() );
          if( file )
          {
            const double averageElements = avgTimes[ size - 1 ] / tasks ;

            file << "# Procs = " << comm_.size() << " * " << maxThreads << " (MPI * threads)" << std::endl ;
            file << "# Timesteps = " << timesteps_ << std::endl ;
            file << "# Max DoFs (per element): " << maxDofs << std::endl;
#ifdef ALUGRID_COUNT_GLOBALCOMM
            file << "# allgather allreduce (counters for global communication)" << std::endl;
            file << std::setw( 10 ) << ALU3DSPACE allgatherCalled() << "    " << ALU3DSPACE allreduceCalled() << std::endl;
#endif
            file << "# Elements / timestep: sum    max    min    average  " << std::endl;
            file << avgTimes[ size-1 ] << "  " << maxTimes[ size-1 ] << "  " << minTimes[ size-1 ] << "  " << ((size_t)averageElements) << std::endl;
            file << "# SOLVE          COMM         ADAPT            LB         TIMESTEP     RESTPROL    ALUGrid-MEMORY (MB)" << std::endl ;

            // multiply avgTimes with maxThhreads since the sum would be to small otherwise
            for(size_t i=0; i<size; ++i)
            {
              avgTimes[ i ] *= maxThreads / tasks ;
            }

            // devide by timesteps
            for(size_t i=0; i<size; ++i)
            {
              avgTimes[ i ] *= ts_1;
              maxTimes[ i ] *= ts_1;
              minTimes[ i ] *= ts_1;
            }

            {
              std::string descr( "( time / timestep in sec )" );
              writeVectors( file, descr, avgTimes, maxTimes, minTimes );
            }
          }
        } // end speedup file

        if( diagnosticsFile_ )
        {
          std::stringstream* str = dynamic_cast< std::stringstream* > (diagnosticsFile_);
          if( str )
          {
            std::ofstream file( runFileName_.c_str() );

            if( ! file )
            {
              std::cerr << "Couldn't open run file <"<<runFileName_<<">, ciao!" << std::endl;
              abort();
            }

            file << str->str();
            file.flush();
            file.close();
          }
        }
      }
    }

    //! write timestep data
    inline void write( const double t,
                       const double ldt,
                       const size_t nElements,
                       const size_t maxDofs,
                       const double dgOperatorTime,
                       const double odeSolve,
                       const double adaptTime,
                       const double lbTime,
                       const double timeStepTime,
                       const double restProlTime,
                       const std::vector<double>& extraSteps = std::vector<double>() )
    {
      std::vector< double > times( 6 + extraSteps.size(), 0.0 );
      int pos = 0;
      times[ pos++ ] = dgOperatorTime ;
      times[ pos++ ] = odeSolve ;
      times[ pos++ ] = adaptTime ;
      times[ pos++ ] = lbTime ;
      times[ pos++ ] = timeStepTime ;
      times[ pos++ ] = restProlTime;
      for(size_t i=0; i<extraSteps.size(); ++i)
      {
        times[ i+pos ] = extraSteps[ i ];
      }

      maxDofs_ = std::max( double(maxDofs), maxDofs_ );

      write( t, ldt, nElements, times );
    }

    //! clone of write method
    inline void write( const double t,
                       const double ldt,
                       const size_t nElements,
                       const std::vector<double>& times)
    {
      if( writeDiagnostics_ )
      {
        const size_t size = times.size() ;
        const size_t oldsize = times_.size();
        if( oldsize < size  )
        {
          times_.resize( size );
          for( size_t i=oldsize; i<size; ++i)
            times_[ i ] = 0;
        }

        elements_ += double( nElements );

        for(size_t i=0; i<size; ++i )
          times_[ i ] += times[ i ] ;

        ++timesteps_ ;

        if( diagnosticsFile_ )
        {
          std::ostream& runfile = (*diagnosticsFile_);
          const int space = 12;
          runfile << std::scientific << t  << "  ";
          runfile << std::setw(space) << ldt << "  ";
          runfile << std::setw(space) << nElements << " ";
          for(size_t i=0; i<size; ++i)
            runfile << std::setw(space) << inMS( times[ i ] ) << " ";
          runfile << std::endl;

          runfile.flush();
        }
      }
    }

  }; // end class Diagnostics

} // end namespace Dune

//! get memory in MB
std::vector<double> getMemoryUsage()
{
  std::vector<double> memUsage;
  // dune-alugrid version
  memUsage.push_back(double(ALUGrid::MyAlloc::allocatedMemory())/1024.0/1024.0);
  struct rusage info;
  getrusage( RUSAGE_SELF, &info );
  // convert to KB
  memUsage.push_back(double(info.ru_maxrss)/ 1024.0);
  return memUsage;
}

#endif
