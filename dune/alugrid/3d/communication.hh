#ifndef DUNE_ALUGRID_3D_COMMUNICATION_HH
#define DUNE_ALUGRID_3D_COMMUNICATION_HH

#include <memory>
#include <utility>

#include <dune/common/stdstreams.hh>

#include <dune/grid/common/datahandleif.hh>
#include <dune/grid/common/gridenums.hh>

#include <dune/alugrid/3d/alu3dinclude.hh>
#include <dune/alugrid/3d/datahandle.hh>

namespace Dune
{

  // Internal Forward Declaration
  // ----------------------------

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  struct ALUCommunication;

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  class ALULeafCommunication;

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  class ALULevelCommunication;



  // External Forward Declarations
  // -----------------------------

  template< int dim, int dimworld, ALU3dGridElementType elType, class Comm >
  class ALU3dGrid;



  // ALUCommunication for ALUGridNoComm
  // ----------------------------------

  template< int dim, int dimworld, ALU3dGridElementType elType >
  struct ALUCommunication< dim, dimworld, elType, ALUGridNoComm >
  {
    typedef ALU3dGrid< dim, dimworld, elType, ALUGridNoComm > Grid;

    bool pending () const { return false; }

    void wait () {}
  };



  // ALULeafCommunication for ALUGridNoComm
  // --------------------------------------

  template< int dim, int dimworld, ALU3dGridElementType elType >
  class ALULeafCommunication< dim, dimworld, elType, ALUGridNoComm >
    : public ALUCommunication< dim, dimworld, elType, ALUGridNoComm >
  {
    typedef ALUCommunication< dim, dimworld, elType, ALUGridNoComm > Base;

  public:
    typedef typename Base::Grid Grid;

    template< class DataHandle, class Data >
    ALULeafCommunication ( const Grid &grid, CommDataHandleIF< DataHandle, Data > &data,
                           InterfaceType iftype, CommunicationDirection dir )
    {}
  };



  // ALULevelCommunication for ALUGridNoComm
  // ---------------------------------------

  template< int dim, int dimworld, ALU3dGridElementType elType >
  class ALULevelCommunication< dim, dimworld, elType, ALUGridNoComm >
    : public ALUCommunication< dim, dimworld, elType, ALUGridNoComm >
  {
    typedef ALUCommunication< dim, dimworld, elType, ALUGridNoComm > Base;

  public:
    typedef typename Base::Grid Grid;

    template< class DataHandle, class Data >
    ALULevelCommunication ( const Grid &grid, CommDataHandleIF< DataHandle, Data > &data,
                           InterfaceType iftype, CommunicationDirection dir, int level )
    {}
  };



  // ALUCommunication for ALUGridMPIComm
  // -----------------------------------

  template< int dim, int dimworld, ALU3dGridElementType elType >
  struct ALUCommunication< dim, dimworld, elType, ALUGridMPIComm >
  {
    typedef ALU3dGrid< dim, dimworld, elType, ALUGridMPIComm > Grid;
    typedef ALU3DSPACE GatherScatter GatherScatter;

  protected:
    typedef MakeableInterfaceObject< typename Grid::Traits::template Codim< 3 >::Entity > VertexObject;
    typedef MakeableInterfaceObject< typename Grid::Traits::template Codim< 2 >::Entity > EdgeObject;
    typedef MakeableInterfaceObject< typename Grid::Traits::template Codim< 1 >::Entity > FaceObject;
    typedef MakeableInterfaceObject< typename Grid::Traits::template Codim< 0 >::Entity > ElementObject;

    typedef typename VertexObject::ImplementationType VertexImpl;
    typedef typename EdgeObject::ImplementationType EdgeImpl;
    typedef typename FaceObject::ImplementationType FaceImpl;
    typedef typename ElementObject::ImplementationType ElementImpl;

    struct Storage
    {
      Storage ( const Grid &grid, int level )
        : vertex( VertexImpl( grid.factory(), level ) ),
          edge( EdgeImpl( grid.factory(), level ) ),
          face( FaceImpl( grid.factory(), level ) ),
          element( ElementImpl( grid.factory(), level ) )
      {}

      virtual ~Storage () {}

      virtual GatherScatter &vertexGatherScatter () = 0;
      virtual GatherScatter &edgeGatherScatter () = 0;
      virtual GatherScatter &faceGatherScatter () = 0;
      virtual GatherScatter &elementGatherScatter () = 0;

