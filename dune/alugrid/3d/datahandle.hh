#ifndef DUNE_ALU3DGRIDDATAHANDLE_HH
#define DUNE_ALU3DGRIDDATAHANDLE_HH

//- system includes
#include <iostream>
#include <type_traits>

#include <dune/common/version.hh>

#include <dune/grid/common/grid.hh>
#include <dune/grid/common/adaptcallback.hh>

#include <dune/alugrid/3d/datacollectorcaps.hh>
#include <dune/alugrid/common/ldbhandleif.hh>

//- local includes
#include "alu3dinclude.hh"

namespace ALUGrid
{

  namespace detail {

    template <int dimension>
    struct Contains
    {
      //This method is called in gitter_dune_pll_impl.cc with arguments 3,codimension
      //So we have to adapt things to the user view, that writes it with
      // 2,codimension
      // return true if dim,codim combination is contained in data set

      template <class DataCollector>
      static bool contains(const DataCollector& dc, const int dim, const int cd)
      {
        //dimension is GridImp::dimension
        if(dim == dimension)
        {
          //the original call
          return dc.contains(dim,cd);
        }
        //adaptation for 2d
        else if(dimension == 2)
        {
          //we do not want to transmit edge data
          if(cd == 2)
            return false;
          else if (cd == 3)
            return dc.contains(dimension, 2);
          else
            return dc.contains(dimension, cd);
        }
        //
        else
        {
          std::cerr << "DataHandle.contains called with non-matching dim and codim" << std::endl;
          return false;
        }
      }
    };
  } // end namespace detail

  //! the corresponding interface class is defined in bsinclude.hh
  template< class GridType, class DataCollectorType, int codim >
  class GatherScatterBaseImpl
  : public GatherScatter
  {
  protected:
    enum { dimension = GridType::dimension };
    const GridType & grid_;
    typedef typename GridType::template Codim<codim>::Entity    EntityType;
    typedef typename GridType::template Codim<codim>::EntityImp RealEntityType;

    typedef typename GridType::MPICommunicatorType Comm;

    typedef Dune::ALU3dImplTraits< GridType::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::template Codim< dimension, codim >::ImplementationType ImplElementType;
    typedef typename ImplTraits::template Codim< dimension, codim >::InterfaceType HElementType;

    EntityType  & entity_;
    RealEntityType & realEntity_;

    DataCollectorType & dc_;

    const bool variableSize_;

    typedef typename GatherScatter :: ObjectStreamType ObjectStreamType;

    typedef typename DataCollectorType:: DataType DataType;

    using GatherScatter :: setData ;
    using GatherScatter :: sendData ;
    using GatherScatter :: recvData ;
    using GatherScatter :: containsItem ;

  public:
    //! Constructor
    GatherScatterBaseImpl(const GridType & grid, EntityType & en,
        RealEntityType & realEntity , DataCollectorType & dc)
      : grid_(grid), entity_(en), realEntity_(realEntity) , dc_(dc)
      , variableSize_( ! dc_.fixedSize(EntityType::dimension,codim) )
    {
    }

    // return true if dim,codim combination is contained in data set
    bool contains(int dim, int cd) const
    {
      return detail::Contains< dimension >::contains( dc_, dim, cd );
    }

    // returns true, if element is contained in set of comm interface
    // this method must be overlaoded by the impl classes
    virtual bool containsItem (const HElementType & elem) const = 0;

    // set elem to realEntity
    virtual void setElement(const HElementType & elem) = 0;

    void setData ( ObjectStreamType & str , HElementType & elem )
    {
      // one of this should be either true
      alugrid_assert ( this->containsItem( elem ) || elem.isGhost() );

      // set element and then start
      setElement(elem);

      // make sure partition type is set correct
      alugrid_assert ( elem.isGhost() == (entity_.partitionType() == Dune :: GhostEntity) );

      size_t size = getSize(str, entity_);
      // use normal scatter method
      dc_.scatter(str,entity_, size );
    }

    //! write Data of one element to stream
    void sendData ( ObjectStreamType & str , HElementType & elem )
    {
      // make sure element is contained in communication interface
      //alugrid_assert ( this->containsItem( elem ) );
      setElement(elem);

      // if varaible size, also send size
      if( variableSize_ )
      {
        size_t size = dc_.size( entity_ );
        str.write( size );
      }

      dc_.gather(str, entity_ );
    }

    //! read Data of one element from stream
    void recvData ( ObjectStreamType & str , HElementType & elem )
    {
      alugrid_assert ( this->containsItem( elem ) );
      setElement( elem );

      size_t size = getSize(str, entity_);
      dc_.scatter(str,entity_, size );
    }

  protected:
    size_t getSize(ObjectStreamType & str, EntityType & en)
    {
      if(variableSize_)
      {
        size_t size;
        str.read(size);
        return size;
      }
      else
        return dc_.size(en);
    }
  };

