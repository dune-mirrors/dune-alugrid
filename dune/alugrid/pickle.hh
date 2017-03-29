#ifndef DUNE_ALUGRID_PICKLE_HH
#define DUNE_ALUGRID_PICKLE_HH

#include <sstream>
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

    // Pickler for ALUGrid
    // -------------------

#if HAVE_DUNE_COREPY
    template< int dim, int dimworld, ALUGridElementType elType, ALUGridRefinementType refineType, class Comm >
    struct Pickler< ALUGrid< dim, dimworld, elType, refineType, Comm >, void >
    {
      typedef ALUGrid< dim, dimworld, elType, refineType, Comm > Self;

      typedef pybind11::bytes State;

      static State getState ( const Self &self )
      {
        std::ostringstream stream;
        self.backup( stream, ::ALUGrid::MacroFileHeader::defaultFormat );
        return stream.str();
      }

      static void setState ( Self &self, const State &state )
      {
        std::istringstream stream( state );
        new (&self) Self();
        self.restore( stream );
      }
    };
#endif // #if HAVE_DUNE_COREPY



    // Pickler for LevelGridView
    // -------------------------

#if HAVE_DUNE_COREPY
    template< class Grid >
    struct Pickler< GridView< ALU3dLevelGridViewTraits< Grid, All_Partition > >, void >
    {
      typedef GridView< ALU3dLevelGridViewTraits< Grid, All_Partition > > Self;

      typedef std::tuple< const Grid &, int > State;

      static State getState ( const Self &self )
      {
        return State( self.grid(), self.impl().level() );
      }

      static void setState ( Self &self, const State &state )
      {
        new (&self) Self( std::get< 0 >( state ).levelGridView( std::get< 1 >( state ) ) );
      }
    };
#endif // #if HAVE_DUNE_COREPY



    // Pickler for LeafGridView
    // ------------------------

#if HAVE_DUNE_COREPY
    template< class Grid >
    struct Pickler< GridView< ALU3dLeafGridViewTraits< Grid, All_Partition > >, void >
    {
      typedef GridView< ALU3dLeafGridViewTraits< Grid, All_Partition > > Self;

      typedef std::tuple< const Grid & > State;

      static State getState ( const Self &self )
      {
        return State( self.grid() );
      }

      static void setState ( Self &self, const State &state )
      {
        new (&self) Self( std::get< 0 >( state ).leafGridView() );
      }
    };
#endif // #if HAVE_DUNE_COREPY

  } // namespace CorePy

} // namespace Dune

#endif // #ifndef DUNE_ALUGRID_PICKLE_HH
