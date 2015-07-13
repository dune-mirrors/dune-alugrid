#ifndef DUNE_ALUGRID_IMPL_PARALLEL_ZCURVE_HH
#define DUNE_ALUGRID_IMPL_PARALLEL_ZCURVE_HH

#include <cstddef>

#include <array>
#include <type_traits>

namespace ALUGridSFC
{

  // ZCurve
  // ------

  template< class I, int dim >
  struct ZCurve
  {
    typedef I Index;

    template <class Coordinate>
    void init( const Coordinate &a, const Coordinate &b )
    {
      for( int i = 0; i < dim; ++i )
      {
        o_[ i ] = std::min( double( a[ i ] ), double( b[ i ] ) );
        h_[ i ] = 1.0 / (std::max( double( a[ i ] ), double( b[ i ] ) ) - o_[ i ]);
      }
    }

    template< class Coordinate>
    ZCurve ( const Coordinate &a, const Coordinate &b )
    {
      init( a, b );
    }

    template< class Coordinate, class Comm >
    ZCurve ( const Coordinate &a, const Coordinate &b, const Comm&  )
    {
      init( a, b );
    }

    template< class Coordinate >
    Index index ( const Coordinate &x ) const
    {
      std::array< double, dim > y;
      for( int i = 0; i < dim; ++i )
        y[ i ] = (double( x[ i ] ) - o_[ i ]) * h_[ i ];
      return unitIndex( y );
    }

  private:
    static Index unitIndex ( std::array< double, dim > x )
    {
      Index index( 0 );
      const std::size_t depth = (8*sizeof( I ) - static_cast< std::size_t >( std::is_signed< I >::value )) / std::size_t( dim );
      for( std::size_t k = 0; k < depth; ++k )
      {
        for( int i = dim-1; i >= 0; --i )
        {
          index <<= 1;
          x[ i ] *= 2.0;
          if( x[ i ] < 1.0 )
            continue;

          x[ i ] -= 1.0;
          index |= 1;
        }
      }
      return index;
    }

    std::array< double, dim > o_, h_;
  };

} // namespace ALUGridSFC

#endif // #ifndef DUNE_ALUGRID_IMPL_PARALLEL_ZCURVE_HH