  //***********************************************************
  //
  //  --specialisation for codim 0
  //
  //***********************************************************

  //! the corresponding interface class is defined in alu3dinclude.hh
  template <class GridType, class DataCollectorType >
  class GatherScatterBaseImpl<GridType,DataCollectorType,0> : public GatherScatter
  {
  protected:
    enum { codim = 0 };
    enum { dimension = GridType::dimension };
    const GridType & grid_;
    typedef typename GridType::template Codim<0>::Entity       EntityType;
    typedef typename GridType::template Codim<0>::EntityImp    RealEntityType;

    typedef typename GridType::MPICommunicatorType Comm;

    typedef Dune::ALU3dImplTraits< GridType::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::template Codim< dimension, codim >::ImplementationType ImplElementType;
    typedef typename ImplTraits::template Codim< dimension, codim >::InterfaceType HElementType;

    typedef typename ImplTraits::template Codim< dimension, 1 >::InterfaceType HFaceType;

    typedef typename ImplTraits::template Codim< dimension, codim >::GhostInterfaceType HGhostType;
    typedef typename ImplTraits::template Codim< dimension, codim >::GhostImplementationType ImplGhostType;

    typedef typename ImplTraits::PllElementType PllElementType;

    EntityType& entity_;
    RealEntityType & realEntity_;

    // data handle
    DataCollectorType & dc_;

    const bool variableSize_;

    // used MessageBuffer
    typedef typename GatherScatter :: ObjectStreamType ObjectStreamType;

    // use all other containsItem from the base class
    using GatherScatter :: setData ;
    using GatherScatter :: sendData ;
    using GatherScatter :: recvData ;

  public:
    // use all other containsItem from the base class
    using GatherScatter :: containsItem ;

    //! Constructor
    GatherScatterBaseImpl(const GridType & grid, EntityType & en,
        RealEntityType & realEntity , DataCollectorType & dc)
      : grid_(grid), entity_(en), realEntity_(realEntity)
      , dc_(dc) , variableSize_ ( ! dc_.fixedSize( EntityType :: dimension, codim ))
    {}

    // This method is called in gitter_dune_pll_impl.cc with arguments 3,codimension
    // So we have to adapt things to the user view, that writes it with
    // 2,codimension
    // return true if dim,codim combination is contained in data set
    bool contains(int dim, int cd) const
    {
      return detail::Contains< dimension >::contains( dc_, dim, cd );
    }

    // return true if item might from entity belonging to data set
    virtual bool containsItem (const HElementType & elem) const
    {
      return elem.isLeafEntity();
    }

    // return true if item might from entity belonging to data set
    virtual bool containsItem (const HGhostType & ghost) const = 0;

    //! write Data of one element to stream
    void sendData ( ObjectStreamType & str , const HElementType & elem )
    {
      alugrid_assert ( this->containsItem(elem) );
      realEntity_.setElement( const_cast<HElementType &> (elem) );

      // write size in case of variable size
      writeSize( str, entity_);
      // gather data
      dc_.gather(str, entity_);
    }

    //! write Data of one ghost element to stream
    void sendData ( ObjectStreamType & str , const HGhostType& ghost)
    {
      alugrid_assert ( this->containsItem( ghost ) );

      // set ghost as entity
      realEntity_.setGhost( const_cast <HGhostType &> (ghost) );

      // write size in case of variable size
      writeSize( str, entity_);
      // gather data
      dc_.gather(str, entity_);
    }

    //! read Data of one element from stream
    void recvData ( ObjectStreamType & str , HElementType & elem )
    {
      // alugrid_assert ( this->containsItem( elem ) );
      realEntity_.setElement( elem );

      size_t size = getSize(str, entity_);
      dc_.scatter(str, entity_, size);
    }

    //! read Data of one element from stream
    void recvData ( ObjectStreamType & str , HGhostType & ghost )
    {
      alugrid_assert ( this->containsItem( ghost ) );

      // set ghost as entity
      realEntity_.setGhost( ghost );

      size_t size = getSize(str , entity_ );
      dc_.scatter(str, entity_, size );
    }

  protected:
    size_t getSize(ObjectStreamType & str, EntityType & en)
    {
      if(variableSize_)
      {
        size_t size;
        str.read(size);
        return size;
      }
      else
        return dc_.size(en);
    }

    // write variable size to stream
    void writeSize(ObjectStreamType & str, EntityType & en)
    {
      if( variableSize_ )
      {
        size_t size = dc_.size( en );
        str.write( size );
      }
    }
  };

