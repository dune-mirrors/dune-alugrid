#ifndef DUNE_ALU_DOUBLE_HH
#define DUNE_ALU_DOUBLE_HH

//- system includes 
#include <iostream>
#include <cmath> 
#include <limits>
#include <vector>

//- Dune includes 

namespace ALUGridSpace
{

#ifdef COUNT_ALUGRID_FLOPS

  template< class FloatImp >
  class FlOpCounter;



  template<>
  class FlOpCounter< void >
  {
  private:
    typedef FlOpCounter< void > ThisType;

  protected:
    unsigned long count_;

  protected:
    inline FlOpCounter ()
    : count_( 0 )
    {
    }

  public:
    inline ~FlOpCounter ()
    {
      std :: cout << "ALUGridSpace: Total number of floating point operations: "
                  << count_ << std :: endl;
    }


    inline ThisType &operator++ ()
    {
      ++count_;
      return *this;
    }

    inline static ThisType &instance ()
    {
      static ThisType instance;
      return instance;
    }
  };



  template< class FloatImp >
  class FlOpCounter
  {
  public:
    typedef FloatImp FloatType;

  private:
    typedef FlOpCounter< FloatType > ThisType;

  protected:
    unsigned long count_;

  protected:
    inline FlOpCounter ()
    : count_( 0 )
    {
    }

  public:
    inline ~FlOpCounter ()
    {
      std :: cout << "ALUGridSpace: Number of floating point operations for "
                  << FloatType :: typeName() << ": "
                  << count_ << std :: endl;
    }

    inline ThisType &operator++ ()
    {
      ++count_;
      ++(FlOpCounter< void > :: instance());
      return *this;
    }

    inline static ThisType &instance ()
    {
      static ThisType instance;
      return instance;
    }
  };
  
#else

  template< class FloatImp >
  class FlOpCounter
  {
  public:
    typedef FloatImp FloatType;

  private:
    typedef FlOpCounter< FloatType > ThisType;

  protected:
    inline FlOpCounter ()
    {
    }

  public:
    inline ThisType &operator++ ()
    {
      return *this;
    }

    inline static ThisType &instance ()
    {
      static ThisType instance;
      return instance;
    }
  };

#endif

  //- forward declaration 
  class Double; 
  // wrap of std power 
  static double pow (const Double& v, const double p);
  // wrap of std log 
  static double log (const Double& v);
  // wrap of std sqrt 
  static double sqrt(const Double& v);
  // wrap of std sin 
  static double cos (const Double& v);
  // wrap of std cos 
  static double sin(const Double& v);
  // wrap of std power 
  static double pow (const Double& v, const double p);

  // wrap of std min  
  static inline double min (const Double& v, const double p);
  // wrap of std min  
  static inline double min (const double v, const Double& p);
  // wrap of std max
  static inline double max (const Double& v, const double p);
  // wrap of std max
  static inline double max (const double v, const Double& p);


  // numeric limits
  // --------------

  class Double
  {
  private:
    typedef Double ThisType;

    friend Double operator+ ( const Double&, const Double& );
    friend Double operator+ ( const Double&, const double );
    friend Double operator+ ( const double, const Double& );
    friend Double operator+ ( const Double&, const int );
    friend Double operator+ ( const int, const Double& );
    friend Double operator+ ( const Double&, const unsigned int );
    friend Double operator+ ( const unsigned int, const Double& );

    friend Double operator- ( const Double&, const Double& );
    friend Double operator- ( const Double&, const double );
    friend Double operator- ( const double, const Double& );
    friend Double operator- ( const Double&, const int );
    friend Double operator- ( const int, const Double& );
    friend Double operator- ( const Double&, const unsigned int );
    friend Double operator- ( const unsigned int, const Double& );
    
    friend Double operator* ( const Double&, const Double& );
    friend Double operator* ( const Double&, const double );
    friend Double operator* ( const double, const Double& );
    friend Double operator* ( const Double&, const int );
    friend Double operator* ( const int, const Double& );
    friend Double operator* ( const Double&, const unsigned int );
    friend Double operator* ( const unsigned int, const Double& );

