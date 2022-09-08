// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:
#ifndef DUNE_ALUGRID_MACROGRIDVIEW_HH
#define DUNE_ALUGRID_MACROGRIDVIEW_HH

#include <type_traits>

#include <dune/common/version.hh>
#include <dune/common/exceptions.hh>

#include <dune/grid/common/capabilities.hh>
#include <dune/grid/common/gridview.hh>

namespace Dune
{

  template< class GridImp, PartitionIteratorType pitype >
  class MacroGridView;

  template< class GridImp, PartitionIteratorType pitype >
  struct MacroGridViewTraits
  {
    typedef MacroGridView< GridImp, pitype > GridViewImp;

    /** \brief type of the grid */
    typedef typename std::remove_const<GridImp>::type Grid;

    /** \brief type of the index set */
    typedef typename Grid :: Traits :: LevelIndexSet IndexSet;

    /** \brief type of the intersection */
    typedef typename Grid :: Traits :: LevelIntersection Intersection;

    /** \brief type of the intersection iterator */
    typedef typename Grid :: Traits :: LevelIntersectionIterator
      IntersectionIterator;

    /** \brief type of the communication */
    typedef typename Grid :: Traits :: Communication Communication;

    template< int cd >
    struct Codim
    {
      typedef typename Grid :: Traits
        :: template Codim< cd > :: template Partition< pitype > :: LevelIterator
        Iterator;

      typedef typename Grid :: Traits :: template Codim< cd > :: Entity Entity;

      typedef typename Grid :: template Codim< cd > :: Geometry Geometry;
      typedef typename Grid :: template Codim< cd > :: LocalGeometry
        LocalGeometry;

      /** \brief Define types needed to iterate over entities of a given partition type */
      template< PartitionIteratorType pit >
      struct Partition
      {
        /** \brief iterator over a given codim and partition type */
        typedef typename Grid :: template Codim< cd >
          :: template Partition< pit > :: LevelIterator
          Iterator;
      };
    };

    enum { conforming = Capabilities :: isLevelwiseConforming< Grid > :: v };
  };


  template< class GridImp, PartitionIteratorType pitype >
  class MacroGridView
  {
    typedef MacroGridView< GridImp, pitype > ThisType;

  public:


    typedef MacroGridViewTraits< GridImp, pitype > Traits;

    /** \brief type of the grid */
    typedef typename Traits::Grid Grid;

    /** \brief type of the index set */
    typedef typename Traits :: IndexSet IndexSet;

    /** \brief type of the intersection */
    typedef typename Traits :: Intersection Intersection;

    /** \brief type of the intersection iterator */
    typedef typename Traits :: IntersectionIterator IntersectionIterator;

    /** \brief type of the communication */
    typedef typename Traits :: Communication Communication;

    /** \brief Codim Structure */
    template< int cd >
    struct Codim : public Traits :: template Codim<cd> {};

    enum {
      //! \brief Export if this grid view is conforming */
      conforming = Traits :: conforming
    };

    /** \brief type used for coordinates in grid */
    typedef typename Grid::ctype ctype;

    enum { //! \brief The dimension of the grid
      dimension = Grid :: dimension
    };

    enum { //! \brief The dimension of the world the grid lives in.
      dimensionworld = Grid :: dimensionworld
    };

    MacroGridView ( const Grid &grid )
    : grid_( &grid ),
      level_( 0 )
    {}

    /** \brief obtain a const reference to the underlying hierarchic grid */
    const Grid &grid () const
    {
      assert( grid_ );
      return *grid_;
    }

    /** \brief obtain the index set */
    const IndexSet &indexSet () const
    {
      return grid().levelIndexSet( level_ );
    }

    /** \brief return true if current state of grid view represents a conforming grid */
    bool isConforming() const
    {
      // the macro level is always conforming
      return true;
    }

    /** \brief obtain number of entities in a given codimension */
    int size ( int codim ) const
    {
      return grid().size( level_, codim );
    }

    /** \brief obtain number of entities with a given geometry type */
    int size ( const GeometryType &type ) const
    {
      return grid().size( level_, type );
    }

    /** \brief obtain begin iterator for this view */
    template< int cd >
    typename Codim< cd > :: Iterator begin () const
    {
      return grid().template lbegin< cd, pitype >( level_ );
    }

    /** \brief obtain begin iterator for this view */
    template< int cd, PartitionIteratorType pit >
    typename Codim< cd > :: template Partition< pit > :: Iterator begin () const
    {
      return grid().template lbegin< cd, pit >( level_ );
    }

    /** \brief obtain end iterator for this view */
    template< int cd >
    typename Codim< cd > :: Iterator end () const
    {
      return grid().template lend< cd, pitype >( level_ );
    }

    /** \brief obtain end iterator for this view */
    template< int cd, PartitionIteratorType pit >
    typename Codim< cd > :: template Partition< pit > :: Iterator end () const
    {
      return grid().template lend< cd, pit >( level_ );
    }

    /** \brief obtain begin intersection iterator with respect to this view */
    IntersectionIterator
    ibegin ( const typename Codim< 0 > :: Entity &entity ) const
    {
      assert( entity.level() == level_ );
      return grid().ilevelbegin( entity );
    }

    /** \brief obtain end intersection iterator with respect to this view */
    IntersectionIterator
    iend ( const typename Codim< 0 > :: Entity &entity ) const
    {
      assert( entity.level() == level_ );
      return grid().ilevelend( entity );
    }

    /** \brief obtain communication object */
    const Communication &comm () const
    {
      return grid().comm();
    }

    /** \brief Return size of the overlap region for a given codim on the grid view.  */
    int overlapSize(int codim) const
    {
      return grid().overlapSize(level_, codim);
    }

    /** \brief Return size of the ghost region for a given codim on the grid view.  */
    int ghostSize(int codim) const
    {
      return grid().ghostSize(level_, codim);
    }

    /** communicate data on this view */
    template< class DataHandleImp, class DataType >
    void communicate ( CommDataHandleIF< DataHandleImp, DataType > &data,
                       InterfaceType iftype,
                       CommunicationDirection dir ) const
    {
      return grid().communicate( data, iftype, dir, level_ );
    }

    ////////////////////////////////////////////////////////////////////////////
    //
    // extra interface methods for load balancing
    //
    ////////////////////////////////////////////////////////////////////////////

    /** \brief return master rank for entities with partitionType != InteriorEntity */
    int master ( const typename Codim< 0 > :: Entity &entity ) const
    {
      return entity.impl().master();
    }

    /** \brief return unique id of macro entity for usage with graph partitioning software */
    int macroId ( const typename Codim< 0 > :: Entity &entity ) const
    {
      return entity.impl().macroId();
    }

    /** \brief return weight associated with the given macro entity */
    int weight ( const typename Codim< 0 > :: Entity &entity ) const
    {
      return entity.impl().weight();
    }

    /** \brief return weight associated with the macro intersection,
               i.e. the graph edge between the two neighboring entities */
    int weight ( const Intersection &intersection ) const
    {
      return intersection.impl().weight();
    }

  protected:
    const Grid *grid_;
    const int level_;
  };
}

#endif