  //! the corresponding interface class is defined in bsinclude.hh
  template< class GridType, class DataCollectorType, int codim >
  class GatherScatterLeafData
  : public GatherScatterBaseImpl< GridType, DataCollectorType, codim >
  {
    enum { dim = GridType :: dimension };

    typedef GatherScatterBaseImpl<GridType,DataCollectorType,codim> BaseType;
    typedef typename GridType::template Codim<codim>::Entity    EntityType;
    typedef typename GridType::template Codim<codim>::EntityImp RealEntityType;

    typedef typename GridType::MPICommunicatorType Comm;

    typedef Dune::ALU3dImplTraits< GridType::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::template Codim< dim, codim >::ImplementationType IMPLElementType;
    typedef typename ImplTraits::template Codim< dim, codim >::InterfaceType HElementType;

    typedef typename ImplTraits::template Codim< dim, 1 >::InterfaceType HFaceType;

    typedef typename ImplTraits::template Codim< dim, 0 >::GhostInterfaceType HGhostType;
    typedef typename ImplTraits::template Codim< dim, 0 >::GhostImplementationType ImplGhostType;

    typedef typename ImplTraits::PllElementType PllElementType;

    using BaseType :: grid_;
  public:
    // use all other containsItem methods from the base class
    using BaseType :: containsItem ;

    //! Constructor
    GatherScatterLeafData(const GridType & grid, EntityType & en,
        RealEntityType & realEntity , DataCollectorType & dc)
      : BaseType(grid,en,realEntity,dc)
    {
      // if leaf vertices are communicated,
      // make sure that vertex list is up2date
      // but only do this, if vertex data contained,
      // because the list update is expensive
      if( (codim == dim) && dc.contains(dim,codim) )
      {
        // call of this method forces update of list,
        // if list is not up to date
        grid.getLeafVertexList();
      }
    }

    // returns true, if element is contained in set of comm interface
    bool containsItem (const HElementType & elem) const
    {
      return (dim == 2 ? elem.is2d() : true) && elem.isLeafEntity();
    }

    // returns true, if element is contained in set of comm interface
    bool containsItem (const HGhostType & ghost) const
    {
      //in 2d ghosts are always 2d, as they are codim 0
      //so we do not need to adapt the switch
      return ghost.isLeafEntity();
    }

    // returns true, if interior element is contained in set of comm interface
    bool containsInterior (const HFaceType & face, PllElementType & pll) const
    {
      return (dim == 2 ? face.is2d() : true) && face.isInteriorLeaf();
    }

    // returns true, if ghost is contianed in set of comm interface
    bool containsGhost (const HFaceType & face , PllElementType & pll) const
    {
      return (dim == 2 ? face.is2d() : true) && pll.ghostLeaf();
    }

    // set elem to realEntity
    void setElement(const HElementType & elem)
    {
      this->realEntity_.setElement(elem, grid_);
    }
  };

  //! the corresponding interface class is defined in bsinclude.hh
  template <class GridType, class DataCollectorType , int codim >
  class GatherScatterLevelData
  : public GatherScatterBaseImpl<GridType,DataCollectorType,codim>
  {
    enum { dim = GridType::dimension };
    typedef GatherScatterBaseImpl<GridType,DataCollectorType,codim> BaseType;
    typedef typename GridType::template Codim<codim>::Entity    EntityType;
    typedef typename GridType::template Codim<codim>::EntityImp RealEntityType;

    typedef typename GridType::MPICommunicatorType Comm;

    typedef Dune::ALU3dImplTraits< GridType::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::template Codim< dim, codim >::ImplementationType IMPLElementType;
    typedef typename ImplTraits::template Codim< dim, codim >::InterfaceType HElementType;

    typedef typename ImplTraits::template Codim< dim, 1 >::InterfaceType HFaceType;

    typedef typename ImplTraits::template Codim< dim, 0 >::GhostInterfaceType HGhostType;
    typedef typename ImplTraits::template Codim< dim, 0 >::GhostImplementationType ImplGhostType;

    typedef typename ImplTraits::PllElementType PllElementType;

    typedef typename GridType::LevelIndexSetImp LevelIndexSetImp;

    const LevelIndexSetImp & levelSet_;
    const int level_;
  public:
    // use containsItem for ghost element from BaseType
    using BaseType :: containsItem ;

    //! Constructor
    GatherScatterLevelData(const GridType & grid, EntityType & en,
        RealEntityType & realEntity , DataCollectorType & dc,
        const LevelIndexSetImp & levelSet, const int level)
      : BaseType(grid,en,realEntity,dc) , levelSet_(levelSet) , level_(level)
    {
    }

    // returns true, if element is contained in set of comm interface
    bool containsItem (const HElementType & elem) const
    {
      return (dim == 2 ? elem.is2d() : true) && levelSet_.containsIndex(codim, elem.getIndex() );
    }

    // set elem to realEntity
    void setElement(const HElementType & elem)
    {
      this->realEntity_.setElement(elem,level_);
    }

  };

