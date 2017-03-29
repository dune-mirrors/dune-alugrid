#ifndef DUNE_ALUGRID_PICKLE_HH
#define DUNE_ALUGRID_PICKLE_HH

#include <memory>
#include <tuple>

#if HAVE_DUNE_COREPY
#include <dune/corepy/common/pickle.hh>
#endif // #if HAVE_DUNE_COREPY

#include <dune/alugrid/impl/macrofileheader.hh>

#include <dune/alugrid/common/declaration.hh>
#include <dune/alugrid/3d/gridview.hh>

namespace Dune
{

  namespace CorePy
  {

    // Pickle for LevelGridView
    // ------------------------

#if HAVE_DUNE_COREPY
    template< class Grid >
    struct Pickle< GridView< ALU3dLevelGridViewTraits< Grid, All_Partition > >, void >
    {
      typedef GridView< ALU3dLevelGridViewTraits< Grid, All_Partition > > Self;

      typedef std::tuple< const Grid &, int > State;

      static State getState ( const Self &self )
      {
        return State( self.grid(), self.impl().level() );
      }

      static std::unique_ptr< Self > setState ( const State &state )
      {
        return std::unique_ptr< Self >( new Self( std::get< 0 >( state ).levelGridView( std::get< 1 >( state ) ) ) );
      }
    };
#endif // #if HAVE_DUNE_COREPY



    // Pickle for LeafGridView
    // -----------------------

#if HAVE_DUNE_COREPY
    template< class Grid >
    struct Pickle< GridView< ALU3dLeafGridViewTraits< Grid, All_Partition > >, void >
    {
      typedef GridView< ALU3dLeafGridViewTraits< Grid, All_Partition > > Self;

      typedef std::tuple< const Grid & > State;

      static State getState ( const Self &self )
      {
        return State( self.grid() );
      }

      static std::unique_ptr< Self > setState ( const State &state )
      {
        return std::unique_ptr< Self >( new Self( std::get< 0 >( state ).leafGridView() ) );
      }
    };
#endif // #if HAVE_DUNE_COREPY

  } // namespace CorePy

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_PICKLE_HH
