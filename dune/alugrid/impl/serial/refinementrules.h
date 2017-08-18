#ifndef REFINEMENTRULES_H_INCLUDED
#define REFINEMENTRULES_H_INCLUDED

#include <dune/alugrid/common/alugrid_assert.hh>
#include <iostream>

namespace ALUGrid
{

  struct RefinementRules
  {
    // Die Verfeinerungsregeln sind nur enumerierte Typen, mit Zuweisung
    // Vergleich und Typkonversion, im Falle der Regeln f"ur die Dreiecks-
    // bzw Vierecksfl"ache sind aber auch Methoden n"otig, die eine Regel
    // mit dem Twist der Fl"ache mitdrehen, damit der "Ubergang der
    // Verfeinerung stimmt.

    struct Hedge1Rule
    {
      enum rule_enum { nosplit=1, iso2=2 };
      typedef signed char rule_t;

      explicit Hedge1Rule ( const rule_t & );
      Hedge1Rule ( const rule_enum & = nosplit );
      operator rule_t () const;
      inline bool isValid () const ;
      static inline bool isValid (const rule_t &);
      inline Hedge1Rule rotate (int) const ;

    private:
      rule_t _r ;
    } ;

    struct Hface3Rule
    {
      enum rule_enum { nosplit=1, e01=2, e12=3, e20=4, iso4=6, undefined=-2 };
      typedef signed char rule_t;

      explicit Hface3Rule ( const rule_t & );
      Hface3Rule ( const rule_enum & = nosplit );
      operator rule_t () const;
      inline bool isValid () const ;
      static inline bool isValid (const rule_t &) ;
      inline Hface3Rule rotate (int) const ;

      // return true if rule is one of the bisection rules
      bool bisection () const { return (_r >= e01) && (_r <= e20); }
    private :
      rule_t _r ;
    } ;

    struct Hface4Rule
    {
      enum rule_enum { nosplit=1, iso4=5,  undefined=-2 };
      typedef signed char rule_t;

      explicit Hface4Rule ( const rule_t & );
      Hface4Rule ( const rule_enum & = nosplit) ;
      operator rule_t () const;
      inline bool isValid () const ;
      static inline bool isValid (const rule_t &) ;
      inline Hface4Rule rotate (int) const ;

      // return true if rule is one of the bisection rules
      bool bisection () const { return false; }

    private :
      rule_t _r ;
    } ;

    struct TetraRule
    {
      enum rule_enum { crs=-1, nosplit=1,
                       e01=2, e12=3, e20=4, e23=5, e30=6, e31=7,
                       regular=8, bisect=9
                     };
      typedef signed char rule_t;

      explicit TetraRule ( const rule_t & );
      TetraRule ( const int );
      TetraRule ( const rule_enum & = nosplit) ;
      operator rule_t () const;
      inline bool isValid () const ;
      static inline bool isValid (const rule_t &) ;

      // return true if rule is one of the bisection rules
      bool bisection () const { return (_r >= e01) && (_r <= e31); }
    private :
      rule_t _r ;
    } ;

    struct HexaRule
    {
      enum rule_enum { crs = -1, nosplit = 1, regular=8 };
      typedef signed char rule_t;

      explicit HexaRule ( const rule_t & );
      HexaRule ( const int );
      HexaRule ( const rule_enum & = nosplit);
      operator rule_t () const;
      inline bool isValid () const ;
      static inline bool isValid (const rule_t &) ;

    private :
      rule_t _r ;
    } ;

  }; // end refinement rules

  // Info class for creating tetrahedrons,
  // bisection simplex type and orientation
  class SimplexTypeFlag
  {
    static constexpr signed char inValid = -127 ;
  public:
    signed char _flag;

    // default constructor
    SimplexTypeFlag() : _flag( inValid ) {}

    // constructor taking orientation and simplex type
    explicit SimplexTypeFlag( const int orientation,
                              const int type )
    {
      _flag = static_cast<signed char> ( type );
      alugrid_assert( _flag <= 2 );
      _flag += static_cast<signed char> (orientation) * 3 ;
      assert( _flag < 6 );
    }

