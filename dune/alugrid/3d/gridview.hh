#ifndef DUNE_ALUGRID_3D_GRIDVIEW_HH
#define DUNE_ALUGRID_3D_GRIDVIEW_HH

#include <type_traits>

#include <dune/common/version.hh>
#include <dune/common/exceptions.hh>

#include <dune/grid/common/capabilities.hh>
#include <dune/grid/common/gridview.hh>

namespace Dune
{

  template< class GridImp, PartitionIteratorType pitype >
  class ALU3dLevelGridView;

  template< class GridImp, PartitionIteratorType pitype >
  class ALU3dLeafGridView;


  template< class GridImp, PartitionIteratorType pitype >
  struct ALU3dLevelGridViewTraits
  {
    typedef ALU3dLevelGridView< GridImp, pitype > GridViewImp;

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
    typedef Communication CollectiveCommunication;

    template< int cd >
    struct Codim
    {
      typedef typename Grid::Traits::template Codim< cd >::Twists Twists;
      typedef typename Twists::Twist Twist;

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
  class ALU3dLevelGridView
  {
    typedef ALU3dLevelGridView< GridImp, pitype > ThisType;

  public:
    typedef ALU3dLevelGridViewTraits<GridImp,pitype> Traits;

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
    typedef Communication CollectiveCommunication;

    /** \brief Codim Structure */
    template< int cd >
    struct Codim : public Traits :: template Codim<cd> {};

    enum { conforming = Traits :: conforming };

    ALU3dLevelGridView ( const Grid &grid, int level )
      : grid_( &grid ),
        level_( level )
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
      // macro level is always conforming, otherwise the level view is
      // conforming if non-conforming refinement is used
      return level_ == 0 ? true : ! grid().conformingRefinement();
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
      return grid().ilevelbegin( entity );
    }

    /** \brief obtain end intersection iterator with respect to this view */
    IntersectionIterator
    iend ( const typename Codim< 0 > :: Entity &entity ) const
    {
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
    template< class DataHandle, class Data >
    typename Grid::LevelCommunication communicate ( CommDataHandleIF< DataHandle, Data > &data,
                                                   InterfaceType iftype,
                                                   CommunicationDirection dir ) const
    {
      return grid().communicate( data, iftype, dir, level_ );
    }

    template< int cd >
    typename Codim< cd >::Twists twists ( GeometryType type ) const
    {
      return grid().template twists< cd >( type );
    }

  private:
    const Grid *grid_;
    int level_;
  };


  template< class GridImp, PartitionIteratorType pitype >
  struct ALU3dLeafGridViewTraits {
    typedef ALU3dLeafGridView< GridImp, pitype > GridViewImp;

    /** \brief type of the grid */
    typedef typename std::remove_const<GridImp>::type Grid;

    /** \brief type of the index set */
    typedef typename Grid :: Traits :: LeafIndexSet IndexSet;

    /** \brief type of the intersection */
    typedef typename Grid :: Traits :: LeafIntersection Intersection;

    /** \brief type of the intersection iterator */
    typedef typename Grid :: Traits :: LeafIntersectionIterator
    IntersectionIterator;

    /** \brief type of the communication */
    typedef typename Grid :: Traits :: Communication Communication;
    typedef Communication CollectiveCommunication;

    template< int cd >
    struct Codim
    {
      typedef typename Grid::Traits::template Codim< cd >::Twists Twists;
      typedef typename Twists::Twist Twist;

      typedef typename Grid :: Traits
      :: template Codim< cd > :: template Partition< pitype > :: LeafIterator
      Iterator;

      typedef typename Grid :: Traits :: template Codim< cd > :: Entity Entity;

      typedef typename Grid :: template Codim< cd > :: Geometry Geometry;
      typedef typename Grid :: template Codim< cd > :: LocalGeometry
      LocalGeometry;

      /** \brief Define types needed to iterate over entities of a given partition type */
      template <PartitionIteratorType pit >
      struct Partition
      {
        /** \brief iterator over a given codim and partition type */
        typedef typename Grid :: template Codim< cd >
        :: template Partition< pit > :: LeafIterator
        Iterator;
      };
    };

    enum { conforming = Capabilities :: isLeafwiseConforming< Grid > :: v };
  };


  template< class GridImp, PartitionIteratorType pitype >
  class ALU3dLeafGridView
  {
    typedef ALU3dLeafGridView< GridImp, pitype > ThisType;

  public:
    typedef ALU3dLeafGridViewTraits<GridImp,pitype> Traits;

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
    typedef Communication CollectiveCommunication;

    /** \brief Codim Structure */
    template< int cd >
    struct Codim : public Traits :: template Codim<cd> {};

    enum { conforming = Traits :: conforming };

  public:
    ALU3dLeafGridView ( const Grid &grid )
      : grid_( &grid )
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
      return grid().leafIndexSet();
    }

    /** \brief return true if current state of grid view represents a conforming grid */
    bool isConforming() const { return grid().conformingRefinement(); }

    /** \brief obtain number of entities in a given codimension */
    int size ( int codim ) const
    {
      return grid().size( codim );
    }

    /** \brief obtain number of entities with a given geometry type */
    int size ( const GeometryType &type ) const
    {
      return grid().size( type );
    }

    /** \brief obtain begin iterator for this view */
    template< int cd >
    typename Codim< cd > :: Iterator begin () const
    {
      return grid().template leafbegin< cd, pitype >();
    }

    /** \brief obtain begin iterator for this view */
    template< int cd, PartitionIteratorType pit >
    typename Codim< cd > :: template Partition< pit > :: Iterator begin () const
    {
      return grid().template leafbegin< cd, pit >();
    }

    /** \brief obtain end iterator for this view */
    template< int cd >
    typename Codim< cd > :: Iterator end () const
    {
      return grid().template leafend< cd, pitype >();
    }

    /** \brief obtain end iterator for this view */
    template< int cd, PartitionIteratorType pit >
    typename Codim< cd > :: template Partition< pit > :: Iterator end () const
    {
      return grid().template leafend< cd, pit >();
    }

    /** \brief obtain begin intersection iterator with respect to this view */
    IntersectionIterator
    ibegin ( const typename Codim< 0 > :: Entity &entity ) const
    {
      return grid().ileafbegin( entity );
    }

    /** \brief obtain end intersection iterator with respect to this view */
    IntersectionIterator
    iend ( const typename Codim< 0 > :: Entity &entity ) const
    {
      return grid().ileafend( entity );
    }

    /** \brief obtain communication object */
    const Communication &comm () const
    {
      return grid().comm();
    }

    /** \brief Return size of the overlap region for a given codim on the grid view.  */
    int overlapSize(int codim) const
    {
      return grid().overlapSize(codim);
    }

    /** \brief Return size of the ghost region for a given codim on the grid view.  */
    int ghostSize(int codim) const
    {
      return grid().ghostSize(codim);
    }

    /** communicate data on this view */
    template< class DataHandle, class Data >
    typename Grid::LeafCommunication communicate ( CommDataHandleIF< DataHandle, Data > &data,
                                                   InterfaceType iftype,
                                                   CommunicationDirection dir ) const
    {
      return grid().communicate( data, iftype, dir );
    }

    template< int cd >
    typename Codim< cd >::Twists twists ( GeometryType type ) const
    {
      return grid().template twists< cd >( type );
    }

  private:
    const Grid *grid_;
  };

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_3D_GRIDVIEW_HH
