#ifndef DUNE_ALU3DGRIDGEOMETRY_HH
#define DUNE_ALU3DGRIDGEOMETRY_HH

// System includes
#include <memory>
#include <type_traits>

// Dune includes
#include <dune/common/math.hh>

#include <dune/grid/common/grid.hh>

// Local includes
#include "alu3dinclude.hh"
#include "topology.hh"
#include "mappings.hh"
#include <dune/alugrid/common/memory.hh>

namespace Dune
{

  // Forward declarations
  template<int cd, int dim, class GridImp>
  class ALU3dGridEntity;
  template<int cd, class GridImp >
  class ALU3dGridEntityPointer;
  template<int mydim, int coorddim, class GridImp>
  class ALU3dGridGeometry;
  template< int dim, int dimworld, ALU3dGridElementType, class >
  class ALU3dGrid;
  class BilinearSurfaceMapping;
  class TrilinearMapping;

  template< class GridImp >
  class ALU3dGridIntersectionIterator;

  template <int cdim>
  class MyALUGridGeometryImplementation
  {
  public:
    typedef FieldVector<alu3d_ctype, cdim> CoordinateVectorType;

    static const signed char invalid      = -1; // means geometry is not meaningful
    static const signed char updated      =  0; // means the point values have been set
    static const signed char buildmapping =  1; // means updated and mapping was build

    template <int dim, int corners, class Mapping>
    class GeometryImplBase
    {
    private:
      // prohibited due to reference counting
      GeometryImplBase( const GeometryImplBase& );

    protected:
      //! number of corners
      static const int corners_ = corners ;

      //! the vertex coordinates
      typedef FieldMatrix<alu3d_ctype, corners , cdim>  CoordinateMatrixType;

      // select coordinate storage for coord_ (pointer for dim == 3)
      typedef typename std::conditional<
          dim == 3,
          std::unique_ptr< CoordinateMatrixType >,
          CoordinateMatrixType >:: type CoordinateStorageType;

      //! the type of the mapping
      typedef Mapping     MappingType;

      //! to coordinates
      CoordinateStorageType coord_ ;

      //! the mapping
      MappingType map_;

      //! volume of element
      double volume_ ;

      //! the status (see different status above)
      signed char status_ ;

    public:
      //! reference counter used by SharedPointer
      unsigned int refCount_;

      //! default constructor
      GeometryImplBase()
        : coord_(),
          map_(),
          volume_( 1.0 )
      {
        invalidate();
      }

      // copy coordinate vector from field vector or alu3d_ctype[cdim]
      template <class CoordPtrType>
      static inline void copy(const CoordPtrType& p,
                              CoordinateVectorType& c)
      {
        // we have either 2d or 3d vectors
        alugrid_assert( cdim > 1 );
        c[0] = p[0];
        c[1] = p[1];
        if( cdim > 2 )
          c[2] = p[2];
      }

      template <class CoordPtrType>
      void update(const CoordPtrType&,
                  const CoordPtrType&,
                  const CoordPtrType&,
                  const CoordPtrType&,
                  const CoordPtrType&,
                  const CoordPtrType&,
                  const CoordPtrType&,
                  const CoordPtrType& ) const
      {
        DUNE_THROW(InvalidStateException,"This method should not be called!");
      }

      template <class CoordPtrType>
      void update(const CoordPtrType&,
                  const CoordPtrType&,
                  const CoordPtrType&,
                  const CoordPtrType& ) const
      {
        DUNE_THROW(InvalidStateException,"This method should not be called!");
      }

      template <class CoordPtrType>
      void update(const CoordPtrType&,
                  const CoordPtrType&,
                  const CoordPtrType& ) const
      {
        DUNE_THROW(InvalidStateException,"This method should not be called!");
      }

      template <class CoordPtrType>
      void update(const CoordPtrType&,
                  const CoordPtrType& ) const
      {
        DUNE_THROW(InvalidStateException,"This method should not be called!");
      }

