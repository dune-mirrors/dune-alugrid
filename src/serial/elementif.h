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
    virtual bool lockAndTry () = 0 ;
    virtual bool unlockAndResume (bool) = 0 ;
    virtual bool lockedAgainstCoarsening () const = 0 ;
} ;

// default implementation 
class EdgePllXDefault : public EdgePllXIF 
{
  protected :
    virtual ~EdgePllXDefault () {}
  private:  
    virtual bool lockAndTry () { assert(false);abort(); return false ; }
    virtual bool unlockAndResume (bool) { assert(false);abort(); return false ; }
    virtual bool lockedAgainstCoarsening () const { assert(false);abort(); return false ; }
} ;


// tpye of ElementPllXIF_t is ElementPllXIF, see parallel.h
class ElementPllXIF : public MacroGridMoverDefault 
{
  protected :
    typedef pair<helement*, int> ghostpair_t ;
    virtual ~ElementPllXIF () {}
  public :
    virtual pair < ElementPllXIF *, int > accessOuterPllX (const pair < ElementPllXIF *, int > &, int) = 0 ;
    virtual pair < const ElementPllXIF *, int > accessOuterPllX (const pair < const ElementPllXIF *, int > &, int) const = 0 ;
    virtual pair < ElementPllXIF *, int > accessInnerPllX (const pair < ElementPllXIF *, int > &, int) = 0 ;
    virtual pair < const ElementPllXIF *, int > accessInnerPllX (const pair < const ElementPllXIF *, int > &, int) const = 0 ;
  public :
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

    virtual void writeStaticState (ObjectStream &, int) const = 0 ;
    virtual void readStaticState (ObjectStream &, int) = 0 ;
    virtual void writeDynamicState (ObjectStream &, int) const = 0 ;
    virtual void readDynamicState (ObjectStream &, int) = 0 ;

    virtual void VertexData2os(ObjectStream &, GatherScatterType &, int) { cout << "ich bin die falsche...\n" << flush; }
    virtual void EdgeData2os  (ObjectStream &, GatherScatterType &, int) { cout << "ich bin die falsche...1\n" << flush; }
    virtual void FaceData2os  (ObjectStream &, GatherScatterType &, int) { cout << "ich bin die falsche...2\n" << flush; }
    virtual void writeElementData (ObjectStream &, GatherScatterType &) { cout << "ich bin die falsche...3\n" << flush; }
    virtual void writeDynamicState(ObjectStream &, GatherScatterType &) const = 0 ;
    virtual void readDynamicState (ObjectStream &, GatherScatterType &) = 0 ;

    // pack as ghost, default does nothing but macro elements are pack as
    // ghosts 
    virtual void packAsGhost(ObjectStream &,int) const {}

    // unpack as ghost data and insert ghost cell, default does nothing
    virtual void insertGhostCell(ObjectStream &,int) {
    }
    
  public :
    virtual int ldbVertexIndex () const = 0 ;
    virtual int & ldbVertexIndex () = 0 ;
    virtual bool ldbUpdateGraphVertex (LoadBalancer :: DataBase &) = 0 ;
  public :
    virtual void packAsBnd (int,int,ObjectStream &) const = 0 ;
    virtual bool erasable () const = 0 ;
  public :
    virtual void getRefinementRequest (ObjectStream &) = 0 ;
    virtual bool setRefinementRequest (ObjectStream &) = 0 ;
  public :
    virtual bool lockAndTry () = 0 ;
    virtual bool unlockAndResume (bool) = 0 ;
} ;

class FacePllXIF : public LinkedObjectDefault //, public MacroGridMoverIF
{
  protected :
    virtual ~FacePllXIF () {}
  public :
    virtual vector < int > checkParallelConnectivity () const = 0 ;
    virtual pair < ElementPllXIF *, int > accessOuterPllX () = 0 ;
    virtual pair < const ElementPllXIF *, int > accessOuterPllX () const = 0 ;
    virtual pair < ElementPllXIF *, int > accessInnerPllX () = 0 ;
    virtual pair < const ElementPllXIF *, int > accessInnerPllX () const = 0 ;

  public :
    virtual void writeStaticState (ObjectStream &) const = 0 ;
    virtual void readStaticState (ObjectStream &) = 0 ;
  public :
    virtual bool ldbUpdateGraphEdge (LoadBalancer :: DataBase &) = 0 ;
} ;

// default implementation (should not be called) 
class FacePllXDefault : public FacePllXIF 
{
  protected :
    virtual ~FacePllXDefault () {}
  private:
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
        inline virtual VertexPllXIF & accessPllX () throw (AccessPllException) 
        {
          assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
          throw AccessPllException () ;
        }
        inline virtual const VertexPllXIF & accessPllX () const throw (AccessPllException)
        {
          assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
          throw AccessPllException () ;
        }
        inline virtual void detachPllXFromMacro () throw (AccessPllException)
        {
          assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
          throw AccessPllException () ;
        }
    } ;

    class EdgeIF : public EdgePllXDefault 
#ifdef ALUGRID_USE_COMM_BUFFER_IN_ITEM
      : public CommunicationBuffer 
#endif
    {
      public :
        virtual ~EdgeIF () {}
        typedef class Key2SLZ identifier_t ;
        inline EdgePllXIF & accessPllX () { return *this; }
        inline const EdgePllXIF & accessPllX () const { return *this; }
        inline void detachPllXFromMacro () {} 
    } ;
    class FaceIF : public FacePllXDefault {
      public :
        virtual ~FaceIF () {}
        typedef class Key3SLZ identifier_t ;
        inline FacePllXIF & accessPllX () { return *this; }
        inline const FacePllXIF & accessPllX () const { return *this; }
        inline void detachPllXFromMacro () {}
    } ;
    class ElementIF {
      public :
        virtual ~ElementIF () {}
        inline virtual ElementPllXIF & accessPllX () throw (AccessPllException) 
        {
          assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
          throw AccessPllException () ;
        }
        inline virtual const ElementPllXIF & accessPllX () const throw (AccessPllException)
        {
          assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
          throw AccessPllException () ;
        }
        inline virtual void detachPllXFromMacro () throw (AccessPllException) 
        {
          assert ((abort (), (cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << endl))) ;
          throw AccessPllException () ;
        }
    } ;
} ;
#endif
