#include <rpc/rpc.h>
#include <rpc/xdr.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef IBM_XLC
  typedef char  xdrbuff_t;
  typedef char  xdraddr_t;
  typedef int   xdrsize_t;
#else
  typedef void  xdrbuff_t;
  typedef void  xdraddr_t;
  typedef u_int xdrsize_t;
#endif

static int read_xdr_file(void *file, xdrbuff_t *buffer, xdrsize_t size);
static int write_xdr_file(void *file, xdrbuff_t *buffer, xdrsize_t size);

XDR *XDRopen(const char *filename,const xdr_op op);
XDR *XDRclose(XDR *x);

/*
class XDRstream 
{
  XDR    _xdrs; 
  FILE * _xdrfile;
public:  
  enum XDRdir { in , out };
  
  XDRstream(const char * filename, XDRdir dir )
  {
    if(dir == in)
      _xdrfile = fopen(filename, "rb");
    if(dir == out)
      _xdrfile = fopen(filename, "wb");

    if(!_xdrfile)
    {
      printf( "\aERROR in XDRstream::XDRstream(..): couldnot open <%s>!\n", filename);
      fflush(stderr);
      abort();
    }
 
    // read xdr 
    xdrstdio_create(&_xdrs, _xdrfile, XDR_DECODE);
  }

  ~XDRstream () 
  {
    //if(_xdrfile) close(_xdrfile);
  }
 
  template <class T> 
  void put (const T & t)
  {
    xdr_bytes(&_xdrs, ((char **) &t) , 1, sizeof(T) );
    //xdr_vector(&_xdrs,((char *) &t),1, sizeof(T) ,(xdrproc_t)xdr_double);
  }
  template <> 
  void put (const int & t)
  {
    xdr_int(&_xdrs, &t );
  }
  
  char get ()
  {
    char t;
    xdr_char(&_xdrs, &t);
    return t;
  }
  
  template <class T> 
  void read (T &t) const 
  {
    xdr_bytes(&_xdrs, ((char **) &t) , 1, sizeof(T) );
  }

  template <> 
  void read (int &t) const 
  {
    xdr_int(&_xdrs, &t );
  }

  XDRstream & operator << (const char * t )
  {
    return *this;
  }
  
  template <class T> 
  XDRstream & operator << (const T & t )
  {
    put(t);
    return *this;
  }
  
  template <class T> 
  XDRstream & operator >> (T & t ) const 
  {
    read(t);
    return *this;
  }

  void precision ( int pre ) 
  {
  }
  template <typename A, typename B> 
  void setf ( const A & a, const B & b ) 
  {
  }
 
};

class XDRstream_out : public XDRstream 
{
public: 
  XDRstream_out(const char * filename) : XDRstream
    (filename,XDRstream::out) {}
};

class XDRstream_in : public XDRstream 
{
public: 
  XDRstream_in(const char * filename) : XDRstream
    (filename,XDRstream::in) {}
};

*/
