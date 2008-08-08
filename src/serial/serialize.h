// (c) bernhard schupp, 1997 - 1998
// (c) new implementation by Robert Kloefkorn 2006 

#ifndef SERIALIZE_H_INCLUDED
#define SERIALIZE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

using namespace std;
#include <iomanip>
#include <sstream>
#include <utility>

  //   Die 'Serializable' Schnittstelle soll eine Objektserialisation er-
  //   m"oglichen, die etwas der in java vorgeschlagenen Situation "ahnelt,
  //   allerdings aus technischen Gr"unden erheblich primitiver ist.
  
class ObjectStream;

// iterface for serialization 
class Serializable {
  friend class ObjectStream ;
protected:
  Serializable () {}
  Serializable (const Serializable  &) {}
public:  
  virtual ~Serializable () {}
  virtual bool readObject (ObjectStream &) = 0 ;
  virtual void writeObject (ObjectStream &) const = 0 ;
} ;

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
  
  // return's true if size > 0 and read position is zero
  // i.e. a read othe stream will result some valid data  
  inline bool validToRead () const { return (_wb > 0) && (_rb == 0); }

  // return size of bytes allready written to stream 
  inline int size() const { return _wb; }

  // reserve s bytes memory 
  inline void reserve(size_t s) { if (s > _len) reallocateBuffer(s); }

  // delete stream 
  inline ~ObjectStreamImpl () { removeObj(); }

  //! assign buffer from os the local buffer, os ownership is false afterwards
  inline const ObjectStreamImpl & operator = (const ObjectStreamImpl & os)
  {
    removeObj();
    assign(os);
    return os;
  }

  // write value to stream 
  template <class T> 
  inline void write (const T & a)
  {
    assert( _owner );
    const size_t ap = _wb;
    _wb += sizeof(T) ;

    // if buffer is to small, reallocate 
    if (_wb > _len) 
    {
      reallocateBuffer(_wb);
    }
    assert( _wb <= _len );

    // call assignment operator of type T 
    static_cast<T &> (*((T *) getBuff(ap) )) = a;
    return ;
  }
 
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
    os.write2Stream( getBuff(_rb) ,length);
    removeObject(length);
  }

  // writes hole stream of os to this stream
  inline void writeStream (const ObjectStreamImpl & os) 
  {
    write2Stream(os._buf,os._wb);
  }
  
  // increments the read position without actualy read data
  inline void removeObject(const size_t length) throw (EOFException) 
  {
    _rb += length; 
    if( _rb > _wb) throw EOFException () ;
  }
 
  //! free allocated memory 
  inline void reset() 
  {
    removeObj();
  }
 
  // static alloc of char buffer for use in mpAccess_MPI 
  inline static char * allocateBuffer(size_t newSize) throw (OutOfMemoryException)
  {
    // make sure that char has size of 1, otherwise check doExchange in
    // mpAccess_MPI.cc 
    char * buffer = (char *) malloc (newSize * sizeof(char)) ;
    if ( ! buffer ) 
    {
      perror ("**EXCEPTION in ObjectStream :: allocateBuffer(size_t) ") ;
      throw OutOfMemoryException () ;
    }
    return buffer;
  }
  
  // static free for use in mpAccess_MPI 
  inline static void freeBuffer(char * buffer)
  {
    assert( buffer );
    free ( buffer );
  }
protected:
  inline char * getBuff (const size_t ap) { return (_buf + ap); }
  inline const char * getBuff (const size_t ap) const { return (_buf + ap); }

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
    if( _buf && _owner ) free (_buf) ;
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
  
  inline void write2Stream(const char * buff, const size_t length )
  {
    assert( _owner );
    if( length == 0 ) return ;

    size_t newWb = _wb + length;
    if (newWb > _len) reallocateBuffer(newWb);
    
    memcpy( getBuff(_wb) , buff , length );
    _wb = newWb;
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
class ObjectStream : public ObjectStreamImpl 
{
  typedef ObjectStreamImpl BaseType;
  
  // 1/4 Megabyte als Chunksize, und 16 MegaByte als oberes Limit,
  // wobei dieses noch nicht getestet wird.
  //enum { BufChunk = 0x40000 } ;
  enum { BufChunk = 65536 * sizeof(double) } ;

public :
  // ENDOFSTREAM should be in range of char, i.e. 0 to 256 
  // and not conflict with refinement rules in gitter_sti.h 
  static const char ENDOFSTREAM = 127;
  
  // create empty object stream 
  inline ObjectStream () : BaseType(BufChunk) 
  {
  } 
  
  // copy constructor 
  inline ObjectStream (const ObjectStream & os) : BaseType(os) {} 
 
public:  
  // assigment of streams, owner ship of buffer is 
  // passed from os to this stream to avoid copy of large memory areas 
  inline const ObjectStream & operator = (const ObjectStream & os) 
  {
    BaseType::operator =(os); 
    return os;
  }
  
  inline void writeObject (const Serializable &) throw (OutOfMemoryException);
  inline void readObject (Serializable &) throw (EOFException) ;
  
  inline void writeObject (double a)  { this->write(a); }
  inline void readObject (double & a) { this->read(a);  }
  inline void writeObject (int a)     { this->write(a); } 
  inline void readObject (int & a)    { this->read(a);  }

  ////////////////////////////////////
  // to behave like stringstream 
  ////////////////////////////////////
  // put char 
  inline void put (const char a)  { this->write(a); }
  // get char 
  inline char get () 
  { 
    char a;
    this->read(a);  
    return a;
  }
  // eof function 
  bool eof () const { return (this->_rb > this->_wb); }
  /////////////////////////////////////
    
protected:  
  // assign pair of char buffer and size to this object stream 
  // osvec will contain zeros after that assignment 
  // used by mpAccess_MPI.cc 
  inline ObjectStream & operator = (pair< char* , int > & osvec)  
  {
    BaseType :: removeObj();
    BaseType :: assign( osvec.first , osvec.second );
    // reset osvec
    osvec.first = 0;
    osvec.second = 0;
    return *this;
  }
  
  friend class MpAccessMPI ;
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
  inline const SmallObjectStream & operator = (const SmallObjectStream & os) 
  {
    BaseType::operator =(os); 
    return os;
  }

  inline void writeObject (double a)  { this->write(a); }
  inline void readObject (double & a) { this->read(a);  }
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

inline void ObjectStream :: readObject (Serializable & oi) throw (EOFException) {
  oi.readObject (*this) ;
  return ; ;
}

inline void ObjectStream :: writeObject (const Serializable & oi) throw (OutOfMemoryException) {
  oi.writeObject (*this) ;
  return ;
}
#endif  // SERIALIZE_H_INCLUDED