    friend Double operator/ ( const Double&, const Double& );
    friend Double operator/ ( const double, const Double& );
    friend Double operator/ ( const Double&, const double );
    friend Double operator/ ( const int, const Double& );
    friend Double operator/ ( const Double&, const int );
    friend Double operator/ ( const unsigned int, const Double& );
    friend Double operator/ ( const Double&, const unsigned int );

    friend bool operator== ( const Double&, const Double& );
    friend bool operator== ( const double, const Double& );
    friend bool operator== ( const Double&, const double );
    friend bool operator== ( const int, const Double& );
    friend bool operator== ( const Double&, const int );
    friend bool operator== ( const unsigned int, const Double& );
    friend bool operator== ( const Double&, const unsigned int );
    
    friend bool operator!= ( const Double&, const Double& );
    friend bool operator!= ( const double, const Double& );
    friend bool operator!= ( const Double&, const double );
    friend bool operator!= ( const int, const Double& );
    friend bool operator!= ( const Double&, const int );
    friend bool operator!= ( const unsigned int, const Double& );
    friend bool operator!= ( const Double&, const unsigned int );
    
    friend bool operator< ( const Double&, const Double& );
    friend bool operator< ( const double, const Double& );
    friend bool operator< ( const Double&, const double );
    friend bool operator< ( const int, const Double& );
    friend bool operator< ( const Double&, const int );
    friend bool operator< ( const unsigned int, const Double& );
    friend bool operator< ( const Double&, const unsigned int );

    friend bool operator<= ( const Double&, const Double& );
    friend bool operator<= ( const double, const Double& );
    friend bool operator<= ( const Double&, const double );
    friend bool operator<= ( const int, const Double& );
    friend bool operator<= ( const Double&, const int );
    friend bool operator<= ( const unsigned int, const Double& );
    friend bool operator<= ( const Double&, const unsigned int );
    
    friend bool operator> ( const Double&, const Double& );
    friend bool operator> ( const double, const Double& );
    friend bool operator> ( const Double&, const double );
    friend bool operator> ( const int, const Double& );
    friend bool operator> ( const Double&, const int );
    friend bool operator> ( const unsigned int, const Double& );
    friend bool operator> ( const Double&, const unsigned int );

    friend bool operator>= ( const Double&, const Double& );
    friend bool operator>= ( const double, const Double& );
    friend bool operator>= ( const Double&, const double );
    friend bool operator>= ( const int, const Double& );
    friend bool operator>= ( const Double&, const int );
    friend bool operator>= ( const unsigned int, const Double& );
    friend bool operator>= ( const Double&, const unsigned int );

    friend std :: ostream &operator<< ( std :: ostream&, const Double& );
    friend std :: istream &operator>> ( std :: istream&, Double& );
    
    friend double pow (const Double& v, const double p);
    friend double log (const Double& v);
    friend double sqrt(const Double& v);
    friend double sin(const Double& v);
    friend double cos(const Double& v);

    friend Double abs ( const Double & );
    friend double min(const Double&, const double);
    friend double min(const double,  const Double&);
    friend double max(const Double&, const double);
    friend double max(const double,  const Double&);

    friend double accumulate( 
      std::vector< Double > :: iterator it,
      const std::vector< Double > :: iterator end, 
      const Double& init );

#if DUNE_FEM_COMPATIBILITY
    friend struct XdrIO< Double >;
#endif

    friend void field_cast ( const Double &, double & );

  protected:
    typedef FlOpCounter< ThisType > FlOpCounterType;

  protected:
    double value_;

  public:
    inline Double ()
    {}

    inline Double ( const double value )
    : value_( value )
    {}

    inline Double ( const ThisType &other )
    : value_( other.value_ )
    {}

/*
    inline operator int () const
    {
      return (int)value_;
    }
*/

