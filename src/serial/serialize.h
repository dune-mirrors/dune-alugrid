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
  const size_t BufChunk; 

public :
  class EOFException {} ;
  class OutOfMemoryException {} ;
  inline ObjectStreamImpl (size_t chunk) 
    : _buf(0), _rb(0) , _wb(0) , _len (0) , BufChunk(chunk)
  {
  }
  inline ObjectStreamImpl (const ObjectStreamImpl & os)
    : _buf(0), _rb(0) , _wb(0) , _len (0) , BufChunk(os.BufChunk)
  {
    assign(os);
  }
  
  // reset write and read postitions 
  inline void clear() { _wb = 0; _rb = 0; }
  // reset read position 
  inline void resetReadPosition() { _rb = 0; }
  // return size of bytes allready written to stream 
  inline int size() const { return _wb; }

  // reserve s bytes memory 
  inline void reserve(size_t s) { if (s > _len) reallocateBuffer(s); }

  // delete stream 
  inline ~ObjectStreamImpl () { removeObj(); }

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
    size_t ap = _wb;
    _wb = ap + sizeof(T) ;
    if (_wb > _len) reallocateBuffer(_wb);
    assert( _wb <= _len );
    T & val = *((T *) getBuff(ap) );
    val = a;
    return ;
  }
 
  // read value from stream 
  template <class T> 
  inline void read (T & a) throw (EOFException) 
  {
    size_t ap = _rb;
    _rb = ap + sizeof(T);
    
    if (_rb > _wb) throw EOFException () ;
    const T & val = *((const T *) getBuff(ap) );
    a = val;
    return ;
  }

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
  
protected:
  inline char * getBuff (const size_t ap) { return (_buf + ap); }
  inline const char * getBuff (const size_t ap) const { return (_buf + ap); }

  void reallocateBuffer(size_t newSize) throw (OutOfMemoryException)
  {
    _len += BufChunk; 
    if(_len < newSize) _len = newSize;
    _buf = (char *) realloc (_buf, _len) ;
    if (!_buf) {
      perror ("**EXCEPTION in ObjectStream :: reallocateBuffer(size_t) ") ;
      throw OutOfMemoryException () ;
    }
  }

  // delete buffer 
  void removeObj() 
  {
    if( _buf ) free (_buf) ;
    _buf = 0; _len = 0; _wb = 0; _rb = 0;
    return ;
  }
 
  // assign buffer 
  void assign(const ObjectStreamImpl & os) throw (OutOfMemoryException)
  {
    assert( _buf == 0 );
    if( os._len > 0 ) 
    {
      _len = os._len;
      _wb  = os._wb; 
      _rb  = os._rb; 
      const_cast<size_t &> (BufChunk) = os.BufChunk;
      
      _buf = (char *) malloc (_len) ;
      if (!_buf) {
        perror ("**AUSNAHME in ObjectStream :: writeObject (double) ") ;
        throw OutOfMemoryException () ;
      }

      assert( os._buf );
      memcpy(_buf, os._buf, _len );
    }
    return ;
  }
  
  inline void write2Stream(const char * buff, const size_t length )
  {
    if( length == 0 ) return ;

    size_t newWb = _wb + length;
    if (newWb > _len) reallocateBuffer(newWb);
    
    memcpy( getBuff(_wb) , buff , length );
    _wb = newWb;
    return ;
  }
  //friend class ObjectStream ; 
} ;

// bufchunk 0.25 Megabyte 
class ObjectStream : public ObjectStreamImpl 
{
  typedef ObjectStreamImpl BaseType;
  
  // 1/4 Megabyte als Chunksize, und 16 MegaByte als oberes Limit,
  // wobei dieses noch nicht getestet wird.
  enum { BufChunk = 0x40000 } ;

public :
  // ENDOFSTREAM should be in range of char, i.e. -127 to 128 
  // and not conflict with refinement rules in gitter_sti.h 
  enum { ENDOFSTREAM = -3 };
  
  inline ObjectStream () : BaseType(BufChunk) 
  {
    this->reallocateBuffer(BufChunk);
  } 
  
  inline ObjectStream (const ObjectStream & os) : BaseType(os) {} 
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
  inline void put (const char a)  { this->write(a); }
  inline char get () 
  { 
    char a;
    this->read(a);  
    return a;
  }
  bool eof () const { return (this->_rb > this->_wb); }
  /////////////////////////////////////
    
  friend class MpAccessMPI ;
} ;

// bufchunk 4 doubls Megabyte 
class SmallObjectStream : public ObjectStreamImpl
{
  typedef ObjectStreamImpl BaseType;
  enum { BufChunk = 4 * sizeof(double) };
public:  
  inline SmallObjectStream () : BaseType(BufChunk) {} 
  
  inline SmallObjectStream (const SmallObjectStream & os) : BaseType(os) {} 
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
