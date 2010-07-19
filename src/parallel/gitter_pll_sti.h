// (c) bernhard schupp 1997 - 1998
#ifndef GITTER_PLL_STI_H_INCLUDED
#define GITTER_PLL_STI_H_INCLUDED

#include "mpAccess.h"
#include "gitter_sti.h"
#include "gitter_pll_ldb.h"

  // Die statische Variable __STATIC_myrank sollte nur zum Debuggen
  // verwendet werden, da ernstere Bez"uge wohl die thread-safeness
  // gef"ahrden w"urden. Sie wird auch erst sp"ater irgendwann mit
  // dem richtigen Wert besetzt. Definiert in gitter_pll_sti.cc .
  //  __STATIC_turn  enth"alt die globale Runde der adaption, damit
  // Fehler in der parallelen Ausgabe, die normalerweise entwas
  // unsortiert ist, zugeordnet werden k"onnen. Die Variablen k"onnen
  // nur in echt verteilten Anwendungen (nicht mt) zum Debuggen
  // verwendet werden.
  // __STATIC_phase enth"alt w"ahrend der Adaption deren Phase:
  // -1 - ausserhalb der Verfeinerung bzw. Vergr"oberung.
  //  1 - sequentielle Verfeinerung,
  //  2 - Fl"achenausgleich des Verfeinerers,
  //  3 - Kantenausgleich des Verfeinerers,
  //  4 - sequentielle Vergr"oberung,
  //  5 - Fl"achenausgleich des Vergr"oberers,
  //  6 - Kantenausgleich des Vergr"oberers.
  
extern int __STATIC_myrank ;
extern int __STATIC_turn ;
extern int __STATIC_phase ;

template < class A > class LeafIteratorTT ;

  // Der Smartpointer 'AccessIteratorTT' ist ein Iteratorproxy, das 
  // genau wie der 'AccessIterator' aus gitter_sti.h funktioniert, aber
  // f"ur die Identifikationsiteratoren verwendet wird. Deshalb hat es
  // auch zwei Handle - Klassen: eine f"ur den inneren und eine f"ur
  // den "ausseren Iterator.

template < class A > class AccessIteratorTT {
  public :
    Refcount ref ;
    virtual ~AccessIteratorTT () ;
  public :
    virtual pair < IteratorSTI < A > *, IteratorSTI < A > * > iteratorTT (const A *, int) = 0 ;
    virtual pair < IteratorSTI < A > *, IteratorSTI < A > * > iteratorTT (const pair < IteratorSTI < A > *, IteratorSTI < A > * > &, int) = 0 ;
    class HandleBase : public IteratorSTI < A > 
                     , public MyAlloc  
    {
      AccessIteratorTT < A > & _fac ;
      int _l ;

      typedef typename AccessIteratorTT< A > :: HandleBase ThisType;
      protected :
        pair < IteratorSTI < A > *, IteratorSTI < A > * > _pw ;
        HandleBase (AccessIteratorTT < A > &, int) ;
        HandleBase (const HandleBase &) ;

        // HandleBase behaves like EmptyIterator (see gitter_sti.h )
        virtual void first () ; 
        virtual void next ()  ; 
        virtual int done () const ;
        virtual int size () ; 
        virtual A & item () const ;
        virtual IteratorSTI < A > * clone () const ;

      public :
        virtual ~HandleBase () ;
    } ;
    class InnerHandle : public HandleBase {
      public :
        InnerHandle (AccessIteratorTT < A > &, int) ;
        InnerHandle (const InnerHandle &) ;
       ~InnerHandle () ;
        void first () ;
        void next () ;
        int done () const ;
        int size () ;
        A & item () const ;
        IteratorSTI < A > * clone () const ;
    } ;
    class OuterHandle : public HandleBase {
      public :
        OuterHandle (AccessIteratorTT < A > &, int) ;
        OuterHandle (const OuterHandle &) ;
       ~OuterHandle () ;
        void first () ;
        void next () ;
        int done () const ;
        int size () ;
        A & item () const ;
        IteratorSTI < A > * clone () const ;
    } ;
} ;

  // Die Listen mit den Identifikationsabbildungen enthalten nicht
  // direkt Objekterefenzen, sondern (teurer aber sicherer) Kopien
  // von Grobgitteriteratoren, die auf das passende Item zeigen.
  // Dadurch kann festgestellt werden, ob noch Referenzen vorhanden
  // sind, wenn z.B. das Gitter umgebaut wird.
  // Der 'listSmartpointer__to__iteratorSTI' Smartpointer verwaltet
  // die Iteratorkopien.