  //! the corresponding interface class is defined in bsinclude.hh
  // this class is for the 2d grid - it masks out the edgeGatherScatter
  template <class GridType, class DataCollectorType , int codim >
  class GatherScatterNoData
  : public GatherScatterBaseImpl<GridType,DataCollectorType,codim>
  {
    enum { dim = GridType::dimension };
    typedef GatherScatterBaseImpl<GridType,DataCollectorType,codim> BaseType;
    typedef typename GridType::template Codim<codim>::Entity    EntityType;
    typedef typename GridType::template Codim<codim>::EntityImp RealEntityType;

    typedef typename GridType::MPICommunicatorType Comm;

    typedef Dune::ALU3dImplTraits< GridType::elementType, Comm > ImplTraits;

    typedef typename ImplTraits::template Codim< 2, codim >::ImplementationType IMPLElementType;
    typedef typename ImplTraits::template Codim< 2, codim >::InterfaceType HElementType;
    //We want to have the real 3d no data gatherscatter so set dim to 3 here
    typedef typename ImplTraits::template Codim< 3, codim >::ImplementationType RealIMPLElementType;
    typedef typename ImplTraits::template Codim< 3, codim >::InterfaceType RealHElementType;

    typedef typename ImplTraits::template Codim< dim, 1 >::InterfaceType HFaceType;

    typedef typename ImplTraits::template Codim< dim, 0 >::GhostInterfaceType HGhostType;
    typedef typename ImplTraits::template Codim< dim, 0 >::GhostImplementationType ImplGhostType;

    typedef typename ImplTraits::PllElementType PllElementType;

    typedef typename GridType::LevelIndexSetImp LevelIndexSetImp;

  public:
    // use containsItem for ghost element from BaseType
    using BaseType :: containsItem ;

    //! Level Constructor
    GatherScatterNoData(const GridType & grid, EntityType & en,
        RealEntityType & realEntity , DataCollectorType & dc,
        const LevelIndexSetImp & levelSet, const int level)
      : BaseType(grid,en,realEntity,dc)
    {
    }

    //! Leaf Constructor
    GatherScatterNoData(const GridType & grid, EntityType & en,
        RealEntityType & realEntity , DataCollectorType & dc)
      : BaseType(grid,en,realEntity,dc)
    {
    }

    // returns true, if element is contained in set of comm interface
    bool containsItem (const HElementType & elem) const
    {
      return false;
    }

    // returns true, if element is contained in set of comm interface
    bool containsItem (const RealHElementType & elem) const
    {
      return false;
    }

    // set elem to realEntity
    void setElement(const HElementType & elem)
    {
      //we should not get here hopefully
      alugrid_assert(false);
      return;
    }

    // set elem to realEntity
    void setElement(const RealHElementType & elem)
    {
      //we should not get here hopefully
      alugrid_assert(false);
      return;
    }

  };


  //! the corresponding interface class is defined in bsinclude.hh
  template <class GridType, class DataCollectorType>
  class GatherScatterLevelData<GridType,DataCollectorType,0>
  : public GatherScatterBaseImpl<GridType,DataCollectorType,0>
  {
    enum { codim = 0 };
    enum { dim  = GridType:: dimension };
    typedef GatherScatterBaseImpl<GridType,DataCollectorType,codim> BaseType;
    typedef typename GridType::template Codim<codim>::Entity     EntityType;
    typedef typename GridType::template Codim<codim>::EntityImp  RealEntityType;

    typedef typename GridType::MPICommunicatorType Comm;

    typedef Dune::ALU3dImplTraits< GridType::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::template Codim< dim, codim >::ImplementationType IMPLElementType;
    typedef typename ImplTraits::template Codim< dim, codim >::InterfaceType HElementType;

    typedef typename ImplTraits::template Codim< dim, 1 >::InterfaceType HFaceType;

    typedef typename ImplTraits::template Codim< dim, 0 >::GhostInterfaceType HGhostType;
    typedef typename ImplTraits::template Codim< dim, 0 >::GhostImplementationType ImplGhostType;

    typedef typename ImplTraits::PllElementType PllElementType;

    typedef typename GridType::LevelIndexSetImp LevelIndexSetImp;

    const LevelIndexSetImp & levelSet_;
    const int level_;
  public:
    //! Constructor
    GatherScatterLevelData(const GridType & grid, EntityType & en,
        RealEntityType & realEntity , DataCollectorType & dc,
        const LevelIndexSetImp & levelSet, const int level)
      : BaseType(grid,en,realEntity,dc) , levelSet_(levelSet) , level_(level) {}

    // returns true, if element is contained in set of comm interface
    bool containsItem (const HElementType & elem) const
    {
      return levelSet_.containsIndex(codim, elem.getIndex() );
    }

    // returns true, if element is contained in set of comm interface
    bool containsItem (const HGhostType & ghost) const
    {
      alugrid_assert ( ghost.getGhost().first );
      return containsItem( * (ghost.getGhost().first) );
    }

    // returns true, if interior element is contained in set of comm interface
    bool containsInterior (const HFaceType & face, PllElementType & pll) const
    {
      // if face level is not level_ then interior cannot be contained
      if(face.level() != level_) return false;

      typedef Gitter::helement_STI HElementType;
      typedef Gitter::hbndseg_STI HBndSegType;

      // check interior element here, might have a coarser level
      std::pair< HElementType *, HBndSegType * > p( (HElementType *)0, (HBndSegType *)0 );
      pll.getAttachedElement( p );
      alugrid_assert ( p.first );
      // check inside level
      bool contained = (p.first->level() == level_);
      alugrid_assert ( contained == this->containsItem( *p.first ));
      return contained;
    }

    // returns true, if ghost is contianed in set of comm interface
    bool containsGhost (const HFaceType & face, PllElementType & pll) const
    {
      // if face level is not level_ then ghost cannot be contained
      if(face.level() != level_) return false;
      // otherwise check ghost level
      return (pll.ghostLevel() == level_);
    }
  };