    inline ThisType &operator= ( const ThisType other )
    {
      value_ = other.value_;
      return *this;
    }
    
    inline ThisType &operator+= ( const ThisType other )
    {
      flOp();
      value_ += other.value_;
      return *this;
    }
    
    inline ThisType &operator-= ( const ThisType other )
    {
      flOp();
      value_ -= other.value_;
      return *this;
    }
    
    inline ThisType &operator*= ( const ThisType other )
    {
      flOp();
      value_ *= other.value_;
      return *this;
    }
    
    inline ThisType &operator/= ( const ThisType other )
    {
      flOp();
      value_ /= other.value_;
      return *this;
    }

    Double operator- () const
    {
      flOp();
      return Double( -value_ );
    }

    static std :: string typeName ()
    {
      return "Double";
    }

  protected:
    static inline void flOp ()
    {
      ++(FlOpCounterType :: instance());
    }
  };

  // min/max
  // ---------

  // wrap of std min  
  static inline double min (const Double& v, const double p)
  {
    return (v.value_ > p) ? p : v.value_;   
  }

  // wrap of std min  
  static inline double min (const double v, const Double& p)
  {
    return (v > p.value_) ? p.value_ : v;   
  }

  // wrap of std max
  static inline double max (const Double& v, const double p) 
  {
    return (v.value_ < p) ? p : v.value_;   
  }

  // wrap of std max
  static inline double max (const double v, const Double& p) 
  {
    return (v < p.value_) ? p.value_ : v;   
  }

  // operator+
  // ---------

  inline Double operator+ ( const Double &a, const Double &b )
  {
    Double :: flOp();
    return Double( a.value_ + b.value_ );
  }

  inline Double operator+ ( const double a, const Double &b )
  {
    Double :: flOp();
    return Double( a + b.value_ );
  }
  
  inline Double operator+ ( const Double &a, const double b )
  {
    Double :: flOp();
    return Double( a.value_ + b );
  }
  
  inline Double operator+ ( const int a, const Double &b )
  {
    Double :: flOp();
    return Double( a + b.value_ );
  }
  
  inline Double operator+ ( const Double &a, const int b )
  {
    Double :: flOp();
    return Double( a.value_ + b );
  }
  
  inline Double operator+ ( const unsigned int a, const Double &b )
  {
    Double :: flOp();
    return Double( a + b.value_ );
  }
  
  inline Double operator+ ( const Double &a, const unsigned int b )
  {
    Double :: flOp();
    return Double( a.value_ + b );
  }



  // operator-
  // ---------
  
  inline Double operator- ( const Double &a, const Double &b )
  {
    Double :: flOp();
    return Double( a.value_ - b.value_ );
  }

  inline Double operator- ( const double a, const Double &b )
  {
    Double :: flOp();
    return Double( a - b.value_ );
  }
  
  inline Double operator- ( const Double &a, const double b )
  {
    Double :: flOp();
    return Double( a.value_ - b );
  }
  
  inline Double operator- ( const int a, const Double &b )
  {
    Double :: flOp();
    return Double( a - b.value_ );
  }
  
  inline Double operator- ( const Double &a, const int b )
  {
    Double :: flOp();
    return Double( a.value_ - b );
  }
  
  inline Double operator- ( const unsigned int a, const Double &b )
  {
    Double :: flOp();
    return Double( a - b.value_ );
  }
  
  inline Double operator- ( const Double &a, const unsigned int b )
  {
    Double :: flOp();
    return Double( a.value_ - b );
  }

  

  // operator*
  // ---------

  inline Double operator* ( const Double &a, const Double &b )
  {
    Double :: flOp();
    return Double( a.value_ * b.value_ );
  }

  inline Double operator* ( const double a, const Double &b )
  {
    Double :: flOp();
    return Double( a * b.value_ );
  }
  
  inline Double operator* ( const Double &a, const double b )
  {
    Double :: flOp();
    return Double( a.value_ * b );
  }
  