template < class A > class listSmartpointer__to__iteratorSTI : public IteratorSTI < A > 
                                                             , public MyAlloc 
{
  // list to iterate 
  list < typename AccessIterator < A > :: Handle > & _l ;
  // current item 
  typedef typename list < typename AccessIterator < A > :: Handle > :: iterator listiterator_t ;
  
  const listiterator_t _end  ;
  listiterator_t _curr ;

  public :
    listSmartpointer__to__iteratorSTI (list < typename AccessIterator < A > :: Handle > &) ;
    listSmartpointer__to__iteratorSTI (const listSmartpointer__to__iteratorSTI < A > &) ;
   ~listSmartpointer__to__iteratorSTI () ;
    void first () ;
    void next () ;
    int done () const ;
    int size () ;
    A & item () const ;
    IteratorSTI< A > * clone () const ;
} ;

// tpye of ElementPllXIF_t is ElementPllXIF, see parallel.h
class ElementPllXIF : public MacroGridMoverIF
{
  protected :
    typedef Gitter :: Geometric :: hasFace4 :: balrule_t balrule_t ;
    typedef Gitter :: ghostpair_STI ghostpair_STI;
    typedef Gitter :: helement_STI  helement_STI;
    
    virtual ~ElementPllXIF () {}
  public :
    virtual pair < ElementPllXIF_t *, int > accessOuterPllX (const pair < ElementPllXIF_t *, int > &, int) = 0 ;
    virtual pair < const ElementPllXIF_t *, int > accessOuterPllX (const pair < const ElementPllXIF_t *, int > &, int) const = 0 ;
    virtual pair < ElementPllXIF_t *, int > accessInnerPllX (const pair < ElementPllXIF_t *, int > &, int) = 0 ;
    virtual pair < const ElementPllXIF_t *, int > accessInnerPllX (const pair < const ElementPllXIF_t *, int > &, int) const = 0 ;
  public :
    virtual ghostpair_STI getGhost () 
    { 
      cerr << "ERROR: method getGhost of Interface class should not be used! in: " << __FILE__ << " line: " <<__LINE__<<"\n";
      abort(); 
      return ghostpair_STI ( (helement_STI*)0 , -1); 
    }

    virtual int ghostLevel () const
    { 
      cerr << "ERROR: method ghostLevel of Interface class should not be used! in: " << __FILE__ << " line: " <<__LINE__<<"\n";
      abort(); 
      return 0; 
    }

    virtual bool ghostLeaf () const
    { 
      cerr << "ERROR: method ghostLeaf of Interface class should not be used! in: " << __FILE__ << " line: " <<__LINE__<<"\n";
      abort(); 
      return 0; 
    }

    virtual void getAttachedElement ( pair < Gitter::helement_STI* , Gitter::hbndseg_STI * > & p)
    {
      cerr << "Overload method in the classes file:" << __FILE__ << " line:" << __LINE__ << "\n";
      abort();
      p.first  = 0;
      p.second = 0;
    }

    virtual void writeStaticState (ObjectStream &, int) const = 0 ;
    virtual void readStaticState (ObjectStream &, int) = 0 ;
    virtual void writeDynamicState (ObjectStream &, int) const = 0 ;
    virtual void readDynamicState (ObjectStream &, int) = 0 ;

    virtual void VertexData2os(ObjectStream &, GatherScatterType &, int) { cout << "ich bin die falsche...\n" << flush; }
    virtual void EdgeData2os  (ObjectStream &, GatherScatterType &, int) { cout << "ich bin die falsche...1\n" << flush; }
    virtual void FaceData2os  (ObjectStream &, GatherScatterType &, int) { cout << "ich bin die falsche...2\n" << flush; }
    virtual void writeElementData (ObjectStream &, GatherScatterType &) { cout << "ich bin die falsche...3\n" << flush; }
    virtual void writeDynamicState(ObjectStream &, GatherScatterType &) const = 0 ;
    virtual void readDynamicState (ObjectStream &, GatherScatterType &) = 0 ;

