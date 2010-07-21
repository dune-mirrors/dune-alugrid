// (c) bernhard schupp, 1997 - 1998
#ifndef ELEMENTIF_H_INCLUDED
#define ELEMENTIF_H_INCLUDED

//////////////////////////////////////////////////////////////////////////////
//
//
//  Interfaces for elements, faces, edges, and vertices for parallel computations 
//
//
//////////////////////////////////////////////////////////////////////////////

class VertexPllXIF : public LinkedObjectDefault //, public MacroGridMoverIF
{
  protected :
    virtual ~VertexPllXIF () {}
  public :
    virtual bool setLinkage (vector < int >) = 0 ;
} ;

class VertexPllXDefault : public VertexPllXIF
{
  protected :
    virtual ~VertexPllXDefault () {}
  public :
    virtual bool setLinkage (vector < int >) { assert(false);abort(); return false ; } 
} ;


class EdgePllXIF : public RefineableObjectDefault //, public LinkedObject, public MacroGridMoverIF
{
  protected :
    virtual ~EdgePllXIF () {}
  public :
    // my identifier class 
    typedef class Key2SLZ identifier_t ;

    virtual bool lockAndTry () = 0 ;
    virtual bool unlockAndResume (bool) = 0 ;
    virtual bool lockedAgainstCoarsening () const = 0 ;

    // only for compatibility for Dune release 2.0 (to be removed later)
    inline EdgePllXIF& accessPllX () { return *this; }
    inline const EdgePllXIF& accessPllX () const { return *this; }
} ;

// default implementation 
class EdgePllXDefault : public EdgePllXIF 
{
  protected :
    virtual ~EdgePllXDefault () {}
  public :
    virtual bool lockAndTry () { assert(false);abort(); return false ; }
    virtual bool unlockAndResume (bool) { assert(false);abort(); return false ; }
    virtual bool lockedAgainstCoarsening () const { assert(false);abort(); return false ; }
} ;

// type of ElementPllXIF_t is ElementPllXIF, see parallel.h
class ElementPllXIF : public MacroGridMoverDefault 
{
  protected :
    virtual ~ElementPllXIF () {}
  public :
    typedef pair < ElementPllXIF *, int > accesspair_t; 
    typedef pair < const ElementPllXIF *, int > constaccesspair_t; 
    virtual void detachPllXFromMacro () {} 

    // default implementation for accessInnerPllX and accessOuterPllX
    virtual accesspair_t accessOuterPllX (const accesspair_t &x, int) { return x ; }
    virtual constaccesspair_t accessOuterPllX (const constaccesspair_t &x, int) const { return x ;}
    virtual accesspair_t accessInnerPllX (const accesspair_t&, int f) { return accesspair_t( this , f ); }
    virtual constaccesspair_t accessInnerPllX (const constaccesspair_t &, int f) const { return constaccesspair_t( this , f ); }
  public :
    typedef pair<helement*, int> ghostpair_t ;
    virtual ghostpair_t getGhost () 
    { 
      cerr << "ERROR: method getGhost of Interface class should not be used! in: " << __FILE__ << " line: " <<__LINE__<<"\n";
      abort(); 
      return ghostpair_t( (helement*)0 , -1); 
    }

    virtual int ghostLevel () const
    { 
      cerr << "ERROR: method ghostLevel of Interface class should not be used! in: " << __FILE__ << " line: " <<__LINE__<<"\n";
      abort(); 
      return 0; 
    }

    virtual bool ghostLeaf () const
    { 
      cerr << "ERROR: method ghostLeaf of Interface class should not be used! in: " << __FILE__ << " line: " <<__LINE__<<"\n";
      abort(); 
      return 0; 
    }

    virtual void getAttachedElement ( pair < helement* , hbndseg * > & p)
    {
      cerr << "Overload method in the classes file:" << __FILE__ << " line:" << __LINE__ << "\n";
      abort();
      p.first  = 0;
      p.second = 0;
    }

    virtual void writeStaticState (ObjectStream &, int) const 
    { assert(false);abort(); }
    virtual void readStaticState (ObjectStream &, int)
    { assert(false);abort(); }
    virtual void writeDynamicState (ObjectStream &, int) const
    { assert(false);abort(); }
    virtual void readDynamicState (ObjectStream &, int)
    { assert(false);abort(); }

    virtual void VertexData2os(ObjectStream &, GatherScatterType &, int) 
    { assert(false);abort(); }
    virtual void EdgeData2os  (ObjectStream &, GatherScatterType &, int)
    { assert(false);abort(); }
    virtual void FaceData2os  (ObjectStream &, GatherScatterType &, int)
    { assert(false);abort(); }
    virtual void writeElementData (ObjectStream &, GatherScatterType &)
    { assert(false);abort(); }
    virtual void writeDynamicState(ObjectStream &, GatherScatterType &) const
    { assert(false);abort(); }
    virtual void readDynamicState (ObjectStream &, GatherScatterType &)
    { assert(false);abort(); }

    // pack as ghost, default does nothing but macro elements are pack as
    // ghosts 
    virtual void packAsGhost(ObjectStream &,int) const {}

    // unpack as ghost data and insert ghost cell, default does nothing
    virtual void insertGhostCell(ObjectStream &,int) {}
    
