// (c) bernhard schupp, 1997 - 1998
// (c) new implementation by Robert Kloefkorn 2006 

#ifndef SERIALIZE_H_INCLUDED
#define SERIALIZE_H_INCLUDED

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>

class ObjectStream;

//  'ObjectStream' ist bereits die volle Implementierung eines einfachen
//  Objektstrommodells auf der Basis der Bibliotheksfunktionen f"ur
//  den Stringstream (sstream). Die Implemetierung ist eher im Sinne
//  eines rohen Datenformats mit einigen Testm"oglichkeiten zu sehen.
  
class ObjectStreamImpl 
{
protected:
  char * _buf ;
  size_t _rb, _wb, _len ;
  const size_t _bufChunk; 
  mutable bool _owner;

public :
  class EOFException {} ;
  class OutOfMemoryException {} ;
  inline ObjectStreamImpl (size_t chunk) 
    : _buf(0), _rb(0) , _wb(0) , _len (0) , _bufChunk(chunk) , _owner(true)
  {
  }
  
  inline ObjectStreamImpl (const ObjectStreamImpl & os)
    : _buf(0), _rb(0) , _wb(0) , _len (0) , _bufChunk(os._bufChunk) , _owner(true)
  {
    assign(os);
  }
  
  // reset write and read postitions 
  inline void clear() { _wb = 0; _rb = 0; }
  // reset read position 
  inline void resetReadPosition() { _rb = 0; }
  
  //! set position of write counter 
  void seekp( const size_t pos ) 
  { 
    _wb = pos ; 
    assert( _wb <= _len );
  }

  // return's true if size > 0 and read position is zero
  // i.e. a read othe stream will result some valid data  
  inline bool validToRead () const { return (_wb > 0) && (_rb == 0); }

  // return size of bytes allready written to stream 
  inline int capacity() const { return _len; }

  // return size of bytes allready written to stream 
  inline int size() const { return _wb; }

  // make sure that s bytes memory can be wrote without reallocation 
  inline void reserve(size_t s) 
  { 
    const size_t newSize = _wb + s ;
    if (newSize > _len) reallocateBuffer( newSize); 
  }

  // delete stream 
  inline ~ObjectStreamImpl () { removeObj(); }

  //! assign buffer from os the local buffer, os ownership is false afterwards
  //inline const ObjectStreamImpl & operator = (const ObjectStreamImpl & os)
  inline ObjectStreamImpl & operator = (const ObjectStreamImpl & os)
  {
    removeObj();
    assign(os);
    return *this;
  }

  // write value to stream 
  template <class T> 
  inline void write (const T & a)
  {
    writeT( a, true );
  }

  template <class T> 
  inline void writeUnchecked( const T& a )
  {
    writeT( a, false );
  }

  ////////////////////////////////////
  // to behave like stringstream 
  ////////////////////////////////////
  // put char 
  inline void put (const char a)  { write(a); }

  // put char with checking buffer size (reserve size before usage)
  inline void putNoChk (const char a)  { writeUnchecked(a); }

  // get char 
  inline char get () 
  { 
    char a;
    read(a);  
    return a;
  }

  // eof function 
  bool eof () const { return (this->_rb > this->_wb); }

  // good function 
  bool good () const { return (this->_rb <= this->_wb); }
  /////////////////////////////////////

protected:  
  template <class T>
  inline void writeT (const T & a, const bool checkLength )  
  {
    assert( _owner );
    const size_t ap = _wb;
    _wb += sizeof(T) ;

    // if buffer is to small, reallocate 
    if (checkLength && _wb > _len) 
    {
      reallocateBuffer(_wb);
    }
    assert( _wb <= _len );

    // call assignment operator of type T 
    static_cast<T &> (*((T *) getBuff(ap) )) = a;
    return ;
  }

public:
  // read value from stream 
  template <class T> 
  inline void read (T & a) throw (EOFException) 
  {
    const size_t ap = _rb;
    _rb += sizeof(T);
    
#ifndef NO_OBJECTSTREAM_DEBUG 
    if (_rb > _wb) throw EOFException () ;
#endif
    assert( _rb <= _wb );

    // call assignment operator of type T 
    a = static_cast<const T &> (*((const T *) getBuff(ap) ));
    return ;
  }

  // read this stream and write to os 
  inline void readStream (ObjectStreamImpl & os) 
  {
    readStream(os,_wb);
  }