      template <class CoordPtrType>
      void update(const CoordPtrType& ) const
      {
        DUNE_THROW(InvalidStateException,"This method should not be called!");
      }

      // update geometry in father coordinates (default impl)
      template <class GeometryImp>
      inline void updateInFather(const GeometryImp &fatherGeom ,
                                 const GeometryImp &myGeom)
      {
        // this version is only for the 2d elements
        alugrid_assert( dim == 2 );

        // compute the local coordinates in father refelem
        for(int i=0; i < myGeom.corners() ; ++i)
        {
          // calculate coordinate
          coord_[i] = fatherGeom.local( myGeom.corner( i ) );

          // to avoid rounding errors
          for(int j=0; j<cdim; ++j)
          {
            if ( coord_[i][j] < 1e-16) coord_[i][j] = 0.0;
          }
        }

        status_ = updated ;
      }

      // set status to invalid
      void invalidate () { status_ = invalid ; }

      // return true if geometry is valid
      bool valid () const { return status_ != invalid ; }

      // set volume
      void setVolume( const double volume ) { volume_ = volume ; }

      // return volume
      double volume() const { return volume_; }
    };

    //! general type of geometry implementation
    template <int dummy, int dim,
              ALU3dGridElementType eltype> class GeometryImpl;
  public:
    // geometry implementation for edges and vertices
    template <int dummy, int dim, ALU3dGridElementType eltype>
    class GeometryImpl : public GeometryImplBase< dim, dim+1, LinearMapping<cdim, dim> >
    {
    protected:
      typedef GeometryImplBase< dim, dim+1, LinearMapping<cdim, dim> > BaseType;

      using BaseType :: corners_ ;
      using BaseType :: copy ;
      using BaseType :: coord_ ;
      using BaseType :: map_ ;
      using BaseType :: status_ ;

      typedef typename BaseType :: MappingType  MappingType ;
    public:
      using BaseType :: update ;
      using BaseType :: valid ;

      // return coordinate vector
      inline const CoordinateVectorType& operator [] (const int i) const
      {
        alugrid_assert ( valid() );
        alugrid_assert ( i>=0 && i<corners_ );
        return coord_[i];
      }

      inline MappingType& mapping()
      {
        alugrid_assert ( valid() );
        if( status_ == buildmapping ) return map_;

        map_.buildMapping( coord_[0] );
        status_ = buildmapping ;
        return map_;
      }

      // update vertex
      template <class CoordPtrType>
      inline void update(const CoordPtrType& p0)
      {
        alugrid_assert ( corners_ == 1 );
        copy( p0, coord_[0] );
        // we need to update the mapping
        status_ = updated ;
      }
    };

    // geometry implementation for edges and vertices
    template <int dummy, ALU3dGridElementType eltype>
    class GeometryImpl<dummy,1,eltype>
      : public GeometryImplBase< 1, 2, LinearMapping<cdim, 1> >
    {
    protected:
      enum { dim = 1 };
      typedef GeometryImplBase< dim, dim+1, LinearMapping<cdim, dim> > BaseType;

      using BaseType :: corners_ ;
      using BaseType :: copy ;
      using BaseType :: coord_ ;
      using BaseType :: map_ ;
      using BaseType :: status_ ;

      typedef typename BaseType :: MappingType  MappingType;
    public:
      using BaseType :: update ;
      using BaseType :: valid ;

      // return coordinate vector
      inline const CoordinateVectorType& operator [] (const int i) const
      {
        alugrid_assert ( valid() );
        alugrid_assert ( i>=0 && i<corners_ );
        return coord_[i];
      }

      inline MappingType& mapping()
      {
        alugrid_assert ( valid() );
        if( status_ == buildmapping ) return map_;

        map_.buildMapping( coord_[0], coord_[1] );
        status_ = buildmapping ;
        return map_;
      }

      // update edge
      template <class CoordPtrType>
      inline void update(const CoordPtrType& p0,
                         const CoordPtrType& p1)
      {
        alugrid_assert ( corners_ == 2 );
        copy( p0, coord_[0] );
        copy( p1, coord_[1] );
        status_ = updated;
      }
    };

