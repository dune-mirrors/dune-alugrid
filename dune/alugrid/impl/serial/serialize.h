// (c) bernhard schupp, 1997 - 1998
// (c) new implementation by Robert Kloefkorn 2006
#ifndef SERIALIZE_H_INCLUDED
#define SERIALIZE_H_INCLUDED

#include <dune/alugrid/common/alugrid_assert.hh>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <utility>

#include <dune/alugrid/impl/byteorder.hh>

#include "myalloc.h"

namespace ALUGrid
{

  //  'ObjectStream' ist bereits die volle Implementierung eines einfachen
  //  Objektstrommodells auf der Basis der Bibliotheksfunktionen f"ur
  //  den Stringstream (sstream). Die Implemetierung ist eher im Sinne
  //  eines rohen Datenformats mit einigen Testm"oglichkeiten zu sehen.

  template< class Traits >
  class BasicObjectStreamImpl
  {
  protected:
    char * _buf ;
    size_t _rb, _wb, _len ;
    const size_t _bufChunk;
    mutable bool _owner;

  public :
    class EOFException : public ALUGridException
    {
    public:
      virtual std::string what () const { return "EOFException"; }
    };
    class OutOfMemoryException {} ;
    inline BasicObjectStreamImpl (size_t chunk)
      : _buf(0), _rb(0) , _wb(0) , _len (0) , _bufChunk(chunk) , _owner(true)
    {
    }

    inline BasicObjectStreamImpl (const BasicObjectStreamImpl & os)
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
      alugrid_assert ( _wb <= _len );
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
    inline ~BasicObjectStreamImpl () { removeObj(); }

    //! assign buffer from os the local buffer, os ownership is false afterwards
    //inline const BasicObjectStreamImpl & operator = (const BasicObjectStreamImpl & os)
    inline BasicObjectStreamImpl & operator = (const BasicObjectStreamImpl & os)
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
    inline void put (const signed char a)  { write(a); }

    // put char with checking buffer size (reserve size before usage)
    inline void putNoChk (const signed char a)  { writeUnchecked(a); }

    // get char
    inline signed char get ()
    {
      signed char a;
      read(a);
      return a;
    }

    // eof function
    bool eof () const { return (this->_rb >= this->_wb); }

    // good function
    bool good () const { return (this->_rb < this->_wb); }
    /////////////////////////////////////

  protected:
    template <class T>
    inline void writeT (const T & a, const bool checkLength )
    {
      alugrid_assert ( _owner );
      const size_t ap = _wb;
      _wb += sizeof(T) ;

      // if buffer is to small, reallocate
      if (checkLength && _wb > _len)
      {
        reallocateBuffer(_wb);
      }
      alugrid_assert ( _wb <= _len );

      // call assignment operator of type T
      Traits::copy( static_cast< void * >( getBuff( ap ) ), &a, 1 );
      return ;
    }

    template<class T>
    inline void readT ( T& a, bool checkLength )
    {
      const size_t ap = _rb;
      _rb += sizeof(T);

#ifndef NO_OBJECTSTREAM_DEBUG
      if ( checkLength && _rb > _wb) throw EOFException () ;
#endif
      alugrid_assert ( _rb <= _wb );

      // call assignment operator of type T
      Traits::copy( &a, static_cast< const void * >( getBuff( ap ) ), 1 );
      return ;
    }

  public:
    // read value from stream
    template <class T>
    inline void read (T & a) { readT( a, true ); }

    template<class T>
    inline void readUnchecked ( T& a ) { readT( a, false ); }

    // read this stream and write to os
    inline void readStream (BasicObjectStreamImpl & os)
    {
      readStream(os,_wb);
    }

    // read length bytes from this stream and stores it to os
    inline void readStream (BasicObjectStreamImpl & os, const size_t length)
    {
      if( length == 0 ) return ;
      // actual read position
      os.write( getBuff(_rb) ,length);
      removeObject(length);
    }

    // writes hole stream of os to this stream
    inline void writeStream (const BasicObjectStreamImpl & os)
    {
      write(os._buf,os._wb);
    }

