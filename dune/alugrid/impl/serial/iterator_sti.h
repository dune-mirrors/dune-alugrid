// (c) Robert Kloefkorn 2004 - 2013
#ifndef ITERATOR_STI_H_INCLUDED
#define ITERATOR_STI_H_INCLUDED

namespace ALUGrid
{

  ////////////////////////////////////////////////////////////////////
  //
  // Schnittstelle des Iterationsobjekts vgl. Gamma, Helm, Johnson &
  // Vlissides: Design Patterns; Addison Wesley
  // Die Schnittstellenbeschreibung wird sowohl polymorph als auch
  // in den verschiedenen Schablonen f"ur einfache Iterationsobjekte
  // s.a. Datei 'walk.h' verwendet.
  //
  ////////////////////////////////////////////////////////////////////
  template < class A > class IteratorSTI
  {
  protected:
    IteratorSTI () {}
  public :
    typedef A val_t;
    virtual ~IteratorSTI () {}
    virtual void first () = 0;
    virtual void next () = 0;
    virtual int done () const = 0;
    virtual int size () = 0;
    virtual val_t & item () const = 0;
    virtual IteratorSTI < A > * clone () const = 0;
  };

} // namespace ALUGrid

#endif // #ifndef ITERATOR_STI_H_INCLUDED
