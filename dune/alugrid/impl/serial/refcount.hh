#ifndef ALUGRID_SRC_SERIAL_REFCOUNT_HH
#define ALUGRID_SRC_SERIAL_REFCOUNT_HH

namespace ALUGrid
{

  // Einfacher Referenzenz"ahler mit cast-around-const
  // feature, der zum Z"ahlen der Referenzen auf Fl"achen
  // Kanten und Knoten verwendet wird. Vorteil: Objekte,
  // die einen Z"ahler dieser Klasse enthalten, werden
  // durch Inkrementierung bzw. Dekrementierung des Z"ahlers
  // nicht ver"andert (k"onnen also auch 'const' sein).

  template <class T>
  class RefcountImpl
  {
#ifdef ALUGRIDDEBUG
#ifdef DEBUG_ALUGRID
    // Der Globale Z"ahler soll helfen, nicht gel"oschte
    // Gitterobjekte oder Iteratorobjekte zu erkennen.
    // (Wird aber nur in den DEBUG-Versionen angelegt.)
    //
    // Refcounting only turned on, if NDEBUG is not defined and
    // DEBUG_ALUGRID is defined
    class Globalcount
    {
      mutable int _c;
    public:
      Globalcount () : _c(0) {}
      void operator++ ( int ) const { ++_c; }
      void operator-- ( int ) const { ++_c; }
    };

    static Globalcount _g;
#endif // #ifdef DEBUG_ALUGRID
#endif // #ifdef ALUGRIDDEBUG


  public:
    mutable T _c;

    void reset () { _c = 0; }
    bool positive () const { return _c > 0; }
    RefcountImpl ()
      : _c(0)
    {
#ifdef ALUGRIDDEBUG
#ifdef DEBUG_ALUGRID
      ++_g;
#endif
#endif
    }
    ~RefcountImpl ()
    {
#ifdef ALUGRIDDEBUG
#ifdef DEBUG_ALUGRID
      --_g;
#endif
#endif
    }

    int operator++ ( int ) const
    {
      alugrid_assert( sizeof(T) == sizeof(unsigned char) ? _c < 255 : true );
      return _c++;
    }
    int operator++ () const
    {
      alugrid_assert( sizeof(T) == sizeof(unsigned char) ? _c < 255 : true );
      ++_c; return _c;
    }
    int operator-- ( int) const { return _c--; }
    int operator-- () const { _c--; return _c; }
    bool operator! () const { return !_c; }
    operator int () const { return _c; }
  };

  typedef RefcountImpl< unsigned char > Refcount ;
  typedef RefcountImpl< unsigned int  > RefcountUnsignedInt ;

  class IteratorRefcount
  {
  public:
    void reset () { }
    bool positive () const { return false; }
    int operator++ ( int ) const { return 0; }
    int operator++ () const { return 0; }
    int operator-- ( int ) const { return 0; }
    int operator-- () const { return 0; }
    bool operator! () const { return false; }
    operator int () const { return 0; }
  };

} // namespace ALUGrid

#endif // #ifndef ALUGRID_SRC_SERIAL_REFCOUNT_HH
