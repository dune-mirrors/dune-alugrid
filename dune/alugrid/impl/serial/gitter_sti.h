// (c) Robert Kloefkorn 2004 - 2013
#ifndef GITTER_STI_H_INCLUDED
#define GITTER_STI_H_INCLUDED

#include <limits>
#include <list>
#include <mutex>
#include <memory>
#include <vector>
#include <array>
#include <utility>

#include "../macrofileheader.hh"
#include "../indexstack.h"
#include "../parallel/gitter_pll_ldb.h"
#include "../projectvertex.h"

#include "myalloc.h"
#include "serialize.h"
#include "parallel.h"
#include "refcount.hh"
#include "refinementrules.h"
#include "walk.h"

namespace ALUGrid
{
  // interface class for projecting vertices for boundary adjustment
  // see ../projectvertex.h
  typedef VertexProjection< ObjectStream, 3, alucoord_t >  ProjectVertex;
  typedef std::shared_ptr< ProjectVertex >                 ProjectVertexPtr;
  typedef std::pair< ProjectVertexPtr , ProjectVertexPtr > ProjectVertexPtrPair;

  // pair of projection and bnd segment index

  typedef enum ALUElementType { triangle=3, quadrilateral=2, tetra=4 , hexa=7 , hexa_periodic , tetra_periodic } grid_t;

  // forward declaration, see ghost_info.h
  class MacroGhostInfoHexa;
  class MacroGhostInfoTetra;


  // enternal parameter for to be used
  // in ALUGrid and that can be changed from outside
  struct ALUGridExternalParameters
  {
    // return precision to be used in standard ostreams
    static size_t& precision()
    {
      static size_t streamPrecision = 16;
      return streamPrecision;
    }

    // return limit of number of ranks to be used
    // with the method MPI_Allgather
    static int &allGatherMaxSize ()
    {
      // default is to use allgather
      static int rankLimit = std::numeric_limits< int > ::max();
      return rankLimit;
    }

    // return true if number of cores is less or equal than all gather max size
    template <class MpAccessGlobal>
    static bool useAllGather( const MpAccessGlobal& mpa )
    {
      const int allGatherMaxProcs = allGatherMaxSize();
      alugrid_assert ( allGatherMaxProcs == mpa.gmax( allGatherMaxProcs ) );
      return mpa.psize() <= allGatherMaxProcs;
    }

    typedef std::pair< std::pair< double, double >, int > LoadBalancerParameterType;
    static LoadBalancerParameterType& loadBalanceParameters()
    {
      static LoadBalancerParameterType ldbParam( std::make_pair( 0.0, 1.2 ), -1 );

      // overload parameters from environment variables, if set.
      if( getenv("ALUGRID_LB_METHOD") )
        ldbParam.second = atoi(getenv("ALUGRID_LB_METHOD"));
      if( getenv("ALUGRID_LB_UNDER") )
        ldbParam.first.first = atof( getenv("ALUGRID_LB_UNDER") );
      if( getenv("ALUGRID_LB_OVER") )
        ldbParam.first.second = atof( getenv("ALUGRID_LB_OVER") );

      return ldbParam;
    }

    static void resetLoadBalanceParameters()
    {
      LoadBalancerParameterType& ldbParam = loadBalanceParameters();
      ldbParam.first.first  = 0.0;
      ldbParam.first.second = 1.2;
      ldbParam.second       = -1;
    }

    static void setLoadBalanceParameters( const int method,
                                          const double ldbUnder,
                                          const double ldbOver )
    {
      LoadBalancerParameterType& ldbParam = loadBalanceParameters();
      ldbParam.first.first  = ldbUnder;
      ldbParam.first.second = ldbOver ;
      ldbParam.second       = method ;
    }

    static int verbosityLevel()
    {
      return getenv("ALUGRID_VERBOSITY_LEVEL") ? atoi (getenv("ALUGRID_VERBOSITY_LEVEL")) : 2;
    }
  };

  // linkage pattern map for parallel grid (stored in IndexManagerStorage for convenience)
  typedef std::vector< int > linkagePattern_t;
  typedef std::map< linkagePattern_t, int > linkagePatternMap_t;

  // forward declaration
  class Gitter;

  class IndexManagerStorage
  {
  public:
    // number of different index manager that exists
    enum { numOfIndexManager = 6 };

    enum { IM_Elements = 0, // 0 == elements
           IM_Faces = 1,    // 1 == faces
           IM_Edges = 2,    // 2 == edges
           IM_Vertices = 3, // 3 == vertices
           IM_Bnd = 4,      // 4 == boundary elements
           IM_Internal = 5  // 5 == internal bnds, parallel only
    };

    IndexManagerStorage() : _myGrid( 0 ), _linkagePatterns(), _myrank(-1), _dim(-1)
    {}

    explicit IndexManagerStorage(Gitter * gitter)
      : _myGrid( gitter ), _linkagePatterns(), _myrank(-1), _dim(-1)
    {}

    void setDimAndGrid( const int dim, Gitter * grid )
    {
      _myGrid = grid;
      for(int i=0; i<numOfIndexManager; ++i)
      {
        _indexmanager[i].setDimension( dim );
      }
    }

    Gitter* myGrid()
    {
      alugrid_assert ( _myGrid );
      return _myGrid;
    }

    const Gitter* myGrid() const
    {
      alugrid_assert ( _myGrid );
      return _myGrid;
    }

    IndexManagerType& get(const int codim)
    {
      alugrid_assert ( codim >= 0 );
      alugrid_assert ( codim < numOfIndexManager );
      return _indexmanager[ codim ];
    }

    // return index
    int getIndex( const int codim ) { return get( codim ).getIndex(); }

    // return MPI rank info
    int myrank () const
    {
      alugrid_assert ( _myrank >= 0 );
      return _myrank;
    }

    // set rank to given value
    void setRank( const int rank ) { _myrank = rank ; }

    void compress()
    {
      for(int i=0; i<numOfIndexManager; ++i)
      {
        _indexmanager[i].compress();
      }
    }

    // return grid's dimension (ie 2 or 3)
    int dimension() const
    {
      return _indexmanager[ 0 ].dimension();
    }

    // needed for the parallel linkage storage
    // (moved here to access in vertices without additional memory consumption)
    linkagePatternMap_t& linkagePatterns () { return _linkagePatterns; }
    const linkagePatternMap_t& linkagePatterns () const { return _linkagePatterns; }
  private:
    IndexManagerStorage( const IndexManagerStorage& );

  protected:
    Gitter* _myGrid;

    // this variable is located here, because all the elements in
    // this lists use this objects to get  thier numbers
    // index provider, for every codim one , 4 is for boundary
    IndexManagerType _indexmanager[ numOfIndexManager ];

    // need for parallel linkage of partitions
    linkagePatternMap_t _linkagePatterns;

    // MPI rank
    int _myrank ;
    int _dim;
  };
  typedef IndexManagerStorage IndexManagerStorageType;


  // EmptyIterator is an iterator of an empty set
  // for some default values
  template < class A > class EmptyIterator : public IteratorSTI < A >
  {
    EmptyIterator ( const EmptyIterator< A > & ) {}

  public :
    typedef A val_t;
    EmptyIterator () {}
    virtual ~EmptyIterator () {}
    virtual void first ();
    virtual void next ();
    virtual int done () const;
    virtual int size ();
    virtual val_t & item () const;
    virtual IteratorSTI < A > * clone () const;
  };

  // AccessIterator < . > ist eine Schnittstellenschablone, die
  // ausgepr"agt mit vertex, hedge, hface, helement und hbndseg
  // die Schnittstellen des Grobgittercontainers zur Erstellung
  // und Kopie von Iterationsobjekten erzeugt.

  template < class A > class any_has_level;

  template < class A > class AccessIterator
  {
  public :
    IteratorRefcount ref;

    // creates leaf iterators
    virtual IteratorSTI < A > * iterator (const A *) const = 0;

    // creates level iterators
    virtual IteratorSTI < A > * levelIterator (const A * a, const any_has_level < A  > & ) const {  return iterator(a); }

  public :

    // Handle ist ein einfaches Iteratorproxy, das ein abstraktes
    // Iterationsobjekt verwendet, um nach aussen die entsprechenden
    // Schnittstellenmethoden zu implementieren. Dabei ist dem Proxy
    // nur bekannt, wie ein Iterator von der entsprechenden Schnittstelle
    // zu bekommen ist, bzw. wie ein bestehender mit einer Schnittstellen-
    // methode kopiert werden kann.

    class Handle : public IteratorSTI < A > {
    protected:
      AccessIterator < A > * _fac;
      A * _a;
      IteratorSTI < A > * _w;
      // typedef handle
      typedef typename AccessIterator < A >::Handle ThisType;
    public :
      Handle (AccessIterator < A > &);
      Handle (const ThisType &);
      Handle ();
      ~Handle ();
      const Handle & operator = (const ThisType &);
      bool operator == (const ThisType &) const;
      bool operator < (const ThisType &) const;
      void first ();
      void next ();
      int done () const;
      int size ();
      A & item () const;
      virtual IteratorSTI< A > * clone () const;
    protected:
      void removeObj();
      void assign (const ThisType &);
    };
  protected :
    AccessIterator () {}

    virtual ~AccessIterator ()
    {
#ifdef ALUGRIDDEBUG
      if( ref )
        std::cerr << "WARNING: (ignored) Some iterators still exist when removing the corresponding grid." << std::endl;
#endif
    }
  };


  // the Leaf Iterators
  template< class A >
  class LeafIterator;

  // the Level Iterators
  template< class A >
  class LevelIterator;


  class Gitter
  {
  public:
    class helement;
    class hbndseg;

    //////////////////////////////////////////////////////////////////////////////
    //
    //
    //  Interfaces for elements, faces, edges, and vertices for parallel computations
    //
    //
    //////////////////////////////////////////////////////////////////////////////

    class VertexPllXIF
    : public LinkedObjectDefault //, public MacroGridMoverIF
    {
    protected :
      class ElementLinkage
      {
        ElementLinkage( const ElementLinkage& );

        int* _elements ;
      public:
        ElementLinkage() : _elements( 0 ) {}
        ~ElementLinkage()
        {
          if( _elements )
          {
            delete [] _elements ;
            _elements = 0;
          }
        }

        template < class container_t >
        bool insertElementLinkage( const container_t& elements )
        {
          if( ! _elements )
          {
            const int elSize = elements.size();
            _elements = new int[ elSize + 1 ];
            // first element holds the number of elements linked
            _elements[ 0 ] = elSize ;

            typedef typename container_t :: const_iterator iterator ;
            const iterator end = elements.end();
            int idx = 1 ;
            for( iterator it = elements.begin(); it != end; ++ it, ++idx )
            {
              _elements[ idx ] = *it ;
            }
            return true ;
          }

          // std::cout << size() << " " << elements.size() << std::endl;
          alugrid_assert( size() == int(elements.size()) );
          return false ;
        }

        bool inactive() const { return _elements == 0 ; }

        int size () const
        {
          alugrid_assert( _elements );
          return _elements[ 0 ];
        }

        int operator [] ( const int i ) const
        {
          alugrid_assert( _elements );
          alugrid_assert( i < size() );
          return _elements[ i+1 ];
        }
      };

      virtual ~VertexPllXIF () {}
    public :
      virtual bool setLinkage ( const std::vector< int >& ) = 0;
      virtual bool setLinkageSorted ( const std::vector< int >& ) = 0;
      virtual void clearLinkage () = 0;
      virtual int linkagePosition () const = 0 ;

      typedef ElementLinkage ElementLinkage_t ;
      typedef std::set< int > linkageset_t ;
      virtual bool insertLinkedElements( const linkageset_t& ) = 0 ;
      virtual const ElementLinkage_t& linkedElements() const = 0 ;
    };

    class VertexPllXDefault : public VertexPllXIF
    {
    protected :
      virtual ~VertexPllXDefault () {}
    public :
      virtual bool setLinkage ( const std::vector< int >& ) { alugrid_assert (false); abort(); return false; }
      virtual bool setLinkageSorted ( const std::vector< int >& ) { return false; }
      virtual void clearLinkage () { alugrid_assert (false); abort(); }
      virtual int linkagePosition () const { alugrid_assert (false); abort(); return -1; }

      typedef VertexPllXIF :: linkageset_t  linkageset_t ;
      virtual bool insertLinkedElements( const linkageset_t& ) { alugrid_assert (false); abort(); return false; }

      typedef VertexPllXIF :: ElementLinkage_t ElementLinkage_t;
      virtual const ElementLinkage_t& linkedElements() const { alugrid_assert (false); abort(); return *((ElementLinkage_t * ) 0 ); }
    };


    class EdgePllXIF : public RefineableObjectDefault //, public LinkedObject, public MacroGridMoverIF
    {
      protected :
        virtual ~EdgePllXIF () {}
      public :
        // my identifier class
        typedef class Key2SLZ identifier_t;

        virtual bool lockAndTry () = 0;
        virtual bool unlockAndResume (bool) = 0;
        virtual bool lockedAgainstCoarsening () const = 0;

        // only for compatibility for Dune release 2.0 (to be removed later)
        EdgePllXIF& accessPllX () { return *this; }
        const EdgePllXIF& accessPllX () const { return *this; }
    };

    // default implementation
    class EdgePllXDefault : public EdgePllXIF
    {
      protected :
        virtual ~EdgePllXDefault () {}
      public :
        virtual bool lockAndTry () { alugrid_assert (false);abort(); return false; }
        virtual bool unlockAndResume (bool) { alugrid_assert (false);abort(); return false; }
        virtual bool lockedAgainstCoarsening () const { alugrid_assert (false);abort(); return false; }
    };

    //  hasFace needs to be one of the basic classes
    //  such that hbndseg and helement can derive from it

    // forward declaration
    class ElementPllXIF;

    // hasFace interface for connections between elements and boundaries.
    class hasFace : public MacroGridMoverDefault
    {
    public :
      // see refinements.h
      typedef RefinementRules::Hface3Rule  Hface3Rule;
      typedef RefinementRules::Hface4Rule  Hface4Rule;

      // provide both methods, each method is only
      // overloaded once in the corresponding derived class
      virtual bool refineBalance (Hface3Rule, int) { abort(); return false; }
      virtual bool refineBalance (Hface4Rule, int) { abort(); return false; }
      virtual bool bndNotifyCoarsen () { abort(); return false; }

      // returns true, if underlying object is real (default impl)
      virtual bool isRealObject () const { return true; }

      virtual int moveTo () const { abort(); return -1; }
    protected :
      hasFace () {}
      virtual ~hasFace () {}
      // provide both methods, each method is only
      // overloaded once in the corresponding derived class
      bool bndNotifyBalance (Hface3Rule, int) { return true; }
      bool bndNotifyBalance (Hface4Rule, int) { return true; }

      typedef ParallelException   stiExtender_t;

    public:
      // default implementation does nothing
      virtual void setLoadBalanceVertexIndex( const int ) {}
      virtual void setMaster( const int ) {}
      // return unique macro graph vertex index (default returns -7)
      virtual int ldbVertexIndex () const { return -7; }
      virtual int master () const { return -1; }

      virtual bool isboundary() const { return false; }
      virtual bool isperiodic() const { return false; }
      virtual int nbLevel() const { abort(); return -1; }
      virtual int nbLeaf() const  { abort(); return -1; }

      // returns true if a vertex projection is set
      virtual bool hasVertexProjection () const { abort(); return false; }

      virtual ElementPllXIF& accessPllX ()
      {
        std::cerr << "ERROR: hasFace::accessPllX has not been overloaded." << std::endl;
        abort();
        throw stiExtender_t::AccessPllException();
      }

      virtual const ElementPllXIF& accessPllX () const
      {
        std::cerr << "ERROR: hasFace::accessPllX has not been overloaded." << std::endl;
        abort();
        throw stiExtender_t::AccessPllException();
      }

      virtual void attachElement2( const int /* destination */, const int /* face */ ) { abort(); }

      // default implementation does nothing
      // this method is overloaded for parallel periodic macro elements
      virtual void attachPeriodic( const int /* destination */ ) {}

      // return ldbVertexIndex (default is -1), overloaded in Tetra and Hexa
      virtual int firstLdbVertexIndex() const { return -1; }
      // return ldbVertexIndex, overloaded in TetraPllMacro and HexaPllMacro
      virtual int otherLdbVertexIndex( const int /* faceIndex */ ) const { return firstLdbVertexIndex(); }
    };

    class vertex ;

    // type of ElementPllXIF_t is ElementPllXIF, see parallel.h
    class ElementPllXIF
    : public hasFace
    {
      protected :
        virtual ~ElementPllXIF () {}
      public :
        typedef std::map< vertex*, std::set< int > > vertexelementlinkage_t;

        typedef std::pair< ElementPllXIF *, int > accesspair_t;
        typedef std::pair< const ElementPllXIF *, int > constaccesspair_t;
        virtual void detachPllXFromMacro () {}

        // default implementation for accessInnerPllX and accessOuterPllX
        virtual accesspair_t accessOuterPllX (const accesspair_t &x, int) { return x; }
        virtual constaccesspair_t accessOuterPllX (const constaccesspair_t &x, int) const { return x;}
        virtual accesspair_t accessInnerPllX (const accesspair_t&, int f) { return accesspair_t( this , f ); }
        virtual constaccesspair_t accessInnerPllX (const constaccesspair_t &, int f) const { return constaccesspair_t( this , f ); }

        virtual void computeVertexLinkage( vertexelementlinkage_t& ) { alugrid_assert (false); abort(); }
      public :
        typedef std::pair< helement *, int > ghostpair_t;

        virtual ghostpair_t getGhost ()
        {
          std::cerr << "ERROR: Method getGhost of Interface class should not be used." << std::endl;
          abort();
          return ghostpair_t( (helement*)0 , -1 );
        }

        virtual int ghostLevel () const
        {
          std::cerr << "ERROR: Method ghostLevel of Interface class should not be used." << std::endl;
          abort();
          return 0;
        }

        virtual bool ghostLeaf () const
        {
          std::cerr << "ERROR: Method ghostLeaf of Interface class should not be used." << std::endl;
          abort();
          return 0;
        }

        virtual void getAttachedElement ( std::pair< helement *, hbndseg * > &p )
        {
          std::cerr << "Overload method in the classes file:" << __FILE__ << " line:" << __LINE__ << std::endl;
          abort();
          p.first  = 0;
          p.second = 0;
        }

        virtual void writeStaticState (ObjectStream &, int) const
        { alugrid_assert (false);abort(); }
        virtual void readStaticState (ObjectStream &, int)
        { alugrid_assert (false);abort(); }

        virtual void writeDynamicState (ObjectStream &, int) const
        { alugrid_assert (false);abort(); }
        virtual void readDynamicState (ObjectStream &, int)
        { alugrid_assert (false);abort(); }

        virtual void VertexData2os(ObjectStream &, GatherScatterType &, int)
        { alugrid_assert (false);abort(); }
        virtual void EdgeData2os  (ObjectStream &, GatherScatterType &, int)
        { alugrid_assert (false);abort(); }
        virtual void FaceData2os  (ObjectStream &, GatherScatterType &, int)
        { alugrid_assert (false);abort(); }
        virtual void writeElementData (ObjectStream &, GatherScatterType &)
        { alugrid_assert (false);abort(); }
        virtual void writeDynamicState(ObjectStream &, GatherScatterType &) const
        { alugrid_assert (false);abort(); }
        virtual void readDynamicState (ObjectStream &, GatherScatterType &)
        { alugrid_assert (false);abort(); }

        // pack as ghost, default does nothing but macro elements are pack as
        // ghosts
        virtual void packAsGhost(ObjectStream &,int) const {}

        // unpack as ghost data and insert ghost cell, default does nothing
        virtual void insertGhostCell(ObjectStream &,int) {}

      public :
        virtual bool ldbUpdateGraphVertex ( LoadBalancer::DataBase &, GatherScatter * )
        { alugrid_assert (false);abort(); return false;  }
      public :
        virtual void packAsBnd (int,int,ObjectStream &, const bool) const
        { alugrid_assert (false);abort(); }
        virtual bool erasable () const
        { alugrid_assert (false);abort(); return false;  }
      public :
        virtual void getRefinementRequest (ObjectStream &)
        { alugrid_assert (false);abort(); }
        virtual bool setRefinementRequest (ObjectStream &)
        { alugrid_assert (false);abort(); return false;  }
      public :
        virtual bool lockAndTry ()
        { alugrid_assert (false);abort(); return false;  }
        virtual bool unlockAndResume (bool)
        { alugrid_assert (false);abort(); return false;  }
    };