  inline Double operator* ( const int a, const Double &b )
  {
    Double :: flOp();
    return Double( a * b.value_ );
  }
  
  inline Double operator* ( const Double &a, const int b )
  {
    Double :: flOp();
    return Double( a.value_ * b );
  }
  
  inline Double operator* ( const unsigned int a, const Double &b )
  {
    Double :: flOp();
    return Double( a * b.value_ );
  }
  
  inline Double operator* ( const Double &a, const unsigned int b )
  {
    Double :: flOp();
    return Double( a.value_ * b );
  }



  // operator/
  // ---------
  
  inline Double operator/ ( const Double &a, const Double &b )
  {
    Double :: flOp();
    return Double( a.value_ / b.value_ );
  }

  inline Double operator/ ( const double a, const Double &b )
  {
    Double :: flOp();
    return Double( a / b.value_ );
  }
  
  inline Double operator/ ( const Double &a, const double b )
  {
    Double :: flOp();
    return Double( a.value_ / b );
  }

  inline Double operator/ ( const int a, const Double &b )
  {
    Double :: flOp();
    return Double( a / b.value_ );
  }
  
  inline Double operator/ ( const Double &a, const int b )
  {
    Double :: flOp();
    return Double( a.value_ / b );
  }

  inline Double operator/ ( const unsigned int a, const Double &b )
  {
    Double :: flOp();
    return Double( a / b.value_ );
  }
  
  inline Double operator/ ( const Double &a, const unsigned int b )
  {
    Double :: flOp();
    return Double( a.value_ / b );
  }
  


  // operator==
  // ----------
  
  inline bool operator== ( const Double &a, const Double &b )
  {
    return (a.value_ == b.value_);
  }
  
  inline bool operator== ( const double a, const Double &b )
  {
    return (a == b.value_);
  }
   
  inline bool operator== ( const Double &a, const double b )
  {
    return (a.value_ == b);
  }
  
  inline bool operator== ( const int a, const Double &b )
  {
    return (a == b.value_);
  }
   
  inline bool operator== ( const Double &a, const int b )
  {
    return (a.value_ == b);
  }

  inline bool operator== ( const unsigned int a, const Double &b )
  {
    return (a == b.value_);
  }
   
  inline bool operator== ( const Double &a, const unsigned int b )
  {
    return (a.value_ == b);
  }



  // operator!=
  // ----------
  
  inline bool operator!= ( const Double &a, const Double &b )
  {
    return (a.value_ != b.value_);
  }
  
  inline bool operator!= ( const double a, const Double &b )
  {
    return (a != b.value_);
  }
   
  inline bool operator!= ( const Double &a, const double b )
  {
    return (a.value_ != b);
  }
  
  inline bool operator!= ( const int a, const Double &b )
  {
    return (a != b.value_);
  }
   
  inline bool operator!= ( const Double &a, const int b )
  {
    return (a.value_ != b);
  }

  inline bool operator!= ( const unsigned int a, const Double &b )
  {
    return (a != b.value_);
  }
   
  inline bool operator!= ( const Double &a, const unsigned int b )
  {
    return (a.value_ != b);
  }



  // operator<
  // ---------
  
  inline bool operator< ( const Double &a, const Double &b )
  {
    return (a.value_ < b.value_);
  }
  
  inline bool operator< ( const double a, const Double &b )
  {
    return (a < b.value_);
  }
   
  inline bool operator< ( const Double &a, const double b )
  {
    return (a.value_ < b);
  }
  
  inline bool operator< ( const int a, const Double &b )
  {
    return (a < b.value_);
  }
   
  inline bool operator< ( const Double &a, const int b )
  {
    return (a.value_ < b);
  }

  inline bool operator< ( const unsigned int a, const Double &b )
  {
    return (a < b.value_);
  }
   
  inline bool operator< ( const Double &a, const unsigned int b )
  {
    return (a.value_ < b);
  }



  // operator<=
  // ----------
  