    signed char orientation () const
    {
      alugrid_assert( _flag != inValid );
      return _flag / 3 ;
    }

    signed char type () const
    {
      alugrid_assert( _flag != inValid );
      return _flag % 3 ;
    }

    template <class stream>
    void write( stream& s ) const
    {
      s.put( _flag );
    }

    template <class stream>
    void read( stream& s )
    {
      _flag = s.get();
    }
  };

  // #     #                                    #    ######
  // #     #  ######  #####    ####   ######   ##    #     #  #    #  #       ######
  // #     #  #       #    #  #    #  #       # #    #     #  #    #  #       #
  // #######  #####   #    #  #       #####     #    ######   #    #  #       #####
  // #     #  #       #    #  #  ###  #         #    #   #    #    #  #       #
  // #     #  #       #    #  #    #  #         #    #    #   #    #  #       #
  // #     #  ######  #####    ####   ######  #####  #     #   ####   ######  ######


  inline RefinementRules :: Hedge1Rule :: Hedge1Rule ( const rule_t &r )
  : _r ( r )
  {
    alugrid_assert ( isValid() );
  }

  inline RefinementRules :: Hedge1Rule :: Hedge1Rule ( const rule_enum &r )
  : _r( r )
  {
    alugrid_assert ( isValid() );
  }

  inline RefinementRules :: Hedge1Rule :: operator rule_t () const
  {
    return int( _r );
  }

  inline bool RefinementRules :: Hedge1Rule :: isValid (const rule_t &r)
  {
    return r == nosplit || r == iso2 ;
  }

  inline bool RefinementRules :: Hedge1Rule :: isValid () const {
    return isValid( _r );
  }

  inline RefinementRules :: Hedge1Rule RefinementRules :: Hedge1Rule :: rotate (int i) const
  {
    alugrid_assert (i == 0 || i == 1) ;
    alugrid_assert ( _r == nosplit || _r == iso2 );
    return Hedge1Rule( _r );
  }

  inline std::ostream &operator<< ( std::ostream &out, const RefinementRules::Hedge1Rule &rule )
  {
    switch( rule )
    {
      case RefinementRules :: Hedge1Rule :: nosplit:
        return out << "nosplit";
      case RefinementRules :: Hedge1Rule :: iso2:
        return out << "iso2";
      default:
        return out << "!!! unknown !!!";
    }
  }


  // #     #                                  #####  ######
  // #     #  ######    ##     ####   ###### #     # #     #  #    #  #       ######
  // #     #  #        #  #   #    #  #            # #     #  #    #  #       #
  // #######  #####   #    #  #       #####   #####  ######   #    #  #       #####
  // #     #  #       ######  #       #            # #   #    #    #  #       #
  // #     #  #       #    #  #    #  #      #     # #    #   #    #  #       #
  // #     #  #       #    #   ####   ######  #####  #     #   ####   ######  ######

  inline RefinementRules :: Hface3Rule :: Hface3Rule ( const rule_t &r )
  : _r( r )
  {
    alugrid_assert ( _r == undefined || isValid() );
  }

  inline RefinementRules :: Hface3Rule :: Hface3Rule ( const rule_enum &r )
  : _r( r )
  {
    alugrid_assert ( _r == undefined || isValid() );
  }

  inline RefinementRules :: Hface3Rule :: operator rule_t () const
  {
    return int( _r );
  }

  inline bool RefinementRules :: Hface3Rule :: isValid (const rule_t& r) {
    return r == nosplit || r == iso4 || r == e01 || r == e12 || r == e20 ;
  }

  inline bool RefinementRules :: Hface3Rule :: isValid () const {
    return isValid( _r );
  }