    class FacePllXIF : public LinkedObjectDefault //, public MacroGridMoverIF
    {
      protected :
        virtual ~FacePllXIF () {}
      public :
        // my identifier class
        typedef class Key3SLZ identifier_t;

        virtual std::vector< int > checkParallelConnectivity () const = 0;
        virtual std::pair< ElementPllXIF *, int > accessOuterPllX () = 0;
        virtual std::pair< const ElementPllXIF *, int > accessOuterPllX () const = 0;
        virtual std::pair< ElementPllXIF *, int > accessInnerPllX () = 0;
        virtual std::pair< const ElementPllXIF *, int > accessInnerPllX () const = 0;

      public :
        virtual bool ldbUpdateGraphEdge (LoadBalancer::DataBase &, const bool ) = 0;

        // only for compatibility for Dune release 2.0 (to be removed later)
        FacePllXIF & accessPllX () { return *this; }
        const FacePllXIF & accessPllX () const { return *this; }
    };

    // default implementation (should not be called)
    class FacePllXDefault : public FacePllXIF
    {
      protected :
        virtual ~FacePllXDefault () {}
      public :
        virtual std::vector< int > checkParallelConnectivity () const { alugrid_assert ( false ); abort(); return std::vector< int >(); }
        virtual std::pair< ElementPllXIF *, int > accessOuterPllX () { alugrid_assert ( false ); abort(); return std::pair< ElementPllXIF *, int > ( (ElementPllXIF *) 0, -1); }
        virtual std::pair< const ElementPllXIF *, int > accessOuterPllX () const  { alugrid_assert ( false); abort(); return std::pair< ElementPllXIF *, int > ( (ElementPllXIF *) 0, -1); }
        virtual std::pair< ElementPllXIF *, int > accessInnerPllX ()  { alugrid_assert ( false); abort(); return std::pair< ElementPllXIF *, int > ( (ElementPllXIF *) 0, -1); }
        virtual std::pair< const ElementPllXIF *, int > accessInnerPllX () const { alugrid_assert ( false); abort(); return std::pair< ElementPllXIF *, int > ( (ElementPllXIF *) 0, -1); }

        virtual bool ldbUpdateGraphEdge (LoadBalancer::DataBase &, const bool ) { alugrid_assert (false);abort(); return false; }
    };

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Parallel If extension
    //
    /////////////////////////////////////////////////////////////////////////////

    class Parallel
    {
    public:
      typedef ParallelException::AccessPllException  AccessPllException;

      class VertexIF
      : public VertexPllXDefault
      {
      public:
        virtual ~VertexIF () {}
        typedef class Key1SLZ identifier_t;
        virtual VertexPllXIF & accessPllX ()
        {
          alugrid_assert ((abort (), (std::cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << std::endl)));
          throw AccessPllException ();
        }
        virtual const VertexPllXIF & accessPllX () const
        {
          alugrid_assert ((abort (), (std::cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << std::endl)));
          throw AccessPllException ();
        }
        virtual void detachPllXFromMacro ()
        {
          alugrid_assert ((abort (), (std::cerr << "  FEHLER in " << __FILE__ << " " << __LINE__ << std::endl)));
          throw AccessPllException ();
        }
      };
    };

  public:
    IteratorRefcount ref;
    static bool debugOption (int = 0);

    typedef Parallel stiExtender_t;  // parallel.h

    // Nachfolgend sind die Iterationsschnittstellen der Knoten,
    // Kanten, Fl"achen, Elemente und Randelemente definiert.
    class DuneIndexProvider
    {
    public:
      enum { interior = 0 , border = 211 , ghost = 222 };

      enum Flag
      {
        // set if index is copy from outside and should not be freed
        flagCopy = 0,
        // set if object is locked against coarsening
        flagLock = 1,
        // non-affine geometry mapping
        flagNonAffine = 2,
        // coarsening of edges (bisection refinement only)
        flagNoCoarsen = 3,
        // if set item belongs to 2d grid (same as edge coarsen which is only set for
        // edges, whereas flagIs2d is only set for elements and faces)
        flagIs2d = 4,
        // if set this indicates that the starting type of a tetrahedron
        // is type 1 otherwise type 0, type 2 not supported yet
        flagType0 = 5,
        // together with flagType0 determines type by simple
        // addition
        flagType1 = 6
      };

    protected:
      // internal index of item
      int _idx;

      // boundary id, zero for internal items,
      // otherwise > 0 (but always positive)
      typedef unsigned char bndid_t;
      bndid_t _bndid;

      // reference counter of leaf elements holding pointer to this item
      typedef unsigned char leafref_t;
      leafref_t _leafref;

      unsigned char _flags;

      // store refcount here to fill up the 8 byte of mem
      Refcount ref;

      // constructor
      DuneIndexProvider () :
        _idx(-1),
        _bndid(interior),
        _leafref(0),
        _flags( 0 )
      {
      }

    public:
      void set ( const Flag &flag )
      {
        _flags |= (1 << flag);
      }

      void unset ( const Flag &flag )
      {
        _flags &= ~(1 << flag);
      }

      bool isSet ( const Flag &flag ) const
      {
        return (_flags & (1 << flag));
      }

      // write index to stream (i.e. ostream or ObjectStream)
      template <class ostream_t>
      void doBackupIndex( ostream_t& os ) const
      {
        // write my index
        os.write( ((const char *) & _idx ), sizeof(int) );
      }

      // write index to stream (i.e. ostream or ObjectStream)
      template <class ostream_t>
      void doBackupFlags( ostream_t& os ) const
      {
        // write flags
        os.put( _flags );
      }

      // read index from stream (i.e. istream or ObjectStream)
      template <class istream_t>
      void doRestoreFlags( istream_t& is )
      {
        // read flags
        _flags = is.get();
      }

      // read index from stream (i.e. istream or ObjectStream)
      template <class istream_t>
      void doRestoreIndex( istream_t& is,
                           RestoreInfo& restoreInfo,
                           const int codim )
      {
        // read my index
        is.read( ((char *) & _idx ), sizeof(int) );

        // change the byte order if necessary
        if( restoreInfo.toggleByteOrder() )
          restoreInfo.changeByteOrder( (char *) & _idx, sizeof(int) );

        // make sure sizes match
        alugrid_assert ( _idx < (int) restoreInfo( codim ).size() );
        // make index to be not a hole
        restoreInfo( codim )[_idx] = false;
      }

      // backupIndexErr message
      void backupIndexErr () const
      {
        std::cerr << "ERROR: DuneIndexProvider::backupIndex implemenation should be in derived class." << std::endl;
        abort();
      }
      // restoreIndexErr message
      void restoreIndexErr () const
      {
        std::cerr << "ERROR: DuneIndexProvider::restoreIndex implemenation should be in derived class." << std::endl;
        abort();
      }

      // return index of item
      int getIndex () const
      {
        alugrid_assert ( _idx >= 0);
        return _idx;
      }
      // set index of item
      void setIndex ( const int index )
      {
        alugrid_assert ( index >= 0 );
        _idx = index;
      }

      void freeIndex ( IndexManagerType & im )
      {
        if( !isSet( flagCopy ) )
        {
          alugrid_assert ( _idx >= 0 );
          im.freeIndex(_idx);
        }
      }

      //for the ghost helements, set index from outside
      void setIndex ( IndexManagerType & im, const int index )
      {
        // free old index
        freeIndex(im);
        // set given index
        setIndex(index);
        // now it's a copy
        set( flagCopy );
      }

      //for the ghost helements, set index from outside
      void setIndexAnd2dFlag( IndexManagerType & im )
      {
        // get index and set
        setIndex( im.getIndex() );

        // check if we have to set the 2d flag
        if( im.dimension() == 2 )
          set2dFlag();
      }

      //for the ghost helements, set index from outside
      void resetGhostIndex( IndexManagerType & im )
      {
        // if already copy then do nothing
        if( !isSet( flagCopy ) && isGhost() )
        {
          // only call this method on ghosts
          // set new index
          setIndex( im.getIndex() );
        }
      }

      //for defining leaf entities in dune notation:]
      void addleaf()
      {
        // check upper limit of unsigned char
        ++_leafref;
      }

      // decrease reference counter by one
      void removeleaf()
      {
        --_leafref;
      }

      // returns true, if item is leaf item
      bool isLeafEntity() const {
        return ( _leafref > 0 );
      }
      // return actual leaf ref counter
      leafref_t leafRefCount() const {
        return _leafref;
      }

      // return bnd id
      bndid_t bndId() const { return _bndid; }

      // set bnd id, id is only set if id is larger then actual id
      void setBndId (const bndid_t id)
      {
        if( id > _bndid ) _bndid = id;
      }

      // set bnd id, id is overwritten in any case
      void setGhostBndId (const bndid_t id)
      {
        _bndid = id;
      }

      // returns trus, if item is interior item (not ghost or border)
      bool isInterior () const
      {
        // interior is also external boundary which is not 0
        return ( ! isGhost()  &&  ! isBorder() );
      }

      // returns true if item is ghost item
      bool isGhost () const
      {
        return (bndId() == ghost);
      }

      // returns trus, if item is border item
      bool isBorder () const
      {
        return (bndId() == border);
      }

      void setNonAffineGeometry()
      {
        // now it's a non-affine goemetry
        set( flagNonAffine );
      }

      // return true if geometry mapping is affine
      bool affineGeometry() const
      {
        return ! isSet( flagNonAffine );
      }

      // don't allow edge to be coarsened
      void disableEdgeCoarsen()
      {
        set( flagNoCoarsen );
      }

      // reset coarsening flag
      void resetCoarsenFlag()
      {
        unset( flagNoCoarsen );
      }

      // return true if edge should not be coarsened
      bool noCoarsen () const
      {
        return isSet( flagNoCoarsen );
      }

      // set 2d flag indicating that item belong to a 2d grid
      void set2dFlag() { set( flagIs2d ); }

      // reset 2d flag
      void reset2dFlag() { unset ( flagIs2d ); }

      // return true if item belongs to a 2d grid
      bool is2d() const { return isSet( flagIs2d ); }

      // indicate that vertex is a fake vertex (ie vertex in a 2d grid that is not used)
      void setFlagFakeVertex() { set( flagNoCoarsen ); }

      // return true if vertex is a fake vertex (ie vertex in a 2d grid that is not used)
      bool isFakeVertex () const { return isSet( flagNoCoarsen ); }

      // indicate that vertex coordinates were changed by projection
      void setVertexWasProjected() { set( flagNonAffine ); }

      // return true if vertex coordinates were changed by projection
      bool wasProjected () const { return isSet( flagNonAffine ); }

      // set flag that indicates the initial element type
      void setSimplexTypeFlagZero () { set( flagType0 ); }

      // set flag that indicates  the initial element type
      void setSimplexTypeFlagOne () { set( flagType1 ); }

      // return macro type
      unsigned char macroSimplexTypeFlag () const { return static_cast<unsigned char > (isSet( flagType1 ) ? 2 : isSet(flagType0) ? 1 : 0 ) ; }
    };

  public :
    class vertex : public stiExtender_t::VertexIF
                 , public DuneIndexProvider
    {
    protected :
      vertex () {}
      virtual ~vertex () {}
    public :
      virtual int ident () const = 0;
      virtual int level () const = 0;

      // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
      virtual void project(const ProjectVertex &pv) = 0;

      // Extrainteger, damit die Element zu Vertex Zuordnug klappt,
      // wenn die Daten zur Visualisierung mit GRAPE rausgeschrieben
      // werden sollen:

      // backup and restore index of vertices, should be overloaded in
      // derived classes, because some need to go down the hierarchiy
      virtual void backupIndex  (std::ostream & ) const { backupIndexErr(); }
      virtual void restoreIndex (std::istream &, RestoreInfo& ) { restoreIndexErr(); }

      // same methods for ObjectStream
      virtual void backupIndex  ( ObjectStream& ) const { backupIndexErr(); }
      virtual void restoreIndex ( ObjectStream&, RestoreInfo& ) { restoreIndexErr(); }

    };

    class hedge : public EdgePllXDefault ,
                  public DuneIndexProvider
    {
    protected :
      hedge () {}
      virtual ~hedge () {}
    public :
      virtual hedge * down () = 0;
      virtual const hedge * down () const = 0;
      virtual hedge  * next () = 0;
      virtual const hedge  * next () const = 0;
      virtual vertex * innerVertex () = 0;
      virtual const vertex * innerVertex () const = 0;
      virtual int level () const = 0;
      virtual int nChild () const = 0;
       int leaf () const;
    public :
      virtual bool coarse () = 0;
      virtual void backup (std::ostream &) const = 0;
      virtual void restore (std::istream &) = 0;

      virtual void backup (ObjectStream &) const = 0;
      virtual void restore (ObjectStream &) = 0;

      // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
      virtual void projectInnerVertex(const ProjectVertex &pv) = 0;

      // backup and restore index of vertices, should be overloaded in
      // derived classes, because some need to go down the hierarchiy
      virtual void backupIndex  (std::ostream & ) const { backupIndexErr(); }
      virtual void restoreIndex (std::istream &, RestoreInfo& ) { restoreIndexErr(); }

      virtual void backupIndex  (ObjectStream& ) const { backupIndexErr(); }
      virtual void restoreIndex (ObjectStream&, RestoreInfo& ) { restoreIndexErr(); }

      // return true if an edge can be coarsened
      virtual bool canCoarsen() const = 0;
    };

    class hface : public FacePllXDefault,
                  public DuneIndexProvider
    {
    protected :
      hface () {}
      virtual ~hface () {}
    public :
      virtual hface * down () = 0;
      virtual const hface * down () const = 0;
      virtual hface * next () = 0;
      virtual const hface * next () const = 0;
      virtual vertex * innerVertex () = 0;
      virtual const vertex * innerVertex () const = 0;
      virtual hedge  * innerHedge () = 0;
      virtual const hedge  * innerHedge () const = 0;
      virtual int level () const = 0;
      virtual int nChild () const = 0;
      int leaf () const;
    public :
      virtual bool coarse () = 0;
      virtual void backup (std::ostream &) const = 0;
      virtual void restore (std::istream &) = 0;

      virtual void backup (ObjectStream &) const = 0;
      virtual void restore (ObjectStream &) = 0;

      // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
      virtual void projectVertex(const ProjectVertex &pv) = 0;

      // returns true if element conected to face is leaf
      virtual bool isInteriorLeaf() const = 0;

      // backup and restore index of vertices, should be overloaded in
      // derived classes, because some need to go down the hierarchiy
      virtual void backupIndex  (std::ostream & ) const { backupIndexErr(); }
      virtual void restoreIndex (std::istream &, RestoreInfo& ) { restoreIndexErr(); }

      virtual void backupIndex  (ObjectStream& ) const { backupIndexErr(); }
      virtual void restoreIndex (ObjectStream&, RestoreInfo& ) { restoreIndexErr(); }

    };


    // class with all extensions for helement
    class Dune_helement : public DuneIndexProvider
    {
    protected:
      // abuse ref to refined tag
      using DuneIndexProvider::ref;
      Dune_helement ()
      {
        // mark as new element by increasing reference counter
        ++ ref;
      }
    public:
      // reset the _refinedTag to false
      void resetRefinedTag();
      // true if element was refined this adaptation step
      bool hasBeenRefined () const;
    };


    struct AdaptRestrictProlong
    {
      virtual ~AdaptRestrictProlong () {}
      virtual int preCoarsening (helement & elem )   = 0;
      virtual int postRefinement (helement  & elem ) = 0;
      virtual int preCoarsening (hbndseg & bnd )     = 0;
      virtual int postRefinement (hbndseg & bnd )    = 0;
    };
    typedef AdaptRestrictProlong AdaptRestrictProlongType;

    // --helement
    class helement : public ElementPllXIF
                   , public Dune_helement
    {
    protected :
      helement () {}
      virtual ~helement () {}
    public :
      //testweise us
      virtual helement * up () = 0;
      virtual const helement * up () const = 0;
      virtual void os2VertexData(ObjectStream &, GatherScatterType &, int) { alugrid_assert (false); abort();}
      virtual void os2EdgeData  (ObjectStream &, GatherScatterType &, int) { alugrid_assert (false); abort();}
      virtual void os2FaceData  (ObjectStream &, GatherScatterType &, int) { alugrid_assert (false); abort();}

      virtual void VertexData2os(ObjectStream &, GatherScatterType &, int) { alugrid_assert (false); abort();}
      virtual void EdgeData2os(ObjectStream &, GatherScatterType &, int) { alugrid_assert (false); abort(); }
      virtual void FaceData2os(ObjectStream &, GatherScatterType &, int) { alugrid_assert (false); abort(); }
      //us
      virtual helement * down () = 0;
      virtual const helement * down () const = 0;
      virtual helement * next () = 0;
      virtual const helement * next () const = 0;
      virtual vertex * innerVertex () = 0;
      virtual const vertex * innerVertex () const = 0;
      virtual hedge * innerHedge () = 0;
      virtual const hedge * innerHedge () const = 0;
      virtual hface * innerHface () = 0;
      virtual const hface * innerHface () const = 0;
      // return level of element, implemented in Top classes
      virtual int level () const = 0;
      // return number of child
      virtual int nChild () const = 0;

      virtual int nFaces() const = 0;
      virtual int nEdges() const = 0;

      // return index of boundary segment
      virtual int segmentId (const int) const { return -1; }

      // return if at least one of the vertex's coordinates was changed through projection
      virtual bool vertexWasProjected () const { return false; }

      // return true if further refinement is needed to create conforming closure
      virtual bool markForConformingClosure () = 0;
      // mark edges for allow or disallow coarsening
      virtual void markEdgeCoarsening () = 0;

      // mark element for using red rule
      virtual int tagForGlobalRefinement () = 0;
      // mark element for coarsening
      virtual int tagForGlobalCoarsening () = 0;
      // set marker of element to nosplit
      virtual int resetRefinementRequest () = 0;
      virtual int tagForBallRefinement (const alucoord_t (&)[3],double,int) = 0;
      virtual int test () const = 0;
      int leaf () const;

      // return element type and orientation
      virtual SimplexTypeFlag simplexTypeFlag() const { return SimplexTypeFlag(); }

      //! default implementation of ldbVertexIndex calls this method on father
      //! the assumption here is, that this method is overloaded appropriately
      //! on the corresponding parallel macro elements
      virtual int ldbVertexIndex () const
      {
        const helement* father = up();
        if( father )
        {
          // call method on father
          return father->ldbVertexIndex ();
        }
        else
        {
          // default implementation for serial elements is simply getIndex
          // since this corresponds to the insertion index also
          return this->getIndex();
        }
      }
      virtual int master () const
      {
        return (alugrid_assert (0),-1);
      }

      virtual void changeVertexCoordinates( const int, const std::array< std::array<alucoord_t,3>, 8 >&, const double ) { std::abort(); }

      using ElementPllXIF::writeDynamicState;
      using ElementPllXIF::readDynamicState;

      virtual void writeDynamicState (ObjectStream &os, int i) const
      {
        alugrid_assert ( up() != 0 );
        up()->writeDynamicState(os,i);
      }
      virtual void readDynamicState (ObjectStream &os, int i)
      {
        alugrid_assert ( up() != 0 );
        up()->readDynamicState(os,i);
      }
      virtual void writeStaticState (ObjectStream &os, int i) const
      {
        alugrid_assert ( up() != 0 );
        up()->writeStaticState(os,i);
      }
      virtual void readStaticState (ObjectStream &os, int i)
      {
        alugrid_assert ( up() != 0 );
        up()->readStaticState(os,i);
      }
      virtual void packAsBnd (int a,int b,ObjectStream &os, const bool ghostCellsEnabled ) const
      {
        alugrid_assert ( up() != 0 );
        up()->packAsBnd(a,b,os, ghostCellsEnabled);
      }
      virtual bool erasable () const
      {
        alugrid_assert ( up() != 0 );
        return up()->erasable();
      }

      virtual double volume () const { alugrid_assert (false); abort(); return 0.0; } //= 0;
      virtual void setIndicesAndBndId (const hface & , int ) { alugrid_assert (false); abort(); }
      virtual void resetGhostIndices() = 0;
    public :
      virtual bool refine () = 0;
      virtual bool coarse () = 0;

      virtual int  backup  (std::ostream &) const = 0;
      virtual void restore (std::istream &) = 0;

      virtual int  backup  (ObjectStream &) const = 0;
      virtual void restore (ObjectStream &) = 0;

      // backup and restore index of vertices, should be overloaded in
      // derived classes, because some need to go down the hierarchiy
      virtual void backupIndex  (std::ostream &) const { backupIndexErr(); }
      virtual void restoreIndex (std::istream &, RestoreInfo& ) { restoreIndexErr(); }

      virtual void backupIndex  (ObjectStream &) const { backupIndexErr(); }
      virtual void restoreIndex (ObjectStream &, RestoreInfo& ) { restoreIndexErr(); }

      // also in hasFace
      virtual int moveTo () const { return -1; }

    public:
      virtual grid_t type() const = 0;
    };

