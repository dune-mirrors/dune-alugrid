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

  class Refcount
  {
#ifndef NDEBUG
#ifdef DEBUG_ALUGRID
    // Der Globale Z"ahler soll helfen, nicht gel"oschte
    // Gitterobjekte oder Iteratorobjekte zu erkennen.
    // (Wird aber nur in den DEBUG-Versionen angelegt.) 
    //
    // Refcounting only turned on, if NDEBUG is not defined and
    // DEBUG_ALUGRID is defined 
    class Globalcount
    {
      int _c;
    public:
      Globalcount ();
      ~Globalcount ();
      void operator++ ( int ) const;
      void operator-- ( int ) const;
    };

    static Globalcount _g;
#endif // #ifdef DEBUG_ALUGRID
#endif // #ifndef NDEBUG

    mutable unsigned char _c;

  public:
    void reset () { _c = 0; }
    bool positive () const { return _c > 0; }
    Refcount ();
    ~Refcount ();
    int operator++ ( int ) const;
    int operator++ () const;
    int operator-- ( int) const;
    int operator-- () const;
    bool operator! () const;
    operator int () const;
  };

  class IteratorRefcount 
#ifdef ALUGRID_ITERATORS_WITH_MYALLOC
  : public Refcount 
#endif // #ifdef ALUGRID_ITERATORS_WITH_MYALLOC
  {
  public:
#ifndef ALUGRID_ITERATORS_WITH_MYALLOC
    void reset () { }
    bool positive () const { return false; }
    int operator++ ( int ) const { return 0; }
    int operator++ () const { return 0; }
    int operator-- ( int ) const { return 0; }
    int operator-- () const { return 0; }  
    bool operator! () const { return false; }
    operator int () const { return 0; }
#endif // #ifndef ALUGRID_ITERATORS_WITH_MYALLOC
  };

} // namespace ALUGrid

#endif // #ifndef ALUGRID_SRC_SERIAL_REFCOUNT_HH