  ////////////////////////////////////////////////////////////////////////////////////////////
  //
  // --GatherScatterLoadBalance: ALU data handle implementation for user defined load balance
  //
  ////////////////////////////////////////////////////////////////////////////////////////////
  template <class GridType, class LoadBalanceHandleType, bool useExternal>
  class GatherScatterLoadBalance : public GatherScatter
  {
  protected:
    typedef typename GridType::MPICommunicatorType Comm;

    typedef Dune::ALU3dImplTraits< GridType::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::template Codim< GridType::dimension,  0 >::InterfaceType  HElementType;

    typedef typename GridType :: template Codim< 0 > :: Entity     EntityType ;
    typedef typename GridType :: template Codim< 0 > :: EntityImp  EntityImpType ;

    template < bool useHandlerOpts, typename D = void>
    struct UseExternalHandlerOpts
    {
      bool importRank( const LoadBalanceHandleType &lb,
                       std::set<int>& ranks ) const
      {
        return lb.importRanks( ranks );
      }
      int destination( const LoadBalanceHandleType &lb,
                       const EntityType& entity ) const
      {
        return lb( entity );
      }
      int loadWeight( const LoadBalanceHandleType &lb,
                      const EntityType& entity ) const
      {
        return lb( entity );
      }
    };
    template <typename D>
    struct UseExternalHandlerOpts< false, D >
    {
      bool importRank( const LoadBalanceHandleType &lb,
                       std::set<int>& ranks ) const
      {
        return false;
      }
      int destination( const LoadBalanceHandleType &lb,
                       const EntityType& entity ) const
      {
        std::abort();
        return -1;
      }
      int loadWeight( const LoadBalanceHandleType &lb,
                      const EntityType& entity ) const
      {
        std::abort();
        return -1;
      }
    };

  private:
    // no copying
    GatherScatterLoadBalance( const GatherScatterLoadBalance& );

  protected:
    GridType & grid_;

    EntityType entity_;

    // pointer to load balancing user interface (if NULL internal load balancing is used)
    LoadBalanceHandleType* ldbHandle_;

    // true if userDefinedPartitioning is used, false if loadWeights is used
    // both are disabled if ldbHandle_ is NULL

  public:
    //! Constructor
    GatherScatterLoadBalance( GridType & grid,
                              LoadBalanceHandleType& ldb)
      : grid_(grid),
        entity_( EntityImpType() ),
        ldbHandle_( &ldb )
    {}

    //! Constructor
    explicit GatherScatterLoadBalance( GridType & grid )
      : grid_(grid),
        entity_( EntityImpType() ),
        ldbHandle_( 0 )
    {}

    // return false, since no user dataHandle is present
    bool hasUserData() const { return false ; }

    // return true if user defined partitioning methods should be used
    bool userDefinedPartitioning () const
    {
      return useExternal && ldbHandle_ ;
    }

    // return true if user defined load balancing weights are provided
    bool userDefinedLoadWeights () const
    {
      return ! useExternal && ldbHandle_ ;
    }

    // returns true if user defined partitioning needs to be readjusted
    bool repartition ()
    {
      return userDefinedPartitioning(); // Note: user calls repartition() before calling loadBalance on the grid
    }