  inline RefinementRules :: Hface3Rule RefinementRules :: Hface3Rule :: rotate (int t) const
  {
    alugrid_assert ((-4 < t) && (t < 3)) ;
    rule_t newr = _r ;
    switch (_r) {
    case nosplit :
    case iso4 :
      break ;
    case e01 :
      {
        //cout << "e01: my twist is " << t << endl;
        static const rule_t retRule [ 6 ] = { e01, e12, e20, e01, e20, e12 }; // double checked
        newr = retRule[ t + 3 ];
        break ;
      }
    case e12 :
      {
        //cout << "e12: my twist is " << t << endl;
        static const rule_t retRule [ 6 ] = { e20, e01, e12, e12, e01, e20 }; // double checked
        newr = retRule[ t + 3 ];
        break ;
      }
    case e20 :
      {
        //cout << "e20: my twist is " << t << endl;
        static const rule_t retRule [ 6 ] = { e12, e20, e01, e20, e12, e01 }; // double checked
        newr = retRule[ t + 3 ];
        break ;
      }
    default :
      std::cerr << __FILE__ << " " << __LINE__ << std::endl;
      abort () ;
      return Hface3Rule (nosplit) ;
    }
    // iso4 is not rotated
    return Hface3Rule( newr );
  }

  inline std::ostream &operator<< ( std::ostream &out, const RefinementRules::Hface3Rule &rule )
  {
    switch( rule )
    {
      case RefinementRules :: Hface3Rule :: nosplit:
        return out << "nosplit";
      case RefinementRules :: Hface3Rule :: e01:
        return out << "e01";
      case RefinementRules :: Hface3Rule :: e12:
        return out << "e12";
      case RefinementRules :: Hface3Rule :: e20:
        return out << "e20";
      case RefinementRules :: Hface3Rule :: iso4:
        return out << "iso4";
      case RefinementRules :: Hface3Rule :: undefined:
        return out << "undefined";
      default:
        return out << "!!! unknown !!!";
    }
  }


  // #     #                                 #       ######
  // #     #  ######    ##     ####   ###### #    #  #     #  #    #  #       ######
  // #     #  #        #  #   #    #  #      #    #  #     #  #    #  #       #
  // #######  #####   #    #  #       #####  #    #  ######   #    #  #       #####
  // #     #  #       ######  #       #      ####### #   #    #    #  #       #
  // #     #  #       #    #  #    #  #           #  #    #   #    #  #       #
  // #     #  #       #    #   ####   ######      #  #     #   ####   ######  ######

  inline RefinementRules :: Hface4Rule :: Hface4Rule ( const rule_t &r )
  : _r( r )
  {
    alugrid_assert ( _r == undefined || isValid() );
  }

  inline RefinementRules :: Hface4Rule :: Hface4Rule ( const rule_enum &r )
  : _r( r )
  {
    alugrid_assert ( _r == undefined || isValid() );
  }

  inline RefinementRules :: Hface4Rule :: operator rule_t () const
  {
    return int( _r );
  }

  inline bool RefinementRules :: Hface4Rule :: isValid (const rule_t& r) {
    return r == nosplit ||  r == iso4 ;
  }

  inline bool RefinementRules :: Hface4Rule :: isValid () const {
    return isValid( _r );
  }

  inline RefinementRules :: Hface4Rule RefinementRules :: Hface4Rule :: rotate (int) const
  {
    switch (_r) {
    case nosplit :
      return Hface4Rule (nosplit) ;
    case iso4 :
      return Hface4Rule (iso4) ;
    default :
      std::cerr << __FILE__ << " " << __LINE__ << std::endl;
      abort () ;
      return Hface4Rule (nosplit) ;
    }
  }

  inline std::ostream &operator<< ( std::ostream &out, const RefinementRules::Hface4Rule &rule )
  {
    switch( rule )
    {
      case RefinementRules :: Hface4Rule :: nosplit:
        return out << "nosplit";
      case RefinementRules :: Hface4Rule :: iso4:
        return out << "iso4";
      case RefinementRules :: Hface4Rule :: undefined:
        return out << "undefined";
      default:
        return out << "!!! unknown !!!";
    }
  }


