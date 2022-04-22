#ifndef DUNE_ALUGRID_COMMON_TWISTS_HH
#define DUNE_ALUGRID_COMMON_TWISTS_HH

#include <dune/common/fvector.hh>

#include <dune/geometry/affinegeometry.hh>
#include <dune/geometry/referenceelements.hh>

#include <dune/alugrid/common/capabilities.hh>

namespace Dune
{

  // Internal Forward Declarations
  // -----------------------------

  template< int corners, int dim >
  class ALUTwist;

  template< int corners, int dim >
  class ALUTwists;



  // ALUTwistIterator
  // ----------------

  template< class Twist >
  struct ALUTwistIterator
  {
    typedef std::input_iterator_tag iterator_category;
    typedef Twist value_type;
    typedef int difference_type;
    typedef Twist* pointer;
    typedef Twist& reference;

    explicit ALUTwistIterator ( Twist twist ) : twist_( twist ) {}

    const Twist &operator* () const { return twist_; }
    const Twist *operator-> () const { return &twist_; }

    bool operator== ( const ALUTwistIterator &other ) const { return (twist_ == other.twist_); }
    bool operator!= ( const ALUTwistIterator &other ) const { return (twist_ != other.twist_); }

    ALUTwistIterator &operator++ () { ++twist_.aluTwist_; return *this; }
    ALUTwistIterator operator++ ( int ) { ALUTwistIterator other( *this ); ++(*this); return other; }

  private:
    Twist twist_;
  };



  // ALUTwist for dimension 2
  // ------------------------

  template< int corners >
  class ALUTwist< corners, 2 >
  {
    typedef ALUTwist< corners, 2 > Twist;

    friend struct ALUTwistIterator< Twist >;

    template< class ctype >
    struct CoordVector
    {
      // type of container for reference elements
      typedef ReferenceElements< ctype, 2 > ReferenceElementContainerType;

      // type of reference element
      typedef std::decay_t< decltype( ReferenceElementContainerType::general( std::declval< const Dune::GeometryType & >() ) ) > ReferenceElementType;

      CoordVector ( const Twist &twist )
        : twist_( twist ),
          refElement_( ReferenceElements< ctype, 2 >::general( twist.type() ) )
      {}

      const FieldVector< ctype, 2 > operator[] ( int i ) const { return refElement().position( twist_( i ), 2 ); }

      const ReferenceElementType& refElement () const { return refElement_; }

    private:
      const Twist &twist_;
      const ReferenceElementType& refElement_;
    };

  public:
    /** \brief dimension */
    static const int dimension = 2;

    /**
     * \name Construction
     * \{
     */

    /** \brief default constructor; results in identity twist */
    ALUTwist () : aluTwist_( 0 ) {}

    explicit ALUTwist ( GeometryType ) : aluTwist_( 0 ) {}

    explicit ALUTwist ( int aluTwist ) : aluTwist_( aluTwist ) {}

    ALUTwist ( int origin, bool positive )
      : aluTwist_( positive ? origin : (origin + corners - 1) % corners - corners )
    {}

    /** \} */

    /**
     * \name Copying and Assignment
     * \{
     */

    /** \brief copy constructor */
    ALUTwist ( const ALUTwist &other ) : aluTwist_( other.aluTwist_ ) {}

    /** \brief assignment operator */
    ALUTwist &operator= ( const ALUTwist &other ) { aluTwist_ = other.aluTwist_; return *this; }

    /** \} */

    /**
     * \name Group Operations
     * \{
     */

    /** \brief composition */
    ALUTwist operator* ( const ALUTwist &other ) const
    {
      return ALUTwist( apply( other.apply( 0 ) ), !(positive() ^ other.positive()) );
    }

    /** \brief composition with inverse */
    ALUTwist operator/ ( const ALUTwist &other ) const { return (*this * other.inverse()); }

    /** \brief composition */
    ALUTwist &operator*= ( const ALUTwist &other ) { return (*this = (*this) * other); }

    /** \brief composition with inverse */
    ALUTwist &operator/= ( const ALUTwist &other ) { return (*this = (*this) / other); }

    /** \brief obtain inverse */
    ALUTwist inverse () const { return ALUTwist( positive() ? (corners - aluTwist_) % corners : aluTwist_ ); }

    /** \} */

    /**
     * \brief Comparison
     * \{
     */

    /** \brief check for equality */
    bool operator== ( const ALUTwist &other ) const { return (aluTwist_ == other.aluTwist_); }

