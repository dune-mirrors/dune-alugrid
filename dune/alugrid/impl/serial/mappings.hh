#ifndef DUNE_ALU3DGRIDMAPPINGS_HH
#define DUNE_ALU3DGRIDMAPPINGS_HH

// System includes
#include <limits>
#include <cmath>

#include "gitter_sti.h"

// Dune includes
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>
#include <dune/common/exceptions.hh>
#include "../../3d/aluinline.hh"



namespace ALUGrid {

  static const alucoord_t ALUnumericEpsilon = 10.0 * std::numeric_limits< alucoord_t >::epsilon();

  //! A trilinear mapping from the Dune reference hexahedron into the physical
  //! space (same as in mapp_cube_3d.h, but for a different reference hexahedron)
  class TrilinearMapping
  {
  public:
    typedef alucoord_t double_t[3];
    typedef Dune::FieldVector<alucoord_t, 3> coord_t;
    typedef Dune::FieldMatrix<alucoord_t, 3, 3> mat_t;
  private:
    static constexpr double _epsilon  = 1.0e-8;

    // the internal mapping
    alucoord_t a [8][3] ;
    mat_t Df;
    mat_t Dfi;
    mat_t invTransposed_;
    alucoord_t DetDf ;

    bool calcedDet_;
    bool calcedLinear_;
    bool calcedInv_;
    bool affine_;

    void linear (const alucoord_t, const alucoord_t, const alucoord_t) ;
    void linear (const coord_t&) ;
    void inverse (const coord_t&) ;
  public :
    TrilinearMapping (const coord_t&, const coord_t&,
                      const coord_t&, const coord_t&,
                      const coord_t&, const coord_t&,
                      const coord_t&, const coord_t&);

    TrilinearMapping (const double_t&, const double_t&,
                      const double_t&, const double_t&,
                      const double_t&, const double_t&,
                      const double_t&, const double_t&);

    // only to call from geometry class
    TrilinearMapping () {}

    TrilinearMapping (const TrilinearMapping &) ;

    ~TrilinearMapping () {}
    alucoord_t volume ();
    alucoord_t det (const coord_t&) ;
    const mat_t& jacobianInverseTransposed(const coord_t&);
    const mat_t& jacobianTransposed(const coord_t&);
    void map2world (const coord_t&, coord_t&) const ;
    void map2world (const alucoord_t , const alucoord_t , const alucoord_t ,
                    coord_t&) const ;
    void map2world (const alucoord_t , const alucoord_t , const alucoord_t ,
        double_t&) const ;
    void world2map (const coord_t&, coord_t&) ;

    static inline void barycenter(const alucoord_t (&p0)[3], const alucoord_t (&p1)[3], const alucoord_t (&p2)[3], const alucoord_t (&p3)[3],
        const alucoord_t (&p4)[3], const alucoord_t (&p5)[3], const alucoord_t (&p6)[3], const alucoord_t (&p7)[3],
        alucoord_t (&barycenter)[3])
    {
      barycenter[0] = 0.125 * (p0[0] + p1[0] + p2[0] + p3[0] + p4[0] + p5[0] + p6[0] + p7[0]);
      barycenter[1] = 0.125 * (p0[1] + p1[1] + p2[1] + p3[1] + p4[1] + p5[1] + p6[1] + p7[1]);
      barycenter[2] = 0.125 * (p0[2] + p1[2] + p2[2] + p3[2] + p4[2] + p5[2] + p6[2] + p7[2]);

#ifdef ALUGRIDDEBUG
      {
        TrilinearMapping map(p0,p1,p2,p3,p4,p5,p6,p7);
        alucoord_t p[3];
        //Reference element is [0,1]^3 - use (0.5,0.5,0.5)
        map.map2world(0.5, 0.5, 0.5, p);
        for(int j=0; j<3; ++j)
        {
          alugrid_assert ( fabs(barycenter[j] - p[j]) < 1e-8 );
        }
      }
#endif
    }
    template <class vector_t>
    void buildMapping(const vector_t&, const vector_t&,
                      const vector_t&, const vector_t&,
                      const vector_t&, const vector_t&,
                      const vector_t&, const vector_t&);

    // returns true if mapping is affine
    inline bool affine () const { return affine_; }
  };

