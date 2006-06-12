// (c) bernhard schupp, 1997 - 1998

#ifndef SERIALIZE_H_INCLUDED
#define SERIALIZE_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iomanip>
  #include <sstream>
  #include <utility>
#else
  #include <iomanip.h>
  #include <strstream.h>
  #include <pair.h>
#endif

	//   Die 'Serializable' Schnittstelle soll eine Objektserialisation er-
	//   m"oglichen, die etwas der in java vorgeschlagenen Situation "ahnelt,
	//   allerdings aus technischen Gr"unden erheblich primitiver ist.
	
class ObjectStream;

class Serializable {
  friend class ObjectStream ;
public:  
  virtual ~Serializable () {}
  virtual bool readObject (ObjectStream &) = 0 ;
  virtual void writeObject (ObjectStream &) const = 0 ;
} ;

	//  'ObjectStream' ist bereits die volle Implementierung eines einfachen
	//  Objektstrommodells auf der Basis der Bibliotheksfunktionen f"ur
	//  den Stringstream (sstream). Die Implemetierung ist eher im Sinne
	//  eines rohen Datenformats mit einigen Testm"oglichkeiten zu sehen.
	
class SmallObjectStream {

  int BufChunk ;

  char * _buf ;
  int _rb, _wb, _len ;

  public :
    class EOFException {} ;
    class OutOfMemoryException {} ;
    inline SmallObjectStream () throw (OutOfMemoryException)
      : BufChunk(4*sizeof(double)) 
      , _buf(0), _rb(0) , _wb(0) , _len (0)
    {
    }
    
    inline void clear() { _wb = 0; _rb = 0; }
    inline void resetReadPosition() { _rb = 0; }
    inline int size() const { return _wb; }

    inline void reserve(size_t s) 
    {
      if ((int) s > _len) 
      {
        _len = (int) s;
        _buf = (char *) realloc (_buf, _len) ;
        if (!_buf) {
          perror ("**AUSNAHME in SmallObjectStream :: reserve (size_t s) ") ;
          throw OutOfMemoryException () ;
        }
      }
    }

    inline ~SmallObjectStream () 
    {
      removeObj();
    }

    inline SmallObjectStream (const SmallObjectStream & os) throw (OutOfMemoryException)
      : BufChunk(os.BufChunk) 
      , _buf(0), _rb(0) , _wb(0) , _len (0)
    {
      assign(os);
    }
    
    inline const SmallObjectStream & operator = (const SmallObjectStream & os) throw (OutOfMemoryException)
    {
      removeObj();
      assign(os);
      return os;
    }

    template <class T> 
    inline void write (const T & a) throw (OutOfMemoryException) 
    {
      writeObject(a);
    }
    
    template <class T> 
    inline void writeObject (const T & a) throw (OutOfMemoryException) 
    {
      const int sizeOfObj = sizeof(T);
      register int ap = sizeOfObj * ((_wb + sizeOfObj - 1)/sizeOfObj ) ;
      _wb = ap + sizeOfObj ;
      if (_wb > _len) {
        _buf = (char *) realloc (_buf, (_len += BufChunk)) ;
        if (!_buf) {
          perror ("**AUSNAHME in ObjectStream :: writeObject (double) ") ;
          throw OutOfMemoryException () ;
        }
      }
      (T &) _buf [ap] = a ;
      return ;
    }
    
    template <class T> 
    inline void read (T & a) throw (EOFException) 
    {
      readObject(a);
    }
    
    template <class T> 
    inline void readObject (T & a) throw (EOFException) 
    {
      const int sizeOfObj = sizeof(T);
      int ap = sizeOfObj * ((_rb + sizeOfObj - 1)/sizeOfObj ) ;
      _rb = ap + sizeOfObj ;
      if (_rb > _wb) throw EOFException () ;
      a = (T &) _buf [ap] ;
      return ;
    }

