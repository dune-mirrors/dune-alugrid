#ifndef __XDRCLASS_INCLUDED__
#define __XDRCLASS_INCLUDED__

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

int read_xdr_file(void *file, xdrbuff_t *buffer, xdrsize_t size);
int write_xdr_file(void *file, xdrbuff_t *buffer, xdrsize_t size);

XDR *XDRopen(const char *filename,const xdr_op op);
XDR *XDRclose(XDR *x);

class XDRstream 
{
  XDR    _xdrs; 
  FILE * _xdrfile;
  unsigned int puts_;
public:  
  enum XDRdir { in , out };
  
  XDRstream(const char * filename, XDRdir dir ) : puts_ (1)
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
    if(_xdrfile) fclose(_xdrfile);
  }
 
  template <class T> 
  void put (const T & t)
  {
    assert(false); 
    cout << "Pute \n";
    //char * tp= const_cast<T *> (&t);
    //xdr_bytes(&_xdrs, ((char **) &t) , 1, sizeof(T) );
    //xdr_bytes(&_xdrs, &tp , &puts_, sizeof(T) );
    //xdr_vector(&_xdrs,((char *) &t),1, sizeof(T) ,(xdrproc_t)xdr_double);
  }
  
  /*
  template <> 
  void put (const char & t)
  {
    cout << "put " << t << "\n";
    xdr_char(&_xdrs, const_cast<char *> (&t) );
  }

  template <> 
  void put (const int & t)
  {
    cout << "Put int \n";
    xdr_int(&_xdrs, const_cast<int *> (&t) );
  }
  */
  
  char get () const
  {
    char t;
    read(t);
    return t;
  }
  
  template <class T> 
  void read (T &t) const 
  {
    assert(false); 
    //xdr_bytes(&_xdrs, ((char **) &t) , 1, sizeof(T) );
  }

  /*
  template <> 
  void read (char &t) const 
  {
    cout << "read char \n";
    xdr_char(const_cast<XDR *> (&_xdrs), &t );
    cout << t << "\n";
  }

  template <> 
  void read (int &t) const 
  {
    xdr_int(const_cast<XDR *> (&_xdrs), &t );
  }
  */

  bool operator ! () {
    return _xdrfile ? true : false;
  }

  XDRstream & operator << ( const char * t )
  {
    cout << " schriebe  char \n";
    xdr_char(&_xdrs, const_cast<char *> (t));
    return *this;
  }
  
  XDRstream & operator >> ( const char * t )
  {
    cout << " lese char \n";
    xdr_char(&_xdrs, const_cast<char *> (t));
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

#endif