  //! A bilinear surface mapping
  // NOTE: this class is different to the BilinearSurfaceMapping in
  // ALUGrid, for example the reference elements differ
  // here we have [0,1]^2 and in ALUGrid its [-1,1]^2
  // also the point numbering is different
  class SurfaceNormalCalculator
  {
  public:
    // our coordinate types
    typedef Dune::FieldVector<alucoord_t, 3> coord3_t;
    typedef Dune::FieldVector<alucoord_t, 2> coord2_t;

    // type of coordinate vectors from elements
    typedef alucoord_t double3_t[3];
  protected:

    alucoord_t _n [3][3] ;

    static constexpr double _epsilon = 1.0e-8;

    bool _affine;

  public :
    //! Constructor creating empty mapping with double , i.e. zero
    SurfaceNormalCalculator();

    SurfaceNormalCalculator (const SurfaceNormalCalculator &) ;
    ~SurfaceNormalCalculator () {}

    // returns true if mapping is affine
    inline bool affine () const { return _affine ; }

    // calcuates normal
    void normal(const coord2_t&, coord3_t&) const ;
    void normal(const alucoord_t, const alucoord_t, coord3_t&)const;

    void negativeNormal(const coord2_t&, coord3_t&) const ;
    void negativeNormal(const alucoord_t, const alucoord_t, coord3_t&)const;

  public:
    // builds _b and _n, called from the constructors
    // public because also used in faceutility
    template <class vector_t>
    void buildMapping (const vector_t & , const vector_t & ,
                       const vector_t & , const vector_t & );
  protected:
    // builds _b and _n, called from the constructors
    // public because also used in faceutility
    template <class vector_t>
    void buildMapping (const vector_t & , const vector_t & ,
                       const vector_t & , const vector_t & ,
                       alucoord_t (&_b)[4][3] );
  } ;


  //! A Linear Surface Mapping
  //This is the linear surface Mapping from mapp_tetra3d
  class LinearSurfaceMapping {
    const alucoord_t (&_p0)[3], (&_p1)[3], (&_p2)[3] ;
    alucoord_t _b [3][3] ;
  protected:
    alucoord_t _n [3] ;
  public :
      inline LinearSurfaceMapping (const alucoord_t (&)[3], const alucoord_t (&)[3], const alucoord_t (&)[3]) ;
      inline LinearSurfaceMapping (const LinearSurfaceMapping &) ;
     ~LinearSurfaceMapping() { }
      inline void map2world(const alucoord_t (&)[3], alucoord_t (&)[3]) const ;
      inline void map2world(alucoord_t x, alucoord_t y, alucoord_t z, alucoord_t (&w)[3]) const ;
      inline void normal(alucoord_t (&)[3]) const;
  } ;



  //! A bilinear surface mapping
  // NOTE: this class is different to the BilinearSurfaceMapping in
  // ALUGrid, for example the reference elements differ
  // here we have [0,1]^2 and in ALUGrid its [-1,1]^2
  // also the point numbering is different
  class BilinearSurfaceMapping : public SurfaceNormalCalculator
  {
  protected:
    typedef SurfaceNormalCalculator BaseType;

    using BaseType :: _n;
    static constexpr double _epsilon = 1.0e-8;

    // our coordinate types
    typedef Dune::FieldVector<alucoord_t, 3> coord3_t;
    typedef Dune::FieldVector<alucoord_t, 2> coord2_t;

    // type of coordinate vectors from elements
    typedef alucoord_t double3_t[3];

    // type for helper matrices
    typedef Dune::FieldMatrix<alucoord_t,3,3> mat3_t;

    // type for inverse matrices
    typedef Dune::FieldMatrix<alucoord_t,2,3> matrix_t;

    // type for inverse matrices
    typedef Dune::FieldMatrix<alucoord_t,3,2> inv_t;

    alucoord_t _b [4][3] ;

    mutable mat3_t Df,Dfi;
    mutable inv_t invTransposed_;
    mutable matrix_t matrix_;
    mutable alucoord_t DetDf;

    mutable coord3_t normal_;
    mutable coord3_t tmp_;

    mutable bool _calcedInv;
    mutable bool _calcedTransposed;
    mutable bool _calcedMatrix;

  public :
    //! Constructor creating empty mapping with double , i.e. zero
    BilinearSurfaceMapping ();