  public :
    virtual int ldbVertexIndex () const
    { assert(false);abort(); return -1;  }
    virtual int & ldbVertexIndex ()
    { assert(false);abort(); return *(new int ());  }
    virtual bool ldbUpdateGraphVertex (LoadBalancer :: DataBase &)
    { assert(false);abort(); return false;  }
  public :
    virtual void packAsBnd (int,int,ObjectStream &) const
    { assert(false);abort(); }
    virtual bool erasable () const
    { assert(false);abort(); return false;  }
  public :
    virtual void getRefinementRequest (ObjectStream &)
    { assert(false);abort(); }
    virtual bool setRefinementRequest (ObjectStream &)
    { assert(false);abort(); return false;  }
  public :
    virtual bool lockAndTry ()
    { assert(false);abort(); return false;  }
    virtual bool unlockAndResume (bool)
    { assert(false);abort(); return false;  }
} ;


class FacePllXIF : public LinkedObjectDefault //, public MacroGridMoverIF
{
  protected :
    virtual ~FacePllXIF () {}
  public :
    // my identifier class 
    typedef class Key3SLZ identifier_t ;

    virtual vector < int > checkParallelConnectivity () const = 0 ;
    virtual pair < ElementPllXIF *, int > accessOuterPllX () = 0 ;
    virtual pair < const ElementPllXIF *, int > accessOuterPllX () const = 0 ;
    virtual pair < ElementPllXIF *, int > accessInnerPllX () = 0 ;
    virtual pair < const ElementPllXIF *, int > accessInnerPllX () const = 0 ;

  public :
    virtual void writeStaticState (ObjectStream &) const = 0 ;
    virtual void readStaticState (ObjectStream &) = 0 ;
    virtual bool ldbUpdateGraphEdge (LoadBalancer :: DataBase &) = 0 ;

    // only for compatibility for Dune release 2.0 (to be removed later)
    inline FacePllXIF & accessPllX () { return *this; }
    inline const FacePllXIF & accessPllX () const { return *this; }
} ;

// default implementation (should not be called) 
class FacePllXDefault : public FacePllXIF 
{
  protected :
    virtual ~FacePllXDefault () {}
  public :
    virtual vector < int > checkParallelConnectivity () const { assert( false ); abort(); return vector<int> (); }
    virtual pair < ElementPllXIF *, int > accessOuterPllX () { assert( false ); abort(); return pair< ElementPllXIF *, int > ( (ElementPllXIF *) 0, -1); }
    virtual pair < const ElementPllXIF *, int > accessOuterPllX () const  { assert( false); abort(); return pair< ElementPllXIF *, int > ( (ElementPllXIF *) 0, -1); }
    virtual pair < ElementPllXIF *, int > accessInnerPllX ()  { assert( false); abort(); return pair< ElementPllXIF *, int > ( (ElementPllXIF *) 0, -1); }
    virtual pair < const ElementPllXIF *, int > accessInnerPllX () const { assert( false); abort(); return pair< ElementPllXIF *, int > ( (ElementPllXIF *) 0, -1); }

    virtual void writeStaticState (ObjectStream &) const { assert(false);abort(); }
    virtual void readStaticState (ObjectStream &) { assert(false);abort(); }
    virtual bool ldbUpdateGraphEdge (LoadBalancer :: DataBase &) { assert(false);abort(); return false ; }
};

/////////////////////////////////////////////////////////////////////////////
//
//  Parallel If extension 
//
/////////////////////////////////////////////////////////////////////////////

class Parallel {
  public :

    class CommunicationBuffer 
    {
      protected:
        typedef vector < SmallObjectStream > BufferType; 
        BufferType * _buff; 

        CommunicationBuffer () : _buff(0) {}
        ~CommunicationBuffer () 
        { 
          if(_buff) delete _buff; 
          _buff = 0;  
        }
      public:   
        void reserveBuffer (const size_t size)
        {
          if(_buff) 
          {
            // adjust size 
            if( size > _buff->size() ) _buff->resize( size );
          }
          else 
          {
            _buff = new BufferType( size ); 
          }
        }
        BufferType & commBuffer () { assert(_buff); return *_buff; }
        const BufferType & commBuffer () const { assert(_buff); return *_buff; }
    };
    
    class AccessPllException {} ;
  
    class VertexIF : public VertexPllXDefault
#ifdef ALUGRID_USE_COMM_BUFFER_IN_ITEM
      : public CommunicationBuffer 
#endif
    {
      public :
        virtual ~VertexIF () {}
        typedef class Key1SLZ identifier_t ;
        virtual VertexPllXIF & accessPllX () throw (AccessPllException) 
        {
          assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
          throw AccessPllException () ;
        }
        virtual const VertexPllXIF & accessPllX () const throw (AccessPllException)
        {
          assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
          throw AccessPllException () ;
        }
        virtual void detachPllXFromMacro () throw (AccessPllException)
        {
          assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
          throw AccessPllException () ;
        }
    } ;

    class hasFacePllXIF {
      public :
        virtual ~hasFacePllXIF () {}
        virtual ElementPllXIF& accessPllX () throw (AccessPllException)
        {
          assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
          throw AccessPllException () ;
        }
        virtual const ElementPllXIF& accessPllX () const throw (AccessPllException) 
        {
          assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
          throw AccessPllException () ;
        }
    } ;
} ;
#endif