  inline bool operator<= ( const Double &a, const Double &b )
  {
    return (a.value_ <= b.value_);
  }
  
  inline bool operator<= ( const double a, const Double &b )
  {
    return (a <= b.value_);
  }
   
  inline bool operator<= ( const Double &a, const double b )
  {
    return (a.value_ <= b);
  }
  
  inline bool operator<= ( const int a, const Double &b )
  {
    return (a <= b.value_);
  }
   
  inline bool operator<= ( const Double &a, const int b )
  {
    return (a.value_ <= b);
  }

  inline bool operator<= ( const unsigned int a, const Double &b )
  {
    return (a <= b.value_);
  }
   
  inline bool operator<= ( const Double &a, const unsigned int b )
  {
    return (a.value_ <= b);
  }



  // operator>
  // ---------
  
  inline bool operator> ( const Double &a, const Double &b )
  {
    return (a.value_ > b.value_);
  }
  
  inline bool operator> ( const double a, const Double &b )
  {
    return (a > b.value_);
  }
   
  inline bool operator> ( const Double &a, const double b )
  {
    return (a.value_ > b);
  }
  
  inline bool operator> ( const int a, const Double &b )
  {
    return (a > b.value_);
  }
   
  inline bool operator> ( const Double &a, const int b )
  {
    return (a.value_ > b);
  }

  inline bool operator> ( const unsigned int a, const Double &b )
  {
    return (a > b.value_);
  }
   
  inline bool operator> ( const Double &a, const unsigned int b )
  {
    return (a.value_ > b);
  }



  // operator>=
  // ----------
  
  inline bool operator>= ( const Double &a, const Double &b )
  {
    return (a.value_ >= b.value_);
  }
  
  inline bool operator>= ( const double a, const Double &b )
  {
    return (a >= b.value_);
  }
   
  inline bool operator>= ( const Double &a, const double b )
  {
    return (a.value_ >= b);
  }
  
  inline bool operator>= ( const int a, const Double &b )
  {
    return (a >= b.value_);
  }
   
  inline bool operator>= ( const Double &a, const int b )
  {
    return (a.value_ >= b);
  }

  inline bool operator>= ( const unsigned int a, const Double &b )
  {
    return (a >= b.value_);
  }
   
  inline bool operator>= ( const Double &a, const unsigned int b )
  {
    return (a.value_ >= b);
  }
  


  // stream operators
  // ----------------
  
  inline std :: ostream &operator<< ( std :: ostream &out, const Double &a )
  {
    return out << a.value_;
  }
  
  inline std :: istream &operator>> ( std :: istream &in, Double &a )
  {
    return in >> a.value_;
  }

  // standard functions
  // ------------------

  inline Double abs ( const Double &a )
  {
    return Double( std::abs( a.value_ ) );
  }

  static inline double pow (const Double& v, const double p)
  {
    return std::pow(v.value_,p);
  }

  static inline double log (const Double& v)
  {
    return std::log(v.value_);
  }

  static inline double sqrt(const Double& v)
  {
    return std::sqrt(v.value_);
  }

  static inline double sin (const Double& v)
  {
    return std::sin(v.value_);
  }

  static inline double cos(const Double& v)
  {
    return std::cos(v.value_);
  }


#if DUNE_FEM_COMPATIBILITY
  // XdrIO
  // -----
  
  template<>
  struct XdrIO< Double >
  {
    static inline int io( XDR *xdrs, Double &a )
    {
      return XdrIO< double > :: io( xdrs, a.value_ );
    }
  };
#endif


  inline void field_cast ( const Double &f1, double &f2 )
  {
    f2 = f1.value_;
  }

  inline double accumulate( 
      std::vector< Double > :: iterator it,
      const std::vector< Double > :: iterator end, 
      const Double& init )
  {
    Double sum = init;
    for( ; it != end; ++it ) 
    {
      sum += *it;
    }
    return sum.value_;
  }

} // end namespace ALUGridSpace 