    inline void write2Stream (const char * buff , int length)
    {
      if( length <= 0 ) return ;

      int newWb = _wb + length;
      if (newWb > _len) 
      {
        _len = newWb ;
        _buf = (char *) realloc (_buf, _len) ;

        if (!_buf) {
          perror ("**AUSNAHME in ObjectStream :: writeObject (double) ") ;
          throw OutOfMemoryException () ;
        }
      }
      memcpy( _buf + _wb , buff , length );
      _wb = newWb;
      return ;
    }
    
  private:
    void removeObj() 
    {
      if( _buf ) free (_buf) ;
      _buf = 0; _len = 0; _wb = 0; _rb = 0;
      return ;
    }
    
    void assign(const SmallObjectStream & os) 
    {
      assert( _buf == 0 );
      if( os._len > 0 ) 
      {
        _len = os._len;
        _wb  = os._wb; 
        _rb  = os._rb; 
        _buf = (char *) malloc (_len) ;
        assert( os._buf );
        memcpy(_buf, os._buf, _len );
      }
      return ;
    }
   friend class ObjectStream ; 
} ;

class ObjectStream {

	// 1/4 Megabyte als Chunksize, und 16 MegaByte als oberes Limit,
	// wobei dieses noch nicht getestet wird.
	
  enum { BufChunk = 0x40000, MemLimit = 0x1000000 } ;
  char * _buf ;
  int _rb, _wb, _len ;

  public :
    class EOFException {} ;
    class OutOfMemoryException {} ;
    inline ObjectStream () throw (OutOfMemoryException) ;
    
    // set read and write counter to zero, i.e. like a newly created
    // ObjectStream 
    inline void clear();

    // set read to zero, 
    inline void resetReadPosition(); 

    inline int size() const { return _wb; }

    inline ~ObjectStream () ;
    inline ObjectStream (const ObjectStream &) throw (OutOfMemoryException) ;
    inline const ObjectStream & operator = (const ObjectStream &) throw (OutOfMemoryException);
    inline void writeObject (const Serializable &) throw (OutOfMemoryException) ;
    inline void readObject (Serializable &) throw (EOFException) ;
    inline void writeObject (double) throw (OutOfMemoryException) ;
    inline void readObject (double &) throw (EOFException) ;
    inline void writeObject (int) throw (OutOfMemoryException) ;
    inline void readObject (int &) throw (EOFException) ;
    
    inline void write (double a) throw (OutOfMemoryException) {
      writeObject(a); }
    inline void read (double &a) throw (EOFException) { readObject(a);}
    inline void write (int a) throw (OutOfMemoryException) {writeObject(a);}
    inline void read (int &a) throw (EOFException) {readObject(a);}

    friend class MpAccessMPI ;
    
    inline void readObject (SmallObjectStream & sm) 
    {
      // to read hole stream, nothing should be read before 
      assert( _rb == 0 );
      sm.write2Stream(_buf,_wb);
    }

    inline void readObject (SmallObjectStream & sm, int length) 
    {
      if( length <= 0 ) return ;
      // actual read position 
      sm.write2Stream(_buf + _rb ,length);
      _rb += length; 
      assert( _rb <= _wb );
    }

    // increments the read position without actualy read data
    inline void removeObject(int length)
    {
      _rb += length; 
      assert( _rb <= _wb );
    }
    
    inline void readObject (ObjectStream & sm, int length) 
    {
      if( length <= 0 ) return ;
      // actual read position 
      sm.write2Stream(_buf + _rb ,length);
      removeObject(length);
    }

    inline void writeObject (const SmallObjectStream & sm) throw (OutOfMemoryException) 
    {
      write2Stream(sm._buf,sm._wb);
    }
    
    inline void writeObject (const ObjectStream & os) throw (OutOfMemoryException) 
    {
      write2Stream(os._buf, os._wb );
    }
   
    inline void write2Stream(const char * buff, int length )
    {
      if( length <= 0 ) return ;

      //int sD = sizeof(double);
      // calculate next double position, to be revised 
      //int ap = sD * ((_wb + sD - 1)/sD) ;
      int ap = _wb;

      int newWb = ap + length;
      if (newWb > _len) 
      {
        _len = newWb + BufChunk ;
        _buf = (char *) realloc (_buf, _len) ;

        if (!_buf) {
          perror ("**AUSNAHME in ObjectStream :: writeObject (double) ") ;
          throw OutOfMemoryException () ;
        }
      }
      
      memcpy( _buf + ap , buff , length );
      _wb = newWb;
      return ;
    }
} ;


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