    // pack as ghost, default does nothing but macro elements are pack as
    // ghosts 
    virtual void packAsGhost(ObjectStream &,int) const {}

    // unpack as ghost data and insert ghost cell, default does nothing
    virtual void insertGhostCell(ObjectStream &,int) {
    }
    
  public :
    virtual int ldbVertexIndex () const = 0 ;
    virtual int & ldbVertexIndex () = 0 ;
    virtual bool ldbUpdateGraphVertex (LoadBalancer :: DataBase &) = 0 ;
  public :
    virtual void packAsBnd (int,int,ObjectStream &) const = 0 ;
    virtual bool erasable () const = 0 ;
  public :
    virtual void getRefinementRequest (ObjectStream &) = 0 ;
    virtual bool setRefinementRequest (ObjectStream &) = 0 ;
  public :
    virtual bool lockAndTry () = 0 ;
    virtual bool unlockAndResume (bool) = 0 ;
} ;

class GitterPll : public virtual Gitter {
  public :
    static inline bool debugOption (int = 0) ;
  public :
    class MacroGitterPll : public virtual Gitter :: Geometric :: BuilderIF,
  public AccessIteratorTT < vertex_STI >, public AccessIteratorTT < hedge_STI >, public AccessIteratorTT < hface_STI > {
      protected :
      
  // Die nachfolgenden Vektoren von Listenpaaren sind die Identifikationsabbildung auf dem Grobgitter:
  // Jeder Vektoreintrag geh"ort zu dem entsprechenden lokalen Link (Verbindung zum Nachbargebiet) und
  // enth"alt ein paar von zwei Listen ('inner' und 'outer'). Die erste Liste enth"alt Referenzen auf
  // die Gitterobjekte, die hier und auf dem anderen Teilgebiet (zum Link) vorliegen und die aber hier
  // als Besitzstand gef"uhrt werden. Die zweite Liste (outer) verweist auf all jene, die zum Besitz
  // des Nachbargebiets zu rechnen sind. Die Ordnung der Listen ist folgendermassen: Durchl"auft man
  // hier 'inner', dann korrespondieren auf dem Nachbargebiet die Objekte in 'outer' in der Reihenfolge
  // des Durchlaufs (und umgekehrt).
      
        vector < pair < list < AccessIterator < vertex_STI > :: Handle >, list < AccessIterator < vertex_STI > :: Handle > > > _vertexTT ;
        vector < pair < list < AccessIterator < hedge_STI > :: Handle >, list < AccessIterator < hedge_STI > :: Handle > > > _hedgeTT ;
        vector < pair < list < AccessIterator < hface_STI > :: Handle >, list < AccessIterator < hface_STI > :: Handle > > > _hfaceTT ;
        virtual set < int, less < int > > secondScan () ;
        virtual void vertexLinkageEstimate (MpAccessLocal &) ;
      public :
        MacroGitterPll () {}
        virtual ~MacroGitterPll () {}
  
  // Die Identifikationslisten k"onnen nicht direkt von aussen zugegriffen werden, sondern nur "uber ein
  // Iterationsobjekt, das durch den Aufruf einer der untenstehenden Methoden erzeugt wird, und um dessen
  // L"oschung der Aufrufer sich k"ummern muss. "Ublicherweise verwendet man das Smartpointerobjekt
  // AccessIteratorTT < . > :: InnerHandle/OuterHandle um die verwaltung der Iterationsobjekte loszuwerden.
  // Diese Smartpointer sehen nach aussen aus wie Iteratorenstandardschnittstellen, delegieren aber alles
  // an die Iterationsobjekte, die sie vom Grobgittercontainer bekommen haben.
  
        pair < IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > * > iteratorTT (const vertex_STI *, int) ;
        pair < IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > * > iteratorTT (const pair < IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > * > &, int) ;
        pair < IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > * > iteratorTT (const hedge_STI *, int) ;
        pair < IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > * > iteratorTT (const pair < IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > * > &, int) ;
        pair < IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > * > iteratorTT (const hface_STI *, int) ;
        pair < IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > * > iteratorTT (const pair < IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > * > &, int) ;
        virtual inline int iterators_attached () const ;
        virtual void identification (MpAccessLocal &) ;
        virtual void fullIntegrityCheck (MpAccessLocal &) ;
    } ;
  public :
  