    protected:
      VertexObject vertex;
      EdgeObject edge;
      FaceObject face;
      ElementObject element;
    };

  public:
    ALUCommunication ( const Grid &grid, Storage *storage, InterfaceType iftype, CommunicationDirection dir )
      : storage_( storage )
    {
      // check interface types
      if( (iftype == Overlap_OverlapFront_Interface) || (iftype == Overlap_All_Interface) )
      {
        dverb << "ALUGrid contains no overlap, therefore no communication for" << std::endl;
        dverb << "Overlap_OverlapFront_Interface or Overlap_All_Interface interfaces!" << std::endl;
      }
      // communication from border to border
      else if( iftype == InteriorBorder_InteriorBorder_Interface )
        grid.myGrid().borderBorderCommunication( storage_->vertexGatherScatter(), storage_->edgeGatherScatter(), storage_->faceGatherScatter(), storage_->elementGatherScatter() );
      // communication from interior to ghost including border
      else if( iftype == InteriorBorder_All_Interface )
      {
        if( dir == ForwardCommunication )
          communication_ = grid.myGrid().interiorGhostCommunication( storage_->vertexGatherScatter(), storage_->edgeGatherScatter(), storage_->faceGatherScatter(), storage_->elementGatherScatter() );
        // reverse communiction interface (here All_InteriorBorder)
        else if( dir == BackwardCommunication )
          communication_ = grid.myGrid().ghostInteriorCommunication( storage_->vertexGatherScatter(), storage_->edgeGatherScatter(), storage_->faceGatherScatter(), storage_->elementGatherScatter() );
      }
      // communication from interior to ghost including border
      else if( iftype == All_All_Interface )
        communication_ = grid.myGrid().allAllCommunication( storage_->vertexGatherScatter(), storage_->edgeGatherScatter(), storage_->faceGatherScatter(), storage_->elementGatherScatter() );
      else
        DUNE_THROW( GridError, "Wrong parameters in ALUCommunication." );
    }

    ALUCommunication ( ALUCommunication &&other )
      : storage_( std::move( other.storage_ ) ),
        communication_( std::move( other.communication_ ) )
    {}

    ALUCommunication &operator= ( ALUCommunication &&other )
    {
      storage_ = std::move( other.storage_ );
      communication_ = std::move( other.communication_ );
      return *this;
    }

    bool pending () const { return communication_.pending(); }

    void wait () { communication_.wait(); }

  private:
    std::unique_ptr< Storage > storage_;
    ALU3DSPACE GitterDunePll::Communication communication_;
  };



  // ALULeafCommunication for ALUGridMPIComm
  // ---------------------------------------

  template< int dim, int dimworld, ALU3dGridElementType elType >
  class ALULeafCommunication< dim, dimworld, elType, ALUGridMPIComm >
    : public ALUCommunication< dim, dimworld, elType, ALUGridMPIComm >
  {
    typedef ALUCommunication< dim, dimworld, elType, ALUGridMPIComm > Base;

  public:
    typedef typename Base::Grid Grid;
    typedef typename Base::GatherScatter GatherScatter;

  protected:
    template< class DataHandle, class Data >
    struct Storage
      : Base::Storage
    {
      typedef Dune::CommDataHandleIF< DataHandle, Data > CommDataHandleIF;

      Storage ( const Grid &grid, CommDataHandleIF &dataHandle )
        : Base::Storage( grid, grid.maxLevel() ),
          vertexGatherScatter_( grid, vertex, Grid::getRealImplementation( vertex ), dataHandle ),
          edgeGatherScatter_( grid, edge, Grid::getRealImplementation( edge ), dataHandle ),
          faceGatherScatter_( grid, face, Grid::getRealImplementation( face ), dataHandle ),
          elementGatherScatter_( grid, element, Grid::getRealImplementation( element ), dataHandle )
      {}

      GatherScatter &vertexGatherScatter () { return vertexGatherScatter_; }
      GatherScatter &edgeGatherScatter () { return edgeGatherScatter_; }
      GatherScatter &faceGatherScatter () { return faceGatherScatter_; }
      GatherScatter &elementGatherScatter () { return elementGatherScatter_; }

    protected:
      using Base::Storage::vertex;
      using Base::Storage::edge;
      using Base::Storage::face;
      using Base::Storage::element;

      ALU3DSPACE GatherScatterLeafData< Grid, CommDataHandleIF, 3 > vertexGatherScatter_;
      ALU3DSPACE GatherScatterLeafData< Grid, CommDataHandleIF, 2 > edgeGatherScatter_;
      ALU3DSPACE GatherScatterLeafData< Grid, CommDataHandleIF, 1 > faceGatherScatter_;
      ALU3DSPACE GatherScatterLeafData< Grid, CommDataHandleIF, 0 > elementGatherScatter_;
    };

  public:
    template< class DataHandle, class Data >
    ALULeafCommunication ( const Grid &grid, CommDataHandleIF< DataHandle, Data > &data,
                           InterfaceType iftype, CommunicationDirection dir )
      : Base( grid, new Storage< DataHandle, Data >( grid, data ), iftype, dir )
    {}

    ALULeafCommunication ( ALULeafCommunication &&other )
      : Base( static_cast< Base && >( other ) )
    {}

    ALULeafCommunication &operator= ( ALULeafCommunication &&other )
    {
      static_cast< Base & >( *this ) = static_cast< Base && >( other );
      return *this;
    }
  };