    // geom impl for simplex faces (triangles)
    template <int dummy>
    class GeometryImpl<dummy, 2, tetra>
      : public GeometryImplBase< 2, 3, LinearMapping<cdim, 2> >
    {
    protected:
      // dim = 2, corners = 3
      typedef GeometryImplBase< 2, 3, LinearMapping<cdim, 2> > BaseType;

      using BaseType :: corners_ ;
      using BaseType :: copy ;
      using BaseType :: coord_ ;
      using BaseType :: map_ ;
      using BaseType :: status_ ;

      typedef typename BaseType :: MappingType  MappingType ;
    public:
      using BaseType :: update ;
      using BaseType :: valid ;

      // return coordinate vector
      inline const CoordinateVectorType& operator [] (const int i) const
      {
        alugrid_assert ( valid() );
        alugrid_assert ( i>=0 && i<corners_ );
        return coord_[i];
      }

      // update geometry coordinates
      template <class CoordPtrType>
      inline void update(const CoordPtrType& p0,
                         const CoordPtrType& p1,
                         const CoordPtrType& p2)
      {
        copy(p0, coord_[0] );
        copy(p1, coord_[1] );
        copy(p2, coord_[2] );
        status_ = updated;
      }

      // return mapping (always up2date)
      inline MappingType& mapping()
      {
        alugrid_assert ( valid() );
        if( status_ == buildmapping ) return map_;

        map_.buildMapping( coord_[0], coord_[1], coord_[2] );
        status_ = buildmapping ;
        return map_;
      }
    };

    ///////////////////////////////////////////////////////////////
    //
    //  hexa specializations
    //
    ///////////////////////////////////////////////////////////////

    // geom impl for quadrilaterals (also hexa faces)
    template <int dummy>
    class GeometryImpl<dummy, 2, hexa>
      : public GeometryImplBase< 2, 4, BilinearMapping< cdim > >
    {
    protected:
      // dim = 2, corners = 4
      typedef GeometryImplBase< 2, 4, BilinearMapping< cdim > > BaseType;

      using BaseType :: corners_ ;
      using BaseType :: copy ;
      using BaseType :: coord_ ;
      using BaseType :: map_ ;
      using BaseType :: status_ ;

      typedef typename BaseType :: MappingType  MappingType ;
    public:
      using BaseType :: update ;
      using BaseType :: valid ;

      // return coordinate vector
      inline const CoordinateVectorType& operator [] (const int i) const
      {
        alugrid_assert ( valid() );
        alugrid_assert ( i>=0 && i<corners_ );
        return coord_[i];
      }

      // update geometry coordinates
      template <class CoordPtrType>
      inline void update(const CoordPtrType& p0,
                  const CoordPtrType& p1,
                  const CoordPtrType& p2,
                  const CoordPtrType& p3)
      {
        copy(p0, coord_[0] );
        copy(p1, coord_[1] );
        copy(p2, coord_[2] );
        copy(p3, coord_[3] );
        status_ = updated;
      }

      // return mapping (always up2date)
      inline MappingType& mapping()
      {
        alugrid_assert ( valid() );
        if( status_ == buildmapping ) return map_;

        map_.buildMapping( coord_[0], coord_[1], coord_[2], coord_[3] );
        status_ = buildmapping ;
        return map_;
      }
    };

