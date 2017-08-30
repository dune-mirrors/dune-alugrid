#ifndef DUNE_ALUGRID_TEST_CHECKTWISTS_HH
#define DUNE_ALUGRID_TEST_CHECKTWISTS_HH

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>

#include <dune/geometry/affinegeometry.hh>
#include <dune/geometry/referenceelements.hh>

namespace Dune
{

  // Permutation
  // -----------

  template< class T >
  struct Permutation
  {
    struct Iterator
    {
      Iterator ( const T &permutation, int index ) : permutation_( &permutation ), index_( index ) {}

      int operator* () const { return (*permutation_)( index_ ); }

      bool operator== ( const Iterator &other ) const { return (index_ == other.index_); }
      bool operator!= ( const Iterator &other ) const { return (index_ != other.index_); }

      Iterator &operator++ () { ++index_; return *this; }

    private:
      const T *permutation_;
      int index_;
    };

    Permutation ( const T &permutation, int size )
      : permutation_( permutation ),
        size_( size )
    {}

    Permutation ( T &&permutation, int size )
      : permutation_( permutation ),
        size_( size )
    {}

    int operator() ( int i ) const { return permutation_( i ); }

    Iterator begin () const { return Iterator( permutation_, 0 ); }
    Iterator end () const { return Iterator( permutation_, size_ ); }

  private:
    T permutation_;
    int size_;
  };

  template< class T >
  inline Permutation< typename std::remove_reference< T >::type > permutation ( T &&t, int size )
  {
    return Permutation< typename std::remove_reference< T >::type >( std::forward< T >( t ), size );
  }

  template< class T >
  inline std::ostream &operator<< ( std::ostream &out, const Permutation< T > &permutation )
  {
    char delim = '[';
    // for( int i : permutation )
    for (auto it=permutation.begin();it!=permutation.end();++it)
    {
      int i=*it;
      out << delim << ' ' << i;
      delim = ',';
    }
    return out << " ]";
  }



  // checkTwists
  // -----------