    // this little helper class stored information for the splitGhost
    // method but is only needed for the parallel case
    class GhostChildrenInfo
    {
      helement *(_ghchl)[4];
      int _gFace[4];
      public:
        GhostChildrenInfo()
        {
          _ghchl[0] = _ghchl[1] = _ghchl[2] = _ghchl[3] = 0;
          _gFace[0] = _gFace[1] = _gFace[2] = _gFace[3] = -1;
        }

        helement * child(int i) const
        {
          alugrid_assert ( i >= 0 && i < 4 );
          return _ghchl[i];
        }

        int face(int i) const
        {
          alugrid_assert ( i >= 0 && i < 4 );
          return _gFace[i];
        }

        void setGhostPair(const std::pair< helement * , int > & g, int i)
        {
          alugrid_assert ( i >= 0 && i < 4 );
          alugrid_assert ( g.first );
          _ghchl[i] = g.first;
          alugrid_assert ( g.second >=0 );
          _gFace[i] = g.second;
        }
    };

    // --hbndseg
    class hbndseg  :
      public DuneIndexProvider,
      public hasFace
    {
    protected :
      hbndseg () {}
    public :
      virtual ~hbndseg () {}

      typedef enum {
        // interior = 0 , border = 211 , ghost = 222
        none = DuneIndexProvider::interior,        // also the value of interior items
        closure = DuneIndexProvider::border,       // also the value of border items
        ghost_closure = DuneIndexProvider::ghost , // also the value of ghost items
        closure_2d = 203, // fake boundary for 2d elements
        periodic   = 254, // periodic boundaries (deprecated)
        undefined  = 255 } bnd_t;

      // returns true if bnd id is in range
      static bool bndRangeCheck (const int pbt)
      {
        const unsigned int bt = std::abs( pbt );
        return ( bt < 255 );
      }

      // returns ranges of boundary type as string
      static std::string validRanges()
      {
        return "+-[0,...,200], [201,...,253] for internal use only!";
      }

      virtual bnd_t bndtype () const = 0;

      // return index of boundary segment
      virtual int   segmentId () const = 0;

      // for dune
      virtual hbndseg * up () = 0;
      virtual const hbndseg * up () const = 0;

      virtual hbndseg * down () = 0;
      virtual const hbndseg * down () const = 0;
      virtual hbndseg * next () = 0;
      virtual const hbndseg * next () const = 0;
      virtual int level () const = 0;
      virtual int nChild () const = 0;
       int leaf () const;

      // for dune
      virtual int ghostLevel () const = 0;
      virtual bool ghostLeaf () const = 0;

      // getGhost returns pointer to ghost, which might be 0 in case that
      // bndseg is external bnd seg,
      // the int is -1 by default, or the internal ghostFace number (
      // getGhostFaceNumber) when ghost is non-zero
      virtual const std::pair< helement * ,int> & getGhost () const = 0;

      // overload this method because this could be called on
      // non-macro internal boundaries in case of the bisection
      // refinement, in this case we direct to the father
      virtual int ldbVertexIndex () const
      {
        const hbndseg* father = up();
        alugrid_assert ( father );
        return father->ldbVertexIndex ();
      }
      virtual int master () const
      {
        return 0;
      }

      //! store shared pointer to boundary projection object (pass empty pointer to remove)
      virtual void setBoundaryProjection( const ProjectVertexPtr& pvPtr ) = 0;

      //! return type of projection if set, otherwise none
      virtual typename ProjectVertex::ProjectionType projectionType() const
      {
        return ProjectVertex::none;
      }

    protected:
      // if ghost element exists, then ghost is splitted, when bnd is splitted
      // info will be filled with the new ghost cells and local face to the
      // internal boundary, default just does nothing
      // implementation see gitter_{tetra,hexa}_top_pll.h
      virtual void splitGhost ( GhostChildrenInfo&  ) {}

      // if ghost element exists, then ghost is coarsened, when bnd is coarsened
      virtual void coarseGhost () {}
      virtual void setGhost ( const std::pair< helement * , int > & ) {}
    public :
      virtual void restoreFollowFace () = 0;
      virtual void attachleafs() { abort(); }
      virtual void detachleafs() { abort(); }
      virtual void projectGhostElement( const int, const std::array< std::array<alucoord_t,3>, 8 >&, const double ) { std::abort(); }
    };


    // tag element for periodic elements which is basically the i
    // same as helement (for historical resons)
    class hperiodic : public helement
    {
    public:
      typedef hbndseg::bnd_t  bnd_t;

      // return the ldbVertexIndex of the first element (inside)
      virtual std::pair<int,int> insideLdbVertexIndex() const
      {
        std::abort();
        return std::pair<int,int> (-1,-1);
      }

      virtual bnd_t bndtype ( const int ) const = 0;
    protected :
      hperiodic () {}
      virtual ~hperiodic () {}
    };

  public :
    typedef hbndseg hbndseg_STI;
    typedef helement  helement_STI;
    typedef hperiodic hperiodic_STI;
    typedef hface hface_STI;
    typedef hedge hedge_STI;
    typedef vertex  vertex_STI;
    typedef std::pair< helement_STI * , int > ghostpair_STI;

    // Die Klassen Internal-*- sind nur daf"ur da, aus einem Element, einer
    // Fl"ache oder einer Kante die inneren geometrischen Objekte, die von
    // dort verwaltet werden 'ans Licht zu f"ordern'. Sie werden ben"otigt,
    // um z.B. von einem Iterator mit Element-Item auf einen Teilbaumiterator
    // der inneren Fl"achen "uberzugehen.

    class InternalVertex {
    public :
      typedef vertex_STI val_t;
      val_t & operator () (helement_STI & e) const { return * e.innerVertex (); }
      val_t & operator () (hface_STI & f) const { return * f.innerVertex (); }
      val_t & operator () (hedge_STI & d) const { return * d.innerVertex (); }
      val_t & operator () (vertex_STI & v) const { return v; }
    };
    class InternalEdge {
    public :
      typedef hedge_STI val_t;
      val_t & operator () (helement_STI & e) const { return * e.innerHedge (); }
      val_t & operator () (hface_STI & f) const { return * f.innerHedge (); }
      val_t & operator () (hedge_STI & e) const { return e; }
    };
    class InternalFace {
    public :
      typedef hface_STI val_t;
      val_t & operator () (helement_STI & e) const { return * e.innerHface (); }
      val_t & operator () (hface_STI & f) const { return f; }
    };

  public :

    // Die allgemeinste Schnittstelle zum Grobgittercontainer enth"alt Methoden zum
    // Anfordern und Kopieren von Iterationsobjekten, die aber nur im Sinne ihres
    // abstrakten Interfaces bekannt sind.
    // Das Makrogitterinterface beerbt erst mal die verschiedenen Auspr"agungen f"ur
    // Schnittstellen zu Anforderung von Iterationsobjekten. Alles ganz abstrakt bis hier.

    class Makrogitter : public AccessIterator < vertex_STI >, public AccessIterator < hedge_STI >,
                        public AccessIterator < hface_STI >, public AccessIterator < hbndseg_STI >,
                        public AccessIterator < helement_STI >, public AccessIterator < hperiodic_STI >
    {
    protected :
      virtual ~Makrogitter ();
    public :
      struct MkGitName
      {
        std::string name;
        std::mutex mtx;
        int ptr;
        MkGitName( const std::string& n ) : name( n ),
              ptr(2-ALUGridExternalParameters::verbosityLevel() )
        {}
        template <class T>
        inline void dump( T t )
        {
          {
            std::unique_lock<std::mutex> lck (mtx,std::defer_lock); lck.lock();
            if( ! ptr && ! t ) { std::cerr << std::endl << name; ptr++ ; }
            lck.unlock();
          }
        } ~MkGitName() { if( ptr == 1 ) std::cout << std::endl << name ; }
      };
      static MkGitName _msg;

      virtual void dumpInfo(int i=1) const { _msg.dump( i ); }

      virtual int iterators_attached () const;
      virtual MacroFileHeader dumpMacroGrid (std::ostream &, const MacroFileHeader::Format ) const = 0;

      // return size of used memory of macro gitter
      // (size of lists storing the pointers )
      virtual size_t memUsage () = 0;
    };
  public :
    class Geometric {

      // Innerhalb des Namensraums Geometric sind zuerst die Klassen mit den
      // Verfeinerungsregeln und dann die Geometriesockelklassen VertexGeo,
      // Hedge1, Hface3, Hface4, Tetra, Hexa, Hbndseg3/4
      // sowie die Polygonverbinder (Schnittstellen) hasFace3/4 definiert.

    public :
      class VertexGeo;
      class hedge1;
      class hface4;
      class hface3;
      class Tetra;
      class Hexa;
      class hbndseg3;
      class hbndseg4;

      // see refinementrules.h for the rules
      typedef RefinementRules::Hedge1Rule  Hedge1Rule;
      typedef RefinementRules::Hface3Rule  Hface3Rule;
      typedef RefinementRules::Hface4Rule  Hface4Rule;
      typedef RefinementRules::TetraRule   TetraRule;
      typedef RefinementRules::HexaRule    HexaRule;

      // Die Geometriesockelklassen sind die Grundlage zur Implementierung
      // numerischer Verfahren auf den bestimmten Elementtypen, und erlauben
      // alle Man"over, die "uber die geometrische Information verf"ugen
      // m"ussen, wie z.B. Navigation zur Fl"ache, zu den Kanten und Knoten,
      // aber auch Anforderungen an den Nachbarn.

      // hasFace is implemented in elementif.h because of the
      // inheritance chain with the virtual methods.
      typedef hasFace  hasFace3;
      typedef hasFace  hasFace4;

      // this class is used as default value for the face neighbour
      // the pointer is set in gitter_geo.cc where the null neigbours are
      // initialized. This means that having no neighbour will not result in
      // a segementation falut, but just return some default values
      class hasFaceEmpty : public hasFace
      {
      public :
        typedef hasFaceEmpty ThisType;
        // returning true, means that face is also refined
        bool refineBalance (Hface3Rule,int) { return true; }
        bool refineBalance (Hface4Rule,int) { return true; }
        // true means coarsening allowed
        bool bndNotifyCoarsen () { return true; }

        // return reference to the one instance we need
        static ThisType & instance ()
        {
          static ThisType singleton;
          return singleton;
        }

        // as we have not a real element or boundary here, return false
        bool isRealObject () const { return false; }
        bool isperiodic() const { return false; }

        // return false for vertex projection
        bool hasVertexProjection() const { return false; }

        // default returns some negative value
        int ldbVertexIndex () const { return -5; }
        int master () const { return -1; }
      private:
        hasFaceEmpty () {}
        hasFaceEmpty (const hasFaceEmpty & );

      public:
        // this is counted as boundary to seperate from elements
        bool isboundary() const { return true; }
      };

      typedef class VertexGeo : public vertex_STI, public MyAlloc
      {
      public:
        // VertexGeo is provided for the vertices on lower levels
        VertexGeo (int,double,double,double, VertexGeo & );
        VertexGeo (int,double,double,double, IndexManagerStorageType & im );
        virtual ~VertexGeo ();

        // return coordinates of vertex
        const alucoord_t (& Point () const) [3] { return _c; }

        // set new coordinates (using for projecting ghost elements)
        void setCoordinates( const std::array< alucoord_t, 3 >& coords )
        {
          _c[ 0 ] = coords[ 0 ];
          _c[ 1 ] = coords[ 1 ];
          _c[ 2 ] = coords[ 2 ];
        }

        // return level of vertex
        int level () const { return lvl(); }

        // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
        virtual void project(const ProjectVertex &pv);

        // overload backupIndex and restoreIndex for std::streams
        void backupIndex  (std::ostream & os ) const;
        void restoreIndex (std::istream & is, RestoreInfo& );

        // overload backupIndex and restoreIndex for ObjectStream
        void backupIndex  (ObjectStream & os ) const;
        void restoreIndex (ObjectStream & is, RestoreInfo& );

        int nChild () const { return 0; }

        // return pointer to grid
        Gitter* myGrid() { return _indexManagerStorage.myGrid(); }
        const Gitter* myGrid() const { return _indexManagerStorage.myGrid(); }
        IndexManagerStorageType& indexManagerStorage () { return _indexManagerStorage; }
        const IndexManagerStorageType& indexManagerStorage () const { return _indexManagerStorage; }

      protected:
        IndexManagerType& indexManager() {
          return _indexManagerStorage.get( IndexManagerStorageType::IM_Vertices );
        }

      private :
        // the coordinates of this vertex
        alucoord_t _c [3]; // 24 byte
      protected:
        // index manager
        IndexManagerStorageType & _indexManagerStorage; // 8 byte

        // use the refcount from DuneIndexProvider for level storage (to avoid +8 byte)
        // the level of creation
        unsigned char& lvl() { return DuneIndexProvider::ref._c; }
        unsigned char lvl() const { return DuneIndexProvider::ref._c; }
      public :
        // reference counter (here unsigned int to avoid overflow)
        RefcountUnsignedInt ref;
        //using DuneIndexProvider::ref; // 8 byte from DuneIndexProvider + 8 vtable = 56 byte
      } vertex_GEO;

      typedef class hedge1 : public hedge_STI , public MyAlloc
      {
      protected :
        typedef VertexGeo myvertex_t;
        hedge1 (myvertex_t *,myvertex_t *);
        int postRefinement ();
        int preCoarsening ();
        bool lockedAgainstCoarsening () const;
      public :
        typedef Hedge1Rule myrule_t;
        virtual ~hedge1 ();
        myvertex_t * myvertex (int);
        const myvertex_t * myvertex (int) const;
        virtual myvertex_t * subvertex (int) = 0;
        virtual const myvertex_t * subvertex (int) const = 0;
        virtual hedge1 * subedge (int) = 0;
        virtual const hedge1 * subedge (int) const = 0;
      public :
        virtual myrule_t getrule () const = 0;
        virtual void refineImmediate (myrule_t) = 0;

        virtual bool canCoarsen () const;
      private :
        myvertex_t * v0, * v1; // 16 bytes + 16 (24 with comm buffer) 32 (40)
      public:
        // reference counter
        using DuneIndexProvider::ref;
      } hedge1_GEO;

      typedef class hface3 : public hface_STI, public MyAlloc {
      public :
        typedef hasFace3   myconnect_t;
        typedef Hface3Rule myrule_t;
        typedef myrule_t   balrule_t;
        enum { polygonlength = 3 };
        class face3Neighbour
        {
          myconnect_t *_faceFront;
          myconnect_t *_faceRear;
          signed char _numFront;
          signed char _numRear;
          signed char s [ polygonlength ];  // 12 bytes (moved here to use padding)

          // count number of attchElement calls (front and rear)
          unsigned char _attachedFront; // 1 byte
          unsigned char _attachedRear ; // 1 byte

        public:
          myrule_t _parRule;  // 1 byte

          typedef std::pair< myconnect_t *, int >  neighbour_t;
          typedef std::pair< const myconnect_t *, int > const_neighbour_t;
        protected:
          void setFront ( const neighbour_t &p );
          void setRear  ( const neighbour_t &p );
          void setNextFront ( const neighbour_t &p );
          void setNextRear  ( const neighbour_t &p );
          void setPrevFront ( const neighbour_t &p );
          void setPrevRear  ( const neighbour_t &p );
        private:
          void operator = (const face3Neighbour &);
        public:
          static const std::pair< myconnect_t *, int > null;
          face3Neighbour ();
          void assign (const face3Neighbour &);
          int complete (const face3Neighbour &);
          neighbour_t front ();
          const_neighbour_t front () const;
          neighbour_t rear ();
          const_neighbour_t rear () const;

          // return true if no element is attached to rear
          bool emptyRear () const { return _attachedRear == 0; }
          // return true if no element is attached to frony
          bool emptyFront () const { return _attachedFront == 0; }

          friend class hface3;
        } nb; // <= 24 bytes
      public:
        typedef VertexGeo   myvertex_t;
        typedef hedge1_GEO  myhedge_t;

        typedef myhedge_t   myhedge1_t;
      protected :
        hface3 (myhedge_t *,int, myhedge_t *, int, myhedge_t *, int);
        int postRefinement ();
        int preCoarsening ();
      public :
        virtual ~hface3 ();
        void attachElement (const std::pair< hasFace3 *, int > &,int);
        void detachElement (int);
        void detachElement (int, const std::pair< hasFace3 *, int > &);

        // return true if more than one attachment was made to rear/front
        bool moreAttachments( const int twst ) const
        {
          return twst < 0 ? nb._attachedRear > 1 : nb._attachedFront > 1;
        }
      public :
        int twist (int) const;
        myvertex_t * myvertex (int);
        const myvertex_t * myvertex (int) const;

        myhedge_t * myhedge (int);
        const myhedge_t * myhedge (int) const;
        // deprecated methods
        myhedge_t * myhedge1 ( int i ) { return myhedge( i ); }
        const myhedge_t * myhedge1 ( int i ) const { return myhedge( i ); }

        virtual hface3 * down () = 0;
        virtual const hface3 * down () const = 0;
        virtual hface3 * next () = 0;
        virtual const hface3 * next () const = 0;
        virtual myvertex_t * subvertex (int) = 0;
        virtual const myvertex_t * subvertex (int) const = 0;
        virtual myhedge1_t * subedge (int) = 0;
        virtual const myhedge1_t * subedge (int) const = 0;
        virtual hface3 * subface (int) = 0;
        virtual const hface3 * subface (int) const = 0;
      public :
        virtual myrule_t getrule () const = 0;
        virtual bool refine (myrule_t,int) = 0;
        virtual void refineImmediate (myrule_t) = 0;
      public :
        myrule_t parentRule() const;

        // returns true, if element conected to face is leaf
        virtual bool isInteriorLeaf() const;

      protected :
        myhedge1_t * e [polygonlength]; // 24 bytes
      public:
        // reference counter
        using DuneIndexProvider::ref;

        // return weight in terms of children of this face
        virtual int weight() const
        {
          const int weight =  TreeIterator < const hface_STI,
                                             is_leaf < const hface_STI > > ( this ).size ();
          return weight;
        }
      } hface3_GEO;