    // geometry impl for hexahedrons
    template <int dummy>
    class GeometryImpl<dummy,3, hexa>
      : public GeometryImplBase< 3, 8, TrilinearMapping >
    {
    protected:
      // dim = 3, corners = 8
      typedef GeometryImplBase< 3, 8, TrilinearMapping > BaseType;

      using BaseType :: corners_ ;
      using BaseType :: copy ;
      using BaseType :: coord_ ;
      using BaseType :: map_ ;
      using BaseType :: status_ ;

      typedef typename BaseType :: MappingType  MappingType ;
      typedef typename BaseType :: CoordinateMatrixType CoordinateMatrixType;

      typedef alu3d_ctype CoordPtrType[cdim];

      // coordinate pointer vector
      const alu3d_ctype* coordPtr_[ corners_ ];
    public:
      using BaseType :: update ;
      using BaseType :: valid ;

      //! constructor creating geo impl
      GeometryImpl() : BaseType()
      {
        // set initialize coord pointers
        for( int i=0; i<corners_; ++i )
          coordPtr_[ i ] = 0;
      }

      const alu3d_ctype* point( const int i ) const
      {
        alugrid_assert ( valid() );
        alugrid_assert ( i>=0 && i<corners_ );
        alugrid_assert ( coordPtr_[i] );
        return coordPtr_[ i ];
      }

      // return coordinates
      inline CoordinateVectorType operator [] (const int i) const
      {
        CoordinateVectorType coord ;
        copy( point( i ), coord );
        return coord ;
      }

      // update geometry coordinates
      inline void update(const CoordPtrType& p0,
                         const CoordPtrType& p1,
                         const CoordPtrType& p2,
                         const CoordPtrType& p3,
                         const CoordPtrType& p4,
                         const CoordPtrType& p5,
                         const CoordPtrType& p6,
                         const CoordPtrType& p7)
      {
        coordPtr_[0] = &p0[ 0 ];
        coordPtr_[1] = &p1[ 0 ];
        coordPtr_[2] = &p2[ 0 ];
        coordPtr_[3] = &p3[ 0 ];
        coordPtr_[4] = &p4[ 0 ];
        coordPtr_[5] = &p5[ 0 ];
        coordPtr_[6] = &p6[ 0 ];
        coordPtr_[7] = &p7[ 0 ];
        status_ = updated;
      }

      // update geometry in father coordinates
      template <class GeometryImp>
      inline void updateInFather(const GeometryImp &fatherGeom ,
                                 const GeometryImp &myGeom)
      {
        if( ! coord_ )
        {
          coord_.reset( new CoordinateMatrixType() );
        }

        CoordinateMatrixType& coord = *coord_;
        // compute the local coordinates in father refelem
        for(int i=0; i < myGeom.corners() ; ++i)
        {
          // calculate coordinate
          coord[i] = fatherGeom.local( myGeom.corner( i ) );

          // set pointer
          coordPtr_[i] = (&(coord[i][0]));

          // to avoid rounding errors
          for(int j=0; j<cdim; ++j)
          {
            if ( coord[i][j] < 1e-16) coord[i][j] = 0.0;
          }
        }

        status_ = updated ;
      }

      // return mapping (always up2date)
      inline MappingType& mapping()
      {
        alugrid_assert ( valid() );
        if( status_ == buildmapping ) return map_;

        map_.buildMapping( point( 0 ), point( 1 ), point( 2 ), point( 3 ),
                           point( 4 ), point( 5 ), point( 6 ), point( 7 ) );

        status_ = buildmapping;
        return map_;
      }

      // set status to invalid
      void invalidate () { status_ = invalid ; }

      // return true if geometry is valid
      bool valid () const { return status_ != invalid ; }
    };