    //! Constructor getting Dune::FieldVectors
    BilinearSurfaceMapping (const coord3_t&, const coord3_t&,
                            const coord3_t&, const coord3_t&) ;
    //! Constructor for double[3]
    BilinearSurfaceMapping (const double3_t &, const double3_t &,
                            const double3_t &, const double3_t &) ;
    BilinearSurfaceMapping (const BilinearSurfaceMapping &) ;
    ~BilinearSurfaceMapping () {}

    void inverse (const coord3_t&) const;
    const inv_t& jacobianInverseTransposed(const coord2_t&) const ;

    const matrix_t& jacobianTransposed(const coord2_t&) const ;

    // calculates determinant of face mapping using the normal
    alucoord_t det(const coord2_t&) const;

    // maps from local coordinates to global coordinates
    void world2map(const coord3_t &, coord2_t & ) const;

    // maps form global coordinates to local (within reference element)
    // coordinates
    void map2world(const coord2_t&, coord3_t&) const ;
    void map2world(const alucoord_t ,const alucoord_t , coord3_t&) const ;

    static inline void barycenter(const alucoord_t (&p0)[3], const alucoord_t (&p1)[3], const alucoord_t (&p2)[3], const alucoord_t (&p3)[3],
        alucoord_t (&barycenter)[3])
    {
      barycenter[0] = 0.25 * (p0[0] + p1[0] + p2[0] + p3[0]);
      barycenter[1] = 0.25 * (p0[1] + p1[1] + p2[1] + p3[1]);
      barycenter[2] = 0.25 * (p0[2] + p1[2] + p2[2] + p3[2]);

#ifdef ALUGRIDDEBUG
      {
        BilinearSurfaceMapping map(p0,p1,p2,p3);
        coord3_t p;
        // reference element is [0,1]^2 with center (0.5,0.5)
        map.map2world(coord2_t(0.5), p);
        for(int j=0; j<3; ++j)
        {
          alugrid_assert ( fabs(barycenter[j] - p[j]) < 1e-8 );
        }
      }
#endif
    }

  private:
    void map2worldnormal(const alucoord_t, const alucoord_t, const alucoord_t , coord3_t&)const;
    void map2worldlinear(const alucoord_t, const alucoord_t, const alucoord_t ) const;

  public:
    // builds _b and _n, called from the constructors
    // public because also used in faceutility
    template <class vector_t>
    void buildMapping (const vector_t & , const vector_t & ,
                       const vector_t & , const vector_t & );
  } ;



  //! A bilinear mapping
  template< int cdim >
  class BilinearMapping
  {
  public:
    typedef alucoord_t ctype;

    typedef Dune::FieldVector< ctype, cdim > world_t;
    typedef Dune::FieldVector< ctype, 2 > map_t;

    typedef Dune::FieldMatrix< ctype, 2, cdim > matrix_t;
    typedef Dune::FieldMatrix< ctype, cdim, 2 > inv_t;

  protected:
    ctype _b [4][cdim];

    mutable ctype det_;
    mutable matrix_t matrix_;
    mutable inv_t invTransposed_;

    mutable bool affine_;
    mutable bool calcedMatrix_;
    mutable bool calcedDet_;
    mutable bool calcedInv_;

  public:
    BilinearMapping ();
    BilinearMapping ( const world_t &p0, const world_t &p1,
                      const world_t &p2, const world_t &p3 );
    BilinearMapping ( const ctype (&p0)[ cdim ], const ctype (&p1)[ cdim ],
                      const ctype (&p2)[ cdim ], const ctype (&p3)[ cdim ] );

    bool affine () const;

    void world2map ( const world_t &, map_t & ) const;
    void map2world ( const ctype x, const ctype y, world_t &w ) const;
    void map2world ( const map_t &, world_t & ) const;

    ctype det ( const map_t & ) const;

    const matrix_t &jacobianTransposed ( const map_t & ) const;
    const inv_t &jacobianInverseTransposed ( const map_t & ) const;

    template< class vector_t >
    void buildMapping ( const vector_t &, const vector_t &,
                        const vector_t &, const vector_t & );

  protected:
    static void multTransposedMatrix ( const matrix_t &, Dune::FieldMatrix< ctype, 2, 2 > & );
    static void multMatrix ( const matrix_t &, const Dune::FieldMatrix< ctype, 2, 2 > &, inv_t & );

    void map2worldlinear ( const ctype, const ctype ) const;
    void inverse ( const map_t & ) const;
  };