      typedef class hface4 : public hface_STI, public MyAlloc {
      public :
        typedef hasFace4  myconnect_t;
        typedef Hface4Rule myrule_t;
        typedef myrule_t   balrule_t;
        enum { polygonlength = 4 };

        class face4Neighbour {
          myconnect_t *_faceFront;
          myconnect_t *_faceRear;
          signed char _numFront;
          signed char _numRear;
          // put here to save memory because of padding
          signed char s [polygonlength];
        public:
          // put here to save memory because of padding
          myrule_t _parRule;

          typedef std::pair< myconnect_t *, int >  neighbour_t;
          typedef std::pair< const myconnect_t *, int > const_neighbour_t;
        private:
          void operator = (const face4Neighbour &);
        public :
          static const std::pair< myconnect_t *, int > null;
          void setFront ( const std::pair< myconnect_t *, int > &p );
          void setRear ( const std::pair< myconnect_t *, int > &p );
          face4Neighbour ();
          void assign (const face4Neighbour &);
          int complete (const face4Neighbour &);
          neighbour_t front ();
          const_neighbour_t front () const;
          neighbour_t rear ();
          const_neighbour_t rear () const;
          friend class hface4;
        } nb; // 24 byte
      public :
        typedef VertexGeo   myvertex_t;
        typedef hedge1_GEO  myhedge_t;

        typedef myhedge_t   myhedge1_t;
      protected :
        hface4 (myhedge_t *, int, myhedge_t *, int, myhedge_t *, int, myhedge_t *, int);
        int postRefinement ();
        int preCoarsening ();
      public :
        virtual ~hface4 ();
        void attachElement (const std::pair< hasFace4 *, int > &,int);
        void detachElement (int);
      public :
        int twist (int) const;
        myvertex_t * myvertex (int);
        const myvertex_t * myvertex (int) const;
        myhedge_t * myhedge (int);
        const myhedge_t * myhedge (int) const;

        // deprecated methods
        myhedge_t * myhedge1 ( int i ) { return myhedge( i ); }
        const myhedge_t * myhedge1 ( int i ) const { return myhedge( i ); }
        virtual hface4 * down () = 0;
        virtual const hface4 * down () const = 0;
        virtual hface4 * next () = 0;
        virtual const hface4 * next () const = 0;
        virtual myvertex_t * subvertex (int) = 0;
        virtual const myvertex_t * subvertex (int) const = 0;
        virtual myhedge_t * subedge (int) = 0;
        virtual const myhedge_t * subedge (int) const = 0;
        virtual hface4 * subface (int) = 0;
        virtual const hface4 * subface (int) const = 0;

        // returns true, if element connected to face is leaf
        virtual bool isInteriorLeaf() const;

      public :
        virtual myrule_t getrule () const = 0;
        virtual bool refine (myrule_t,int) = 0;
        virtual void refineImmediate (myrule_t) = 0;
      public :
        myrule_t parentRule() const;
      private :
        myhedge_t * e [polygonlength]; // polygonlength * 8 = 32

      public:
        // reference counter
        using DuneIndexProvider::ref;

        // return weight in terms of children of this face
        virtual int weight() const
        {
          const int weight = TreeIterator < const hface_STI,
                                            is_leaf < const hface_STI > > ( this ).size ();
          return weight;
        }
      } hface4_GEO; // 56 + 16 = 72

      // Geometriesockelklasse des Tetraeders: Vorsicht der Prototyp der dem
      // Tetraeder zugrunde liegt, hat eine nach links (gegen Uhrzeigersinn)
      // umlaufende Numerierung der Knoten auf den Randfl"achen, wenn aus
      // dem Element herausgeblickt wird. Die Konvention f"ur den Hexaeder
      // ist leider genau umgekehrt. Dies sollte beim Aufbau von Pyramiden
      // und Prismen sorgf"altig bedacht werden.
      // Der Prototyp steht in 'gitter_geo.cc'.

      typedef class Tetra
        : public helement_STI, public MyAlloc
      {
      public :
        typedef VertexGeo  myvertex_t;
        typedef hedge1_GEO myhedge_t;
        typedef hface3_GEO myhface_t;

        typedef myhedge_t myhedge1_t;
        typedef myhface_t myhface3_t;

        typedef TetraRule  myrule_t;
        typedef Hface3Rule balrule_t;

        typedef std::pair< hasFace3 *, int > myneighbour_t;

      protected :
        Tetra (myhface_t *, int, myhface_t *, int,
               myhface_t *, int, myhface_t *, int);
        int postRefinement ();
        int preCoarsening ();
      public :
        using hasFace3    ::accessPllX;
        static const int prototype [4][3];
        static const int edgeMap [6][2];

        static const int edgeTwist [6][3];
        static const int vertexTwist [6][3];

        // returns 3 which is the lenght of the edges not on face number
        static const std::vector< std::vector< int > > _verticesNotOnFace;
        static const std::vector< std::vector< int > > _edgesNotOnFace;
        static const std::vector< std::vector< int > > _facesNotOnFace;

        static std::vector< std::vector< int > > initVerticesNotOnFace ();
        static std::vector< std::vector< int > > initEdgesNotOnFace    ();
        static std::vector< std::vector< int > > initFacesNotOnFace    ();

        static const std::vector< int > &verticesNotOnFace ( int face );
        static const std::vector< int > &edgesNotOnFace ( int face );
        static const std::vector< int > &facesNotOnFace ( int face );

        virtual ~Tetra ();
        myhface_t* myhface (int);
        const myhface_t* myhface (int) const;

        myhface_t* myhface3 ( int i ) { return myhface( i ); } // don't use these methods, use the ones without number
        const myhface_t* myhface3 ( int i ) const { return myhface( i ); } // don't use these methods, use the ones without number

        VertexGeo * myvertex (int);
        const VertexGeo * myvertex (int) const;
        myhedge_t* myhedge(int);
        const  myhedge_t* myhedge(int) const;
        // deprecated methods
        myhedge_t* myhedge1( int i ) { return myhedge( i ); }
        const  myhedge_t* myhedge1( int i ) const { return myhedge( i ); }
        VertexGeo * myvertex (int,int);
        const VertexGeo * myvertex (int,int) const;
        std::pair< hasFace3 *, int > myneighbour (int);
        std::pair< const hasFace3 *, int > myneighbour (int) const;

        // Dune extension
        // return pair, first = pointer to face, second = twist of face
        std::pair< myhface_t*, int > myintersection (int);
        std::pair< const myhface_t*, int > myintersection (int) const;

        virtual int nFaces() const { return 4; }
        virtual int nEdges() const { return 6; }
        int twist (int) const;
        int test () const;
        // returns level of this object
        virtual int nbLevel() const {return level();}
        // returns leaf
        virtual int nbLeaf() const {return leaf();}

        // returns false because only bnd segments have projections
        virtual bool hasVertexProjection () const { return false; }

        // return if at least one of the vertex's coordinates was changed through projection
        virtual bool vertexWasProjected () const
        {
          for( int i=0; i<4; ++i )
          {
            if( myvertex( i )->wasProjected() )
              return true;
          }
          return false;
        }

      public :
        // return the rule that lead to this tetra
        virtual myrule_t getrule () const = 0;

        // return rule which was set by request
        virtual myrule_t requestrule () const = 0;

        virtual void request (myrule_t) = 0;
        int tagForGlobalRefinement ();
        int tagForGlobalCoarsening ();
        int resetRefinementRequest ();
        int tagForBallRefinement (const alucoord_t (&)[3],double,int);

        virtual bool isboundary() const { return false; }
        virtual bool isperiodic() const { return false; }
        virtual grid_t type() const { return tetra; }
        virtual void attachleafs() { abort(); }
        virtual void detachleafs() { abort(); }
        // return weight in terms of children of this face
        virtual int weight() const
        {
          const int weight = TreeIterator < const helement_STI,
                                            is_leaf < const helement_STI > > ( this ).size ();
          return weight;
        }

      private :
        int evalVertexTwist(int, int) const;
        int evalEdgeTwist(int, int) const;

        int originalVertexTwist(int, int) const;
        int originalEdgeTwist(int, int) const;
      protected:
        myhface_t * f [4];
        signed char s [4];

        // counter for bisection refinement if more than once
        signed char _count;
      } tetra_GEO;

      // Geometriesockelklasse des periodischen Randelements mit zwei
      // 3-Punkt-Fl"achen.

      typedef class Periodic3 : public hperiodic_STI,
                                public MyAlloc
      {
      public:
        typedef VertexGeo  myvertex_t;
        typedef hedge1_GEO myhedge_t;
        typedef hface3_GEO myhface_t;

        typedef myhedge_t myhedge1_t;
        typedef myhface_t myhface3_t;

        typedef Hface3Rule myrule_t;
        typedef myrule_t   balrule_t;

        typedef std::pair< hasFace3 *, int > myneighbour_t;
        typedef std::pair< const hasFace3 *, int > const_myneighbour_t;
      protected:
        Periodic3 (myhface_t *, int, myhface_t *, int);
        int postRefinement ();
        int preCoarsening ();

      public :
        using hasFace3    ::accessPllX;
        static const int prototype [2][3];
        virtual ~Periodic3 ();

        myhface_t* myhface (int);
        const myhface_t* myhface (int) const;

        myhface_t* myhface3 ( int i ) { return myhface( i ); } // use the ones without number
        const myhface_t* myhface3 ( int i ) const { return myhface( i ); } // use the ones without number

        VertexGeo * myvertex (int);
        const VertexGeo * myvertex (int) const;
        VertexGeo * myvertex (int,int);
        const VertexGeo * myvertex (int,int) const;

        myneighbour_t       myneighbour (int);
        const_myneighbour_t myneighbour (int) const;

        virtual int nFaces () const { return 2; }

        virtual int nEdges () const
        {
          std::cerr << "ERROR: Periodic3::nEdges not implemented." << std::endl;
          abort();
          return 6;
        }

        int twist (int) const;
        int test () const;

      public:
        virtual myrule_t getrule () const = 0;
        virtual void request (myrule_t) = 0;
        virtual bool markForConformingClosure () { return true; }
        virtual void markEdgeCoarsening () { }
        int tagForGlobalRefinement ();
        int tagForGlobalCoarsening ();
        int resetRefinementRequest ();
        int tagForBallRefinement (const alucoord_t (&)[3],double,int);
        virtual bool isboundary() const { return true; }
        virtual bool isperiodic() const { return true; }
        virtual grid_t type() const { return tetra_periodic; }

        // just returns level
        virtual int nbLevel() const {return level();}
        // just returns leaf
        virtual int nbLeaf() const {return leaf();}

        // return false for vertex projection
        virtual bool hasVertexProjection() const { return false; }
        // return weight in terms of children of this face
        virtual int weight() const
        {
          const int weight = TreeIterator < const helement_STI,
                                            is_leaf < const helement_STI > > ( this ).size ();
          return weight;
        }
      private :
        myhface_t * f [2];
        signed char s [2];
      } periodic3_GEO;

      // Anfang - Neu am 23.5.02 (BS)

      // Der Prototyp f"ur das Hexaederelement bedingt eine im Uhrzeigersinn
      // umlaufende Numerierung der lokalen Knoten einer Aussenfl"ache, falls
      // aus dem Element herausgeschaut wird. Gegensatz zum Tetraeder.
      // Der Prototyp steht in 'gitter_geo.cc'

      typedef class Hexa
        : public helement_STI,
          public MyAlloc
      {
      public :
        typedef VertexGeo myvertex_t;
        typedef hedge1_GEO myhedge_t;
        typedef hface4_GEO myhface_t;

        typedef myhedge_t  myhedge1_t;
        typedef myhface_t  myhface4_t;

        typedef HexaRule    myrule_t;
        typedef Hface4Rule  balrule_t;

        typedef std::pair< hasFace4 *, int > myneighbour_t;

      protected :
        Hexa (myhface_t *, int, myhface_t *, int,
              myhface_t *, int, myhface_t *, int,
              myhface_t *, int, myhface_t *, int);
        int postRefinement ();
        int preCoarsening ();

      public :
        using hasFace4    ::accessPllX;
        static const int prototype [6][4];
        static const int oppositeFace [6];
        static const int edgeMap [12][2];

        // cached possible twists
        static const int edgeTwist [8][4];
        static const int vertexTwist [8][4];

        static const int vertex2Face[8][2];

        static const std::vector< std::vector< int > > _verticesNotOnFace;
        static const std::vector< std::vector< int > > _edgesNotOnFace;
        static const std::vector< std::vector< int > > _facesNotOnFace;

        static std::vector< std::vector< int > > initVerticesNotOnFace ();
        static std::vector< std::vector< int > > initEdgesNotOnFace    ();
        static std::vector< std::vector< int > > initFacesNotOnFace    ();

        static const std::vector< int > &verticesNotOnFace ( const int face );
        static const std::vector< int > &edgesNotOnFace    ( const int face );
        static const std::vector< int > &facesNotOnFace    ( const int face );

        virtual ~Hexa ();
        myhface_t* myhface (int);
        const myhface_t* myhface (int) const;

        // don't use these methods use the ones without numbers
        myhface_t* myhface4 ( int i ) { return myhface( i ); }
        const myhface_t* myhface4 ( int i ) const { return myhface( i ); }

        VertexGeo * myvertex (int);
        const VertexGeo * myvertex (int) const;
        myhedge_t* myhedge(int);
        const myhedge_t* myhedge(int) const;

        myhedge_t* myhedge1( int i ) { return myhedge( i ); }
        const myhedge_t* myhedge1( int i ) const { return myhedge( i ); }

        VertexGeo * myvertex (int,int);
        const VertexGeo * myvertex (int,int) const;
        std::pair< hasFace4 *, int > myneighbour (int);
        std::pair< const hasFace4 *, int > myneighbour (int) const;

        // Dune extension
        // return pair, first = pointer to face, second = twist of face
        std::pair< myhface_t*, int > myintersection (int);
        std::pair< const myhface_t*, int > myintersection (int) const;
        virtual int nFaces() const { return 6; }
        virtual int nEdges() const { return 12; }

        int twist (int) const;
        int test () const;
        // just returns level
        virtual int nbLevel() const {return level();}
        // just returns leaf
        virtual int nbLeaf() const {return leaf();}

        // returns false because only bnd segments have projections
        virtual bool hasVertexProjection () const { return false; }

        // return if at least one of the vertex's coordinates was changed through projection
        virtual bool vertexWasProjected () const
        {
          for( int i=0; i<8; ++i )
          {
            if( myvertex( i )->wasProjected() )
              return true;
          }
          return false;
        }

      public :
        virtual myrule_t getrule () const = 0;
        virtual myrule_t requestrule () const = 0;
        virtual void request (myrule_t) = 0;
        virtual bool markForConformingClosure () { return true; }
        virtual void markEdgeCoarsening () { }
        int tagForGlobalRefinement ();
        int tagForGlobalCoarsening ();
        int resetRefinementRequest ();
        int tagForBallRefinement (const alucoord_t (&)[3],double,int);

        virtual bool isboundary() const { return false; }
        virtual bool isperiodic() const { return false; }
        virtual grid_t type() const { return hexa; }

        virtual void attachleafs() { abort(); }
        virtual void detachleafs() { abort(); }
        // return weight in terms of children of this face
        virtual int weight() const
        {
          const int weight = TreeIterator < const helement_STI,
                                            is_leaf < const helement_STI > > ( this ).size ();
          return weight;
        }
      private :
        // original formulas of twist evaluation
        int originalVertexTwist(int, int) const;
        int originalEdgeTwist(int, int) const;

        // cached twist
        int evalVertexTwist(int, int) const;
        int evalEdgeTwist(int, int) const;
      private:
        myhface_t * f [6];
        signed char s [6];
      } hexa_GEO;

      // Geometriesockelklasse des periodischen Randelements mit zwei
      // 4-Punkt-Fl"achen.

      typedef class Periodic4 : public hperiodic_STI,
                                public MyAlloc
      {
      public:
        typedef VertexGeo  myvertex_t;
        typedef hedge1_GEO myhedge_t;
        typedef hface4_GEO myhface_t;

        typedef myhedge_t myhedge1_t;
        typedef myhface_t myhface4_t;

        typedef Hface4Rule myrule_t;
        typedef myrule_t   balrule_t;

        typedef std::pair< hasFace4 *, int > myneighbour_t;
        typedef std::pair< const hasFace4 *, int > const_myneighbour_t;
      protected:
        Periodic4 (myhface_t *, int, myhface_t *, int);
        int postRefinement ();
        int preCoarsening ();

      public :
        using hasFace4    ::accessPllX;
        static const int prototype [2][4];
        virtual ~Periodic4 ();
        myhface_t* myhface (int);
        const myhface_t* myhface (int) const;
        myhface_t* myhface4 ( int i ) { return myhface( i ); }
        const myhface_t* myhface4 ( int i ) const { return myhface( i ); }

        VertexGeo * myvertex (int);
        const VertexGeo * myvertex (int) const;
        VertexGeo * myvertex (int,int);
        const VertexGeo * myvertex (int,int) const;

        myneighbour_t       myneighbour (int);
        const_myneighbour_t myneighbour (int) const;

        virtual int nFaces () const { return 2; }
        virtual int nEdges () const
        {
          std::cerr << "ERROR: Periodic4::nEdges not implemented." << std::endl;
          abort();
          return 8;
        }

        int twist ( int ) const;
        int test () const;

        virtual bool isboundary() const { return true; }
        virtual bool isperiodic() const { return true; }
        virtual grid_t type() const { return hexa_periodic; }

      public :
        virtual myrule_t getrule () const = 0;
        virtual void request (myrule_t) = 0;
        virtual bool markForConformingClosure () { return false; }
        virtual void markEdgeCoarsening () { }
        int tagForGlobalRefinement ();
        int tagForGlobalCoarsening ();
        int resetRefinementRequest ();
        int tagForBallRefinement (const alucoord_t (&)[3],double,int);
        // just returns level
        virtual int nbLevel() const {return level();}
        // just returns leaf
        virtual int nbLeaf() const {return leaf();}

        // returns false because only bnd segments have projections
        virtual bool hasVertexProjection () const { return false; }
        // return weight in terms of children of this face
        virtual int weight() const
        {
          const int weight = TreeIterator < const helement_STI,
                                            is_leaf < const helement_STI > > ( this ).size ();
          return weight;
        }
      private :
        myhface_t * f [2];
        signed char s [2];
      } periodic4_GEO;


      // Auch hier ist Vorsicht geboten: Der Protoyp des Dreiecksrandelement
      // numeriert seine Knoten gegen den Uhrzeigersinn, wenn aus dem Randelement
      // auf die Randfl"ache geschaut wird. Das Vierecksrandelement hat die
      // entgegengesetzte Konvention.

      typedef class hbndseg3
        : public hbndseg_STI, public MyAlloc
      {
      public :
        typedef VertexGeo   myvertex_t;
        typedef hedge1_GEO  myhedge_t;
        typedef hface3_GEO  myhface_t;
        typedef myhedge_t   myhedge1_t;
        typedef myhface_t   myhface3_t;
        typedef Hface3Rule  myrule_t;
        typedef myrule_t    balrule_t;

        typedef hbndseg_STI::bnd_t bnd_t;
      protected :
        hbndseg3 (myhface_t *,int);
        int postRefinement ();
        int preCoarsening ();
        bool lockedAgainstCoarsening () const { return false; }
      public :
        virtual ~hbndseg3 ();
        myrule_t getrule () const;
        virtual bool refineLikeElement (balrule_t) = 0;
        myvertex_t * myvertex (int,int) const;
        myhface_t * myhface3 ( int i ) const { return myhface( i ); } // use the method without number
        myhface_t * myhface (int) const;
        int twist (int) const;
        hface3_GEO * subface (int,int) const;

        virtual bool isboundary() const { return true; }
        virtual bool isperiodic() const { return false; }

        virtual bool markForConformingClosure () { return false; }
        virtual void markEdgeCoarsening () { }
        virtual int nChild () const;
        // just returns level
        virtual int nbLevel() const { return level(); }
        // just returns leaf
        virtual int nbLeaf() const { return leaf(); }

        // mark edges and vertices as leaf
        virtual void attachleafs();

        // unmark edges and vertices as leaf
        virtual void detachleafs();

        //! store shared pointer to boundary projection object (pass empty pointer to remove)
        virtual void setBoundaryProjection( const ProjectVertexPtr& pvPtr )
        {
          if( pvPtr && pvPtr->projectionType() != ProjectVertex::none )
          {
            _pvPtr = pvPtr;
          }
          else
            _pvPtr.reset();
        }

        //! return type of projection if set, otherwise none
        virtual typename ProjectVertex::ProjectionType projectionType() const
        {
          return ( _pvPtr ) ? _pvPtr->projectionType() : ProjectVertex::none;
        }

        bool hasVertexProjection() const { return bool(_pvPtr); }
        // no projection for ghost faces
        const ProjectVertex& projection() const { alugrid_assert( _pvPtr ); return *_pvPtr; }
        ProjectVertex& projection() { alugrid_assert( _pvPtr ); return *_pvPtr; }

      protected :
        myhface_t*       _face;   //  8 bytes
        ProjectVertexPtr _pvPtr;  // 16 bytes

        int _twist;               //  4 bytes
        unsigned char _lvl;       //  1 byte    = 8 bytes
      public:
      } hbndseg3_GEO;