    // return set of ranks data is imported from during load balance
    // this method is only used for user defined repartitioning
    bool importRanks( std::set<int>& ranks ) const
    {
      alugrid_assert( userDefinedPartitioning() );
      return UseExternalHandlerOpts<useExternal>().importRank( ldbHandle(), ranks );
    }

    // return set of ranks data is exported to during load balance
    // this method is only used for user defined repartitioning
    bool exportRanks( std::set<int>& ranks ) const
    {
      // NOTE: This feature is not yet include in the user interface
      //alugrid_assert( userDefinedPartitioning() );
      //return ldbHandle().exportRanks( ranks );
      return false ;
    }

    // return destination (i.e. rank) where the given element should be moved to
    // this needs the methods userDefinedPartitioning to return true
    int destination ( HElementType &elem )
    {
      // make sure userDefinedPartitioning is enabled
      alugrid_assert ( elem.level () == 0 );
      alugrid_assert ( userDefinedPartitioning() );
      return UseExternalHandlerOpts<useExternal>().destination( ldbHandle(), setEntity( elem ) );
    }

    // return load weight of given element
    int loadWeight ( HElementType &elem )
    {
      // make sure userDefinedLoadWeights is enabled
      alugrid_assert( userDefinedLoadWeights() );
      alugrid_assert ( elem.level() == 0 );
      static const bool useWeights = std::is_same<LoadBalanceHandleType, GatherScatter> :: value == false ;
      return UseExternalHandlerOpts< useWeights >().loadWeight( ldbHandle(), setEntity( elem ) );
    }

  protected:
    EntityType& setEntity( HElementType& elem )
    {
      entity_.impl().setElement( elem );
      return entity_ ;
    }

    LoadBalanceHandleType& ldbHandle()
    {
      alugrid_assert( ldbHandle_ );
      return *ldbHandle_;
    }

    const LoadBalanceHandleType& ldbHandle() const
    {
      alugrid_assert( ldbHandle_ );
      return *ldbHandle_;
    }

  };