  //! A linear mapping
  template< int cdim, int mydim >
  class LinearMapping
  {
  public:
    typedef alucoord_t ctype;

    typedef ctype double_t[ cdim ];

    typedef Dune::FieldVector< ctype, cdim > world_t;
    typedef Dune::FieldVector< ctype, mydim > map_t;

    typedef Dune::FieldMatrix< ctype, mydim, cdim > matrix_t;
    typedef Dune::FieldMatrix< ctype, cdim, mydim > inv_t;

  protected:
    matrix_t      _matrix;        //!< transformation matrix (transposed)
    mutable inv_t _invTransposed; //!< storage for inverse of jacobian (transposed)
    world_t _p0;                  //! P[0]

    //! stores the determinant of the inverse
    mutable ctype _det;

    //! true if inverse has been calculated
    mutable bool _calcedInv;

    //! true if determinant has been calculated
    mutable bool _calcedDet;

  public:
    //! Constructor creating empty mapping with double , i.e. zero
    LinearMapping ();

    //! copy constructor
    LinearMapping (const LinearMapping &) ;

    // returns true if mapping is affine (which is always true)
    inline bool affine () const { return true ; }

    // return reference to transposed jacobian
    const matrix_t& jacobianTransposed(const map_t &) const ;

    // return reference to transposed jacobian inverse
    const inv_t& jacobianInverseTransposed(const map_t &) const ;

    // calculates determinant of mapping
    ctype det(const map_t&) const;

    // maps from local coordinates to global coordinates
    void world2map(const world_t &, map_t &) const;

    // maps form global coordinates to local (within reference element)
    // coordinates
    void map2world(const map_t &, world_t &) const ;

    static inline void barycenter(const alucoord_t (&p0)[3], const alucoord_t (&p1)[3],
        const alucoord_t (&p2)[3], const alucoord_t (&p3)[3],
        alucoord_t (&barycenter)[3])
    {
      //make sure we only call this method internally
      alugrid_assert(cdim == 3 && mydim == 3);
      barycenter[0] = 0.25 * (p0[0] + p1[0] + p2[0] + p3[0]);
      barycenter[1] = 0.25 * (p0[1] + p1[1] + p2[1] + p3[1]);
      barycenter[2] = 0.25 * (p0[2] + p1[2] + p2[2] + p3[2]);
      // TODO: Make this a unit test
#if 0
      LinearMapping<3,3> map(world_t(p0),world_t(p1),world_t(p2),world_t(p3));
      world_t p ;
      map.map2world(map_t(.25), p) ;
      for(int j=0; j<3; ++j)
      {
        alugrid_assert ( fabs(barycenter[j] - p[j]) < 1e-8 );
      }
#endif
    }

  protected:
    // calculate inverse
    void inverse (const map_t&) const;

    // calculate inverse one codim one entity
    void inverseCodimOne (const map_t&) const;

    // calculate determinant
    void calculateDeterminant (const map_t&) const;

    void multTransposedMatrix(const matrix_t& matrix,
                              Dune::FieldMatrix<ctype, mydim, mydim>& result) const;

    void multMatrix ( const matrix_t& A,
                      const Dune::FieldMatrix< ctype, mydim, mydim> &B,
                      inv_t& ret ) const ;

  public:
    // builds _b and _n, called from the constructors
    // public because also used in faceutility
    template <class vector_t>
    void buildMapping (const vector_t & , const vector_t & ,
                       const vector_t & , const vector_t & );

    // builds _b and _n, called from the constructors
    // public because also used in faceutility
    template <class vector_t>
    void buildMapping (const vector_t & , const vector_t & ,
                       const vector_t & );

    // builds _b and _n, called from the constructors
    // public because also used in faceutility
    template <class vector_t>
    void buildMapping (const vector_t & , const vector_t & );

    template <class vector_t>
    void buildMapping (const vector_t & );
  } ;
} // end namespace

namespace Dune {
  using ::ALUGrid::LinearMapping;
  using ::ALUGrid::BilinearMapping;
  using ::ALUGrid::BilinearSurfaceMapping;
  using ::ALUGrid::SurfaceNormalCalculator;
  using ::ALUGrid::TrilinearMapping;
}


#if COMPILE_ALUGRID_INLINE
#include "mappings.cc"
#endif

#endif