      typedef class hbndseg4 :
          public hbndseg_STI,
          public MyAlloc
      {
      public :
        typedef VertexGeo myvertex_t;
        typedef hedge1_GEO  myhedge_t;
        typedef hface4_GEO  myhface_t;

        typedef myhedge_t   myhedge1_t;
        typedef myhface_t   myhface4_t;
        typedef Hface4Rule  myrule_t;
        typedef myrule_t    balrule_t;

        typedef hbndseg_STI::bnd_t bnd_t;
      protected :
        hbndseg4 (myhface_t *,int);
        int postRefinement ();
        int preCoarsening ();
        bool lockedAgainstCoarsening () const { return false; }
      public :
        virtual ~hbndseg4 ();
        myrule_t getrule () const;
        virtual bool refineLikeElement (balrule_t) = 0;
        myvertex_t * myvertex (int,int) const;
        myhface_t * myhface (int) const;
        myhface_t * myhface4 ( int i ) const { return myhface( i ); }
        int twist (int) const;
        hface4_GEO * subface (int,int) const;

        virtual bool markForConformingClosure () { return false; }
        virtual void markEdgeCoarsening () { }

        virtual bool isboundary() const { return true; }
        virtual bool isperiodic() const { return false; }
        virtual int nChild () const;
        virtual int nbLevel() const {return level();}
        virtual int nbLeaf() const {return leaf();}
        virtual void attachleafs();

        virtual void detachleafs();

        //! store shared pointer to boundary projection object (pass empty pointer to remove)
        virtual void setBoundaryProjection( const ProjectVertexPtr& pvPtr )
        {
          if( pvPtr && pvPtr->projectionType() != ProjectVertex::none )
          {
            _pvPtr = pvPtr;
          }
          else
            _pvPtr.reset();
        }

        //! return type of projection if set, otherwise none
        virtual typename ProjectVertex::ProjectionType projectionType() const
        {
          return ( _pvPtr ) ? _pvPtr->projectionType() : ProjectVertex::none;
        }

        bool hasVertexProjection() const { return bool(_pvPtr); }

        // no projection for ghost faces
        const ProjectVertex& projection() const { alugrid_assert( _pvPtr ); return *_pvPtr; }
        ProjectVertex& projection() { alugrid_assert( _pvPtr ); return *_pvPtr; }

      protected :
        myhface_t*       _face;   //  8 bytes
        ProjectVertexPtr _pvPtr;  // 16 bytes

        int _twist;               //  4 bytes
        unsigned char _lvl;       //  1 byte   = 8 bytes

      public:
      } hbndseg4_GEO;

      class InternalHasFace3 {
      public :
        typedef hasFace3 val_t;
        val_t * operator () (hasFace3 * x) const { return x; }
        val_t & operator () (hasFace3 & x) const { return x; }
      };

      class InternalHasFace4 {
      public :
        typedef hasFace4 val_t;
        val_t * operator () (hasFace4 * x) const { return x; }
        val_t & operator () (hasFace4 & x) const { return x; }
      };
    public :
      class BuilderIF : public Makrogitter {

        // BuilderIF ist die Stelle des Makrogitters an der der Builder angreift, wenn das
        // Gitter erbaut werden soll. Der Builder geht direkt mit den Listen um und
        // wendet sich an die Factorymethoden insert_--*-- (), um neue Objekte zu erhalten.
      public:
        typedef std::vector< VertexGeo * >     vertexlist_t;
        typedef std::vector< hedge1_GEO * >    hedge1list_t;
        typedef std::vector< hface4_GEO * >    hface4list_t;
        typedef std::vector< hface3_GEO * >    hface3list_t;
        typedef std::vector< tetra_GEO * >     tetralist_t;
        typedef std::vector< periodic3_GEO * > periodic3list_t;

        typedef std::vector< periodic4_GEO * > periodic4list_t;
        typedef std::vector< hexa_GEO * >      hexalist_t;

        typedef std::vector< hbndseg3_GEO * >  hbndseg3list_t;
        typedef std::vector< hbndseg4_GEO * >  hbndseg4list_t;

      private:
        // macro object lists
        vertexlist_t    _vertexList;
        hedge1list_t    _hedge1List;
        hface4list_t    _hface4List;
        hface3list_t    _hface3List;
        tetralist_t     _tetraList;
        periodic3list_t _periodic3List;

        periodic4list_t _periodic4List;
        hexalist_t      _hexaList;

        hbndseg3list_t  _hbndseg3List;
        hbndseg4list_t  _hbndseg4List;

      protected :
        BuilderIF ()
          : _computeLinkage( true ),
            _vertexElementLinkageComputed( false )
        {}

        virtual ~BuilderIF ();

        // return size of used memory in bytes
        virtual size_t memUsage ();

        // generates macro image from macro file
        void generateRawHexaImage (std::istream &, std::ostream &);

        virtual void macrogridBuilder (std::istream &);

        virtual VertexGeo     * insert_vertex (double, double, double, int) = 0;
        virtual VertexGeo     * insert_ghostvx(double, double, double, int) = 0;
        virtual hedge1_GEO    * insert_hedge1 (VertexGeo *, VertexGeo *) = 0;
        virtual hface3_GEO    * insert_hface3 (hedge1_GEO *(&)[3], int (&)[3]) = 0;
        virtual hface4_GEO    * insert_hface4 (hedge1_GEO *(&)[4], int (&)[4]) = 0;
        virtual tetra_GEO     * insert_tetra (hface3_GEO *(&)[4], int (&)[4], SimplexTypeFlag) = 0;

        virtual periodic3_GEO * insert_periodic3 (hface3_GEO *(&)[2], int (&)[2], const hbndseg_STI::bnd_t (&)[2] ) = 0;
        virtual periodic4_GEO * insert_periodic4 (hface4_GEO *(&)[2], int (&)[2], const hbndseg_STI::bnd_t (&)[2] ) = 0;

        virtual hexa_GEO      * insert_hexa (hface4_GEO *(&)[6], int (&)[6]) = 0;

        virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, hbndseg_STI::bnd_t) = 0;

        // insert ghost element
        virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, hbndseg_STI:: bnd_t,
                                              MacroGhostInfoTetra* ) = 0;

        virtual hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, hbndseg_STI::bnd_t) = 0;