namespace std
{

  inline ALUGridSpace::Double abs ( const ALUGridSpace::Double &a )
  {
    return ALUGridSpace::abs( a );
  }

  inline ALUGridSpace::Double fabs ( const ALUGridSpace::Double &a )
  {
    return ALUGridSpace::abs( a );
  }

  // wrap of std power 
  inline double min (const ALUGridSpace::Double& v, const double p)
  {
    return ALUGridSpace::min(v,p);
  }

  // wrap of std power 
  inline double min (const double v, const ALUGridSpace::Double& p)
  {
    return ALUGridSpace::min(v,p);
  }

  // wrap of std power 
  inline double max (const ALUGridSpace::Double& v, const double p) 
  {
    return ALUGridSpace::max(v,p);
  }

  // wrap of std power 
  inline double max (const double v, const ALUGridSpace::Double& p) 
  {
    return ALUGridSpace::max(v,p);
  }

  // numeric limits
  // --------------

  template<>
  struct numeric_limits< ALUGridSpace :: Double >
  {
    static const bool is_specialized = true;

    static const int radix = numeric_limits< double > :: radix;
    static const int digits = numeric_limits< double > :: digits;
    static const int digits10 = numeric_limits< double > :: digits10;

    static const bool is_signed = numeric_limits< double > :: is_signed;
    static const bool is_integer = numeric_limits< double > :: is_integer;
    static const bool is_exact = numeric_limits< double > :: is_exact;

    inline static ALUGridSpace :: Double min () throw ()
    {
      return ALUGridSpace :: Double( numeric_limits< double > :: min() );
    }

    inline static ALUGridSpace :: Double max () throw ()
    {
      return ALUGridSpace :: Double( numeric_limits< double > :: max() );
    }

    inline static ALUGridSpace :: Double epsilon () throw ()
    {
      return ALUGridSpace :: Double( numeric_limits< double > :: epsilon() );
    }

    inline static ALUGridSpace :: Double round_error () throw ()
    {
      return ALUGridSpace :: Double( numeric_limits< double > :: round_error() );
    }

    inline static ALUGridSpace :: Double infinity () throw ()
    {
      return ALUGridSpace :: Double( numeric_limits< double > :: infinity() );
    }

    inline static ALUGridSpace :: Double quiet_NaN () throw ()
    {
      return ALUGridSpace :: Double( numeric_limits< double > :: quiet_NaN() );
    }

    inline static ALUGridSpace :: Double signaling_NaN () throw ()
    {
      return ALUGridSpace :: Double( numeric_limits< double > :: signaling_NaN() );
    }

    inline static ALUGridSpace :: Double denorm_min () throw ()
    {
      return ALUGridSpace :: Double( numeric_limits< double > :: denorm_min() );
    }

    static const int min_exponent = numeric_limits< double > :: min_exponent;
    static const int max_exponent = numeric_limits< double > :: max_exponent;
    static const int min_exponent10 = numeric_limits< double > :: min_exponent10;
    static const int max_exponent10 = numeric_limits< double > :: max_exponent10;

    static const bool has_infinity = numeric_limits< double > :: has_infinity;
    static const bool has_quiet_NaN = numeric_limits< double > :: has_quiet_NaN;
    static const bool has_signaling_NaN = numeric_limits< double > :: has_signaling_NaN;
    static const float_denorm_style has_denorm = numeric_limits< double > :: has_denorm;
    static const bool has_denorm_loss = numeric_limits< double > :: has_denorm_loss;

    static const bool is_iec559 = numeric_limits< double > :: is_iec559;
    static const bool is_bounded = numeric_limits< double > :: is_bounded;
    static const bool is_modulo = numeric_limits< double > :: is_modulo;

    static const bool traps = numeric_limits< double > :: traps;
    static const bool tinyness_before = numeric_limits< double > :: tinyness_before;
    static const float_round_style round_style
      = numeric_limits< double > :: round_style;
  };
}

#endif