  // Das verteilte Gitter "uberschreibt die meisten Methoden der
  // unterliegenden Monogitter durch eigene Implementierungen.
  // printSizeTT () ist neu und schreibt die Gr"ossen der
  // Identifikationsabbildungen auf die Standarausgabe.
  
    virtual void printsize () ;
    virtual void fullIntegrityCheck () ;
    virtual void backupCMode (const char*,const char *) ;
    virtual void backupCMode (ostream &) ;
    virtual void backup (const char *,const char*) ;
    virtual void backup (ostream &) ;
    virtual void restore (const char *,const char *) ;
    virtual void restore (istream &) ;
    virtual bool refine () ;
    virtual void coarse () ;
    virtual bool adapt () ;
    virtual void printSizeTT () ;
    
    // new xdr backup and restore methods 
    virtual void backup (XDRstream_out &) ;
    virtual void restore (XDRstream_in &) ;

  protected :
    virtual Makrogitter & container () = 0 ;
    virtual const Makrogitter & container () const = 0 ;
    virtual MacroGitterPll & containerPll () = 0 ;
    virtual const MacroGitterPll & containerPll () const = 0 ;
    virtual MpAccessLocal & mpAccess () = 0 ;
    virtual const MpAccessLocal & mpAccess () const = 0 ;
    
  // Der nachfolgende Methodenblock dient dazu, das Verhalten des 
  // parallelen Gitters einigermassen unter Kontrolle zu bringen.
  // Dabei wird von einem Schichtenmodell ausgegangen:
  // - der statische Zustand des Gitters ist die Verteilung des
  //   Grobgitters und wird nur durch die Lastverteilung ge"andert
  // - der dynamische Zustand des Gitters ist die Verfeinerungs-
  //   situation,und "andert sich infolge der Gitterqeitenanpassung.
  // - "Anderungen in den Benutzerdaten werden nicht modelliert, das
  //   bleibt der entsprechenden Implemntierung "uberlassen.
  // Dementsprechend werden die exchange--*-- Methoden immer
  // aufgerufen, sobald sich der zugeh"orige Zustand ge"andert hat.
    
    virtual void exchangeStaticState () ;
    virtual void exchangeDynamicState () ;
    virtual void repartitionMacroGrid (LoadBalancer :: DataBase &) ;
    
    virtual void loadBalancerGridChangesNotify () ;
    virtual void loadBalancerMacroGridChangesNotify () ;
    virtual void notifyGridChanges () ;
    virtual void notifyMacroGridChanges () ;
    
  // Die Methoden iteratorTT (const . *, int)  sind der Zugang zu den
  // Identifikationsabbildungen des hierarchischen Gitters f"ur die
  // feinsten Objekte in der Hierarchie. Sie erzeugen ein Paar von 
  // Iterationsobjekten, die zu einem entsprechenden Link, d.h. zu einer
  // bestimmten Verbindung mit einem benachbarten Teilgitter geh"oren.
  // Der erste Iterator im Paar verweist auf die Objekte, die es hier
  // und beim Nachbargitter gibt, die als eigener Besitz gelten.
  // Der zweite Iterator bezeichnet jene, die sich im Besitz des Nachbarn
  // befinden. Die Identifikation verl"auft folgendermassen: Die Objekte,
  // die der erste Iterator hier zeigt, korrespondieren zu denen die der
  // zweite Iterator auf dem Nachbargitter abl"auft (und umgekehrt).
    
    pair < IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > *> iteratorTT (const vertex_STI *, int) ;
    pair < IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > *> iteratorTT (const hedge_STI *, int) ;
    pair < IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > *> iteratorTT (const hface_STI *, int) ;