        // method to insert internal boundary with ghost
        virtual hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, hbndseg_STI::bnd_t,
                                              MacroGhostInfoHexa* ) = 0;

        IteratorSTI < vertex_STI >    *iterator (const vertex_STI *) const;
        IteratorSTI < vertex_STI >    *iterator (const IteratorSTI < vertex_STI > *) const;
        IteratorSTI < hedge_STI >     *iterator (const hedge_STI *) const;
        IteratorSTI < hedge_STI >     *iterator (const IteratorSTI < hedge_STI > *) const;
        IteratorSTI < hface_STI >     *iterator (const hface_STI *) const;
        IteratorSTI < hface_STI >     *iterator (const IteratorSTI < hface_STI > *) const;
        IteratorSTI < helement_STI >  *iterator (const helement_STI *) const;
        IteratorSTI < helement_STI >  *iterator (const IteratorSTI < helement_STI > *) const;
        IteratorSTI < hperiodic_STI > *iterator (const hperiodic_STI *) const;
        IteratorSTI < hperiodic_STI > *iterator (const IteratorSTI < hperiodic_STI > *) const;
        IteratorSTI < hbndseg_STI >   *iterator (const hbndseg_STI *) const;
        IteratorSTI < hbndseg_STI >   *iterator (const IteratorSTI < hbndseg_STI > *) const;
      public:
        enum { numOfIndexManager = IndexManagerStorageType::numOfIndexManager };

        // number of different index manager that exists
        enum { IM_Elements = IndexManagerStorageType::IM_Elements, // 0 == elements
               IM_Faces    = IndexManagerStorageType::IM_Faces,    // 1 == faces
               IM_Edges    = IndexManagerStorageType::IM_Edges,    // 2 == edges
               IM_Vertices = IndexManagerStorageType::IM_Vertices, // 3 == vertices
               IM_Bnd      = IndexManagerStorageType::IM_Bnd,      // 4 == boundary elements
               IM_Internal = IndexManagerStorageType::IM_Internal  // 5 == internal bnds, parallel only
        };

      protected:
        // this variable is located here, because all the elements in
        // this lists use this objects to get  thier numbers
        // index provider, for every codim one , 4 is for boundary
        IndexManagerStorageType _indexManagerStorage;

        // pair of projection pointers first = global, second = surface
        ProjectVertexPtrPair  _projections;

        bool _computeLinkage ; // if true vertexLinkageEstimate is done
        bool _vertexElementLinkageComputed;

      public :
        void disableLinkageCheck() { _computeLinkage = true ; }
        void linkageComputed() { _computeLinkage = false ; }
        bool computeLinkage () const { return _computeLinkage; }

        bool vertexElementLinkageComputed() const { return _vertexElementLinkageComputed; }
        void notifyVertexElementLinkageComputed () { _vertexElementLinkageComputed = true ; }

        // return reference to indexManager
        virtual IndexManagerType& indexManager(int codim);
        // return reference to indexManagerStorage
        virtual IndexManagerStorageType& indexManagerStorage();

        virtual const ProjectVertexPtr& globalProjection()  const { return _projections.first;  }
        virtual const ProjectVertexPtr& surfaceProjection() const { return _projections.second; }

        // return number of macro boundary segments
        virtual size_t numMacroBndSegments() const;

        // compress all index manager
        virtual void compressIndexManagers();

        virtual MacroFileHeader dumpMacroGrid (std::ostream &, const MacroFileHeader::Format ) const;
        friend class MacroGridBuilder;
        friend class MacroGhostBuilder;
        friend class ParallelGridMover;

      protected:
        void setProjections( const ProjectVertexPtrPair& ppv )
        {
          _projections.first  = ppv.first;
          _projections.second = ppv.second;
        }

        template <class ostream_t>
        void dumpMacroGridImpl (ostream_t &) const;
      };
    };
  private :
    IteratorSTI < vertex_STI >   * iterator (const vertex_STI *);
    IteratorSTI < hedge_STI >    * iterator (const hedge_STI *);
    IteratorSTI < hface_STI >    * iterator (const hface_STI *);
    IteratorSTI < hbndseg_STI >  * iterator (const hbndseg_STI *);
    IteratorSTI < helement_STI > * iterator (const helement_STI *);

    IteratorSTI < vertex_STI >   * levelIterator (const vertex_STI *, const any_has_level<vertex_STI> &);
    IteratorSTI < hedge_STI >    * levelIterator (const hedge_STI *, const any_has_level<hedge_STI> & );
    IteratorSTI < hface_STI >    * levelIterator (const hface_STI * , const any_has_level<hface_STI> &);
    IteratorSTI < hbndseg_STI >  * levelIterator (const hbndseg_STI *, const any_has_level<hbndseg_STI> &);
    IteratorSTI < helement_STI > * levelIterator (const helement_STI *, const any_has_level<helement_STI> &);

  public:
    template <class StopRule_t >
    IteratorSTI < hedge_STI >    * createIterator(const hedge_STI * , const StopRule_t rule);

    template <class StopRule_t >
    IteratorSTI < hface_STI >    * createIterator(const hface_STI * , const StopRule_t rule);

    template <class StopRule_t >
    IteratorSTI < helement_STI > * createIterator(const helement_STI * ,const StopRule_t rule);

    template <class StopRule_t >
    IteratorSTI < hbndseg_STI >  * createIterator(const hbndseg_STI * , const StopRule_t rule);

  protected :
    // methods for refining and coarsening
    virtual bool refine ();
    // returns true if conforming closure is still needed
    virtual bool markForConformingClosure ();
    virtual bool markEdgeCoarsening ();
    virtual void coarse ();
    void doCoarse ();
    void resetEdgeCoarsenFlags ();

    virtual Makrogitter & container () = 0;
    virtual const Makrogitter & container () const = 0;
    virtual int iterators_attached () const;
    virtual void notifyMacroGridChanges ();
  protected :
    // bisectionRefinement is disabled by default
    Gitter ()
      : bisectionRefinement_( false ),
        enableGhostCells_( true )
     {}

    virtual ~Gitter ();

  public :
    // callback for Dune
    virtual int preCoarsening ( helement_STI & ) { return 0; }
    virtual int postRefinement( helement_STI & ) { return 0; }
    // callback for Dune
    virtual int preCoarsening ( hbndseg_STI & ) { return 0; }
    virtual int postRefinement( hbndseg_STI & ) { return 0; }

    // return pointer to vertex projection
    // virtual ProjectVertex* vertexProjection() const = 0;

    virtual void fullIntegrityCheck ();
    virtual void printsize ();

    // return grid's dimension (ie 2 or 3)
    virtual int dimension () { return indexManagerStorage().dimension(); }

  protected:
    // make this method protected to avoid usage
    virtual bool adapt ();

  public:
    // this method just calls adapt
    virtual bool adaptWithoutLoadBalancing();
    // adaptation with callback functionality
    virtual bool duneAdapt ( AdaptRestrictProlongType & );
    virtual bool loadBalance ( GatherScatterType* gs = 0 ) { return false; }
    virtual void refineGlobal ();
    virtual void markForBallRefinement(const alucoord_t (&)[3],double,int);
    virtual void refineRandom (double);

    // print memory consumption of grid
    virtual void printMemUsage () = 0;

    virtual size_t numMacroBndSegments() const = 0;

    // return index manager of macro grid
    virtual IndexManagerType & indexManager (int codim) = 0;

    // return reference to indexManagerStorage
    virtual IndexManagerStorageType& indexManagerStorage() = 0;

    virtual void tovtk( const std::string &fn);
  protected:
    template <class element_t, class bnd_t>
    void tovtkImpl( const std::string &fn,
                    const int, const element_t*, const bnd_t* );

    template <class stream_t>
    void backupHierarchy( stream_t& );

    template <class stream_t>
    void restoreHierarchy( stream_t&, const bool restoreBndFaces );

    // these classes are friend because the must call the method iterator on grid
    friend class LeafIterator < helement_STI >;
    friend class LeafIterator < vertex_STI >;
    friend class LeafIterator < hbndseg_STI >;
    friend class LeafIterator < hedge_STI >;
    friend class LeafIterator < hface_STI >;

    friend class LevelIterator < helement_STI >;
    friend class LevelIterator < vertex_STI >;
    friend class LevelIterator < hbndseg_STI >;
    friend class LevelIterator < hedge_STI >;
    friend class LevelIterator < hface_STI >;

  public:
    virtual bool conformingClosureNeeded() const { return bisectionRefinement_; }
    virtual void enableConformingClosure() { bisectionRefinement_ = true; }

    virtual bool ghostCellsEnabled() const { return enableGhostCells_; }
    virtual void disableGhostCells() { enableGhostCells_ = false; }

    // flags to say if an edge can be coarsened during conform bisection
  protected:
    // true if conforming closure is needed closure
    bool bisectionRefinement_;
    // true if ghost cells are available
    bool enableGhostCells_;

  };
  // --endGitter

  // --parallel types
  typedef Gitter::Parallel Parallel;
  typedef Gitter::VertexPllXIF  VertexPllXIF_t;
  typedef Gitter::EdgePllXIF    EdgePllXIF_t;
  typedef Gitter::FacePllXIF    FacePllXIF_t;
  typedef Gitter::ElementPllXIF ElementPllXIF_t;

  // "Ausseres Iteratorproxy oder auch einfach ein Smartpointer
  // um von aussen vom Gitter Iterationsobjekte zu bekommen und
  // zu verwalten.

  template < class A > class LeafIterator
  {
    Gitter * _grd;
    IteratorSTI < A > * _w;
    const A * _a;
    void * operator new (size_t);
    void operator delete (void *);
    LeafIterator ();
  public :
    typedef A val_t;
    LeafIterator (Gitter &);
    LeafIterator (const LeafIterator < A > & );
    ~LeafIterator ();
    IteratorSTI < A > * operator -> () const;
    IteratorSTI < A > & operator * () const;
    LeafIterator < A > & operator = (const LeafIterator < A > &);
  private:
    void removeObj();
    void assign(const LeafIterator < A > & );
  };

  template < class A, class StopRule_t > class GridIterator
  {
    Gitter * _grd;
    IteratorSTI < A > * _w;
    const A * _a;
    void * operator new (size_t);
    void operator delete (void *);
    GridIterator ();
  public :
    typedef A val_t;
    GridIterator (Gitter &, const StopRule_t );
    GridIterator (const GridIterator < A , StopRule_t > & );
    ~GridIterator ();
    IteratorSTI < A > * operator -> () const;
    IteratorSTI < A > & operator * () const;
    GridIterator < A , StopRule_t > & operator = (const GridIterator < A , StopRule_t > &);
  private:
    void removeObj();
    void assign(const GridIterator < A , StopRule_t > & );
  };

  // LevelIterator is the same construct as LeafIterator, but the iterator
  // rule differs, here we use any_has_level, see walk.h
  template < class A > class LevelIterator
  {
    Gitter * _grd;
    const any_has_level < A > _ahl;
    IteratorSTI   < A > * _w;
    const A * _a;
    void * operator new (size_t);
    void operator delete (void *);
    LevelIterator ();
  public :
    typedef A val_t;
    // constructor with no level given creates macro iterator
    LevelIterator (Gitter &, int l = 0);
    LevelIterator (const LevelIterator < A > & );
    ~LevelIterator ();
    IteratorSTI < A > * operator -> () const;
    IteratorSTI < A > & operator * () const;
    LevelIterator < A > & operator = (const LevelIterator < A > &);
  private:
    void removeObj();
    void assign(const LevelIterator < A > & );
  };

  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //
  // start of implementation part
  // --

  inline std::pair< int, int > operator += (std::pair< int, int> & a, const std::pair< int, int > & b) {
    return std::pair< int, int > (a.first += b.first, a.second += b.second);
  }

  //////////////////////////////////////////////////////////////////
  //
  // Empty Iterator
  //
  //////////////////////////////////////////////////////////////////

  template < class A > inline void EmptyIterator < A >::first () {
    return;
  }

  template < class A > inline void EmptyIterator < A >::next () {
    return;
  }

  template < class A > inline int EmptyIterator < A >::done () const {
    return 1;
  }

  template < class A > inline int EmptyIterator < A >::size () {
    return 0;
  }

  template < class A > inline A & EmptyIterator < A >::item () const
  {
    // don't dereference an empty iterator
    alugrid_assert ( ! done () );
    abort();
    A * p = 0;
    return *p;
  }

  template < class A > inline IteratorSTI < A > * EmptyIterator < A >::clone () const
  {
    return new EmptyIterator < A > (*this);
  }

  template < class A > inline AccessIterator < A >::Handle::Handle (AccessIterator < A > & f)
    : _fac (&f), _a (0), _w (0)
  {
    _fac->ref ++;
    _w = _fac->iterator (_a);
    return;
  }

  template < class A > inline AccessIterator < A >::Handle::Handle (const ThisType & p)
    : _fac (0), _a (0) , _w(0)
  {
    assign(p);
    return;
  }

  template < class A > inline IteratorSTI< A > * AccessIterator < A >::Handle::
  clone () const
  {
    return new ThisType (*this);
  }

  template < class A > inline AccessIterator < A >::Handle::Handle ()
    : _fac (0)
    , _a (0)
    , _w ( new EmptyIterator < A > () )
  {
    return;
  }

  template < class A > inline AccessIterator < A >::Handle::~Handle () {
    removeObj();
    return;
  }

  template < class A > inline void AccessIterator < A >::Handle::
  removeObj ()
  {
    if(_fac) _fac->ref--;
    _fac = 0;
    if(_w) delete _w;
    _w = 0;
    return;
  }

  template < class A > inline const typename AccessIterator < A >::Handle & AccessIterator < A >::Handle::operator = (const ThisType & x)
  {
    removeObj();
    assign(x);
    return x;
  }

  template < class A > inline void
  AccessIterator < A >::Handle::assign (const ThisType & x)
  {
    alugrid_assert ( _fac == 0 );
    alugrid_assert ( _w == 0 );

    _fac = x._fac;
    if( _fac ) _fac->ref ++;
    _w = x._w->clone();
  }

  template < class A > inline bool AccessIterator < A >::Handle::operator == (const ThisType & x) const {
    return (x._fac == _fac) ? ((& x._w->item ()) == (& _w->item ()) ? 1 : 0) : 0;
  }

  template < class A > inline bool AccessIterator < A >::Handle::operator < (const ThisType & x) const {
    return (abort (), false );
  }

  template < class A > inline void AccessIterator < A >::Handle::first () {
    _w->first ();
    return;
  }

  template < class A > inline void AccessIterator < A >::Handle::next () {
    _w->next ();
    return;
  }

  template < class A > inline int AccessIterator < A >::Handle::done () const {
    return _w->done ();
  }

  template < class A > inline int AccessIterator < A >::Handle::size () {
    return _w->size ();
  }

  template < class A > inline A & AccessIterator < A >::Handle::item () const {
    return _w->item ();
  }


  template <class StopRule_t>
  inline IteratorSTI< Gitter ::hedge_STI > * Gitter::createIterator(const hedge_STI * , const StopRule_t rule)
  {
    typedef Insert < AccessIterator < hedge_STI >::Handle,
                     TreeIterator < hedge_STI, StopRule_t > > level_edge__macro_edge__iterator;

    std::vector< IteratorSTI < hedge_STI > * > _iterators;

    _iterators.push_back ( new level_edge__macro_edge__iterator (container (), rule ));
    Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > > nf (container ());
    Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_edge < helement_STI > > > ne (container ());
    Wrapper < Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > ef (nf);
    Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge > ee (ne);

    _iterators.push_back ( new  Insert < Wrapper < Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, StopRule_t > > (ef , rule ));
    _iterators.push_back ( new Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, StopRule_t > > (ee, rule ));
    Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > > nef (container ());
    Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fnef (nef);
    Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > > fie (fnef);
    Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > efie (fie);
    _iterators.push_back (new Insert < Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, StopRule_t > > (efie, rule ));
    return new VectorAlign < hedge_STI > (_iterators);
  }

  template <class StopRule_t>
  inline IteratorSTI< Gitter::hface_STI > * Gitter::createIterator(const hface_STI * , const StopRule_t rule)
  {
    typedef Insert < AccessIterator < hface_STI >::Handle,
                     TreeIterator < hface_STI, StopRule_t > >  macro_face__iterator;

    macro_face__iterator w1 (container (), rule );
    Insert < AccessIterator < helement_STI >::Handle,
             TreeIterator < helement_STI, has_int_face < helement_STI > > > nw (container ());

    Wrapper < Insert < AccessIterator < helement_STI >::Handle,
              TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > ww (nw);

    Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
                TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
                TreeIterator < hface_STI, StopRule_t > > www (nw, rule );

    return new AlignIterator < macro_face__iterator,
    Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, StopRule_t > >, hface_STI > (w1, www);
  }

  template <class StopRule_t>
  inline IteratorSTI< Gitter::helement_STI > * Gitter::createIterator(const helement_STI * , const StopRule_t rule)
  {
    typedef Insert < AccessIterator < Gitter::helement_STI >::Handle,
                     TreeIterator < Gitter::helement_STI, StopRule_t > >
               tree_element__macro_element__iterator;

    return new tree_element__macro_element__iterator (container (), rule );
  }

  template <class StopRule_t>
  inline IteratorSTI< Gitter::hbndseg_STI > * Gitter::createIterator(const hbndseg_STI * ,const StopRule_t rule)
  {
    typedef Insert < AccessIterator < hbndseg_STI >::Handle,
      TreeIterator < hbndseg_STI, StopRule_t > >
            tree_bnd__macro_bnd__iterator;
    return new tree_bnd__macro_bnd__iterator (container (), rule );
  }

  inline bool Gitter::debugOption (int level) {
#ifdef ALUGRIDDEBUG
    return (getenv ("VERBOSE") ? ( atoi (getenv ("VERBOSE")) > level ? true : (level == 0)) : false);
#else
    return false ;
#endif
  }

  inline int Gitter::iterators_attached () const {
    return ref;
  }

  inline int Gitter::hedge::leaf () const {
    return ! down ();
  }

  inline int Gitter::hface::leaf () const {
    return ! down ();
  }

  inline int Gitter::helement::leaf () const {
    return ! down ();
  }

  // Dune extensions
  inline void Gitter::Dune_helement::resetRefinedTag () {
    ref.reset ();
  }

  inline bool Gitter::Dune_helement::hasBeenRefined () const {
    return ref.positive();
  }

  inline int Gitter::hbndseg::leaf () const {
    return ! down ();
  }

  inline std::ostream& operator<< (std::ostream& s, const Gitter::Geometric::VertexGeo* v )
  {
    if( v )
    {
      s << "vx ( " << v->getIndex()
        //<< ", " << v->ident()
        << " : ";
      for (int i=0; i<3; ++i)
        s << ((i>0) ? " " : "") << v->Point()[i];
      s << " ) ";
    }
    else
      s << "nullptr";
    return s;
  }


  inline std::ostream &operator<< ( std::ostream &s, const Gitter::Geometric::Tetra *tetra )
  {
    if( tetra )
    {
      const Gitter::helement_STI *father = tetra->up();
      s << "Tetra[" << tetra->getIndex() << "] ";
      if ( father )
        s << " (father " << father->getIndex() << ")";
      s << " :";
      for( int i = 0; i < 4; ++i )
        s << " " << tetra->myvertex( i );
      s << std::endl;
      for( int i = 0; i < 4; ++i )
      {
        s << "T-Face " << i << " (tw = " << tetra->twist( i ) << ")";
        for( int j = 0; j < 3; ++j )
          s << " " << tetra->myhface( i )->myvertex( j );
        s << std::endl;
      }
      s << std::endl;
    }
    else
      s << "nullptr";
    return s;
  }


  inline std::ostream &operator<< ( std::ostream &s, const Gitter::Geometric::Hexa *hexa )
  {
    if( hexa )
    {
      const Gitter::helement_STI *father = hexa->up();
      s << "Hexa[" << hexa->getIndex() << "] ";
      if ( father )
        s << " (father " << father->getIndex() << ")";
      s << " :";
      for( int i = 0; i < 8; ++i )
        s << " " << hexa->myvertex( i );
      s << std::endl;
      /*
      for( int i = 0; i < 6; ++i )
      {
        s << "T-Face " << i << " ";
        for( int j = 0; j < 3; ++j )
          s << " " << tetra->myvertex( i, j );
        s << std::endl;
      }
      s << std::endl;
      */
    }
    else
      s << "nullptr";
    return s;
  }

  inline std::ostream &operator<< ( std::ostream &s, const Gitter::Geometric::hedge1 *edge )
  {
    if( edge )
    {
      s << "edge ( " << edge->getIndex()
        //<< ", " << v->ident()
        << " :";
      for ( int i = 0; i < 2; ++i )
        s << " " << edge->myvertex( i );
      s << std::endl;
    }
    else
      s << "nullptr";
    return s;
  }

  inline std::ostream &operator<< ( std::ostream &s, const Gitter::Geometric::hface3 *face )
  {
    if( face )
    {
      s << "face ( " << face->getIndex()
        //<< ", " << v->ident()
        << " :";
      for( int i = 0; i < 3; ++i )
        s << " " << face->myvertex( i );
      for( int i = 0; i < 3; ++i )
        s << " " << face->myhedge( i );
      s << std::endl;
    }
    else
      s << "nullptr";
    return s;
  }

  inline std::ostream &operator<< ( std::ostream &s, const Gitter::Geometric::hface4 *face )
  {
    if( face )
    {
      s << "face ( " << face->getIndex()
        //<< ", " << v->ident()
        << " :";
      for( int i = 0; i < 4; ++i )
        s << " " << face->myvertex( i );
      s << std::endl;
    }
    else
      s << "nullptr";
    return s;
  }




  // #     #                                          #####
  // #     #  ######  #####    #####  ######  #    # #     #  ######   ####
  // #     #  #       #    #     #    #        #  #  #        #       #    #
  // #     #  #####   #    #     #    #####     ##   #  ####  #####   #    #
  //  #   #   #       #####      #    #         ##   #     #  #       #    #
  //   # #    #       #   #      #    #        #  #  #     #  #       #    #
  //    #     ######  #    #     #    ######  #    #  #####   ######   ####

  inline Gitter::Geometric::VertexGeo::VertexGeo (int l, double x, double y, double z, IndexManagerStorageType & ims)
    : _indexManagerStorage (ims)
  {
    lvl() = l;
    _c [0] = x; _c [1] = y; _c [2] = z;
    this->setIndex( indexManager().getIndex() );
    //cout << "Create " << this << endl;
    return;
  }

  inline Gitter::Geometric::VertexGeo::VertexGeo (int l, double x, double y, double z, VertexGeo & vx)
    : _indexManagerStorage ( vx._indexManagerStorage )
  {
    lvl() = l;
    _c [0] = x; _c [1] = y; _c [2] = z;
    this->setIndex( indexManager().getIndex() );
    //cout << "Create " << this << endl;
    return;
  }

  inline Gitter::Geometric::VertexGeo::~VertexGeo ()
  {
    this->freeIndex( indexManager() );
    alugrid_assert (ref ? (std::cerr << "**WARNING VertexGeo::refcount was " << ref << std::endl, 1) : 1);
    return;
  }

  inline void Gitter::Geometric::VertexGeo::project(const ProjectVertex &pv)
  {
    // copy current coordinates
    const alucoord_t p[3] = {_c[0],_c[1],_c[2]};
    // call projection operator
    const int ok = pv( p, _c );

    // affine geometry flag not used for vertices
    this->setVertexWasProjected();

    if ( ! ok )
    {
      std::cerr << "ERROR in Gitter::Geometric::VertexGeo::project( const ProjectVertex &pv ): boundary projection not possible." << std::endl;
      for( int i = 0; i < 3; ++i )
        _c[ i ] = p[ i ];
    }
  }

  inline void Gitter::Geometric::VertexGeo::backupIndex ( std::ostream &os ) const
  {
    // backup index
    doBackupIndex( os );
  }

  inline void Gitter::Geometric::VertexGeo::backupIndex ( ObjectStream &os ) const
  {
    // backup index
    doBackupIndex( os );
  }

  inline void Gitter::Geometric::VertexGeo::restoreIndex ( std::istream &is, RestoreInfo& restoreInfo )
  {
    typedef Gitter::Geometric::BuilderIF BuilderIF;
    // restore index
    doRestoreIndex( is, restoreInfo, BuilderIF::IM_Vertices );
  }

  inline void Gitter::Geometric::VertexGeo::restoreIndex ( ObjectStream& is, RestoreInfo& restoreInfo )
  {
    typedef Gitter::Geometric::BuilderIF BuilderIF;
    // restore index
    doRestoreIndex( is, restoreInfo, BuilderIF::IM_Vertices );
  }



  // #     #                                    #
  // #     #  ######  #####    ####   ######   ##
  // #     #  #       #    #  #    #  #       # #
  // #######  #####   #    #  #       #####     #
  // #     #  #       #    #  #  ###  #         #
  // #     #  #       #    #  #    #  #         #
  // #     #  ######  #####    ####   ######  #####

  inline Gitter::Geometric::hedge1::hedge1 ( myvertex_t *a, myvertex_t *b )
  : v0( a ), v1( b )
  {
    ++v0->ref;
    ++v1->ref;
  }

  inline Gitter::Geometric::hedge1::~hedge1 ()
  {
    alugrid_assert (ref ? (std::cerr << "**WARNING hedge1::refcount was " << ref << std::endl, 1) : 1);
    alugrid_assert ( ref == 0 );
    v0->ref --;
    v1->ref --;
    return;
  }

  inline int Gitter::Geometric::hedge1::postRefinement () { return 0; }

  inline int Gitter::Geometric::hedge1::preCoarsening () { return 0; }

  inline bool Gitter::Geometric::hedge1::lockedAgainstCoarsening () const { return false; }

  inline Gitter::Geometric::VertexGeo * Gitter::Geometric::hedge1::myvertex (int i)
  {
    alugrid_assert (i == 0 || i == 1);
    return i == 1 ? v1 : v0;
  }

  inline const Gitter::Geometric::hedge1::myvertex_t * Gitter::Geometric::hedge1::myvertex (int i) const {
    alugrid_assert (i == 0 || i == 1);
    return i == 1 ? v1 : v0;
  }

  inline bool Gitter::Geometric::hedge1::canCoarsen() const
  {
    // noCoarsen is implemented in DuneIndexProvider
    if( this->noCoarsen() ) return false;
    const hedge* dwn = this->down();
    if( dwn ) return dwn->canCoarsen();
    const hedge* nxt = this->next();
    if( nxt ) return nxt->canCoarsen();
    return true;
  }


  //                                         #####
  // #    #  ######    ##     ####   ###### #     #
  // #    #  #        #  #   #    #  #            #
  // ######  #####   #    #  #       #####   #####
  // #    #  #       ######  #       #            #
  // #    #  #       #    #  #    #  #      #     #
  // #    #  #       #    #   ####   ######  #####
  //

  inline Gitter::Geometric::hface3::face3Neighbour::face3Neighbour ()
   : _attachedFront( 0 ), _attachedRear( 0 )
  {
    // initialize front and rear
    setFront( null );
    setRear( null );
  }

  inline void
  Gitter::Geometric::hface3::face3Neighbour::setFront ( const std::pair< myconnect_t *, int > &p )
  {
    _faceFront = p.first;
    _numFront  = p.second;
  }

  inline void
  Gitter::Geometric::hface3::face3Neighbour::setNextFront ( const std::pair< myconnect_t *, int > &p )
  {
    setFront( p );

    // increase front counter
    ++ _attachedFront;
  }

  inline void
  Gitter::Geometric::hface3::face3Neighbour::setRear ( const std::pair< myconnect_t *, int > &p )
  {
    _faceRear = p.first;
    _numRear  = p.second;
  }

  inline void
  Gitter::Geometric::hface3::face3Neighbour::setNextRear ( const std::pair< myconnect_t *, int > &p )
  {
    setRear( p );

    // increase front counter
    ++ _attachedRear;
  }

  inline void
  Gitter::Geometric::hface3::face3Neighbour::setPrevFront ( const std::pair< myconnect_t *, int > &p )
  {
    setFront( p );

    alugrid_assert ( _attachedFront > 0 );
    // decrease front counter
    -- _attachedFront;
  }

  inline void
  Gitter::Geometric::hface3::face3Neighbour::setPrevRear ( const std::pair< myconnect_t *, int > &p )
  {
    setRear( p );

    alugrid_assert ( _attachedRear > 0 );
    // decrease attached counter
    -- _attachedRear;
  }

  inline void Gitter::Geometric::hface3::face3Neighbour::assign (const face3Neighbour & n)
  {
    _faceFront     = n._faceFront;
    _faceRear      = n._faceRear;
    _numFront      = n._numFront;
    _numRear       = n._numRear;
    // this is needed due to the copy of this structure in
    // gitter_tetra_top.cc:381
    _attachedFront = 0;
    _attachedRear  = 0;
  }

  inline int Gitter::Geometric::hface3::face3Neighbour::complete (const face3Neighbour & n)
  {
    int ret = 0;

    if( front() == null )
    {
      setFront( std::pair< hasFace3 *, int >( n._faceFront, n._numFront ) );
      ++ret;
    }

    if( rear() == null )
    {
      setRear( std::pair< hasFace3 *, int >( n._faceRear, n._numRear ) );
      ++ret;
    }

    return ret;
  }

  inline std::pair< Gitter::Geometric::hface3::myconnect_t *, int >
  Gitter::Geometric::hface3::face3Neighbour::front ()
  {
    return std::pair< myconnect_t *, int >( _faceFront, _numFront );
  }

  inline std::pair< const Gitter::Geometric::hface3::myconnect_t *, int >
  Gitter::Geometric::hface3::face3Neighbour::front () const
  {
    return std::pair< const myconnect_t *, int >( _faceFront, _numFront );
  }

  inline std::pair< Gitter::Geometric::hface3::myconnect_t *, int >
  Gitter::Geometric::hface3::face3Neighbour::rear ()
  {
    return std::pair< myconnect_t *, int >( _faceRear, _numRear );
  }

  inline std::pair< const Gitter::Geometric::hface3::myconnect_t *, int >
  Gitter::Geometric::hface3::face3Neighbour::rear () const
  {
    return std::pair< const myconnect_t *, int >( _faceRear, _numRear );
  }

  inline Gitter::Geometric::hface3::
  hface3 (myhedge_t * e0, int s0, myhedge_t * e1, int s1, myhedge_t * e2, int s2)
  {
    alugrid_assert ( nb.emptyFront() );
    alugrid_assert ( nb.emptyRear() );
    nb._parRule = (Hface3Rule::undefined);
    alugrid_assert (e0 && e1 && e2);
    (e [0] = e0)->ref ++; nb.s [0] = s0;
    (e [1] = e1)->ref ++; nb.s [1] = s1;
    (e [2] = e2)->ref ++; nb.s [2] = s2;
    return;
  }

  inline Gitter::Geometric::hface3::~hface3 ()
  {
    alugrid_assert ( nb.emptyFront() );
    alugrid_assert ( nb.emptyRear() );
    alugrid_assert (ref ? (std::cerr << "**WARNING hface3::refcount was " << ref << std::endl, 1) : 1);
    e [0] -> ref --;
    e [1] -> ref --;
    e [2] -> ref --;
    return;
  }

  inline void Gitter::Geometric::hface3::
  attachElement (const std::pair< myconnect_t *, int > & p, int t)
  {
    alugrid_assert ( ref == 0 ? (nb._attachedRear + nb._attachedFront) == 0 : true );
    if ( t < 0 )
    {
      // if nothing was attached to rear then increase ref
      if( nb.emptyRear() ) ref ++;

      // set pair to rear
      nb.setNextRear( p );
    }
    else
    {
      // if nothing was attached to front then increase ref
      if( nb.emptyFront() ) ref ++;

      // set pair to front
      nb.setNextFront( p );
    }
    alugrid_assert ( ref <= 2 );
  }

  // detachElement and set connector to null
  inline void Gitter::Geometric::hface3::detachElement (int t)
  {
    // set null element
    detachElement( t, nb.null );
  }

  // detachElement with a given new pair of connectors
  inline void Gitter::Geometric::hface3::detachElement (int t, const std::pair< myconnect_t *, int > & p)
  {
    if ( t < 0 )
    {
      // set replacement
      nb.setPrevRear( p );

      // decrease ref counter if rear is empty
      if( nb.emptyRear() ) ref --;
    }
    else
    {
      // set replacement
      nb.setPrevFront( p );

      // decrease ref counter if front is empty
      if( nb.emptyFront() ) ref --;
    }
  }

  inline int Gitter::Geometric::hface3::postRefinement () {
    return 0;
  }

  inline int Gitter::Geometric::hface3::preCoarsening () {
    return 0;
  }

  inline int Gitter::Geometric::hface3::twist (int i) const {
    alugrid_assert (i < 3);
    return nb.s [i];
  }

  inline Gitter::Geometric::hface3::myhedge_t * Gitter::Geometric::hface3::myhedge (int i) {
    alugrid_assert (i < 3);
    return e [i];
  }

  inline const Gitter::Geometric::hface3::myhedge_t * Gitter::Geometric::hface3::myhedge (int i) const {
    alugrid_assert (i < 3);
    return e [i];
  }

  inline Gitter::Geometric::hface3::myvertex_t * Gitter::Geometric::hface3::myvertex (int i) {
    alugrid_assert (0<=i && i < 3);
    return myhedge (i)->myvertex ( nb.s[i] );
  }

  inline const Gitter::Geometric::hface3::myvertex_t * Gitter::Geometric::hface3::myvertex (int i) const {
    alugrid_assert (0<=i && i < 3);
    return myhedge (i)->myvertex (nb.s[i]);
  }

  inline Gitter::Geometric::hface3::myrule_t
  Gitter::Geometric::hface3::parentRule() const {
    return (myrule_t) nb._parRule;
  }

  inline bool Gitter::Geometric::hface3::
  isInteriorLeaf() const
  {
    const myconnect_t & nbRear  = *(nb.rear().first);
    const myconnect_t & nbFront = *(nb.front().first);

    if( nbFront.isboundary() )
    {
      return (nbRear.nbLeaf() && nbRear.nbLevel() == this->level());
    }
    else if (nbRear.isboundary())
    {
      return (nbFront.nbLeaf() && nbFront.nbLevel() == this->level());
    }
    else
    {
      return (nbRear.nbLeaf() || (nbFront.nbLeaf()));
    }
  }

  //                                        #
  // #    #  ######    ##     ####   ###### #    #
  // #    #  #        #  #   #    #  #      #    #
  // ######  #####   #    #  #       #####  #    #
  // #    #  #       ######  #       #      #######
  // #    #  #       #    #  #    #  #           #
  // #    #  #       #    #   ####   ######      #

  inline Gitter::Geometric::hface4::face4Neighbour::face4Neighbour ()
  {
    setFront( null );
    setRear( null );
    return;
  }

  inline void
  Gitter::Geometric::hface4::face4Neighbour::setFront ( const std::pair< myconnect_t *, int > &p )
  {
    _faceFront = p.first;
    _numFront = p.second;
  }

  inline void
  Gitter::Geometric::hface4::face4Neighbour::setRear ( const std::pair< myconnect_t *, int > &p )
  {
    _faceRear = p.first;
    _numRear = p.second;
  }

  inline void Gitter::Geometric::hface4::face4Neighbour::assign (const face4Neighbour & n)
  {
    _faceFront = n._faceFront;
    _faceRear = n._faceRear;
    _numFront = n._numFront;
    _numRear = n._numRear;
    return;
  }

  inline int Gitter::Geometric::hface4::face4Neighbour::complete (const face4Neighbour & n)
  {
    int ret = 0;

    if( front() == null )
    {
      setFront( std::pair< myconnect_t *, int >( n._faceFront, n._numFront ) );
      ++ret;
    }

    if( rear() == null )
    {
      setRear( std::pair< myconnect_t *, int >( n._faceRear, n._numRear ) );
      ++ret;
    }

    return ret;
  }

  inline std::pair< Gitter::Geometric::hface4::myconnect_t *, int >
  Gitter::Geometric::hface4::face4Neighbour::front ()
  {
    return std::pair< myconnect_t *, int >( _faceFront, _numFront );
  }

  inline std::pair< const Gitter::Geometric::hface4::myconnect_t *, int >
  Gitter::Geometric::hface4::face4Neighbour::front () const
  {
    return std::pair< const hasFace4 *, int >( _faceFront, _numFront );
  }

  inline std::pair< Gitter::Geometric::hface4::myconnect_t *, int >
  Gitter::Geometric::hface4::face4Neighbour::rear ()
  {
    return std::pair< myconnect_t *, int >( _faceRear, _numRear );
  }

  inline std::pair< const Gitter::Geometric::hface4::myconnect_t *, int >
  Gitter::Geometric::hface4::face4Neighbour::rear () const
  {
    return std::pair< const myconnect_t *, int >( _faceRear, _numRear );
  }

  inline Gitter::Geometric::hface4::
  hface4 (myhedge_t * e0, int s0, myhedge_t * e1, int s1, myhedge_t * e2, int s2, myhedge_t * e3, int s3)
  {
    alugrid_assert (e0 && e1 && e2 && e3);
    (e [0] = e0)->ref ++; nb.s [0] = s0;
    (e [1] = e1)->ref ++; nb.s [1] = s1;
    (e [2] = e2)->ref ++; nb.s [2] = s2;
    (e [3] = e3)->ref ++; nb.s [3] = s3;
    nb._parRule = (Hface4Rule::undefined);
  }

  inline Gitter::Geometric::hface4::~hface4 () {
    alugrid_assert (ref ? (std::cerr << "**WARNING hface4::refcount was " << ref << std::endl, 1) : 1);
    e [0] -> ref --;
    e [1] -> ref --;
    e [2] -> ref --;
    e [3] -> ref --;
    return;
  }

  inline void Gitter::Geometric::hface4::attachElement (const std::pair< myconnect_t *, int > & p, int t)
  {
    if( t < 0 )
      nb.setRear( p );
    else
      nb.setFront( p );
    ref ++;
    return;
  }

  inline void Gitter::Geometric::hface4::detachElement (int t)
  {
    if( t < 0 )
      nb.setRear( nb.null );
    else
      nb.setFront( nb.null );
    ref --;
    return;
  }

  inline int Gitter::Geometric::hface4::postRefinement () {
    return 0;
  }

  inline int Gitter::Geometric::hface4::preCoarsening () {
    return 0;
  }

  inline int Gitter::Geometric::hface4::twist (int i) const {
    alugrid_assert (i < 4);
    return nb.s [i];
  }

  inline Gitter::Geometric::hface4::myhedge_t * Gitter::Geometric::hface4::myhedge (int i) {
    alugrid_assert (i < 4);
    return e [i];
  }

  inline const Gitter::Geometric::hface4::myhedge_t * Gitter::Geometric::hface4::myhedge (int i) const {
    alugrid_assert (i < 4);
    return e [i];
  }

  inline Gitter::Geometric::hface4::myvertex_t * Gitter::Geometric::hface4::myvertex (int i) {
    alugrid_assert (0<=i && i < 4);
    return myhedge (i)->myvertex (nb.s[i]);
  }

  inline const Gitter::Geometric::hface4::myvertex_t * Gitter::Geometric::hface4::myvertex (int i) const {
    alugrid_assert (0<=i && i < 4);
    return myhedge (i)->myvertex (nb.s[i]);
  }

  inline Gitter::Geometric::hface4::myrule_t
  Gitter::Geometric::hface4::parentRule() const {
    return nb._parRule;
  }

  inline bool
  Gitter::Geometric::hface4::isInteriorLeaf() const
  {
    const myconnect_t & nbRear  = *(nb.rear().first);
    const myconnect_t & nbFront = *(nb.front().first);

    if (nbFront.isboundary())
    {
      return ( nbRear.nbLeaf() &&
               nbRear.nbLevel() == this->level());
    }
    else if (nbRear.isboundary())
    {
      return ( nbFront.nbLeaf() &&
               nbFront.nbLevel() == this->level());
    }
    else
    {
      return (nbRear.nbLeaf() || nbFront.nbLeaf());
    }
  }


  // #######
  //    #     ######   #####  #####     ##
  //    #     #          #    #    #   #  #
  //    #     #####      #    #    #  #    #
  //    #     #          #    #####   ######
  //    #     #          #    #   #   #    #
  //    #     ######     #    #    #  #    #

  inline Gitter::Geometric::Tetra::
  Tetra (myhface_t * f0, int t0, myhface_t * f1, int t1,
         myhface_t * f2, int t2, myhface_t * f3, int t3)
  {
    (f [0] = f0)->attachElement (std::pair< hasFace3 *, int > (InternalHasFace3 ()(this), 0),(s [0] = t0));
    (f [1] = f1)->attachElement (std::pair< hasFace3 *, int > (InternalHasFace3 ()(this), 1),(s [1] = t1));
    (f [2] = f2)->attachElement (std::pair< hasFace3 *, int > (InternalHasFace3 ()(this), 2),(s [2] = t2));
    (f [3] = f3)->attachElement (std::pair< hasFace3 *, int > (InternalHasFace3 ()(this), 3),(s [3] = t3));
    return;
  }

  inline Gitter::Geometric::Tetra::~Tetra ()
  {
#if 0
    // this code has been moved to ~TetraTop in gitter_tetra_top.cc
    {
      f [0] ->detachElement (s [0]);
      f [1] ->detachElement (s [1]);
      f [2] ->detachElement (s [2]);
      f [3] ->detachElement (s [3]);
    }
    return;
#endif
  }

  inline int Gitter::Geometric::Tetra::twist (int i) const {
    alugrid_assert (i < 4);
    return s [i];
  }

  inline Gitter::Geometric::Tetra::myhface_t * Gitter::Geometric::Tetra::myhface (int i) {
    alugrid_assert ( i <  4 );
    alugrid_assert ( i >= 0 );
    alugrid_assert ( f [i] );
    return f [i];
  }

  inline const Gitter::Geometric::Tetra::myhface_t * Gitter::Geometric::Tetra::myhface (int i) const {
    alugrid_assert ( i < 4 );
    alugrid_assert ( i >= 0 );
    alugrid_assert ( f [i] );
    return f [i];
  }

  inline int Gitter::Geometric::Tetra::originalVertexTwist(int face, int vertex) const {
    // twist ==  0  -->  identity
    // twist == -1  -->  flip vertex 1 and 2, keep 0
    return (twist(face) < 0 ?
            (7 - vertex + twist(face)) % 3 :
            (vertex + twist(face)) % 3);
  }

  inline int Gitter::Geometric::Tetra::evalVertexTwist(int face, int vertex) const {
    // make sure vertex and face are in range is
    alugrid_assert ( (twist(face) + 3 >= 0) && (twist(face)+3 < 6) );
    alugrid_assert ( vertex >= 0 && vertex < 3 );
    // make sure that we get the same result
    alugrid_assert ( originalVertexTwist(face,vertex) == vertexTwist[twist(face)+3][vertex] );
    return vertexTwist[twist(face)+3][vertex];
  }

  inline int Gitter::Geometric::Tetra::originalEdgeTwist(int face, int vertex) const
  {
    // twist == 0  --> identity
    return (twist(face) < 0 ?
            (6 - vertex + twist(face)) % 3 :
            (vertex + twist(face)) % 3);
  }

  inline int Gitter::Geometric::Tetra::evalEdgeTwist(int face, int vertex) const
  {
    // make sure vertex and face are in range is
    alugrid_assert ( (twist(face) + 3 >= 0) && (twist(face)+3 < 6) );
    alugrid_assert ( vertex >= 0 && vertex < 3 );
    // make sure that we get the same result
    alugrid_assert ( originalEdgeTwist(face,vertex) == edgeTwist[twist(face)+3][vertex]);
    return edgeTwist[twist(face)+3][vertex];
  }

  inline Gitter::Geometric::Tetra::myhedge_t * Gitter::Geometric::Tetra::myhedge (int edge)
  {
    alugrid_assert (edge >= 0 && edge < 6);

    typedef Gitter::Geometric::Tetra ThisType;

    return myhface(ThisType::edgeMap[edge][0])->
      myhedge(evalEdgeTwist(ThisType::edgeMap[edge][0],ThisType::edgeMap[edge][1]));
  }

  inline const Gitter::Geometric::Tetra::myhedge_t * Gitter::Geometric::Tetra::myhedge (int edge) const
  {
    alugrid_assert (edge >= 0 && edge < 6);

    typedef Gitter::Geometric::Tetra ThisType;

    return myhface(ThisType::edgeMap[edge][0])->
      myhedge(evalEdgeTwist(ThisType::edgeMap[edge][0],ThisType::edgeMap[edge][1]));

  }

  inline Gitter::Geometric::Tetra::myvertex_t * Gitter::Geometric::Tetra::myvertex (int i, int j) {
    return myhface(i)->myvertex(evalVertexTwist(i, j));
  }

  inline const Gitter::Geometric::Tetra::myvertex_t * Gitter::Geometric::Tetra::myvertex (int i, int j) const {
    return myhface(i)->myvertex(evalVertexTwist(i, j));
  }

  //- --tetramyvertex
  inline Gitter::Geometric::Tetra::myvertex_t * Gitter::Geometric::Tetra::myvertex (int i) {
    alugrid_assert (0 <= i && i < 4);
    return (i < 3) ? myvertex (3,i) : myvertex (2,1);
  }

  inline const Gitter::Geometric::Tetra::myvertex_t * Gitter::Geometric::Tetra::myvertex (int i) const {
    alugrid_assert (0 <= i && i < 4);
    return (i < 3) ? myvertex (3,i) : myvertex (2,1);
  }

  inline std::pair< Gitter::Geometric::hasFace3 *, int > Gitter::Geometric::Tetra::myneighbour (int i)
  {
    return twist (i) < 0 ? myhface( i )->nb.front () : myhface( i )->nb.rear ();
  }

  inline std::pair< const Gitter::Geometric::hasFace3 *, int > Gitter::Geometric::Tetra::myneighbour (int i) const {
    return twist (i) < 0 ? std::pair< const hasFace3 *, int > (myhface( i )->nb.front ().first, myhface( i )->nb.front ().second)
      : std::pair< const hasFace3 *, int > (myhface( i )->nb.rear ().first, myhface( i )->nb.rear ().second);
  }

  inline std::pair< Gitter::Geometric::hface3_GEO *, int > Gitter::Geometric::Tetra::myintersection (int i)
  {
    // return pair, first = pointer to face, second = twist of face
    return std::pair< Gitter::Geometric::hface3_GEO *,int> (myhface( i ) ,twist (i));
  }

  inline std::pair< const Gitter::Geometric::hface3_GEO *, int > Gitter::Geometric::Tetra::myintersection (int i) const
  {
    // return pair, first = pointer to face, second = twist of face
    return  std::pair< const Gitter::Geometric::hface3_GEO * , int > (myhface( i ) , twist (i) );
  }

  inline int Gitter::Geometric::Tetra::postRefinement () {
    return 0;
  }

  inline int Gitter::Geometric::Tetra::preCoarsening () {
    return 0;
  }

  // ######                                                           #####
  // #     #  ######  #####      #     ####   #####      #     ####  #     #
  // #     #  #       #    #     #    #    #  #    #     #    #    #       #
  // ######   #####   #    #     #    #    #  #    #     #    #       #####
  // #        #       #####      #    #    #  #    #     #    #            #
  // #        #       #   #      #    #    #  #    #     #    #    # #     #
  // #        ######  #    #     #     ####   #####      #     ####   #####

  inline Gitter::Geometric::Periodic3::
  Periodic3 (myhface_t * f0, int t0, myhface_t * f1, int t1)
  {
    (f [0] = f0)->attachElement (std::pair< hasFace3 *, int > (InternalHasFace3 ()(this), 0),(s [0] = t0));
    (f [1] = f1)->attachElement (std::pair< hasFace3 *, int > (InternalHasFace3 ()(this), 1),(s [1] = t1));
    return;
  }

  inline Gitter::Geometric::Periodic3::~Periodic3 () {
    f [0] ->detachElement (s [0]);
    f [1] ->detachElement (s [1]);
    return;
  }

  inline int Gitter::Geometric::Periodic3::twist (int i) const {
    alugrid_assert (0 <= i && i < 2);
    return s [i];
  }

  inline Gitter::Geometric::Periodic3::myhface_t * Gitter::Geometric::Periodic3::myhface (int i) {
    alugrid_assert (0 <= i && i < 2);
    return f [i];
  }

  inline const Gitter::Geometric::Periodic3::myhface_t * Gitter::Geometric::Periodic3::myhface (int i) const {
    alugrid_assert (0 <= i && i < 2);
    return f [i];
  }

  inline Gitter::Geometric::Periodic3::myvertex_t * Gitter::Geometric::Periodic3::myvertex (int i, int j) {
    alugrid_assert (0 <= i && i < 2);
    return (twist(i) < 0) ? myhface( i )->myvertex((7 - j + twist(i)) % 3) : myhface( i )->myvertex((j + twist(i)) % 3);
  }

  inline const Gitter::Geometric::Periodic3::myvertex_t * Gitter::Geometric::Periodic3::myvertex (int i, int j) const {
    return (twist(i) < 0) ? myhface( i )->myvertex((7 - j + twist(i)) % 3) : myhface( i )->myvertex((j + twist(i)) % 3);
  }

  inline Gitter::Geometric::Periodic3::myvertex_t * Gitter::Geometric::Periodic3::myvertex (int i) {
    alugrid_assert (0 <= i && i < 6);

    // Der Ausdruck liefert 0-> (0,0)
    //      1-> (0,1)
    //      2-> (0,2)
    //      3-> (1,0)
    //      4-> (1,2)
    //      5-> (1,1)
    return (i < 3) ? myvertex (0,i) : myvertex (1,(6-i)%3);
  }

  inline const Gitter::Geometric::Periodic3::myvertex_t * Gitter::Geometric::Periodic3::myvertex (int i) const {
    alugrid_assert (0 <= i && i < 6);
    return (i < 3) ? myvertex (0,i) : myvertex (1,(6-i)%3);
  }

  inline std::pair< Gitter::Geometric::hasFace3 *, int > Gitter::Geometric::Periodic3::myneighbour (int i) {
    alugrid_assert (0 <= i && i < 2);
    return twist (i) < 0 ? myhface( i )->nb.front () : myhface( i )->nb.rear ();
  }

  inline std::pair< const Gitter::Geometric::hasFace3 *, int > Gitter::Geometric::Periodic3::myneighbour (int i) const {
    alugrid_assert (0 <= i && i < 2);
    return twist (i) < 0 ? std::pair< const hasFace3 *, int > (myhface( i )->nb.front ().first, myhface( i )->nb.front ().second)
      : std::pair< const hasFace3 *, int > (myhface( i )->nb.rear ().first, myhface( i )->nb.rear ().second);
  }

  inline int Gitter::Geometric::Periodic3::postRefinement () {
    return 0;
  }

  inline int Gitter::Geometric::Periodic3::preCoarsening () {
    return 0;
  }


  // ######                                                          #
  // #     #  ######  #####      #     ####   #####      #     ####  #    #
  // #     #  #       #    #     #    #    #  #    #     #    #    # #    #
  // ######   #####   #    #     #    #    #  #    #     #    #      #    #
  // #        #       #####      #    #    #  #    #     #    #      #######
  // #        #       #   #      #    #    #  #    #     #    #    #      #
  // #        ######  #    #     #     ####   #####      #     ####       #

  inline Gitter::Geometric::Periodic4::
  Periodic4 (myhface_t * f0, int t0, myhface_t * f1, int t1)
  {
    (f [0] = f0)->attachElement (std::pair< hasFace4 *, int > (InternalHasFace4 ()(this), 0),(s [0] = t0));
    (f [1] = f1)->attachElement (std::pair< hasFace4 *, int > (InternalHasFace4 ()(this), 1),(s [1] = t1));
    return;
  }

  inline Gitter::Geometric::Periodic4::~Periodic4 () {
    f [0] ->detachElement (s [0]);
    f [1] ->detachElement (s [1]);
    return;
  }

  inline int Gitter::Geometric::Periodic4::twist (int i) const {
    alugrid_assert (0 <= i && i < 2);
    return s [i];
  }

  inline Gitter::Geometric::Periodic4::myhface_t * Gitter::Geometric::Periodic4::myhface (int i) {
    alugrid_assert (0 <= i && i < 2);
    return f [i];
  }

  inline const Gitter::Geometric::Periodic4::myhface_t * Gitter::Geometric::Periodic4::myhface (int i) const {
    alugrid_assert (0 <= i && i < 2);
    return f [i];
  }

  inline Gitter::Geometric::Periodic4::myvertex_t * Gitter::Geometric::Periodic4::myvertex (int i, int j) {
    alugrid_assert (0 <= i && i < 2);
    return (twist(i) < 0) ? myhface(i)->myvertex((9 - j + twist(i)) % 4) : myhface(i)->myvertex((j + twist(i)) % 4);
  }

  inline const Gitter::Geometric::Periodic4::myvertex_t * Gitter::Geometric::Periodic4::myvertex (int i, int j) const {
    alugrid_assert (0 <= i && i < 2);
    return (twist(i) < 0) ? myhface(i)->myvertex((9 - j + twist(i)) % 4) : myhface(i)->myvertex((j + twist(i)) % 4);
  }

  inline Gitter::Geometric::Periodic4::myvertex_t * Gitter::Geometric::Periodic4::myvertex (int i) { // ok
    alugrid_assert (0 <= i && i < 8);
    return (i < 4) ? myvertex (0, (4 - i) % 4) : myvertex (1, i - 4);
  }

  inline const Gitter::Geometric::Periodic4::myvertex_t * Gitter::Geometric::Periodic4::myvertex (int i) const { // ok
    alugrid_assert (0 <= i && i < 8);
    return (i < 4) ? myvertex (0,i) : myvertex (1,(8-i)%4);
  }

  inline std::pair< Gitter::Geometric::hasFace4 *, int > Gitter::Geometric::Periodic4::myneighbour (int i) {
    alugrid_assert (0 <= i && i < 2);
    return twist (i) < 0 ? myhface( i )->nb.front () : myhface( i )->nb.rear ();
  }

  inline std::pair< const Gitter::Geometric::hasFace4 *, int > Gitter::Geometric::Periodic4::myneighbour (int i) const {
    alugrid_assert (0 <= i && i < 2);
    return twist (i) < 0 ? std::pair< const hasFace4 *, int > (myhface( i )->nb.front ().first, myhface( i )->nb.front ().second)
      : std::pair< const hasFace4 *, int > (myhface( i )->nb.rear ().first, myhface( i )->nb.rear ().second);
  }

  inline int Gitter::Geometric::Periodic4::postRefinement () {
    return 0;
  }

  inline int Gitter::Geometric::Periodic4::preCoarsening () {
    return 0;
  }



  // #     #
  // #     #  ######  #    #    ##
  // #     #  #        #  #    #  #
  // #######  #####     ##    #    #
  // #     #  #         ##    ######
  // #     #  #        #  #   #    #
  // #     #  ######  #    #  #    #

  inline Gitter::Geometric::Hexa::
  Hexa (myhface_t * f0, int t0, myhface_t * f1, int t1,
        myhface_t * f2, int t2, myhface_t * f3, int t3,
        myhface_t * f4, int t4, myhface_t * f5, int t5)
  {
    (f [0] = f0)->attachElement (std::pair< hasFace4 *, int > (InternalHasFace4 ()(this), 0),(s [0] = t0));
    (f [1] = f1)->attachElement (std::pair< hasFace4 *, int > (InternalHasFace4 ()(this), 1),(s [1] = t1));
    (f [2] = f2)->attachElement (std::pair< hasFace4 *, int > (InternalHasFace4 ()(this), 2),(s [2] = t2));
    (f [3] = f3)->attachElement (std::pair< hasFace4 *, int > (InternalHasFace4 ()(this), 3),(s [3] = t3));
    (f [4] = f4)->attachElement (std::pair< hasFace4 *, int > (InternalHasFace4 ()(this), 4),(s [4] = t4));
    (f [5] = f5)->attachElement (std::pair< hasFace4 *, int > (InternalHasFace4 ()(this), 5),(s [5] = t5));
    return;
  }

  inline Gitter::Geometric::Hexa::~Hexa () {
    f [0] ->detachElement (s [0]);
    f [1] ->detachElement (s [1]);
    f [2] ->detachElement (s [2]);
    f [3] ->detachElement (s [3]);
    f [4] ->detachElement (s [4]);
    f [5] ->detachElement (s [5]);
    return;
  }

  inline int Gitter::Geometric::Hexa::twist (int i) const {
    alugrid_assert (i < 6);
    return s [i];
  }

  inline Gitter::Geometric::Hexa::myhface_t * Gitter::Geometric::Hexa::myhface (int i) {
    alugrid_assert (i < 6);
    return f [i];
  }

  inline const Gitter::Geometric::Hexa::myhface_t * Gitter::Geometric::Hexa::myhface (int i) const {
    alugrid_assert (i < 6);
    return f [i];
  }

  inline Gitter::Geometric::Hexa::myvertex_t * Gitter::Geometric::Hexa::myvertex (int i, int j)
  {
    return myhface(i)->myvertex(evalVertexTwist(i, j));
  }

  inline const Gitter::Geometric::Hexa::myvertex_t * Gitter::Geometric::Hexa::myvertex (int i, int j) const
  {
    return myhface(i)->myvertex(evalVertexTwist(i, j));
  }

  inline Gitter::Geometric::Hexa::myvertex_t *
  Gitter::Geometric::Hexa::myvertex (int i)
  {
    alugrid_assert (0 <= i && i < 8);
    return myvertex( vertex2Face[i][0] , vertex2Face[i][1] );
  }

  inline const Gitter::Geometric::Hexa::myvertex_t *
  Gitter::Geometric::Hexa::myvertex (int i) const
  {
    alugrid_assert (0 <= i && i < 8);
    return myvertex( vertex2Face[i][0] , vertex2Face[i][1] );
  }

  inline Gitter::Geometric::Hexa::myhedge_t * Gitter::Geometric::Hexa::myhedge(int i) {
    alugrid_assert (0 <= i && i < 12);

    typedef Gitter::Geometric::Hexa MyType;
    return myhface(MyType::edgeMap[i][0])->
      myhedge(evalEdgeTwist(MyType::edgeMap[i][0], MyType::edgeMap[i][1]));
  }

  inline const Gitter::Geometric::Hexa::myhedge_t * Gitter::Geometric::Hexa::myhedge(int i) const {
    alugrid_assert (0 <= i && i < 12);

    typedef Gitter::Geometric::Hexa MyType;
    return myhface(MyType::edgeMap[i][0])->
      myhedge(evalEdgeTwist(MyType::edgeMap[i][0], MyType::edgeMap[i][1]));
  }

  inline std::pair< Gitter::Geometric::hasFace4 *, int > Gitter::Geometric::Hexa::myneighbour (int i) {
    return twist (i) < 0 ? myhface( i )->nb.front () : myhface( i )->nb.rear ();
  }

  inline std::pair< const Gitter::Geometric::hasFace4 *, int > Gitter::Geometric::Hexa::myneighbour (int i) const {
    return twist (i) < 0 ? std::pair< const hasFace4 *, int > (myhface( i )->nb.front ().first, myhface( i )->nb.front ().second)
      : std::pair< const hasFace4 *, int > (myhface( i )->nb.rear ().first, myhface( i )->nb.rear ().second);
  }

  inline std::pair< Gitter::Geometric::hface4_GEO *, int >
  Gitter::Geometric::Hexa::myintersection ( int i )
  {
    return std::make_pair( myhface( i ), twist( i ) );
  }

  inline std::pair< const Gitter::Geometric::hface4_GEO *, int >
  Gitter::Geometric::Hexa::myintersection ( int i ) const
  {
    return std::make_pair( myhface( i ), twist( i ) );
  }

  inline int Gitter::Geometric::Hexa::postRefinement () { return 0; }
  inline int Gitter::Geometric::Hexa::preCoarsening () { return 0; }

  inline int Gitter::Geometric::Hexa::evalVertexTwist (int face, int vertex) const
  {
    alugrid_assert ( (twist(face) + 4 >= 0) && (twist(face)+4 < 8) );
    alugrid_assert ( vertex >= 0 && vertex < 4 );
    // make sure that we get the same result
    alugrid_assert ( originalVertexTwist(face,vertex) == vertexTwist[twist(face)+4][vertex] );
    return vertexTwist[twist(face)+4][vertex];
  }

  inline int Gitter::Geometric::Hexa::evalEdgeTwist (int face, int edge) const
  {
    alugrid_assert ( (twist(face) + 4 >= 0) && (twist(face)+4 < 8) );
    alugrid_assert ( edge >= 0 && edge < 4 );
    // make sure that we get the same result
    alugrid_assert ( originalEdgeTwist(face,edge) == edgeTwist[twist(face)+4][edge] );
    return edgeTwist[twist(face)+4][edge];
  }


  inline int Gitter::Geometric::Hexa::originalVertexTwist (int face, int vertex) const
  {
    return (twist(face) < 0 ?
            (9 - vertex + twist(face)) % 4 :
            (vertex + twist(face)) % 4);
  }

  inline int Gitter::Geometric::Hexa::originalEdgeTwist (int face, int edge) const {
    return (twist(face) < 0 ?
            (8 - edge + twist(face)) % 4 :
            (edge + twist(face)) % 4);
  }


  // #     #                                                  #####
  // #     #  #####   #    #  #####    ####   ######   ####  #     #
  // #     #  #    #  ##   #  #    #  #       #       #    #       #
  // #######  #####   # #  #  #    #   ####   #####   #       #####
  // #     #  #    #  #  # #  #    #       #  #       #  ###       #
  // #     #  #    #  #   ##  #    #  #    #  #       #    # #     #
  // #     #  #####   #    #  #####    ####   ######   ####   #####

  inline Gitter::Geometric::hbndseg3::
  hbndseg3 (myhface_t * a, int b)
    : _face( a ),
      _twist (b)
  {
    _face->attachElement (std::pair< hasFace3 *, int > (InternalHasFace3 ()(this),0), _twist);
    return;
  }

  inline Gitter::Geometric::hbndseg3::~hbndseg3 () {
    _face->detachElement (_twist);
    return;
  }

  inline void Gitter::Geometric::hbndseg3::attachleafs ()
  {
    this->addleaf();

    myhface_t & face = *(myhface(0));
    face.addleaf();
    for (int i=0; i<3; ++i)
    {
      face.myhedge(i)->addleaf();
      face.myvertex(i)->addleaf();
    }
  }

  inline void Gitter::Geometric::hbndseg3::detachleafs ()
  {
    this->removeleaf();

    myhface_t & face = *(myhface(0));
    face.removeleaf();
    for (int i=0; i<3; ++i)
    {
      face.myhedge(i)->removeleaf();
      face.myvertex(i)->removeleaf();
    }
  }

  inline int Gitter::Geometric::hbndseg3::postRefinement ()
  {
    // don't project vertices on internal borders
    // except for 2d fake boundaries on surface grids
    if( this->bndtype() > closure_2d ) return 0;

    if ( hasVertexProjection() )
    {
      alugrid_assert( this->bndtype() == closure_2d ?
                        this->projectionType() == ProjectVertex :: surface : true );
      myhface(0)->projectVertex( projection() );
    }

    return 0;
  }

  inline int Gitter::Geometric::hbndseg3::preCoarsening ()
  {
    return 0;
  }

  inline int Gitter::Geometric::hbndseg3::twist (int i) const {
    alugrid_assert (i == 0);
    return _twist;
  }

  inline Gitter::Geometric::hbndseg3::myhface_t * Gitter::Geometric::hbndseg3::myhface (int i) const {
    alugrid_assert (i == 0);
    return _face;
  }

  inline Gitter::Geometric::hbndseg3::myvertex_t * Gitter::Geometric::hbndseg3::myvertex (int,int j) const {
    return (twist (0) < 0) ? myhface(0)->myvertex ((7 - j + twist (0)) % 3) : myhface(0)->myvertex ((j + twist (0)) % 3);
  }

  inline Gitter::Geometric::hbndseg3::myhface_t * Gitter::Geometric::hbndseg3::subface (int,int i) const {
    return myhface(0)->subface (i);
  }

  inline Gitter::Geometric::hbndseg3::myrule_t Gitter::Geometric::hbndseg3::getrule () const {
    return myhface(0)->getrule ();
  }

  inline int Gitter::Geometric::hbndseg3::nChild () const {
    alugrid_assert (_face);
    return _face->nChild ();
  }


  // #     #                                                 #
  // #     #  #####   #    #  #####    ####   ######   ####  #    #
  // #     #  #    #  ##   #  #    #  #       #       #    # #    #
  // #######  #####   # #  #  #    #   ####   #####   #      #    #
  // #     #  #    #  #  # #  #    #       #  #       #  ### #######
  // #     #  #    #  #   ##  #    #  #    #  #       #    #      #
  // #     #  #####   #    #  #####    ####   ######   ####       #

  inline Gitter::Geometric::hbndseg4::hbndseg4 (myhface_t * a, int b)
    : _face( a ),
      _twist (b)
  {
    _face->attachElement (std::pair< hasFace4 *, int > (InternalHasFace4 ()(this),0), _twist);
    return;
  }

  inline Gitter::Geometric::hbndseg4::~hbndseg4 () {
    _face->detachElement (_twist);
    return;
  }

  inline void Gitter::Geometric::hbndseg4::attachleafs ()
  {
    alugrid_assert (this->leafRefCount()==0);
    this->addleaf();

    myhface_t& face = *(myhface(0));
    face.addleaf();
    for (int i=0; i<4; ++i)
    {
      face.myhedge(i)->addleaf();
      face.myvertex(i)->addleaf();
    }
  }

  inline void Gitter::Geometric::hbndseg4::detachleafs ()
  {
    alugrid_assert (this->leafRefCount()==1);
    this->removeleaf();

    myhface_t& face = *(myhface(0));
    face.removeleaf();
    for (int i=0; i<4; ++i)
    {
      face.myhedge(i)->removeleaf();
      face.myvertex(i)->removeleaf();
    }
  }

  inline int Gitter::Geometric::hbndseg4::postRefinement ()
  {
    // don't project vertices on internal borders
    // except for 2d fake boundaries on surface grids
    if( this->bndtype() > closure_2d ) return 0;

    if( hasVertexProjection() )
    {
      alugrid_assert( this->bndtype() == closure_2d ?
                        this->projectionType() == ProjectVertex :: surface : true );
      myhface(0)->projectVertex( projection() );
    }
    return 0;
  }

  inline int Gitter::Geometric::hbndseg4::preCoarsening ()
  {
    return 0;
  }

  inline int Gitter::Geometric::hbndseg4::twist (int i) const
  {
    alugrid_assert (i == 0);
    return _twist;
  }

  inline Gitter::Geometric::hbndseg4::myhface_t * Gitter::Geometric::hbndseg4::myhface (int i) const {
    alugrid_assert (i == 0);
    return _face;
  }

  inline Gitter::Geometric::hbndseg4::myvertex_t * Gitter::Geometric::hbndseg4::myvertex (int,int j) const {
    return (twist (0) < 0) ? myhface(0)->myvertex ((9 - j + twist (0)) % 4) : myhface(0)->myvertex ((j + twist (0)) % 4);
  }

  inline Gitter::Geometric::hbndseg4::myhface_t * Gitter::Geometric::hbndseg4::subface (int,int i) const {
    return myhface(0)->subface (i);
  }

  inline Gitter::Geometric::hbndseg4::myrule_t Gitter::Geometric::hbndseg4::getrule () const {
    return myhface(0)->getrule ();
  }

  inline int Gitter::Geometric::hbndseg4::nChild () const {
    alugrid_assert (_face);
    return _face->nChild ();
  }


  // #                                 ###
  // #        ######    ##    ######    #      #####  ######  #####     ##     #####   ####   #####
  // #        #        #  #   #         #        #    #       #    #   #  #      #    #    #  #    #
  // #        #####   #    #  #####     #        #    #####   #    #  #    #     #    #    #  #    #
  // #        #       ######  #         #        #    #       #####   ######     #    #    #  #####
  // #        #       #    #  #         #        #    #       #   #   #    #     #    #    #  #   #
  // #######  ######  #    #  #        ###       #    ######  #    #  #    #     #     ####   #    #

  template < class A > inline LeafIterator < A >::LeafIterator () : _grd (0), _w (0) {
    return;
  }

  template < class A > inline LeafIterator < A >::LeafIterator (Gitter & g)
    : _grd (&g), _w (0) , _a(0)
  {
    _grd->ref ++;
    _w = _grd->iterator (_a);
    return;
  }

  template < class A > inline LeafIterator < A >::LeafIterator (const LeafIterator < A > & x) : _grd(0), _w(0), _a(0)
  {
    assign(x);
    return;
  }

  template < class A > inline LeafIterator < A > &
  LeafIterator < A >::operator = (const LeafIterator < A > & x)
  {
    removeObj();
    assign(x);
    return *this;
  }

  template < class A > inline LeafIterator < A >::~LeafIterator () {
    removeObj();
    return;
  }

  template < class A > inline void LeafIterator < A >::removeObj ()
  {
    if (_grd)
    {
      _grd->ref --;
      _grd = 0;
    }
    if(_w) delete _w;
    _w = 0;
  }

  template < class A > inline void LeafIterator < A >::assign (const LeafIterator < A > & x)
  {
    alugrid_assert ( _grd == 0 );
    alugrid_assert ( _w   == 0 );
    _grd = x._grd;
    _grd->ref ++;
    alugrid_assert ( x._w );
    _w = x._w->clone();
  }

  template < class A > inline IteratorSTI < A > * LeafIterator < A >::operator -> () const {
    return _w;
  }

  template < class A > inline IteratorSTI < A > & LeafIterator < A >::operator * () const {
    return * _w;
  }

  ////////////////////////////////////////////////////////////////////////////////////
  //
  //  --GridIterator
  //
  ///////////////////////////////////////////////////////////////////////////////////

  template < class A , class StopRule_t >
  inline GridIterator < A , StopRule_t >::GridIterator () : _grd (0), _w (0) {
    return;
  }

  template < class A , class StopRule_t >
  inline GridIterator < A , StopRule_t >::
  GridIterator (Gitter & g , const StopRule_t rule )
    : _grd (&g), _w (0) , _a(0)
  {
    _grd->ref ++;
    _w = _grd->createIterator(_a,rule);
    return;
  }

  template < class A , class StopRule_t >
  inline GridIterator < A , StopRule_t >::GridIterator (const GridIterator < A , StopRule_t > & x)
    : _grd(0), _w(0), _a(0)
  {
    assign(x);
    return;
  }

  template < class A , class StopRule_t >
  inline GridIterator < A , StopRule_t > &
  GridIterator < A , StopRule_t >::operator = (const GridIterator < A , StopRule_t > & x)
  {
    removeObj();
    assign(x);
    return *this;
  }

  template < class A , class StopRule_t >
  inline GridIterator < A , StopRule_t >::~GridIterator () {
    removeObj();
    return;
  }

  template < class A , class StopRule_t >
  inline void GridIterator < A , StopRule_t >::removeObj ()
  {
    if (_grd)
    {
      _grd->ref --;
      _grd = 0;
    }
    if(_w)
    {
      delete _w;
      _w = 0;
    }
  }

  template < class A , class StopRule_t >
  inline void GridIterator < A , StopRule_t >::
  assign (const GridIterator < A , StopRule_t > & x)
  {
    alugrid_assert ( _grd == 0 );
    alugrid_assert ( _w   == 0 );
    _grd = x._grd;
    _grd->ref ++;
    alugrid_assert ( x._w );
    _w = x._w->clone();
  }

  template < class A , class StopRule_t >
  inline IteratorSTI < A > *
  GridIterator < A , StopRule_t >::operator -> () const {
    return _w;
  }

  template < class A , class StopRule_t >
  inline IteratorSTI < A > &
  GridIterator < A , StopRule_t >::operator * () const {
    return * _w;
  }

  //////////////////////////////////////////////////////////////////////////////
  //
  //  --LevelIterator
  //
  //////////////////////////////////////////////////////////////////////////////

  template < class A > inline LevelIterator < A >::LevelIterator () : _grd (0), _w (0) {
    return;
  }

  template < class A > inline LevelIterator < A >::LevelIterator (Gitter & g , int l ) : _grd (&g), _ahl (l) , _w (0) , _a(0)
  {
    _grd->ref ++;
    _w = _grd->levelIterator (_a,_ahl);
    return;
  }

  template < class A > inline LevelIterator < A >::LevelIterator (const LevelIterator < A > & x)
  : _grd (0), _ahl(x._ahl), _w (0) , _a(0)
  {
    assign(x);
    return;
  }

  template < class A > inline LevelIterator < A > &
  LevelIterator < A >::operator = (const LevelIterator < A > & x)
  {
    removeObj();
    assign(x);
    return *this;
  }


  template < class A > inline LevelIterator < A >::~LevelIterator ()
  {
    removeObj();
    return;
  }

  template < class A > inline IteratorSTI < A > * LevelIterator < A >::operator -> () const
  {
    return _w;
  }

  template < class A > inline IteratorSTI < A > & LevelIterator < A >::operator * () const
  {
    return * _w;
  }

  template < class A > inline void LevelIterator < A >::removeObj ()
  {
    if (_grd) _grd->ref --;
    _grd = 0;
    if(_w) delete _w;
    _w = 0;
  }

  template < class A > inline void LevelIterator < A >::assign (const LevelIterator < A > & x)
  {
    alugrid_assert ( _grd == 0 );
    alugrid_assert ( _w == 0 );
    _grd = x._grd;
    _grd->ref ++;
    alugrid_assert ( x._w );
    _w = x._w->clone();
  }

} // namespace ALUGrid

#endif // #ifndef GITTER_STI_H_INCLUDED
