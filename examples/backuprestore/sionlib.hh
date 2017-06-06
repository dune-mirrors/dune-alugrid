#ifndef SIONLIB_BACKUPRESTORE_HH
#define SIONLIB_BACKUPRESTORE_HH

#include <iostream>
#include <sstream>
#include <cassert>

// include mpi stuff before sionlib header
#include <dune/common/exceptions.hh>

// include mpi stuff before sionlib header
#include <dune/common/parallel/mpihelper.hh>

#if HAVE_SIONLIB
#define SION_MPI
#include <sion.h>
#endif

inline void backupSION( const std::string& filename,          // filename
                        const int rank,                       // MPI rank
                        const std::stringstream& datastream ) // data stream
{
  // the following only makes sense if SIONlib and MPI are available
#if HAVE_SIONLIB && HAVE_MPI
  // get MPI communicator
  typedef Dune::MPIHelper::MPICommunicator MPICommunicatorType;
  MPICommunicatorType mpiComm = Dune::MPIHelper::getCommunicator() ;

  // get data from stream
  std::string data = datastream.str();

  // get chunk size for this process
  // use sionlib int64
  sion_int64 chunkSize = data.size();

  // file mode is: write byte
  const char* fileMode = "wb";

  // number of physical files to be created
  // number of files cannot be bigger than number of tasks
  int numFiles = 1; // user dependent choice of number of files

  // block size of filesystem, -1 use system default
  int blockSize = -1 ;

  // file pointer
  FILE* file = 0;

  // my MPI rank, variable might be altered by sionlib
  int sRank = rank ;

  // open sion file
  int sid =
    sion_paropen_mpi( (char *) filename.c_str(),
                      fileMode,
                      &numFiles,  // number of physical files
                      mpiComm,    // global comm
                      &mpiComm,   // local comm
                      &chunkSize, // maximal size of data to be written
                      &blockSize, // filesystem block size
                      &sRank,     // my rank
                      &file,      // file pointer that is set by sion lib
                      NULL
                    );
  if( sid == -1 )
    DUNE_THROW( Dune::IOError, "opening sion_paropen_mpi for writing failed!" << filename );

  // get pointer to buffer
  const char* buffer = data.c_str();
  // write data
  sion_fwrite( buffer, sizeof(char), chunkSize, sid);

  // close file
  sion_parclose_mpi( sid );

#endif // HAVE_SIONLIB && HAVE_MPI
}

inline bool restoreSION( const std::string& filename,    // filename
                         const int rank,                 // MPI rank
                         std::stringstream& datastream ) // data stream
{
  // the following only makes sense if SIONlib and MPI are available
#if HAVE_SIONLIB && HAVE_MPI
  // get MPI communicator
  typedef Dune::MPIHelper::MPICommunicator MPICommunicatorType;
  MPICommunicatorType mpiComm = Dune::MPIHelper::getCommunicator() ;

  // chunkSize is set by sion_paropen_mpi
  sion_int64 chunkSize = 0;

  // file mode is: read byte
  const char* fileMode = "rb";

  // blockSize, is recovered from stored files
  int blockSize = -1 ;

  // file handle
  FILE* file = 0;

  // number of files, is overwritten by sion_open
  int numFiles = 1;

  int sRank = rank ;

  // open sion file
  int sid = sion_paropen_mpi( (char *) filename.c_str(),
                              fileMode,
                              &numFiles,  // numFiles
                              mpiComm,    // global comm
                              &mpiComm,   // local comm
                              &chunkSize, // is set by library
                              &blockSize, // block size
                              &sRank,     // my rank
                              &file,      // file pointer that is set by sion lib
                              NULL
                            );

  if( sid == -1 )
    DUNE_THROW( Dune::IOError, "opening sion_paropen_mpi for reading failed!" << filename );

  // get bytes available for reading (might differ from total chunkSize)
  chunkSize = sion_bytes_avail_in_block( sid );

  // create data buffer
  char* buffer = new char[ chunkSize ];

  // read data
  sion_fread( buffer, sizeof(char), chunkSize, sid );

  // write data to stream
  datastream.write( buffer, chunkSize );

  delete [] buffer ;

  return true;
#else // #if HAVE_SIONLIB && HAVE_MPI
  return false;
#endif // #else // #if HAVE_SIONLIB && HAVE_MPI
}

#endif