    // constructor, if verbose = true, output is given 
    GitterPll (bool verbose = false) ;
   ~GitterPll () {}
  friend class LeafIteratorTT < vertex_STI > ;
  friend class LeafIteratorTT < hedge_STI > ;
  friend class LeafIteratorTT < hface_STI > ;
  protected :
    template <class StopRule_t> 
    inline pair < IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > *> 
    createEdgeIteratorTT (const StopRule_t *, int) ;

    template <class StopRule_t> 
    inline pair < IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > *> 
    createFaceIteratorTT (const StopRule_t rule , int) ;
  
  // Die drei Variablen der Klasse Gitter sollen erstmal als
  // Murksl"osung dazu dienen, den Lastverteiler "uber ein
  // File ("lastverteilung.cfg") rekonfigurieren zu k"onnen.
  
    double  _ldbOver, _ldbUnder ;
    LoadBalancer :: DataBase :: method _ldbMethod ;
    
  // Die Variable _refineLoops dient nur der Kommunikation
  // zwischen adapt () und refine (), damit die Zahl der
  // Iterationen am Ende ausgegeben werden kann.
 
    int _refineLoops ;
} ;

template < class A > class LeafIteratorTT : public MyAlloc 
{
  GitterPll & _grd ;
  int _link ;
  A * _a ;
  pair < IteratorSTI < A > *, IteratorSTI < A > * > _p ;
  public :
    inline IteratorSTI < A > & inner () ;
    inline const IteratorSTI < A > & inner () const ;
    inline IteratorSTI < A > & outer () ;
    inline const IteratorSTI < A > & outer () const ;
    inline LeafIteratorTT (GitterPll &, int) ;
    inline LeafIteratorTT (const LeafIteratorTT & ) ;
    inline ~LeafIteratorTT () ;
} ;


  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //


inline int GitterPll :: MacroGitterPll :: iterators_attached () const {
  return AccessIteratorTT < vertex_STI > :: ref + AccessIteratorTT < hedge_STI > :: ref + AccessIteratorTT < hface_STI > :: ref ;
}

template < class A > inline AccessIteratorTT < A > :: ~AccessIteratorTT () {
  assert (!ref) ;
}

template < class A > inline AccessIteratorTT < A > :: HandleBase :: 
HandleBase (AccessIteratorTT < A > & f, int i) : _fac (f), _l (i) 
{
  this->_fac.ref ++ ;
  this->_pw = _fac.iteratorTT ((A *)0,_l) ;
}

template < class A > inline AccessIteratorTT < A > :: HandleBase :: 
HandleBase (const ThisType& org) 
  : _fac (org._fac), _l (org._l) 
  , _pw( org._pw.first ->clone() , org._pw.second->clone() )
{
  this->_fac.ref ++ ;
}

template < class A > inline AccessIteratorTT < A > :: HandleBase :: 
~HandleBase () {
  this->_fac.ref -- ;
  delete this->_pw.first ;
  delete this->_pw.second ;
}

template < class A > inline void AccessIteratorTT < A > :: HandleBase :: 
first () 
{
} 

template < class A > inline void AccessIteratorTT < A > :: HandleBase :: 
next ()  
{
} 

template < class A > inline int AccessIteratorTT < A > :: HandleBase :: 
done () const 
{ 
  return 1; 
}

template < class A > inline int AccessIteratorTT < A > :: HandleBase :: 
size () 
{ 
  return 0; 
}

template < class A > inline A & AccessIteratorTT < A > :: HandleBase :: 
item () const 
{ 
  assert( ! done ()); 
  A * a = 0;
  return *a; 
}

template < class A > inline IteratorSTI < A > * AccessIteratorTT < A > :: HandleBase :: 
clone () const 
{
  return new ThisType(*this);
}

template < class A > inline AccessIteratorTT < A > :: InnerHandle :: 
InnerHandle (AccessIteratorTT < A > & f, int i) : HandleBase (f,i) 
{
}

template < class A > inline AccessIteratorTT < A > :: InnerHandle :: 
InnerHandle (const InnerHandle & p) : HandleBase (p) {
}

template < class A > inline AccessIteratorTT < A > :: InnerHandle :: ~InnerHandle () {
}

template < class A > inline void AccessIteratorTT < A > :: InnerHandle :: first () {
  this->_pw.first->first () ;
}

