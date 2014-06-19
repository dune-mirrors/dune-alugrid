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
                        const int rank,                             // MPI rank 
                        const std::stringstream& datastream ) // data stream 
{
  // get MPI communicator 
  typedef Dune::MPIHelper::MPICommunicator MPICommunicatorType;
  MPICommunicatorType mpiComm = Dune::MPIHelper::getCommunicator() ;

  // the following only makes sense if SIONlib and MPI are available
#if HAVE_SIONLIB && HAVE_MPI
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

  int sRank = rank ;

  // open sion file 
  int sid =
    sion_paropen_mpi( (char *) filename.c_str(),
                      fileMode,
                      &numFiles, // number of physical files 
                      mpiComm, // global comm 
                      &mpiComm, // local comm 
                      &chunkSize, // maximal size of data to be written 
                      &blockSize, // filesystem block size
                      &sRank, // my rank 
                      &file, // file pointer that is set by sion lib
                      NULL
                    );
  if( sid == -1 )
    DUNE_THROW( Dune::IOError, "opening sion_paropen_mpi for writing failed!" << filename );

  assert( file );

  // get pointer to buffer 
  const char* buffer = data.c_str();
  // write data 
  assert( sizeof(char) == 1 );
  sion_fwrite( &chunkSize, sizeof(sion_int64), 1, sid);
  sion_fwrite( buffer, 1, chunkSize, sid);

  std::cout << chunkSize << " wrote chunk " << std::endl;

  // close file 
  sion_parclose_mpi( sid );

#endif // HAVE_SIONLIB && HAVE_MPI
}

inline void restoreSION( const std::string& filename,    // filename 
                         const int rank,                 // MPI rank 
                         std::stringstream& datastream ) // data stream 
{
  // get MPI communicator 
  typedef Dune::MPIHelper::MPICommunicator MPICommunicatorType;
  MPICommunicatorType mpiComm = Dune::MPIHelper::getCommunicator() ;

  // the following only makes sense if SIONlib and MPI are available
#if HAVE_SIONLIB && HAVE_MPI
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
                              &numFiles, // numFiles 
                              mpiComm, // global comm 
                              &mpiComm, // local comm 
                              &chunkSize, // is set by library
                              &blockSize, // block size
                              &sRank, // my rank 
                              &file, // file pointer that is set by sion lib
                              NULL
                            );

  if( sid == -1 )
    DUNE_THROW( Dune::IOError, "opening sion_paropen_mpi for reading failed!" << filename );

  // read chunk size that was stored 
  sion_fread( &chunkSize, sizeof(sion_int64), 1, sid );

  // create buffer 
  std::string data; 
  data.resize( chunkSize );
  assert( sizeof(char) == 1 );

  // get pointer to buffer 
  char* buffer = (char *) data.c_str();
  assert( sizeof(char) == 1 );

  // read data 
  sion_fread( buffer, 1, chunkSize, sid );

  // write data to stream 
  datastream.write( buffer, chunkSize );

#endif // HAVE_SIONLIB && HAVE_MPI
}

#endif
