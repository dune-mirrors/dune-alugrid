#ifndef DUNE_ALUGRIDGEOMETRYSTORAGE_HH
#define DUNE_ALUGRIDGEOMETRYSTORAGE_HH

#include <dune/common/exceptions.hh>

#include <dune/grid/common/grid.hh>
#include <dune/grid/common/gridfactory.hh>

#include <dune/alugrid/common/declaration.hh>
#include <dune/alugrid/3d/alu3dinclude.hh>
#include <dune/alugrid/2d/alu2dinclude.hh>

namespace Dune
{
  template< class GridImp, class GeometryImpl, int nChild >
  class ALULocalGeometryStorage 
  {
    typedef ALULocalGeometryStorage< GridImp, GeometryImpl, nChild > ThisType;

    // array with pointers to the geometries
    Dune::array< GeometryImpl *, nChild > geoms_;

    // count local geometry creation
    int count_;
  
    // true if geoms have been initialized 
    bool initialized_;

    // type of grid impl 
    typedef typename GridImp :: ctype ctype; 
    enum{ dimension       = GridImp :: dimension };
    enum{ dimensionworld  = GridImp :: dimensionworld };

    template <int dummy, int dim, int dimworld, int > 
    struct CreateGeometries;

    template <int dummy, int dimworld> 
    struct CreateGeometries<dummy, 2, dimworld, ALU2DSPACE triangle >
    {
      template <class Storage> 
      static void createGeometries(Storage& storage, 
                                   const GeometryType& type,
                                   const bool nonConform )
      {
        if( nonConform ) 
        {
          typedef ALUGrid< 2, dimworld, simplex, nonconforming, ALUGridNoComm > Grid;
          storage.template createGeometries< Grid > (type);
        }
        else 
        {
          typedef ALUGrid< 2, dimworld, simplex, conforming, ALUGridNoComm > Grid;
          storage.template createGeometries< Grid > (type);
        }
      }
    };

    template <int dummy> 
    struct CreateGeometries<dummy, 3, 3, ALU3DSPACE tetra >
    {
      template <class Storage> 
      static void createGeometries(Storage& storage, 
                                   const GeometryType& type,
                                   const bool nonConform )
      {
        alugrid_assert ( nonConform ) ;
        {
          typedef ALUGrid< 3, 3, simplex, nonconforming, ALUGridNoComm > Grid;
          storage.template createGeometries< Grid > (type);
        }
        /*
         // TODO, implement this for refinement of all edges (conforming)
        else 
        {
          typedef ALUGrid< 3, 3, simplex, conforming, ALUGridNoComm > Grid;
          storage.template createGeometries< Grid > (type);
        }
        */
      }
    };

    template <int dummy, int dimworld> 
    struct CreateGeometries<dummy, 2, dimworld, ALU2DSPACE quadrilateral >
    {
      template <class Storage> 
      static void createGeometries(Storage& storage, 
                                   const GeometryType& type,
                                   const bool nonConform )
      {
        alugrid_assert ( nonConform ) ;
        {
          typedef ALUGrid< 2, dimworld, cube, nonconforming, ALUGridNoComm > Grid;
          storage.template createGeometries< Grid > (type);
        }
      }
    };

    template <int dummy> 
    struct CreateGeometries<dummy, 3, 3, ALU3DSPACE hexa >
    {
      template <class Storage> 
      static void createGeometries(Storage& storage, 
                                   const GeometryType& type,
                                   const bool nonConform )
      {
        alugrid_assert ( nonConform );
        {
          typedef ALUGrid< 3, 3, cube, nonconforming, ALUGridNoComm > Grid;
          storage.template createGeometries< Grid > (type);
        }
      }
    };

    // create empty storage
    ALULocalGeometryStorage ( const GeometryType type, const bool nonConform )
    : count_( 0 ), initialized_( false )
    {
      // nullify geometries 
      geoms_.fill( (GeometryImpl *) 0 );

      // initialize geometries 
      initialize( type, nonConform );
    }

    // create empty storage
    ALULocalGeometryStorage ()
    : count_( 0 ), initialized_( false )
    {
      // nullify geometries 
      geoms_.fill( (GeometryImpl *) 0 );
    }

  public:  
    //! initialize local geometries 
    bool initialize( const GeometryType type, const bool nonConform )
    {
      if( ! initialized_ ) 
      {
        // first set flag, because this method might be called again during 
        // creation of local geometries and then result in an infinite loop
        initialized_ = true ;

        // the idea is to create a grid containing the reference element,
        // refine once and the store the father - child relations 
        CreateGeometries<0, dimension, dimensionworld, GridImp :: elementType >
          ::createGeometries(*this, type, nonConform);
        return true;
      }
      return false;
    }

    // check if geometry has been created
    bool geomCreated(int child) const { return geoms_[child] != 0; }

    // return reference to local geometry
    const GeometryImpl& operator [] (int child) const
    {
      alugrid_assert ( geomCreated(child) );
      // this method is not thread safe yet
      assert( GridImp :: thread() == 0 );
      return *(geoms_[child]);
    }