    /** \brief check for inequality */
    bool operator!= ( const ALUTwist &other ) const { return (aluTwist_ != other.aluTwist_); }

    /** \} */

    /**
     * \brief Topological Corner Mapping
     * \{
     */

    /** \brief obtain type of reference element */
    GeometryType type () const
    {
      if( corners == 3 )
        return GeometryTypes::simplex(dimension);
      else
        return GeometryTypes::cube(dimension);
    }

    /**
     * \brief map i-th corner
     *
     * \param[in]  i  corner index
     *
     * \returns mapped index
     */
    int operator() ( int i ) const { return aluVertex2duneVertex( apply( duneVertex2aluVertex( i ) ) ); }

    int operator() ( int i, int codim ) const { return alu2dune( apply( dune2alu( i, codim ), codim ), codim ); }

    /** \} */

    /**
     * \brief Geometric Equivalent
     * \{
     */

    /** \brief cast into affine geometry \f$F\f$ */
    template< class ctype >
    operator AffineGeometry< ctype, dimension, dimension > () const
    {
      const CoordVector< ctype > coordVector( *this );
      return AffineGeometry< ctype, dimension, dimension >( coordVector.refElement(), coordVector );
    }

    /** \brief equivalent to \f$|DF| > 0\f$ */
    bool positive () const { return (aluTwist_ >= 0); }

    /** \} */

    // non-interface methods

    operator int () const { return aluTwist_; }

    // apply twist in ALU numbering
    int apply ( int i ) const { return ((positive() ? i : 2*corners + 1 - i) + aluTwist_) % corners; }
    int apply ( int i, int codim ) const { return (codim == 0 ? i : (codim == 1 ? applyEdge( i ) : apply( i ))); }

  private:
    int applyEdge ( int i ) const { return ((positive() ? i : 2*corners - i) + aluTwist_) % corners; }


    static int aluEdge2duneEdge ( int i ) { return ((corners == 3) ? (3 - i) % 3 : (6 - swap23( i )) % 4); }
    static int duneEdge2aluEdge ( int i ) { return ((corners == 3) ? (3 - i) % 3 : swap23( (6 - i) % 4 )); }

    static int aluVertex2duneVertex ( int i ) { return ((corners == 3) ? i : swap23( i )); }
    static int duneVertex2aluVertex ( int i ) { return ((corners == 3) ? i : swap23( i )); }

    static int alu2dune ( int i, int codim ) { return (codim == 0 ? i : (codim == 1 ? aluEdge2duneEdge( i ) : aluVertex2duneVertex( i ))); }
    static int dune2alu ( int i, int codim ) { return (codim == 0 ? i : (codim == 1 ? duneEdge2aluEdge( i ) : aluVertex2duneVertex( i ))); }

    static int swap23 ( int i ) { return i ^ (i >> 1); }

    int aluTwist_;
  };



  // ALUTwist for dimension 1
  // ------------------------

  template<>
  class ALUTwist< 2, 1 >
  {
    typedef ALUTwist< 2, 1 > Twist;

    friend struct ALUTwistIterator< Twist >;

    template< class ctype >
    struct CoordVector
    {
      CoordVector ( const Twist &twist ) : twist_( twist ) {}

      FieldVector< ctype, 1 > operator[] ( int i ) const { return FieldVector< ctype, 1 >( ctype( twist_( i ) ) ); }

    private:
      const Twist &twist_;
    };

  public:
    /** \brief dimension */
    static const int dimension = 1;

    /**
     * \name Construction
     * \{
     */

    /** \brief default constructor; results in identity twist */
    ALUTwist () : aluTwist_( 0 ) {}

    explicit ALUTwist ( GeometryType ) : aluTwist_( 0 ) {}

    explicit ALUTwist ( int aluTwist ) : aluTwist_( aluTwist ) {}

    explicit ALUTwist ( bool positive ) : aluTwist_( positive ) {}

    /** \} */

    /**
     * \name Copying and Assignment
     * \{
     */

    /** \brief copy constructor */
    ALUTwist ( const ALUTwist &other ) : aluTwist_( other.aluTwist_ ) {}

    /** \brief assignment operator */
    ALUTwist &operator= ( const ALUTwist &other ) { aluTwist_ = other.aluTwist_; return *this; }

    /** \} */

    /**
     * \name Group Operations
     * \{
     */

    /** \brief composition */
    ALUTwist operator* ( const ALUTwist &other ) const { return ALUTwist( aluTwist_ ^ other.aluTwist_ ); }

