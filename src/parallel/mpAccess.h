// (c) bernhard schupp 1997 - 1998
#ifndef MPACCESS_H_INCLUDED
#define MPACCESS_H_INCLUDED

#include "serialize.h"

class MpAccessGlobal {
  public :
    class CommIF
    {
    protected:
      CommIF () {}
    public:
      virtual ~CommIF() {}
      virtual CommIF* clone () const = 0;
    };

    inline virtual ~MpAccessGlobal () ;
    virtual int psize () const = 0 ;
    virtual int myrank () const = 0 ;
    virtual int barrier () const = 0 ;
    virtual int gmax (int) const = 0 ;
    virtual int gmin (int) const = 0 ;
    virtual int gsum (int) const = 0 ;
    virtual long gmax (long) const = 0 ;
    virtual long gmin (long) const = 0 ;
    virtual long gsum (long) const = 0 ;
    virtual double gmax (double) const = 0 ;
    virtual double gmin (double) const = 0 ;
    virtual double gsum (double) const = 0 ;
    virtual void gmax (double*,int,double*) const = 0 ;
    virtual void gmin (double*,int,double*) const = 0 ;
    virtual void gsum (double*,int,double*) const = 0 ;
    virtual pair<double,double> gmax (pair<double,double>) const = 0 ;
    virtual pair<double,double> gmin (pair<double,double>) const = 0 ;
    virtual pair<double,double> gsum (pair<double,double>) const = 0 ;
    virtual vector < int > gcollect (int) const = 0 ;
    virtual vector < double > gcollect (double) const = 0 ;
    virtual vector < vector < int > > gcollect (const vector < int > &) const = 0 ;
    virtual vector < vector < double > > gcollect (const vector < double > &) const = 0 ;
    virtual vector < ObjectStream > gcollect (const ObjectStream &) const = 0 ;

    //! return address of communicator (not optimal but avoid explicit MPI types here)
    virtual const CommIF* communicator() const = 0;
} ;

class MpAccessLocal : public MpAccessGlobal {
  map < int, int, less < int > > _linkage ;
  public :
    inline virtual ~MpAccessLocal () ;
    void printLinkage (ostream &) const ;
    inline void removeLinkage () ;
    inline int nlinks () const ;
    inline int link (int) const ;
    vector < int > dest () const ;
    int insertRequestSymetric (set < int, less < int > >) ;
    virtual vector < vector < int > > exchange (const vector < vector < int > > &) const = 0 ;
    virtual vector < vector < double > > exchange (const vector < vector < double > > &) const = 0 ;
    virtual vector < vector < char > > exchange (const vector < vector < char > > &) const = 0 ;
    // exchange data and return new vector of object streams 
    virtual vector < ObjectStream > exchange (const vector < ObjectStream > &) const = 0 ;
    // exchange data with given send and receive buffers 
    virtual void exchange (const vector < ObjectStream > &, vector < ObjectStream > &) const = 0 ;
} ;


	//
	//    #    #    #  #          #    #    #  ######
	//    #    ##   #  #          #    ##   #  #
	//    #    # #  #  #          #    # #  #  #####
	//    #    #  # #  #          #    #  # #  #
	//    #    #   ##  #          #    #   ##  #
	//    #    #    #  ######     #    #    #  ######
	//

inline MpAccessGlobal :: ~MpAccessGlobal () {
}

inline MpAccessLocal :: ~MpAccessLocal () {
}

inline int MpAccessLocal :: link (int i) const {
  assert (_linkage.end () != _linkage.find (i)) ;
  return (* _linkage.find (i)).second ;
}

inline int MpAccessLocal :: nlinks () const {
  return _linkage.size () ;
}

inline void MpAccessLocal :: removeLinkage () 
{
  _linkage.clear();
  //_linkage.erase (_linkage.begin (), _linkage.end ()) ;
  return ;
}

#endif