    // geometry impl for hexahedrons
    template <int dummy>
    class GeometryImpl<dummy,3, tetra>
      : public GeometryImplBase< 3, 4, LinearMapping<cdim, cdim> >
    {
      // dim = 3, corners = 8
      typedef GeometryImplBase< 3, 4, LinearMapping<cdim, cdim> > BaseType;

      using BaseType :: corners_ ;
      using BaseType :: copy ;
      using BaseType :: coord_ ;
      using BaseType :: map_ ;
      using BaseType :: status_ ;

      typedef typename BaseType :: MappingType  MappingType ;
      typedef typename BaseType :: CoordinateMatrixType CoordinateMatrixType;

      typedef alu3d_ctype CoordPtrType[cdim];

      // coordinate pointer vector
      const alu3d_ctype* coordPtr_[ corners_ ];
    public:
      using BaseType :: update ;
      using BaseType :: valid ;

      // default constructor
      GeometryImpl() : BaseType()
      {
        // set initialize coord pointers
        for( int i=0; i<corners_; ++i )
          coordPtr_[ i ] = 0;
      }

      const alu3d_ctype* point( const int i ) const
      {
        alugrid_assert ( valid() );
        alugrid_assert ( i>=0 && i<corners_ );
        alugrid_assert ( coordPtr_[ i ] );
        return coordPtr_[ i ];
      }

      // return coordinate vector
      inline CoordinateVectorType operator [] (const int i) const
      {
        CoordinateVectorType coord ;
        copy( point( i ), coord );
        return coord ;
      }

      // update geometry coordinates
      inline void update(const CoordPtrType& p0,
                         const CoordPtrType& p1,
                         const CoordPtrType& p2,
                         const CoordPtrType& p3)
      {
        coordPtr_[0] = &p0[ 0 ];
        coordPtr_[1] = &p1[ 0 ];
        coordPtr_[2] = &p2[ 0 ];
        coordPtr_[3] = &p3[ 0 ];
        status_ = updated;
      }

      // update geometry in father coordinates
      template <class GeometryImp>
      inline void updateInFather(const GeometryImp &fatherGeom ,
                          const GeometryImp & myGeom)
      {
        if( ! coord_ )
        {
          coord_.reset(new CoordinateMatrixType());
        }

        CoordinateMatrixType& coord = *coord_;
        // compute the local coordinates in father refelem
        for(int i=0; i < myGeom.corners() ; ++i)
        {
          // calculate coordinate
          coord[i] = fatherGeom.local( myGeom.corner( i ) );

          // set pointer
          coordPtr_[i] = (&(coord[i][0]));

          // to avoid rounding errors
          for(int j=0; j<cdim; ++j)
          {
            if ( coord[i][j] < 1e-16) coord[i][j] = 0.0;
          }
        }

        status_ = updated;
      }

      // return mapping (always up2date)
      inline MappingType& mapping()
      {
        alugrid_assert ( valid() );
        if( status_ == buildmapping ) return map_;

        map_.buildMapping( point( 0 ), point( 1 ), point( 2 ), point( 3 ) );

        status_ = buildmapping;
        return map_;
      }
    };
  }; // end of class ALUGridGeometryImplementation

