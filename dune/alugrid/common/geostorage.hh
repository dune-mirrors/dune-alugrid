#ifndef DUNE_ALUGRIDGEOMETRYSTORAGE_HH
#define DUNE_ALUGRIDGEOMETRYSTORAGE_HH

#include <array>
#include <memory>

#include <dune/common/exceptions.hh>

#include <dune/grid/common/grid.hh>
#include <dune/grid/common/gridfactory.hh>

#include <dune/alugrid/common/declaration.hh>
#include <dune/alugrid/3d/alu3dinclude.hh>

namespace Dune
{
  template< class Grid >
  class ReferenceGridFactory;


  template< class GridImp, class GeometryImpl, int nChild >
  class ALULocalGeometryStorage
  {
    typedef ALULocalGeometryStorage< GridImp, GeometryImpl, nChild > ThisType;

    // array with pointers to the geometries
    std::array< GeometryImpl, nChild > geoms_;

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
    struct CreateGeometries<dummy, 2, dimworld, ALU3DSPACE triangle >
    {
      template <class Storage>
      static inline void createGeometries(Storage& storage,
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

    template <int dummy, int dimworld>
    struct CreateGeometries<dummy, 2, dimworld, ALU3DSPACE tetra >
    {
      template <class Storage>
      static inline void createGeometries(Storage& storage,
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
      static inline void createGeometries(Storage& storage,
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
    struct CreateGeometries<dummy, 2, dimworld, ALU3DSPACE quadrilateral >
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

    template <int dummy, int dimworld>
    struct CreateGeometries<dummy, 2, dimworld, ALU3DSPACE hexa >
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

  public:
    // create empty storage
    inline ALULocalGeometryStorage ( const GeometryType type, const bool nonConform )
    : count_( 0 ), initialized_( false )
    {
      // initialize geometries
      initialize( type, nonConform );
    }

    // create empty storage
    inline ALULocalGeometryStorage ()
    : count_( 0 ), initialized_( false )
    {
    }

    // return reference to local geometry
    inline const GeometryImpl& operator [] (int child) const
    {
      alugrid_assert ( geomCreated(child) );
      // this method is not thread safe yet
      return geoms_[child];
    }

    //! access local geometry storage
    static inline const ThisType& storage( const GeometryType type, const bool nonConforming )
    {
      if( type.isSimplex() )
      {
        // create static variable for this thread
        static ThisType simplexGeoms( type, nonConforming );
        return simplexGeoms ;
      }
      else
      {
        // should be a cube geometry a this point
        alugrid_assert( type.isCube() );

        // create static variable
        static ThisType cubeGeoms( type, nonConforming );
        return cubeGeoms ;
      }
    }

  protected:
    // check if geometry has been created
    inline bool geomCreated(int child) const { return geoms_[child].valid(); }

    //! initialize local geometries
    inline bool initialize( const GeometryType type, const bool nonConform )
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

    template < class Grid >
    inline void createGeometries(const GeometryType& type)
    {
      static bool firstCall = true ;
      if( firstCall )
      {
        firstCall = false ;

        // create factory for the reference element grid
        ReferenceGridFactory< Grid > factory;

        const auto& refElem =
          Dune::ReferenceElements< ctype, dimension >::general( type );

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

        std::unique_ptr< Grid > gridPtr( factory.createGrid() );
        Grid& grid = *gridPtr;

        // refine once to get children in the reference element
        const int level = 1;
        grid.globalRefine( level );

        {
          typedef typename Grid :: MacroGridView MacroGridView;
          MacroGridView macroView = grid.template macroGridView< All_Partition > ();
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
      }
    }

    // create local geometry
    template< class Geometry >
    inline void create ( const Geometry &father,
                  const Geometry &son,
                  const int child )
    {
      alugrid_assert ( !geomCreated( child ) );
      alugrid_assert ( (child >= 0) && (child < nChild) );

      alugrid_assert ( (count_ < nChild) );
      ++count_;

      geoms_[ child ].buildGeomInFather( father, son );
    }

  };

} // namespace Dune

#endif // #ifndef DUNE_ALUGRIDGEOMETRYSTORAGE_HH