  // read length bytes from this stream and stores it to os 
  inline void readStream (ObjectStreamImpl & os, const size_t length) 
  {
    if( length == 0 ) return ;
    // actual read position 
    os.write( getBuff(_rb) ,length);
    removeObject(length);
  }

  // writes hole stream of os to this stream
  inline void writeStream (const ObjectStreamImpl & os) 
  {
    write(os._buf,os._wb);
  }
  
  // increments the read position without actualy read data
  inline void removeObject(const size_t length) throw (EOFException) 
  {
    _rb += length; 
#ifndef NO_OBJECTSTREAM_DEBUG 
    if( _rb > _wb) throw EOFException () ;
#endif
    assert( _rb <= _wb );
  }
 
  //! free allocated memory 
  inline void reset() 
  {
    removeObj();
  }
 
  // static alloc of char buffer for use in mpAccess_MPI 
  inline static char * allocateBuffer(const size_t newSize) throw (OutOfMemoryException)
  {
    // do nothing for size = 0
    if( newSize == 0 ) return 0;

    // make sure that char has size of 1, 
    // otherwise check doExchange in mpAccess_MPI.cc 
    char * buffer = (char *) malloc (newSize * sizeof(char)) ;
    if( !buffer )
    {
      perror( "**EXCEPTION in ObjectStream::allocateBuffer( size_t ) " );
      throw OutOfMemoryException();
    }
    return buffer;
  }
  
  // static free for use with all buffers here  
  inline static void freeBuffer(char * buffer)
  {
    // free buffer if not zero 
    if( buffer ) free( buffer );
  }

  // compatibility with ostream 
  inline void write(const char* buff, const size_t length )
  {
    assert( _owner );
    if( length == 0 ) return ;

    const size_t newWb = _wb + length;
    if (newWb > _len) reallocateBuffer(newWb);
    
    memcpy( getBuff(_wb) , buff , length );
    _wb = newWb;
  }
  
  // compatibility with istream 
  inline void read(char* buff, const size_t length )
  {
    if( length == 0 ) return ;

    const size_t newRb = _rb + length;
#ifndef NO_OBJECTSTREAM_DEBUG 
    if (newRb > _wb) throw EOFException () ;
#endif
    assert( newRb <= _wb );
    
    memcpy( buff, getBuff(_rb), length );
    _rb = newRb;
  }
  
  inline char * getBuff (const size_t ap) { return (_buf + ap); }
  inline const char * getBuff (const size_t ap) const { return (_buf + ap); }

protected:
  // reallocated the buffer if necessary 
  inline void reallocateBuffer(size_t newSize) throw (OutOfMemoryException)
  {
    assert( _owner );
    _len += _bufChunk; 
    if(_len < newSize) _len = newSize;
    _buf = (char *) realloc (_buf, _len) ;
    if (!_buf) {
      perror ("**EXCEPTION in ObjectStream :: reallocateBuffer(size_t) ") ;
      throw OutOfMemoryException () ;
    }
  }

  // delete buffer 
  inline void removeObj() 
  {
    if( _owner ) freeBuffer( _buf );
    _buf = 0; _len = 0; _wb = 0; _rb = 0; _owner = true;
    return ;
  }
  
  // assign buffer 
  inline void assign(const ObjectStreamImpl & os) throw (OutOfMemoryException)
  {
    assert( _buf == 0 );
    if( os._len > 0 ) 
    {
      _len = os._len;
      _wb  = os._wb; 
      _rb  = os._rb; 
      const_cast<size_t &> (_bufChunk) = os._bufChunk;

      // overtake buffer and set ownership of os to false  
      _buf = os._buf;
      os._owner = false;
      // we are owner now 
      _owner = true;
    }
    return ;
  }
  
  inline void assign(char * buff, const size_t length )
  {
    if( length == 0 ) return ;

    // if length > 0, buff should be valid 
    assert( buff );
    
    // set length 
    _wb = _len = length;
    // set buffer 
    _buf = buff;

    // read status is zero 
    _rb = 0;
    
    // we are the owner 
    _owner = true; 
    return ;
  }
} ;