template < class A > inline void AccessIteratorTT < A > :: InnerHandle :: next () {
  this->_pw.first->next () ;
}

template < class A > inline int AccessIteratorTT < A > :: InnerHandle :: done () const {
  return this->_pw.first->done () ;
}

template < class A > inline int AccessIteratorTT < A > :: InnerHandle :: size () {
  return this->_pw.first->size () ;
}

template < class A > inline A & AccessIteratorTT < A > :: InnerHandle :: item () const {
  assert ( ! done ()) ;
  return this->_pw.first->item () ;
}

template < class A > inline IteratorSTI < A > * AccessIteratorTT < A > :: InnerHandle ::  
clone () const 
{
  return new typename AccessIteratorTT < A > :: InnerHandle (*this);
}

template < class A > inline AccessIteratorTT < A > :: OuterHandle :: OuterHandle (AccessIteratorTT < A > & f, int i) : HandleBase (f,i) {
}

template < class A > inline AccessIteratorTT < A > :: OuterHandle :: 
OuterHandle (const OuterHandle & p) : HandleBase (p) 
{
}

template < class A > inline AccessIteratorTT < A > :: OuterHandle :: ~OuterHandle () {
}

template < class A > inline void AccessIteratorTT < A > :: OuterHandle :: first () {
  this->_pw.second->first () ;
}

template < class A > inline void AccessIteratorTT < A > :: OuterHandle :: next () {
  this->_pw.second->next () ;
}

template < class A > inline int AccessIteratorTT < A > :: OuterHandle :: done () const {
  return this->_pw.second->done () ;
}

template < class A > inline int AccessIteratorTT < A > :: OuterHandle :: size () {
  return this->_pw.second->size () ;
}

template < class A > inline A & AccessIteratorTT < A > :: OuterHandle :: item () const {
  assert (! done ()) ;
  return this->_pw.second->item () ;
}

template < class A > inline IteratorSTI < A > * AccessIteratorTT < A > :: OuterHandle ::  
clone () const 
{
  return new typename AccessIteratorTT < A > :: OuterHandle (*this);
}


template < class A > listSmartpointer__to__iteratorSTI < A > :: 
listSmartpointer__to__iteratorSTI (list < typename AccessIterator < A > :: Handle > & a) 
 : _l (a),  _end( _l.end() ), _curr( _end )
{
}

template < class A > listSmartpointer__to__iteratorSTI < A > :: 
listSmartpointer__to__iteratorSTI (const listSmartpointer__to__iteratorSTI < A > & a) 
  : _l (a._l), _end( _l.end() ), _curr(a._curr) {}

template < class A > listSmartpointer__to__iteratorSTI < A > :: ~listSmartpointer__to__iteratorSTI () {
}

template < class A > void listSmartpointer__to__iteratorSTI < A > :: first () {
  _curr = _l.begin () ;
}

template < class A > void listSmartpointer__to__iteratorSTI < A > :: next () 
{
  ++_curr ;
}

template < class A > int listSmartpointer__to__iteratorSTI < A > :: done () const {
  return (_curr == _end) ? 1 : 0 ;
}

template < class A > int listSmartpointer__to__iteratorSTI < A > :: size () {
  return _l.size () ;
}

template < class A > A & listSmartpointer__to__iteratorSTI < A > :: item () const {
  assert (! done ()) ;
  return (*_curr).item () ;
}

template < class A > IteratorSTI < A > * listSmartpointer__to__iteratorSTI < A > :: 
clone () const 
{
  return new listSmartpointer__to__iteratorSTI < A > (*this);
}

inline bool GitterPll :: debugOption (int level) {
  return (getenv ("VERBOSE_PLL") ? ( atoi (getenv ("VERBOSE_PLL")) > level ? true : (level == 0)) : false) ;
}