    /** \brief composition with inverse */
    ALUTwist operator/ ( const ALUTwist &other ) const { return (*this * other.inverse()); }

    /** \brief composition */
    ALUTwist &operator*= ( const ALUTwist &other ) { return (*this = (*this) * other); }

    /** \brief composition with inverse */
    ALUTwist &operator/= ( const ALUTwist &other ) { return (*this = (*this) / other); }

    /** \brief obtain inverse */
    ALUTwist inverse () const { return *this; }

    /** \} */

    /**
     * \brief Comparison
     * \{
     */

    /** \brief check for equality */
    bool operator== ( const ALUTwist &other ) const { return (aluTwist_ == other.aluTwist_); }

    /** \brief check for inequality */
    bool operator!= ( const ALUTwist &other ) const { return (aluTwist_ != other.aluTwist_); }

    /** \} */

    /**
     * \brief Topological Corner Mapping
     * \{
     */

    /** \brief obtain type of reference element */
    GeometryType type () const { return GeometryTypes::cube(dimension); }

    /**
     * \brief map i-th corner
     *
     * \param[in]  i  corner index
     *
     * \returns mapped index
     */
    int operator() ( int i ) const { return (i ^ aluTwist_); }

    int operator() ( int i, int codim ) const { return (codim == 0 ? i : (*this)( i )); }

    /** \} */

    /**
     * \brief Geometric Equivalent
     * \{
     */

    /** \brief cast into affine geometry \f$F\f$ */
    template< class ctype >
    operator AffineGeometry< ctype, dimension, dimension > () const
    {
      const CoordVector< ctype > coordVector( *this );
      return AffineGeometry< ctype, dimension, dimension >( type(), coordVector );
    }

    /** \brief equivalent to \f$|DF| > 0\f$ */
    bool positive () const { return (aluTwist_ == 0); }

    /** \} */

    operator int () const { return aluTwist_; }

  private:
    int aluTwist_;
  };




  // ALUTwists for dimension 2
  // -------------------------

  template< int corners >
  class ALUTwists< corners, 2 >
  {
  public:
    /** \brief dimension */
    static const int dimension = 2;

    typedef ALUTwist< corners, 2 > Twist;

    typedef ALUTwistIterator< Twist > Iterator;

    /** \brief obtain type of reference element */
    GeometryType type () const
    {
      if( corners == 3 )
        return GeometryTypes::simplex(dimension);
      else
        return GeometryTypes::cube(dimension);
    }

    /** \brief obtain number of twists in the group */
    std::size_t size () const { return 2*corners; }

    /** \brief obtain index of a given twist */
    std::size_t index ( const Twist &twist ) const { return static_cast< int >( twist ) + corners; }

    /**
     * \name Iterators
     * \{
     */

    /** \brief obtain begin iterator */
    Iterator begin () const { return Iterator( Twist( -corners ) ); }

    /** \brief obtain end iterator */
    Iterator end () const { return Iterator( Twist( corners ) ); }

    template< class Permutation >
    Iterator find ( const Permutation &permutation ) const
    {
      // calculate twist (if permutation is a valid twist)
      const int origin = duneVertex2aluVertex( permutation( aluVertex2duneVertex( 0 ) ) );
      const int next = duneVertex2aluVertex( permutation( aluVertex2duneVertex( 1 ) ) );
      const Twist twist( origin, (origin + 1) % corners == next );

      // check twist equals permutation (i.e., permutation is valid)
      for( int i = 0; i < corners; ++i )
      {
        if( twist( i ) != permutation( i ) )
          return end();
      }
      return Iterator( twist );
    }

    /** \} */

  private:
    static int aluVertex2duneVertex ( int i ) { return ((corners == 3) ? i : swap23( i )); }
    static int duneVertex2aluVertex ( int i ) { return ((corners == 3) ? i : swap23( i )); }

    static int swap23 ( int i ) { return i ^ (i >> 1); }
  };



  // ALUTwists for dimension 1
  // -------------------------

  template<>
  class ALUTwists< 2, 1 >
  {
  public:
    /** \brief dimension */
    static const int dimension = 1;

    typedef ALUTwist< 2, 1 > Twist;

    typedef ALUTwistIterator< Twist > Iterator;

    /** \brief obtain type of reference element */
    GeometryType type () const { return GeometryTypes::cube(dimension); }

    /** \brief obtain number of twists in the group */
    std::size_t size () const { return 2; }