  ////////////////////////////////////////////////////////////////////////////////////////
  //
  // --GatherScatterLoadBalance: ALU data handle implementation for CommDataHandleIF
  //
  ////////////////////////////////////////////////////////////////////////////////////////
  template <class GridType, class LoadBalanceHandleType, class DataHandleImpl, class Data, bool useExternal>
  class GatherScatterLoadBalanceDataHandle
    : public GatherScatterLoadBalance< GridType, LoadBalanceHandleType, useExternal >
  {
    // no copying
    GatherScatterLoadBalanceDataHandle( const GatherScatterLoadBalanceDataHandle& );

    typedef GatherScatterLoadBalance< GridType, LoadBalanceHandleType, useExternal > BaseType ;
  protected:
    static const int dimension = GridType :: dimension ;
    typedef typename GridType :: Traits :: HierarchicIterator HierarchicIterator;

    template< int codim >
    struct Codim
    {
      typedef typename GridType :: Traits :: template Codim< codim > :: Entity Entity;
    };

    typedef typename GridType::MPICommunicatorType Comm;
    typedef Dune::ALU3dImplTraits< GridType::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::template Codim< GridType::dimension, 0 >::InterfaceType HElementType;

    typedef typename BaseType :: EntityType  EntityType ;

    typedef Dune::CommDataHandleIF< DataHandleImpl, Data > DataHandleType;

    template <class DH, bool>
    struct CompressAndReserve
    {
      static DataHandleImpl& asImp( DH& dh ) { return static_cast<DataHandleImpl &> (dh); }

      static void reserveMemory( DH& dataHandle, const size_t newElements )
      {
        asImp( dataHandle ).reserveMemory( newElements );
      }
      static void compress( DH& dataHandle )
      {
        asImp( dataHandle ).compress();
      }
    };

    template <class DH>
    struct CompressAndReserve< DH, false >
    {
      static void reserveMemory( DH& dataHandle, const size_t newElements ) {}
      static void compress( DH& dataHandle ) {}
    };

    // check whether DataHandleImpl is derived from LoadBalanceHandleWithReserveAndCompress
    static const bool hasCompressAndReserve =  std::is_base_of< LoadBalanceHandleWithReserveAndCompress, DataHandleImpl >::value;
    // don't transmit size in case we have special DataHandleImpl
    static const bool transmitSize = ! hasCompressAndReserve ;

    typedef CompressAndReserve< DataHandleType, hasCompressAndReserve >  CompressAndReserveType;

    // data handle (CommDataHandleIF)
    DataHandleType& dataHandle_;

    // used MessageBuffer
    typedef typename GatherScatter :: ObjectStreamType ObjectStreamType;

    using BaseType :: grid_ ;
    using BaseType :: setEntity ;
    using BaseType :: entity_ ;

    // return true if maxLevel is the same on all cores
    bool maxLevelConsistency() const
    {
      int maxLevel = grid_.maxLevel();
      maxLevel = grid_.comm().max( maxLevel );
      return maxLevel == grid_.maxLevel();
    }
  public:
    //! Constructor taking load balance handle and data handle
    GatherScatterLoadBalanceDataHandle( GridType & grid,
                                        DataHandleType& dh,
                                        LoadBalanceHandleType& ldb)
      : BaseType( grid, ldb ),
        dataHandle_( dh )
    {
      alugrid_assert( maxLevelConsistency() );
    }

    //! Constructor for DataHandle only
    GatherScatterLoadBalanceDataHandle( GridType& grid, DataHandleType& dh )
      : BaseType( grid ),
        dataHandle_( dh )
    {
      alugrid_assert( maxLevelConsistency() );
    }

    // return true if dim,codim combination is contained in data set
    bool contains(int dim, int cd) const
    {
      //dimension is GridImp::dimension
      if(dim == dimension)
      {
        //the original call
        return dataHandle_.contains(dim,cd);
      }
      //adaptation for 2d
      else if(dimension == 2)
      {
        //we do not want to transmit edge data
        if(cd == 2)
          return false;
        else if (cd == 3)
          return dataHandle_.contains(dimension, 2);
        else
          return dataHandle_.contains(dimension, cd);
      }
      //
      else
      {
        std::cerr << "DataHandle.contains called with non-matching dim and codim" << std::endl;
        return false;
      }
    }

    // return true if user dataHandle is present which is the case here
    bool hasUserData() const { return true ; }

    //! this method is called from the dunePackAll method of the corresponding
    //! here the data is written to the ObjectStream
    void inlineData ( ObjectStreamType & str , HElementType & elem, const int estimatedElements )
    {
      // store number of elements to be written (for restore)
      str.write(estimatedElements);
      // set element and then start
      alugrid_assert ( elem.level () == 0 );

      // pack data for the whole hierarchy
      inlineHierarchy( str, elem );
    }

    //! this method is called from the duneUnpackSelf method of the corresponding
    //! here the data is read from the ObjectStream
    void xtractData ( ObjectStreamType & str , HElementType & elem )
    {
      alugrid_assert ( elem.level () == 0 );

      // read number of elements to be restored
      int newElements = 0 ;
      str.read( newElements );

      // if data handle provides reserve feature, reserve memory
      // the data handle has to be derived from LoadBalanceHandleWithReserveAndCompress
      CompressAndReserveType :: reserveMemory( dataHandle_, newElements );

      // unpack data for the hierarchy
      xtractHierarchy( str, elem );
    }

    //! call compress on data
    void compress ()
    {
      // if data handle provides compress, do compress here
      // the data handle has to be derived from LoadBalanceHandleWithReserveAndCompress
      CompressAndReserveType :: compress( dataHandle_ );
    }

  protected:
    // inline data for the hierarchy
    void inlineHierarchy( ObjectStreamType & str, HElementType& elem )
    {
      // pack elements data
      inlineElementData( str, setEntity( elem ) );
      // pack using deep first strategy
      for( HElementType* son = elem.down(); son ; son = son->next() )
        inlineHierarchy( str, *son );
    }

    // inline data for the hierarchy
    void xtractHierarchy( ObjectStreamType & str, HElementType& elem )
    {
      xtractElementData( str, setEntity( elem ) );
      // reset element is new flag
      elem.resetRefinedTag();
      // unpack using deep first strategy
      for( HElementType* son = elem.down(); son ; son = son->next() )
        xtractHierarchy( str, *son );
    }

    void inlineElementData ( ObjectStreamType &stream, const EntityType &element )
    {
      // call element data direct without creating entity pointer
      if( dataHandle_.contains( dimension, 0 ) )
      {
        inlineEntityData<0>( stream, element );
      }

      // now call all higher codims
      inlineCodimData< 1 >( stream, element );
      inlineCodimData< 2 >( stream, element );
      if(dimension == 3)
        inlineCodimData< dimension >( stream, element );
    }

    void xtractElementData ( ObjectStreamType &stream, const EntityType &element )
    {
      // call element data direct without creating entity pointer
      if( dataHandle_.contains( dimension, 0 ) )
      {
        xtractEntityData<0>( stream, element );
      }

      // now call all higher codims
      xtractCodimData< 1 >( stream, element );
      xtractCodimData< 2 >( stream, element );
      if(dimension == 3)
        xtractCodimData< dimension >( stream, element );
    }

    template <int codim>
    int subEntities( const EntityType &element ) const
    {
      return element.subEntities( codim );
    }

    template< int codim >
    void inlineCodimData ( ObjectStreamType &stream, const EntityType &element ) const
    {
      if( dataHandle_.contains( dimension, codim ) )
      {
        const int numSubEntities = this->template subEntities< codim >( element );
        for( int i = 0; i < numSubEntities; ++i )
        {
          inlineEntityData< codim >( stream, element.template subEntity< codim >( i ) );
        }
      }
    }

    template< int codim >
    void xtractCodimData ( ObjectStreamType &stream, const EntityType &element )
    {
      if( dataHandle_.contains( dimension, codim ) )
      {
        const int numSubEntities = this->template subEntities< codim >( element );
        for( int i = 0; i < numSubEntities; ++i )
        {
          xtractEntityData< codim >( stream, element.template subEntity< codim >( i ) );
        }
      }
    }

    template< int codim >
    void inlineEntityData ( ObjectStreamType &stream,
                            const typename Codim< codim > :: Entity &entity ) const
    {
      if( transmitSize )
      {
        const size_t size = dataHandle_.size( entity );
        stream.write( size );
      }
      dataHandle_.gather( stream, entity );
    }

    template< int codim >
    void xtractEntityData ( ObjectStreamType &stream,
                            const typename Codim< codim > :: Entity &entity )
    {
      size_t size = 0;
      if( transmitSize )
      {
        stream.read( size );
      }
      dataHandle_.scatter( stream, entity, size );
    }
  };