template <class StopRule_t>
inline pair < IteratorSTI < GitterPll :: hedge_STI > *, IteratorSTI < GitterPll :: hedge_STI > * > GitterPll ::
  createEdgeIteratorTT(const StopRule_t * fake, int l) {

  AccessIteratorTT < hedge_STI > :: InnerHandle mdi (containerPll (), l) ;
  AccessIteratorTT < hedge_STI > :: OuterHandle mdo (containerPll (), l) ;

  Insert < AccessIteratorTT < hedge_STI > :: InnerHandle, TreeIterator < hedge_STI, StopRule_t> > ei (mdi) ;
  Insert < AccessIteratorTT < hedge_STI > :: OuterHandle, TreeIterator < hedge_STI, StopRule_t> > eo (mdo) ;

  AccessIteratorTT < hface_STI > :: InnerHandle mfi (containerPll (), l) ;
  AccessIteratorTT < hface_STI > :: OuterHandle mfo (containerPll (), l) ;

  Insert < AccessIteratorTT < hface_STI > :: InnerHandle, TreeIterator < hface_STI, has_int_edge < hface_STI > > > fimi (mfi) ;
  Insert < AccessIteratorTT < hface_STI > :: OuterHandle, TreeIterator < hface_STI, has_int_edge < hface_STI > > > fimo (mfo) ;
  
  Wrapper < Insert < AccessIteratorTT < hface_STI > :: InnerHandle,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > dfimi (fimi) ;
  Wrapper < Insert < AccessIteratorTT < hface_STI > :: OuterHandle,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > dfimo (fimo) ;
  
  Insert < Wrapper < Insert < AccessIteratorTT < hface_STI > :: InnerHandle,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, StopRule_t> > eifi (dfimi) ;
  
  Insert < Wrapper < Insert < AccessIteratorTT < hface_STI > :: OuterHandle,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, StopRule_t> > eifo (dfimo) ;

  return pair < IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > * >
    (new AlignIterator < Insert < AccessIteratorTT < hedge_STI > :: InnerHandle, TreeIterator < hedge_STI, StopRule_t> >,
  Insert < Wrapper < Insert < AccessIteratorTT < hface_STI > :: InnerHandle,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, StopRule_t> >, hedge_STI > (ei,eifi),
     new AlignIterator < Insert < AccessIteratorTT < hedge_STI > :: OuterHandle, TreeIterator < hedge_STI, StopRule_t> >,
  Insert < Wrapper < Insert < AccessIteratorTT < hface_STI > :: OuterHandle,
  TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, StopRule_t> >, hedge_STI > (eo, eifo)) ;
}

template <class StopRule_t>
inline pair < IteratorSTI < GitterPll :: hface_STI > *, IteratorSTI < GitterPll :: hface_STI > *> 
  GitterPll :: createFaceIteratorTT (const StopRule_t rule , int l) 
{
  AccessIteratorTT < hface_STI > :: InnerHandle mif (containerPll () , l) ;
  AccessIteratorTT < hface_STI > :: OuterHandle mof (containerPll () , l) ;
  return pair < IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > * >
  (new Insert < AccessIteratorTT < hface_STI > :: InnerHandle, TreeIterator < hface_STI, StopRule_t > > (mif,rule),
   new Insert < AccessIteratorTT < hface_STI > :: OuterHandle, TreeIterator < hface_STI, StopRule_t > > (mof,rule)) ;
}


template < class A > inline LeafIteratorTT < A > :: LeafIteratorTT (GitterPll & g, int l) : _grd (g), _link (l), _a (0) {
  _p = _grd.iteratorTT (_a, _link) ;
}

template < class A > inline LeafIteratorTT < A > :: LeafIteratorTT (const LeafIteratorTT & org)
  : _grd (org._grd), _link (org._link), _a (0) 
  , _p( org._p.first->clone() , org._p.second->clone() ) 
{
}

template < class A > inline LeafIteratorTT < A > :: ~LeafIteratorTT () 
{
  delete _p.first ;
  delete _p.second ;
}

template < class A > inline IteratorSTI < A > & LeafIteratorTT < A > :: inner () {
  return * _p.first ;
}
 
template < class A > const inline IteratorSTI < A > & LeafIteratorTT < A > :: inner () const {
  return * _p.first ;
}

template < class A > inline IteratorSTI < A > & LeafIteratorTT < A > :: outer () {
  return * _p.second ;
}

template < class A > const inline IteratorSTI < A > & LeafIteratorTT < A > :: outer () const {
  return * _p.second ;
}

#endif  // GITTER_PLL_STI_H_INCLUDED