  // #######                                 ######
  //    #     ######   #####  #####     ##   #     #  #    #  #       ######
  //    #     #          #    #    #   #  #  #     #  #    #  #       #
  //    #     #####      #    #    #  #    # ######   #    #  #       #####
  //    #     #          #    #####   ###### #   #    #    #  #       #
  //    #     #          #    #   #   #    # #    #   #    #  #       #
  //    #     ######     #    #    #  #    # #     #   ####   ######  ######

  inline RefinementRules :: TetraRule :: TetraRule ( const rule_t &r )
  : _r( r )
  {
    alugrid_assert ( isValid() );
  }

  inline RefinementRules :: TetraRule :: TetraRule ( const int r )
  : _r( (rule_t) r )
  {
    alugrid_assert ( isValid() );
  }

  inline RefinementRules :: TetraRule :: TetraRule ( const rule_enum &r )
  : _r( r )
  {
    alugrid_assert ( isValid() );
  }

  inline RefinementRules :: TetraRule :: operator rule_t () const
  {
    return int( _r );
  }

  inline bool RefinementRules :: TetraRule :: isValid (const rule_t& r) {
    return r == crs || r == nosplit || r == regular || r == bisect ||
           r == e01 || r == e12     || r == e20  || r == e23    || r == e30 || r == e31;
  }

  inline bool RefinementRules :: TetraRule :: isValid () const {
    return isValid( _r );
  }

  inline std::ostream &operator<< ( std::ostream &out, const RefinementRules::TetraRule &rule )
  {
    switch( rule )
    {
      case RefinementRules :: TetraRule :: crs:
        return out << "coarsen";
      case RefinementRules :: TetraRule :: nosplit:
        return out << "nosplit";
      case RefinementRules :: TetraRule :: e01:
        return out << "e01";
      case RefinementRules :: TetraRule :: e12:
        return out << "e12";
      case RefinementRules :: TetraRule :: e20:
        return out << "e20";
      case RefinementRules :: TetraRule :: e23:
        return out << "e23";
      case RefinementRules :: TetraRule :: e30:
        return out << "e30";
      case RefinementRules :: TetraRule :: e31:
        return out << "e31";
      case RefinementRules :: TetraRule :: regular:
        return out << "regular";
      case RefinementRules :: TetraRule :: bisect:
        return out << "bisection";
      default:
        return out << "!!! unknown !!!";
    }
  }

  // #     #                         ######
  // #     #  ######  #    #    ##   #     #  #    #  #       ######
  // #     #  #        #  #    #  #  #     #  #    #  #       #
  // #######  #####     ##    #    # ######   #    #  #       #####
  // #     #  #         ##    ###### #   #    #    #  #       #
  // #     #  #        #  #   #    # #    #   #    #  #       #
  // #     #  ######  #    #  #    # #     #   ####   ######  ######

  inline RefinementRules :: HexaRule :: HexaRule ( const rule_t &r )
  : _r( r )
  {
    alugrid_assert ( isValid() );
  }

  inline RefinementRules :: HexaRule :: HexaRule ( const int r )
  : _r( (rule_t) r )
  {
    alugrid_assert ( isValid() );
  }

  inline RefinementRules :: HexaRule :: HexaRule ( const rule_enum &r )
  : _r( r )
  {
    alugrid_assert ( isValid() );
  }

  inline RefinementRules :: HexaRule :: operator rule_t () const
  {
    return int( _r );
  }

  inline bool RefinementRules :: HexaRule :: isValid (const rule_t& r) {
    return r == crs || r == nosplit || r == regular ;
  }

  inline bool RefinementRules :: HexaRule :: isValid () const {
    return isValid( _r );
  }

  inline std::ostream &operator<< ( std::ostream &out, const RefinementRules::HexaRule &rule )
  {
    switch( rule )
    {
      case RefinementRules :: HexaRule :: crs:
        return out << "coarsen";
      case RefinementRules :: HexaRule :: nosplit:
        return out << "nosplit";
      case RefinementRules :: HexaRule :: regular:
        return out << "regular";
      default:
        return out << "!!! unknown !!!";
    }
  }

} // namespace ALUGrid

#endif // #ifndef REFINEMENTRULES_H_INCLUDED