  /////////////////////////////////////////////////////////////////
  //
  //  --AdaptRestrictProlong
  //
  /////////////////////////////////////////////////////////////////
  template< class GridType, class AdaptDataHandle >
  class AdaptRestrictProlongImpl
  : public AdaptRestrictProlongType
  {
    GridType & grid_;
    typedef typename GridType::template Codim<0>::Entity     EntityType;
    typedef typename GridType::template Codim<0>::EntityImp  RealEntityType;

    EntityType entity_;

    AdaptDataHandle &rp_;

    typedef typename GridType::MPICommunicatorType Comm;

    typedef Dune::ALU3dImplTraits< GridType::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::HElementType HElementType;
    typedef typename ImplTraits::HBndSegType  HBndSegType;
    typedef typename ImplTraits::BNDFaceType  BNDFaceType;

    //using AdaptRestrictProlongType :: postRefinement ;
    //using AdaptRestrictProlongType :: preCoarsening ;

  public:
    //! Constructor
    AdaptRestrictProlongImpl ( GridType &grid,
                               AdaptDataHandle &rp )
      : grid_(grid)
      , entity_( RealEntityType() )
      , rp_(rp)
    {
    }

    virtual ~AdaptRestrictProlongImpl ()
    {
    }

    //! restrict data for elements
    int preCoarsening ( HElementType & father )
    {
      entity_.impl().setElement( father );
      rp_.preCoarsening( entity_ );

      // reset refinement marker
      father.resetRefinedTag();
      return 0;
    }

    //! prolong data for elements
    int postRefinement ( HElementType & father )
    {
      entity_.impl().setElement( father );
      rp_.postRefinement( entity_ );

      // reset refinement markers
      father.resetRefinedTag();
      for( HElementType *son = father.down(); son ; son = son->next() )
        son->resetRefinedTag();

      return 0;
    }

    //! restrict data for ghost elements
    int preCoarsening ( HBndSegType & ghost ) { return 0; }


    //! prolong data for ghost elements
    int postRefinement ( HBndSegType & ghost ) { return 0; }
  };



  template< class GridType, class AdaptDataHandle, class GlobalIdSetImp >
  class AdaptRestrictProlongGlSet
  : public AdaptRestrictProlongImpl< GridType, AdaptDataHandle >
  {
    typedef AdaptRestrictProlongImpl< GridType, AdaptDataHandle > BaseType;
    GlobalIdSetImp & set_;
    typedef typename GridType::template Codim<0>::Entity     EntityType;
    typedef typename GridType::template Codim<0>::EntityImp  RealEntityType;

    typedef typename GridType::MPICommunicatorType Comm;

    typedef Dune::ALU3dImplTraits< GridType::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::HElementType HElementType;
    typedef typename ImplTraits::HBndSegType HBndSegType;

    using AdaptRestrictProlongType :: postRefinement ;
    using AdaptRestrictProlongType :: preCoarsening ;

  public:
    //! Constructor
    AdaptRestrictProlongGlSet ( GridType &grid,
                                AdaptDataHandle &rp,
                                GlobalIdSetImp & set )
    : BaseType( grid, rp ),
      set_( set )
    {}

    virtual ~AdaptRestrictProlongGlSet () {}

    //! prolong data, elem is the father
    int postRefinement ( HElementType & elem )
    {
      set_.postRefinement( elem );
      return BaseType :: postRefinement(elem );
    }
  };

} // namespace ALUGrid

#endif // #ifndef DUNE_ALU3DGRIDDATAHANDLE_HH