    /** \brief obtain index of a given twist */
    std::size_t index ( const Twist &twist ) const { return static_cast< int >( twist ); }

    /**
     * \name Iterators
     * \{
     */

    /** \brief obtain begin iterator */
    Iterator begin () const { return Iterator( Twist( 0 ) ); }

    /** \brief obtain end iterator */
    Iterator end () const { return Iterator( Twist( int( size() ) ) ); }

    template< class Permutation >
    Iterator find ( const Permutation &permutation ) const { return Iterator( Twist( permutation( 0 ) ) ); }

    /** \} */
  };



  // TrivialTwist
  // ------------

  template< unsigned int topologyId, int dim >
  struct TrivialTwist
  {
    /** \brief dimension */
    static const int dimension = dim;

    /**
     * \name Construction
     * \{
     */

    /** \brief default constructor; results in identity twist */
    TrivialTwist () {}

    explicit TrivialTwist ( GeometryType ) {}

    /** \} */

    /**
     * \name Copying and Assignment
     * \{
     */

    /** \brief copy constructor */
    TrivialTwist ( const TrivialTwist & ) {}

    /** \brief assignment operator */
    TrivialTwist &operator= ( const TrivialTwist & ) { return *this; }

    /** \} */

    /**
     * \name Group Operations
     * \{
     */

    /** \brief composition */
    TrivialTwist operator* ( const TrivialTwist & ) const { return *this; }

    /** \brief composition with inverse */
    TrivialTwist operator/ ( const TrivialTwist & ) const { return *this; }

    /** \brief composition */
    TrivialTwist &operator*= ( const TrivialTwist & ) const { return *this; }

    /** \brief composition with inverse */
    TrivialTwist &operator/= ( const TrivialTwist & ) const { return *this; }

    /** \brief obtain inverse */
    TrivialTwist inverse () const { return *this; }

    /** \} */

    /**
     * \brief Comparison
     * \{
     */

    /** \brief check for equality */
    bool operator== ( const TrivialTwist & ) const { return true; }

    /** \brief check for inequality */
    bool operator!= ( const TrivialTwist & ) const { return false; }

    /** \} */

    /**
     * \brief Topological Corner Mapping
     * \{
     */

    /** \brief obtain type of reference element */
    GeometryType type () const { return GeometryType( topologyId, dim ); }

    /**
     * \brief map i-th corner
     *
     * \param[in]  i  corner index
     *
     * \returns mapped index
     */
    int operator() ( int i ) const { return i; }

    int operator() ( int i, int codim ) const { return i; }

    /** \} */

    /**
     * \brief Geometric Equivalent
     * \{
     */

    /** \brief cast into affine geometry \f$F\f$ */
    template< class ctype >
    operator AffineGeometry< ctype, dimension, dimension > () const
    {
      return ReferenceElements< ctype, dimension >::general( type() ).template geometry< 0 >( 0 );
    }

    /** \brief equivalent to \f$|DF| > 0\f$ */
    bool positive () const { return true; }

    /** \} */

    operator int () const { return 0; }
  };



  // TrivialTwists
  // -------------

  template< unsigned int topologyId, int dim >
  struct TrivialTwists
    : private TrivialTwist< topologyId, dim >
  {
    /** \brief dimension */
    static const int dimension = dim;

    typedef TrivialTwist< topologyId, dim > Twist;

    typedef const Twist *Iterator;

    TrivialTwists () {}
    explicit TrivialTwists ( GeometryType type ) {}

    /** \brief obtain type of reference element */
    GeometryType type () const { return twist().type(); }

    /** \brief obtain number of twists in the group */
    static std::size_t size () { return 1; }

    /** \brief obtain index of a given twist */
    static std::size_t index ( const Twist & ) { return 0; }

    /**
     * \name Iterators
     * \{
     */

    /** \brief obtain begin iterator */
    Iterator begin () const { return &twist(); }

    /** \brief obtain end iterator */
    Iterator end () const { return begin() + size(); }

    template< class Permutation >
    Iterator find ( const Permutation &permutation ) const // noexcept
    {
      // check whether permutation is the identity (i.e., permutation is valid)
      const int corners = ReferenceElements< double, dimension >::general( type() ).size( dimension );
      for( int i = 0; i < corners; ++i )
      {
        if( i != permutation( i ) )
          return end();
      }
      return begin();
    }

    /** \} */

  private:
    const Twist &twist () const { return *this; }
  };

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_COMMON_TWISTS_HH