    // increments the read position without actualy read data
    inline void removeObject(const size_t length)
    {
      _rb += length;
#ifndef NO_OBJECTSTREAM_DEBUG
      if( _rb > _wb) throw EOFException () ;
#endif
      alugrid_assert ( _rb <= _wb );
    }

    //! free allocated memory
    inline void reset()
    {
      removeObj();
    }

    // static alloc of char buffer for use in mpAccess_MPI
    inline static char * allocateBuffer(const size_t newSize)
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
      alugrid_assert ( _owner );
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
      alugrid_assert ( newRb <= _wb );

      memcpy( buff, getBuff(_rb), length );
      _rb = newRb;
    }

    // return pointer to buffer memory
    inline char* raw () { return getBuff( 0 ); }
    inline const char* raw () const { return getBuff( 0 ); }

    inline char * getBuff (const size_t ap) { return (_buf + ap); }
    inline const char * getBuff (const size_t ap) const { return (_buf + ap); }

  protected:
    // reallocated the buffer if necessary
    inline void reallocateBuffer(size_t newSize)
    {
      alugrid_assert ( _owner );
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
    inline void assign(const BasicObjectStreamImpl & os)
    {
      alugrid_assert ( _buf == 0 );
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
      alugrid_assert ( buff );

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



  // BasicObjectStream
  // -----------------

  // bufchunk 0.25 Megabyte
  template< class Traits >
  class BasicObjectStream
    : public BasicObjectStreamImpl< Traits >
  {
    typedef BasicObjectStreamImpl< Traits > BaseType;

    // default chunk size increase
    enum { BufChunk = 4096 * sizeof(double) } ;

    // true if object stream was not set
    bool notReceived_ ;

  public:
    // ENDOFSTREAM should be in range of char, i.e. 0 to 256
    // and not conflict with refinement rules in gitter_sti.h
    static const char ENDOFSTREAM = 127;

    // create empty object stream
    BasicObjectStream ()
    : BaseType( BufChunk ),
      notReceived_( true )
    {}

    // create empty object stream with given chunk size
    explicit BasicObjectStream ( const std::size_t chunkSize )
      : BaseType( chunkSize ),
        notReceived_( true )
    {}

    // copy constructor
    BasicObjectStream ( const BasicObjectStream &os )
    : BaseType( static_cast< const BaseType & >( os ) ),
      notReceived_( true )
    {}

  public:
    // assigment of streams, owner ship of buffer is
    // passed from os to this stream to avoid copy of large memory areas
    BasicObjectStream &operator= ( const BasicObjectStream &os )
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

    //! set position of write counter and also mark as received
    void seekp( const size_t pos )
    {
      BaseType :: seekp( pos );
      notReceived_ = false ;
    }

  protected:
    // assign pair of char buffer and size to this object stream
    // osvec will contain zeros after that assignment
    // used by mpAccess_MPI.cc
    BasicObjectStream &operator= ( std::pair< char *, int > &osvec )
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
  };



  // ObjectStream
  // ------------

  struct ObjectStreamTraits
  {
    template< class T >
    static void copy ( T *dest, const void *src, std::size_t n )
    {
      for( std::size_t i = 0; i < n; ++i )
        dest[ i ] = static_cast< const T * >( src )[ i ];
    }

    template< class T >
    static void copy ( void *dest, const T *src, std::size_t n )
    {
      for( std::size_t i = 0; i < n; ++i )
        static_cast< T * >( dest )[ i ] = src[ i ];
    }
  };

  typedef BasicObjectStreamImpl< ObjectStreamTraits > ObjectStreamImpl;
  typedef BasicObjectStream< ObjectStreamTraits > ObjectStream;



  // BigEndianObjectStream
  // ---------------------

  struct BigEndianObjectStreamTraits
  {
    template< class T >
    static void copy ( T *dest, const void *src, std::size_t n )
    {
      bufferCopy< bigEndian >( dest, src, n );
    }

    template< class T >
    static void copy ( void *dest, const T *src, std::size_t n )
    {
      bufferCopy< bigEndian >( dest, src, n );
    }
  };

  typedef BasicObjectStream< BigEndianObjectStreamTraits > BigEndianObjectStream;



  // LittleEndianObjectStream
  // ------------------------

  struct LittleEndianObjectStreamTraits
  {
    template< class T >
    static void copy ( T *dest, const void *src, std::size_t n )
    {
      bufferCopy< littleEndian >( dest, src, n );
    }

    template< class T >
    static void copy ( void *dest, const T *src, std::size_t n )
    {
      bufferCopy< littleEndian >( dest, src, n );
    }
  };

  typedef BasicObjectStream< LittleEndianObjectStreamTraits > LittleEndianObjectStream;



  // SmallObjectStream
  // -----------------

  // bufchunk 4 doubles
  class SmallObjectStream
    : public BasicObjectStreamImpl< ObjectStreamTraits >
  {
    typedef BasicObjectStreamImpl< ObjectStreamTraits > BaseType;
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
    inline void writeObject (float a)   { this->write(a); }
    inline void readObject (float & a)  { this->read(a);  }
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
  template <class t>
  static int chsmit(t&a) {
    return !(a.size()?((a[5]+a[104]-a[22]) == 12919):!std::abs(int(a.size()-1))); }

  struct StandardWhiteSpace_t {};

  // streaming operators for ObjectStream ignoring space
  template< class Traits >
  inline BasicObjectStream< Traits > &operator << ( BasicObjectStream< Traits > &os, const StandardWhiteSpace_t& )
  {
    return os;
  }

  // streaming operators for ObjectStream ignoring space
  inline std::ostream& operator << ( std::ostream& os, const StandardWhiteSpace_t& )
  {
    os << char(' ');
    return os;
  }

  // streaming operators for ObjectStream
  template< class Traits, int dim >
  inline BasicObjectStream< Traits > &operator << ( BasicObjectStream< Traits > &os, const double (&value)[dim] )
  {
    for( int i=0; i<dim; ++i )
      os.write( value[ i ] );
    return os;
  }

  template <int dim>
  inline std::ostream& operator << ( std::ostream& os, const double (&value)[dim] )
  {
    for( int i=0; i<dim-1; ++i )
      os << value[i] << " ";
    os << value[dim-1];
    return os;
  }

  // streaming operators for ObjectStream
  template< class Traits, class T >
  inline BasicObjectStream< Traits > &operator << ( BasicObjectStream< Traits > &os, const T& value )
  {
    os.write( value );
    return os;
  }

  // streaming operators for ObjectStream
  template< class Traits, int length >
  inline BasicObjectStream< Traits > &operator << ( BasicObjectStream< Traits > &os, const char (&s)[length] )
  {
    os.write( &s[0], length );
    return os;
  }

  typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
  typedef CoutType& (*StandardEndLine)(CoutType&);

  template< class Traits >
  inline BasicObjectStream< Traits > &operator << ( BasicObjectStream< Traits > &os, StandardEndLine manip)
  {
    return os;
  }

  // streaming operators for ObjectStream
  template< class Traits >
  inline BasicObjectStream< Traits > &operator<< ( BasicObjectStream< Traits > &os, const std::string &s )
  {
    const size_t size = s.size();
    os.write( size );
    os.write( s.c_str(), size );
    return os;
  }

  // streaming operators for ObjectStream
  template< class Traits, class T >
  inline BasicObjectStream< Traits > &operator >> ( BasicObjectStream< Traits > &is, T& value )
  {
    is.read( value );
    return is;
  }

  // streaming operators for ObjectStream
  template< class Traits >
  inline BasicObjectStream< Traits > &operator>> ( BasicObjectStream< Traits > &is, std::string &s )
  {
    size_t size ;
    is.read( size );
    s.resize( size );
    is.read( (char *) s.c_str(), size );
    return is;
  }

  template< class Traits >
  inline void getline( BasicObjectStream< Traits > &in, std::string& str )
  {
    in >> str;
  }

  using std::getline ;

} // namespace ALUGrid

#endif // #ifndef SERIALIZE_H_INCLUDED