    template < class Grid >
    void createGeometries(const GeometryType& type) 
    {
      static bool firstCall = true ;
      if( firstCall ) 
      {
        firstCall = false ;
        // create factory without verbosity 
        GridFactory< Grid > factory( false );

        const Dune::ReferenceElement< ctype, dimension > &refElem
          = Dune::ReferenceElements< ctype, dimension >::general( type );

        // insert vertices 
        FieldVector<ctype, dimensionworld> pos( 0 );
        const int vxSize = refElem.size(dimension);  
        for(int i=0; i<vxSize; ++i)
        {
          FieldVector<ctype, dimension> position = refElem.position(i, dimension );
          // copy position 
          for(int d = 0; d<dimension; ++d )
            pos[ d ] = position[ d ];

          factory.insertVertex( pos );
        }

        std::vector< unsigned int > vertices( vxSize );
        // create grid with reference element
        for(size_t i=0; i<vertices.size(); ++i) vertices[ i ] = i;
        factory.insertElement(type, vertices);

        // save original sbuf
        std::streambuf* cerr_sbuf = std::cerr.rdbuf();
        std::stringstream tempout; 
        // redirect 'cerr' to a 'fout' to avoid unnecessary output in constructors 
        std::cerr.rdbuf(tempout.rdbuf()); 

        Grid* gridPtr = factory.createGrid();
        Grid& grid    = *gridPtr; 

        // restore the original stream buffer
        std::cerr.rdbuf(cerr_sbuf); 

        //std::cerr = savecerr;
        
        // refine once to get children 
        const int level = 1;
        grid.globalRefine( level );

        {
          typedef typename Grid :: template Partition< All_Partition >:: LevelGridView MacroGridView;
          MacroGridView macroView = grid.template levelView< All_Partition > ( 0 );
          typedef typename MacroGridView :: template Codim< 0 > :: Iterator Iterator;

          Iterator it = macroView.template begin<0> (); 

          if( it == macroView.template end<0>() ) 
            DUNE_THROW(InvalidStateException,"Empty Grid, should contain at least 1 element");

          typedef typename Iterator :: Entity EntityType;

          const EntityType& entity = *it;
          const typename EntityType :: Geometry& geo = entity.geometry();
          typedef typename EntityType :: HierarchicIterator HierarchicIteratorType;
          const HierarchicIteratorType end = entity.hend( level );

          int childNum = 0;
          for( HierarchicIteratorType child = entity.hbegin( level ); 
               child != end; ++child, ++childNum ) 
          {
            create( geo, child->geometry(), childNum );
          }
        }

        // delete grid 
        delete gridPtr;
      }
    }

    // create local geometry
    template< class Geometry >
    void create ( const Geometry &father, 
                  const Geometry &son, 
                  const int child )
    {
      alugrid_assert ( !geomCreated( child ) );
      alugrid_assert ( (child >= 0) && (child < nChild) );

      alugrid_assert ( (count_ < nChild) );
      ++count_;

      geoms_[ child ] = new GeometryImpl();
      geoms_[ child ]->buildGeomInFather( father, son );
    }

  public:
    // desctructor deleteing geometries
    ~ALULocalGeometryStorage () 
    {
      for(size_t i=0; i<geoms_.size(); ++i)
        if(geoms_[i]) delete geoms_[i];
    }

    //! access local geometry storage 
    static const ThisType& storage( const GeometryType type, const bool nonConforming )
    {
      if( type.isSimplex() ) 
      {
        // create static variable on heap
        static ThisType simplexGeoms; 
        // initialize (only done once), note that this is called recursively during initialize
        // so only check geoms when they were actually really created
        if( simplexGeoms.initialize( type, nonConforming ) )
        {
          if( type != simplexGeoms[ 0 ].type() )
            DUNE_THROW(InvalidStateException,"Local geometries were not initialized");
        }
        return simplexGeoms ;
      }
      else 
      {
        // should be a cube geometry a this point 
        alugrid_assert( type.isCube() );

        // create static variable on heap
        static ThisType cubeGeoms;
        // initialize (only done once), note that this is called recursively during initialize
        // so only check geoms when they were actually really created
        if( cubeGeoms.initialize( type, nonConforming ) )
        {
          if( type != cubeGeoms[ 0 ].type() )
            DUNE_THROW(InvalidStateException,"Local geometries were not initialized");
        }
        return cubeGeoms ;
      }
    }

    //! access local geometries 
    static const GeometryImpl& geom( const GeometryType type, const bool nonConforming, const int child ) 
    {
      // this method is not thread safe yet
      assert( GridImp :: thread() == 0 );
      // create static variable on heap
      static ThisType instance( type, nonConforming );
      // make sure the geometry type is the same 
      alugrid_assert ( type == instance[ child ].type() );
      return instance[ child ];
    }
  };  

} // namespace Dune

#endif // #ifndef DUNE_ALUGRIDGEOMETRYSTORAGE_HH