  // ALULevelCommunication for ALUGridMPIComm
  // ----------------------------------------

  template< int dim, int dimworld, ALU3dGridElementType elType >
  class ALULevelCommunication< dim, dimworld, elType, ALUGridMPIComm >
    : public ALUCommunication< dim, dimworld, elType, ALUGridMPIComm >
  {
    typedef ALUCommunication< dim, dimworld, elType, ALUGridMPIComm > Base;

  public:
    typedef typename Base::Grid Grid;
    typedef typename Base::GatherScatter GatherScatter;

  protected:
    template< class DataHandle, class Data >
    struct Storage
      : Base::Storage
    {
      typedef Dune::CommDataHandleIF< DataHandle, Data > CommDataHandleIF;

      Storage ( const Grid &grid, int level, CommDataHandleIF &dataHandle )
        : Base::Storage( grid, level ),
          indexSet_( grid.getLevelIndexSet( level ) ),
          vertexGatherScatter_( grid, vertex, Grid::getRealImplementation( vertex ), dataHandle, *indexSet_.first, level ),
          edgeGatherScatter_( grid, edge, Grid::getRealImplementation( edge ), dataHandle, *indexSet_.first, level ),
          faceGatherScatter_( grid, face, Grid::getRealImplementation( face ), dataHandle, *indexSet_.first, level ),
          elementGatherScatter_( grid, element, Grid::getRealImplementation( element ), dataHandle, *indexSet_.first, level )
      {}

      ~Storage ()
      {
        if( !indexSet_.second )
          delete indexSet_.first;
      }

      GatherScatter &vertexGatherScatter () { return vertexGatherScatter_; }
      GatherScatter &edgeGatherScatter () { return edgeGatherScatter_; }
      GatherScatter &faceGatherScatter () { return faceGatherScatter_; }
      GatherScatter &elementGatherScatter () { return elementGatherScatter_; }

    protected:
      using Base::Storage::vertex;
      using Base::Storage::edge;
      using Base::Storage::face;
      using Base::Storage::element;

      std::pair< typename Grid::LevelIndexSetImp *, bool > indexSet_;
      ALU3DSPACE GatherScatterLevelData< Grid, CommDataHandleIF, 3 > vertexGatherScatter_;
      ALU3DSPACE GatherScatterLevelData< Grid, CommDataHandleIF, 2 > edgeGatherScatter_;
      ALU3DSPACE GatherScatterLevelData< Grid, CommDataHandleIF, 1 > faceGatherScatter_;
      ALU3DSPACE GatherScatterLevelData< Grid, CommDataHandleIF, 0 > elementGatherScatter_;
    };

  public:
    template< class DataHandle, class Data >
    ALULevelCommunication ( const Grid &grid, CommDataHandleIF< DataHandle, Data > &data,
                            InterfaceType iftype, CommunicationDirection dir, int level )
      : Base( grid, new Storage< DataHandle, Data >( grid, level, data ), iftype, dir )
    {}

    ALULevelCommunication ( ALULevelCommunication &&other )
      : Base( static_cast< Base && >( other ) )
    {}

    ALULevelCommunication &operator= ( ALULevelCommunication &&other )
    {
      static_cast< Base & >( *this ) = static_cast< Base && >( other );
      return *this;
    }
  };

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_3D_COMMUNICATION_HH