// bufchunk 0.25 Megabyte 
class ObjectStream
: public ObjectStreamImpl
{
  typedef ObjectStreamImpl BaseType;
  
  // default chunk size increase 
  enum { BufChunk = 4096 * sizeof(double) } ;

  // true if object stream was not set 
  bool notReceived_ ;

public:
  // ENDOFSTREAM should be in range of char, i.e. 0 to 256 
  // and not conflict with refinement rules in gitter_sti.h 
  static const char ENDOFSTREAM = 127;
  
  // create empty object stream 
  ObjectStream () 
  : BaseType( BufChunk ),
    notReceived_( true )
  {} 
  
  // create empty object stream with given chunk size 
  explicit ObjectStream ( const std::size_t chunkSize ) 
    : BaseType( chunkSize ),
      notReceived_( true )
  {} 
  
  // copy constructor 
  ObjectStream ( const ObjectStream &os )
  : BaseType( static_cast< const BaseType & >( os ) ),
    notReceived_( true )
  {} 
 
public:  
  // assigment of streams, owner ship of buffer is 
  // passed from os to this stream to avoid copy of large memory areas 
  ObjectStream &operator= ( const ObjectStream &os )
  {
    static_cast< BaseType & >( *this ) = static_cast< const BaseType & >( os );
    notReceived_ = os.notReceived_;
    return *this;
  }
  
  inline void writeObject (double a)  { this->write(a); }
  inline void readObject (double & a) { this->read(a);  }
  inline void writeObject (float a)  { this->write(a); }
  inline void readObject (float & a) { this->read(a);  }
  inline void writeObject (int a)     { this->write(a); } 
  inline void readObject (int & a)    { this->read(a);  }

  // return true if object stream was not set yet 
  bool notReceived () const { return notReceived_; } 
    
protected:  
  // assign pair of char buffer and size to this object stream 
  // osvec will contain zeros after that assignment 
  // used by mpAccess_MPI.cc 
  ObjectStream &operator= ( std::pair< char *, int > &osvec )
  {
    BaseType::removeObj();
    BaseType::assign( osvec.first, osvec.second );
    // reset osvec
    osvec.first = 0;
    osvec.second = 0;
    notReceived_ = false ;
    return *this;
  }
  
  friend class NonBlockingExchangeMPI ;
  friend class MpAccessGlobal ;
  friend class MpAccessMPI ;
  friend class MpAccessSTAR_MPI ;
} ;

// bufchunk 4 doubles 
class SmallObjectStream : public ObjectStreamImpl
{
  typedef ObjectStreamImpl BaseType;
  enum { BufChunk = 4 * sizeof(double) };
public:  
  // create empty stream 
  inline SmallObjectStream () : BaseType(BufChunk) {} 
  
  // copy constructor 
  inline SmallObjectStream (const SmallObjectStream & os) : BaseType(os) {} 
  
  // assignment , ownership changes 
  inline SmallObjectStream & operator = (const SmallObjectStream & os) 
  {
    BaseType::operator =(os); 
    return *this; 
  }

  inline void writeObject (double a)  { this->write(a); }
  inline void readObject (double & a) { this->read(a);  }
  inline void writeObject (float a)  { this->write(a); }
  inline void readObject (float & a) { this->read(a);  }
  inline void writeObject (int a)     { this->write(a); } 
  inline void readObject (int & a)    { this->read(a);  }
};

  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //

// streaming operators for ObjectStream 
template <class T>
inline ObjectStream& operator << ( ObjectStream& os, const T& value ) 
{
  os.write( value );
  return os;
}

// streaming operators for ObjectStream 
template <int length>
inline ObjectStream& operator << ( ObjectStream& os, const char (&s)[length] ) 
{
  os.write( &s[0], length );
  return os;
}

/*
typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
typedef CoutType& (*StandardEndLine)(CoutType&);

inline ObjectStream& operator << ( ObjectStream& os, StandardEndLine manip)
{ 
  return os; 
}
*/

// streaming operators for ObjectStream 
inline ObjectStream &operator<< ( ObjectStream &os, const std::string &s )
{
  const size_t size = s.size();
  os.write( size ); 
  os.write( s.c_str(), size ); 
  return os;
}

// streaming operators for ObjectStream 
template <class T>
inline ObjectStream& operator >> ( ObjectStream& is, T& value )
{
  is.read( value );
  return is;
}

// streaming operators for ObjectStream 
inline ObjectStream &operator>> ( ObjectStream &is, std::string &s )
{
  size_t size ;
  is.read( size ); 
  s.resize( size );
  is.read( (char *) s.c_str(), size ); 
  return is;
}

#endif  // SERIALIZE_H_INCLUDED