inline void ObjectStream :: readObject (double & a) throw (EOFException) {
  int ap = sizeof (a) * ((_rb + sizeof (a) - 1)/sizeof (a)) ;
  _rb = ap + sizeof (a) ;
  if (_rb > _wb) throw EOFException () ;
  a = (double &) _buf [ap] ;
  return ;
}

inline void ObjectStream :: writeObject (double a) throw (OutOfMemoryException) {
  register int ap = sizeof (a) * ((_wb + sizeof (a) - 1)/sizeof (a)) ;
  _wb = ap + sizeof (a) ;
  if (_wb > _len) {
    _buf = (char *) realloc (_buf, (_len += BufChunk)) ;
    if (!_buf) {
      perror ("**AUSNAHME in ObjectStream :: writeObject (double) ") ;
      throw OutOfMemoryException () ;
    }
  }
  (double &) _buf [ap] = a ;
  return ;
}

inline void ObjectStream :: readObject (int & i) throw (EOFException) {
  int ap = sizeof (i) * ((_rb + sizeof (i) - 1)/sizeof (i)) ;
  _rb = ap + sizeof (i) ;
  if (_rb > _wb) throw EOFException () ;
  i = (int &) _buf [ap] ;
  return ;
}

inline void ObjectStream :: writeObject (int i) throw (OutOfMemoryException) {
  register int ap = sizeof (i) * ((_wb + sizeof (i) - 1)/sizeof (i)) ;
  _wb = ap + sizeof (i) ;
  if (_wb > _len) {
    _buf = (char *) realloc (_buf, (_len += BufChunk)) ;
    if (!_buf) {
      perror ("**AUSNAHME in ObjectStream :: writeObject (int) ") ;
      throw OutOfMemoryException () ;
    }
  }
  (int &) _buf [ap] = i ;
  return ;
}

inline ObjectStream :: ObjectStream () throw (OutOfMemoryException) 
	: _buf (0), _rb (0), _wb (0)
  , _len (BufChunk) 
{
  _buf = (char *) malloc (BufChunk) ;
  if (!_buf) {
    perror ("**AUSNAHME in ObjectStream :: ObjectStream ()") ;
    throw OutOfMemoryException () ;
  }
  return ;
}

inline void ObjectStream :: clear () 
{
  _rb = 0;
  _wb = 0;
}

inline void ObjectStream :: resetReadPosition() 
{
  _rb = 0;
}

inline ObjectStream :: ~ObjectStream () 
{
  if( (_len > 0) && _buf ) free (_buf) ;
  return ;
}

inline ObjectStream :: ObjectStream (const ObjectStream & os) throw (OutOfMemoryException)
	: _buf (0), _rb (os._rb), _wb (os._wb), _len (os._len) {
  if (_len > 0) {
    _buf = (char *) malloc (_len) ;

    if( _wb > 0 )
    {
      if (_buf) {
        memcpy (_buf, os._buf, _len) ;
      } else {
        perror (" in ObjectStream (const ObjectStream &) ") ;
        throw OutOfMemoryException () ;
      }
    }
  }
  return ;
}

inline const ObjectStream & ObjectStream :: operator = (const ObjectStream & os) throw (OutOfMemoryException) {	
  if (&os != this) 
  {
    _rb = os._rb ;
    _wb = os._wb ;
    _len = os._len ;
    if (_len) 
    {
      _buf = (char *) realloc (_buf, _len) ;
      if( _wb > 0) 
      {
        if (_buf) 
        {
          memcpy (_buf, os._buf, _len) ;
        } else 
        {
          perror (" in ObjectStream :: operator = (const ObjectStream &) ") ;
          	throw OutOfMemoryException () ;
        }
      }
      
    } 
    else 
    {
      free (_buf) ;
      _buf = 0 ;
    }
  }
  return os ;
}
#endif	// SERIALIZE_H_INCLUDED
