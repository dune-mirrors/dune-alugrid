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
	
class ObjectStream {

	// 1/4 Megabyte als Chunksize, und 16 MegaByte als oberes Limit,
	// wobei dieses noch nicht getestet wird.
	
  enum { BufChunk = 0x40000, MemLimit = 0x1000000 } ;
  char * _buf ;
  int _rb, _wb, _len ;
//  dequeue < pair < char *, int > > dqe ;
  public :
    class EOFException {} ;
    class OutOfMemoryException {} ;
    inline ObjectStream () throw (OutOfMemoryException) ;
    
    // set read and write counter to zero, i.e. like a newly created
    // ObjectStream 
    inline void clear();

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
	: _buf (0), _rb (0), _wb (0), _len (BufChunk) {
  _buf = (char *) malloc (BufChunk) ;
  if (!_buf) {
    perror ("**AUSNAHME in ObjectStream :: ObjectStream ()") ;
    throw OutOfMemoryException () ;
  }
  return ;
}

inline void ObjectStream :: clear () {
  _rb = 0;
  _wb = 0;
}

inline ObjectStream :: ~ObjectStream () {
  free (_buf) ;
  return ;
}

inline ObjectStream :: ObjectStream (const ObjectStream & os) throw (OutOfMemoryException)
	: _buf (0), _rb (os._rb), _wb (os._wb), _len (os._len) {
  if (_len) {
    _buf = (char *) malloc (_len) ;
    if (_buf) {
      memcpy (_buf, os._buf, _len) ;
    } else {
      perror (" in ObjectStream (const ObjectStream &) ") ;
      throw OutOfMemoryException () ;
    }
  }
  return ;
}

inline const ObjectStream & ObjectStream :: operator = (const ObjectStream & os) throw (OutOfMemoryException) {	
  if (&os != this) {
    _rb = os._rb ;
    _wb = os._wb ;
    _len = os._len ;
    if (_len) {
      _buf = (char *) realloc (_buf, _len) ;
      if (_buf) {
        memcpy (_buf, os._buf, _len) ;
      } else {
        perror (" in ObjectStream :: operator = (const ObjectStream &) ") ;
	throw OutOfMemoryException () ;
      }
    } else {
      free (_buf) ;
      _buf = 0 ;
    }
  }
  return os ;
}
#endif	// SERIALIZE_H_INCLUDED