  template <int mydim, int cdim, class GridImp>
  class ALU3dGridGeometry :
    public GeometryDefaultImplementation<mydim, cdim, GridImp, ALU3dGridGeometry>
  {
    static const ALU3dGridElementType elementType = GridImp::elementType;

    typedef typename GridImp::MPICommunicatorType Comm;

    //friend class ALU3dGridIntersectionIterator<GridImp>;
    typedef ALU3dImplTraits< elementType, Comm > ALU3dImplTraitsType ;

  public:
    typedef typename ALU3dImplTraitsType::IMPLElementType IMPLElementType;
    typedef typename ALU3dImplTraitsType::GEOFaceType     GEOFaceType;
    typedef typename ALU3dImplTraitsType::GEOEdgeType     GEOEdgeType;
    typedef typename ALU3dImplTraitsType::GEOVertexType   GEOVertexType;

    // interface types
    typedef typename ALU3dImplTraitsType::HFaceType   HFaceType;
    typedef typename ALU3dImplTraitsType::HEdgeType   HEdgeType;
    typedef typename ALU3dImplTraitsType::VertexType  VertexType;


    typedef ElementTopologyMapping<elementType> ElementTopo;
    typedef FaceTopologyMapping<elementType> FaceTopo;

    static const int corners_  = (elementType == hexa) ? Dune::power( int(2) , int((mydim> -1) ? mydim : 0 ) ) : mydim+1;

    // type of specialized geometry implementation
    typedef typename MyALUGridGeometryImplementation<cdim> ::
      template GeometryImpl<0, mydim, elementType > GeometryImplType;

  public:
    typedef typename GridImp :: ctype ctype;

    //! type of local coordinates
    typedef FieldVector<ctype, mydim> LocalCoordinate;

    //! type of the global coordinates
    typedef FieldVector<ctype, cdim > GlobalCoordinate;

    //! type of jacobian inverse transposed
    typedef FieldMatrix<ctype,cdim,mydim> JacobianInverseTransposed;

    //! type of jacobian transposed
    typedef FieldMatrix< ctype, mydim, cdim > JacobianTransposed;

    // type of coordinate matrix for faces
    typedef FieldMatrix<ctype,
            GridImp::dimension == 3 ? EntityCount< elementType > :: numVerticesPerFace : 2 , cdim> FaceCoordinatesType;

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    GeometryType type () const;

    //! return the number of corners of this element. Corners are numbered 0..n-1
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    GlobalCoordinate corner (int i) const;

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    GlobalCoordinate global (const LocalCoordinate& local) const;

    //! maps a global coordinate within the element to a
    //! local coordinate in its reference element
    LocalCoordinate local (const GlobalCoordinate& global) const;

    //! A(l) , see grid.hh
    ctype integrationElement (const LocalCoordinate& local) const;

    //! can only be called for dim=dimworld! (Trivially true, since there is no
    //! other specialization...)
    const JacobianInverseTransposed &jacobianInverseTransposed (const LocalCoordinate& local) const;

    //! jacobian transposed
    const JacobianTransposed& jacobianTransposed (const LocalCoordinate& local) const;

    //! returns true if mapping is affine
    bool affine () const;

    //! returns volume of geometry
    ctype volume () const;

    //***********************************************************************
    //!  Methods that not belong to the Interface, but have to be public
    //***********************************************************************
    //! generate the geometry out of a given ALU3dGridElement
    bool buildGeom(const IMPLElementType & item);
    bool buildGeom(const HFaceType & item, int twist);
    bool buildGeom(const HEdgeType & item, int twist);
    bool buildGeom(const VertexType & item, int twist);

    // this method is used by the intersection iterator
    bool buildGeom(const FaceCoordinatesType& coords);

    // this method is used by the intersection iterator
    template <class coord_t>
    bool buildGeom(const coord_t& p0,
                   const coord_t& p1,
                   const coord_t& p2,
                   const coord_t& p3);

    // this method is used by the intersection iterator
    template <class coord_t>
    bool buildGeom(const coord_t& p0,
                   const coord_t& p1,
                   const coord_t& p2);

    // this method is used by the intersection iterator
    template <class coord_t>
    bool buildGeom(const coord_t& p0,
                   const coord_t& p1);

    //! build geometry of local coordinates relative to father
    template <class Geometry>
    bool buildGeomInFather(const Geometry &fatherGeom , const Geometry &myGeom);

    //! print internal data
    //! no interface method
    void print (std::ostream& ss) const;

    //! invalidate geometry implementation to avoid errors
    void invalidate () { geoImplPtr_.invalidate(); }

    //! invalidate geometry implementation to avoid errors
    bool valid () const { return geoImpl().valid(); }

  protected:
    // return reference to geometry implementation
    GeometryImplType& geoImpl() const
    {
      return *geoImplPtr_;
    }

    // proxy object holding GeometryImplType* with reference counting
    mutable ALU3DSPACE SharedPointer< GeometryImplType > geoImplPtr_;
  };

} // namespace Dune

#include "geometry_imp.cc"

#endif