  template< class Twists >
  bool checkTwists ( const Twists &twists )
  {
    const int dimension = Twists::dimension;

    typedef typename Twists::Twist Twist;

    bool success = true;

    const GeometryType type = twists.type();
    const auto& refElement = ReferenceElements< double, dimension >::general( type );
    const int corners = refElement.size( dimension );


    // checking indices

    const std::size_t count = std::distance( twists.begin(), twists.end() );
    if( count != twists.size() )
    {
      std::cerr << "Error: iterator visits " << count << " twists, but size is " << twists.size() << "." << std::endl;
      success = false;
    }

    if( success )
    {
      std::vector< bool > flags( twists.size(), false );
      // for( Twist t : twists )
      for (auto tit=twists.begin();tit!=twists.end();++tit)
      {
        Twist t=*tit;
        if( flags[ twists.index( t ) ] )
          std::cerr << "Error: Index " << twists.index( t ) << " encountered twice." << std::endl;
        flags[ twists.index( t ) ] = true;
      }
      for( std::size_t i = 0; i < twists.size(); ++i )
      {
        if( !flags[ i ] )
          std::cerr << "Error: Index " << i << " not used." << std::endl;
      }
    }


    // check group operations

    // for( Twist t : twists )
    for (auto tit=twists.begin();tit!=twists.end();++tit)
    {
      Twist t=*tit;
      if( (t.inverse().inverse() != t) || (t.inverse() * t != Twist( type )) || (t * t.inverse() != Twist( type )) )
      {
        std::cerr << "Error: Wrong inverse twist for " << permutation( t, corners ) << ": " << permutation( t.inverse(), corners ) << std::endl;
        success = false;
      }

      // for( Twist s : twists )
      for (auto sit=twists.begin();sit!=twists.end();++sit)
      {
        Twist s=*sit;
        const Twist st = s * t;
        if( twists.index( st ) >= twists.size() )
        {
          std::cerr << "Error: Composition " << permutation( s, corners ) << " * " << permutation( t, corners ) << " has invalid index: " << twists.index( st ) << std::endl;
          success = false;
        }

        bool equals = true;
        for( int i = 0; i < corners; ++i )
          equals &= (st( i ) == s( t( i ) ));
        if( !equals )
        {
          std::cerr << "Error: " << permutation( s, corners ) << " * " << permutation( t, corners ) << " != " << permutation( st, corners )
                    << ", i.e., " << twists.index( s ) << " * " << twists.index( t ) << " != " << twists.index( st ) << "." << std::endl;
          success = false;
        }

        if( st.positive() != !(s.positive() ^ t.positive()) )
        {
          std::cerr << "Error: Composition " << permutation( s, corners ) << " * " << permutation( t, corners ) << " has wrong positivity." << std::endl;
          success = false;
        }
      }
    }


    // check twists for intermediate codimensions

    // for( Twist t : twists )
    for (auto tit=twists.begin();tit!=twists.end();++tit)
    {
      Twist t=*tit;
      for( int codim = 1; codim < dimension; ++codim )
      {
        for( int i = 0; i < refElement.size( codim ); ++i )
        {
          const int j = t( i, codim );
          if( (j < 0) || (j >= refElement.size( codim )) )
          {
            std::cerr << "Error: " << permutation( t, corners )
                      << " maps subentity (" << i << ", " << codim << ") to invalid subentity (" << j << ", " << codim << ")."
                      << std::endl;
            success = false;
            continue;
          }

          if( refElement.type( i, codim ) != refElement.type( j, codim ) )
          {
            std::cerr << "Error: " << permutation( t, corners )
                      << " maps subentity (" << i << ", " << codim << ") of type " << refElement.type( i, codim )
                      << " to subentity (" << j << ", " << codim << ") of type " << refElement.type( j, codim ) << "."
                      << std::endl;
            success = false;
            continue;
          }

          std::vector< int > v1( refElement.size( i, codim, dimension ) );
          for( int k = 0; k < refElement.size( i, codim, dimension ); ++k )
            v1[ k ] = t( refElement.subEntity( i, codim, k, dimension ) );
          std::sort( v1.begin(), v1.end() );

          std::vector< int > v2( refElement.size( j, codim, dimension ) );
          for( int k = 0; k < refElement.size( j, codim, dimension ); ++k )
            v2[ k ] = refElement.subEntity( j, codim, k, dimension );
          std::sort( v2.begin(), v2.end() );

          assert( v1.size() == v2.size() );
          if( !std::equal( v1.begin(), v1.end(), v2.begin() ) )
          {
            std::cerr << "Error: " << permutation( t, corners ) << std::endl;
            std::cerr << "       maps corners of subentity (" << i << ", " << codim << ") to ";
            char delim = '[';
            for( int k = 0; k < refElement.size( i, codim, dimension ); ++k, delim = ',' )
              std::cerr << delim << ' ' << t( refElement.subEntity( i, codim, k, dimension ) );
            std::cerr <<" ]," << std::endl;
            std::cerr << "       but maps subentity (" << i << ", " << codim << ") to subentity (" << j << ", " << codim << ") with corners ";
            delim = '[';
            for( int k = 0; k < refElement.size( j, codim, dimension ); ++k, delim = ',' )
              std::cerr << delim << ' ' << refElement.subEntity( j, codim, k, dimension );
            std::cerr <<" ]," << std::endl;
            success = false;
            continue;
          }
        }
      }
    }


    // check twist geometry

    // for( Twist t : twists )
    for (auto tit=twists.begin();tit!=twists.end();++tit)
    {
      Twist t=*tit;
      const AffineGeometry< double, dimension, dimension > geometry( t );

      for( int i = 0; i < corners; ++i )
      {
        const FieldVector< double, dimension > &pos = refElement.position( t( i ), dimension );
        if( (geometry.corner( i ) - pos).two_norm() > 1e-10 )
        {
          std::cerr << "Error: Twist geometry maps corner " << i << " to " << geometry.corner( i ) << " instead of " <<  pos << "." << std::endl;
          success = false;
        }
      }

      const bool positive = (geometry.jacobianTransposed( refElement.position( 0, 0 ) ).determinant() > 0.0);
      if( t.positive() != positive )
      {
        std::cerr << "Error: Wrong result for positive: " << t.positive() << " (should be " << positive << ")." << std::endl;
        success = false;
      }
    }

    return success;
  }

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_TEST_CHECKTWISTS_HH
