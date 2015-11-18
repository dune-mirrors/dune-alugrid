// (c) bernhard schupp 1997 - 1998
#ifndef GITTER_PLL_STI_H_INCLUDED
#define GITTER_PLL_STI_H_INCLUDED

#include <list>
#include <set>
#include <utility>
#include <vector>

#include "mpAccess.h"
#include "../serial/gitter_sti.h"
#include "gitter_pll_ldb.h"

namespace ALUGrid
{
  template < class A > class LeafIteratorTT;

  // Der Smartpointer 'AccessIteratorTT' ist ein Iteratorproxy, das
  // genau wie der 'AccessIterator' aus gitter_sti.h funktioniert, aber
  // f"ur die Identifikationsiteratoren verwendet wird. Deshalb hat es
  // auch zwei Handle - Klassen: eine f"ur den inneren und eine f"ur
  // den "ausseren Iterator.

  template < class A > class AccessIteratorTT {
    public :
      IteratorRefcount ref;
      virtual ~AccessIteratorTT ();
    public :
      virtual std::pair< IteratorSTI < A > *, IteratorSTI < A > * > iteratorTT (const A *, int) = 0;
      virtual std::pair< IteratorSTI < A > *, IteratorSTI < A > * > iteratorTT (const std::pair< IteratorSTI < A > *, IteratorSTI < A > * > &, int) = 0;
      class HandleBase : public IteratorSTI < A >
      {
        AccessIteratorTT < A > & _fac;
        int _l;

        typedef typename AccessIteratorTT< A >::HandleBase ThisType;
        protected :
          std::pair< IteratorSTI < A > *, IteratorSTI < A > * > _pw;
          HandleBase (AccessIteratorTT < A > &, int);
          HandleBase (const HandleBase &);

          // HandleBase behaves like EmptyIterator (see gitter_sti.h )
          virtual void first ();
          virtual void next () ;
          virtual int done () const;
          virtual int size ();
          virtual A & item () const;
          virtual IteratorSTI < A > * clone () const;

        public :
          virtual ~HandleBase ();
      };
      class InnerHandle : public HandleBase {
        public :
          InnerHandle (AccessIteratorTT < A > &, int);
          InnerHandle (const InnerHandle &);
         ~InnerHandle ();
          void first ();
          void next ();
          int done () const;
          int size ();
          A & item () const;
          IteratorSTI < A > * clone () const;
      };
      class OuterHandle : public HandleBase {
        public :
          OuterHandle (AccessIteratorTT < A > &, int);
          OuterHandle (const OuterHandle &);
         ~OuterHandle ();
          void first ();
          void next ();
          int done () const;
          int size ();
          A & item () const;
          IteratorSTI < A > * clone () const;
      };
  };

    // Die Listen mit den Identifikationsabbildungen enthalten nicht
    // direkt Objekterefenzen, sondern (teurer aber sicherer) Kopien
    // von Grobgitteriteratoren, die auf das passende Item zeigen.
    // Dadurch kann festgestellt werden, ob noch Referenzen vorhanden
    // sind, wenn z.B. das Gitter umgebaut wird.
    // Der 'listSmartpointer__to__iteratorSTI' Smartpointer verwaltet
    // die Iteratorkopien.

  template < class A > class listSmartpointer__to__iteratorSTI : public IteratorSTI < A >
  {
    typedef std::list< A* > container_t ;
    // list to iterate
    container_t& _l;
    // current item
    typedef typename container_t::iterator listiterator_t;

    const listiterator_t _end ;
    listiterator_t _curr;

    public :
      listSmartpointer__to__iteratorSTI ( container_t & );
      listSmartpointer__to__iteratorSTI (const listSmartpointer__to__iteratorSTI < A > &);
     ~listSmartpointer__to__iteratorSTI ();
      void first ();
      void next ();
      int done () const;
      int size ();
      A & item () const;
      IteratorSTI< A > * clone () const;
  };

  class GitterPll : public virtual Gitter {
    public :
      static inline bool debugOption (int = 0);
    public :
    class MacroGitterPll : public virtual Gitter::Geometric::BuilderIF,
        public AccessIteratorTT < vertex_STI >,
        public AccessIteratorTT < hedge_STI >,
        public AccessIteratorTT < hface_STI >
    {
    protected :

      // Die nachfolgenden Vektoren von Listenpaaren sind die Identifikationsabbildung auf dem Grobgitter:
      // Jeder Vektoreintrag geh"ort zu dem entsprechenden lokalen Link (Verbindung zum Nachbargebiet) und
      // enth"alt ein paar von zwei Listen ('inner' und 'outer'). Die erste Liste enth"alt Referenzen auf
      // die Gitterobjekte, die hier und auf dem anderen Teilgebiet (zum Link) vorliegen und die aber hier
      // als Besitzstand gef"uhrt werden. Die zweite Liste (outer) verweist auf all jene, die zum Besitz
      // des Nachbargebiets zu rechnen sind. Die Ordnung der Listen ist folgendermassen: Durchl"auft man
      // hier 'inner', dann korrespondieren auf dem Nachbargebiet die Objekte in 'outer' in der Reihenfolge
      // des Durchlaufs (und umgekehrt).

      typedef std::vector< std::pair< std::list< vertex_STI* >, std::list< vertex_STI* > > > vertexTT_t;
      typedef std::vector< std::pair< std::list< hedge_STI*  >, std::list< hedge_STI*  > > > hedgeTT_t ;
      typedef std::vector< std::pair< std::list< hface_STI*  >, std::list< hface_STI*  > > > hfaceTT_t ;
      vertexTT_t  _vertexTT;
      hedgeTT_t   _hedgeTT;
      hfaceTT_t   _hfaceTT;

      virtual void secondScan ( std::set< int >& ) = 0 ;
    public:
      virtual void clearLinkagePattern () = 0;
      virtual void vertexLinkageEstimate (MpAccessLocal &, const bool );
      // vertexLinkageEstimation with gcollect( MPI_Allgather, memory consuming )
      // time = log p, memory = O(p)
      void vertexLinkageEstimateGCollect (MpAccessLocal &, const bool );
      // vertexLinkageEstimation with bcast( MPI_Bcast, memory O(1), more time consuming )
      // time = p log p, memory = O(1)
      void vertexLinkageEstimateBcast (MpAccessLocal &, const bool );
      void computeElementDestinations(MpAccessLocal &, LoadBalancer::DataBase& );
    public :
      MacroGitterPll () {}
      virtual ~MacroGitterPll () {}

      // Die Identifikationslisten k"onnen nicht direkt von aussen zugegriffen werden, sondern nur "uber ein
      // Iterationsobjekt, das durch den Aufruf einer der untenstehenden Methoden erzeugt wird, und um dessen
      // L"oschung der Aufrufer sich k"ummern muss. "Ublicherweise verwendet man das Smartpointerobjekt
      // AccessIteratorTT < . >::InnerHandle/OuterHandle um die verwaltung der Iterationsobjekte loszuwerden.
      // Diese Smartpointer sehen nach aussen aus wie Iteratorenstandardschnittstellen, delegieren aber alles
      // an die Iterationsobjekte, die sie vom Grobgittercontainer bekommen haben.

      std::pair< IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > * > iteratorTT (const vertex_STI *, int);
      std::pair< IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > * > iteratorTT (const std::pair< IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > * > &, int);
      std::pair< IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > * > iteratorTT (const hedge_STI *, int);
      std::pair< IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > * > iteratorTT (const std::pair< IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > * > &, int);
      std::pair< IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > * > iteratorTT (const hface_STI *, int);
      std::pair< IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > * > iteratorTT (const std::pair< IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > * > &, int);
      virtual inline int iterators_attached () const;
      virtual void identification (MpAccessLocal &, LoadBalancer::DataBase* , const bool );
      virtual void fullIntegrityCheck (MpAccessLocal &);
    }; // end MacroGitterPll

    public :

    // Das verteilte Gitter "uberschreibt die meisten Methoden der
    // unterliegenden Monogitter durch eigene Implementierungen.
    // printSizeTT () ist neu und schreibt die Gr"ossen der
    // Identifikationsabbildungen auf die Standarausgabe.

      virtual void printsize ();
      virtual void fullIntegrityCheck ();

    protected:
      virtual bool refine ();
      virtual void coarse ();
      virtual bool adapt ();
      virtual bool checkRepeat( const bool, const unsigned int ) const;
    public:
      virtual void printSizeTT ();

      // communication of border data
      virtual void borderBorderCommunication (
               GatherScatterType & vertexData ,
               GatherScatterType & edgeData,
               GatherScatterType & faceData ,
              GatherScatterType & elementData )
      {
        abort();
      }

    protected :
      virtual Makrogitter & container () = 0;
      virtual const Makrogitter & container () const = 0;
      virtual MacroGitterPll & containerPll () = 0;
      virtual const MacroGitterPll & containerPll () const = 0;
      virtual MpAccessLocal & mpAccess () = 0;
      virtual const MpAccessLocal & mpAccess () const = 0;

    // Der nachfolgende Methodenblock dient dazu, das Verhalten des
    // parallelen Gitters einigermassen unter Kontrolle zu bringen.
    // Dabei wird von einem Schichtenmodell ausgegangen:
    // - der dynamische Zustand des Gitters ist die Verfeinerungs-
    //   situation,und "andert sich infolge der Gitterqeitenanpassung.
    // - "Anderungen in den Benutzerdaten werden nicht modelliert, das
    //   bleibt der entsprechenden Implemntierung "uberlassen.
    // Dementsprechend werden die exchange--*-- Methoden immer
    // aufgerufen, sobald sich der zugeh"orige Zustand ge"andert hat.

      virtual void exchangeStaticState ();
      virtual void exchangeDynamicState ();

    protected:
      void computeGraphVertexIndices();
      void doNotifyMacroGridChanges ( LoadBalancer::DataBase* db = 0 );
    public:
      virtual void repartitionMacroGrid (LoadBalancer::DataBase &, GatherScatterType* );

      virtual bool checkPartitioning(LoadBalancer::DataBase &, GatherScatterType* );
      virtual bool loadBalance ( GatherScatterType* gs = 0 );
      virtual void loadBalancerMacroGridChangesNotify ();
      virtual void notifyMacroGridChanges ();

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

      std::pair< IteratorSTI < vertex_STI > *, IteratorSTI < vertex_STI > *> iteratorTT (const vertex_STI *, int);
      std::pair< IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > *> iteratorTT (const hedge_STI *, int);
      std::pair< IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > *> iteratorTT (const hface_STI *, int);

      // constructor, take communicator for parameter communication
      explicit GitterPll ( MpAccessLocal & mpa );
     ~GitterPll () {}

      friend class LeafIteratorTT < vertex_STI >;
      friend class LeafIteratorTT < hedge_STI >;
      friend class LeafIteratorTT < hface_STI >;
    protected :
      template <class StopRule_t>
      inline std::pair< IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > *>
      createEdgeIteratorTT (const StopRule_t *, int);

      template <class StopRule_t>
      inline std::pair< IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > *>
      createFaceIteratorTT (const StopRule_t rule , int);

      bool serialPartitioner()      const { return LoadBalancer::DataBase::serialPartitionerUsed( _ldbMethod ); }
      bool storeLinkageInVertices() const { return LoadBalancer::DataBase::storeLinkageInVertices( _ldbMethod ); }

      /////////////////////////////////////
      //  member variables
      /////////////////////////////////////
      std::vector<int> _graphSizes;  // only used for serial partitioners
      std::vector<int> _elementCuts; // only used for parallel sfc partitioning

      // Load Balancer parameters
      double  _ldbOver, _ldbUnder;
      LoadBalancer::DataBase::method _ldbMethod;

      // Die Variable _refineLoops dient nur der Kommunikation
      // zwischen adapt () und refine (), damit die Zahl der
      // Iterationen am Ende ausgegeben werden kann.

      unsigned int _minRefineLoops;
      unsigned int _refinesDone;
      unsigned int _averageRefineLoops;
      bool _ldbVerticesComputed;
  };

  template < class A > class LeafIteratorTT
  {
    GitterPll & _grd;
    int _link;
    A * _a;
    std::pair< IteratorSTI < A > *, IteratorSTI < A > * > _p;
    public :
      inline IteratorSTI < A > & inner ();
      inline const IteratorSTI < A > & inner () const;
      inline IteratorSTI < A > & outer ();
      inline const IteratorSTI < A > & outer () const;
      inline LeafIteratorTT (GitterPll &, int);
      inline LeafIteratorTT (const LeafIteratorTT & );
      inline ~LeafIteratorTT ();
  };


    //
    //    #    #    #  #          #    #    #  ######
    //    #    ##   #  #          #    ##   #  #
    //    #    # #  #  #          #    # #  #  #####
    //    #    #  # #  #          #    #  # #  #
    //    #    #   ##  #          #    #   ##  #
    //    #    #    #  ######     #    #    #  ######
    //


  inline int GitterPll::MacroGitterPll::iterators_attached () const {
    return AccessIteratorTT < vertex_STI >::ref + AccessIteratorTT < hedge_STI >::ref + AccessIteratorTT < hface_STI >::ref;
  }

  template < class A > inline AccessIteratorTT < A >::~AccessIteratorTT () {
    //alugrid_assert (!ref);
  }

  template < class A > inline AccessIteratorTT < A >::HandleBase::
  HandleBase (AccessIteratorTT < A > & f, int i) : _fac (f), _l (i)
  {
    this->_fac.ref ++;
    this->_pw = _fac.iteratorTT ((A *)0,_l);
  }

  template < class A > inline AccessIteratorTT < A >::HandleBase::
  HandleBase (const ThisType& org)
    : _fac (org._fac), _l (org._l)
    , _pw( org._pw.first ->clone() , org._pw.second->clone() )
  {
    this->_fac.ref ++;
  }

  template < class A > inline AccessIteratorTT < A >::HandleBase::
  ~HandleBase () {
    this->_fac.ref --;
    delete this->_pw.first;
    delete this->_pw.second;
  }

  template < class A > inline void AccessIteratorTT < A >::HandleBase::
  first ()
  {
  }

  template < class A > inline void AccessIteratorTT < A >::HandleBase::
  next ()
  {
  }

  template < class A > inline int AccessIteratorTT < A >::HandleBase::
  done () const
  {
    return 1;
  }

  template < class A > inline int AccessIteratorTT < A >::HandleBase::
  size ()
  {
    return 0;
  }

  template < class A > inline A & AccessIteratorTT < A >::HandleBase::
  item () const
  {
    alugrid_assert ( ! done ());
    A * a = 0;
    return *a;
  }

  template < class A > inline IteratorSTI < A > * AccessIteratorTT < A >::HandleBase::
  clone () const
  {
    return new ThisType(*this);
  }

  template < class A > inline AccessIteratorTT < A >::InnerHandle::
  InnerHandle (AccessIteratorTT < A > & f, int i) : HandleBase (f,i)
  {
  }

  template < class A > inline AccessIteratorTT < A >::InnerHandle::
  InnerHandle (const InnerHandle & p) : HandleBase (p) {
  }

  template < class A > inline AccessIteratorTT < A >::InnerHandle::~InnerHandle () {
  }

  template < class A > inline void AccessIteratorTT < A >::InnerHandle::first () {
    this->_pw.first->first ();
  }

  template < class A > inline void AccessIteratorTT < A >::InnerHandle::next () {
    this->_pw.first->next ();
  }

  template < class A > inline int AccessIteratorTT < A >::InnerHandle::done () const {
    return this->_pw.first->done ();
  }

  template < class A > inline int AccessIteratorTT < A >::InnerHandle::size () {
    return this->_pw.first->size ();
  }

  template < class A > inline A & AccessIteratorTT < A >::InnerHandle::item () const {
    alugrid_assert ( ! done ());
    return this->_pw.first->item ();
  }

  template < class A > inline IteratorSTI < A > * AccessIteratorTT < A >::InnerHandle::
  clone () const
  {
    return new typename AccessIteratorTT < A >::InnerHandle (*this);
  }

  template < class A > inline AccessIteratorTT < A >::OuterHandle::OuterHandle (AccessIteratorTT < A > & f, int i) : HandleBase (f,i) {
  }

  template < class A > inline AccessIteratorTT < A >::OuterHandle::
  OuterHandle (const OuterHandle & p) : HandleBase (p)
  {
  }

  template < class A > inline AccessIteratorTT < A >::OuterHandle::~OuterHandle () {
  }

  template < class A > inline void AccessIteratorTT < A >::OuterHandle::first () {
    this->_pw.second->first ();
  }

  template < class A > inline void AccessIteratorTT < A >::OuterHandle::next () {
    this->_pw.second->next ();
  }

  template < class A > inline int AccessIteratorTT < A >::OuterHandle::done () const {
    return this->_pw.second->done ();
  }

  template < class A > inline int AccessIteratorTT < A >::OuterHandle::size () {
    return this->_pw.second->size ();
  }

  template < class A > inline A & AccessIteratorTT < A >::OuterHandle::item () const {
    alugrid_assert (! done ());
    return this->_pw.second->item ();
  }

  template < class A > inline IteratorSTI < A > * AccessIteratorTT < A >::OuterHandle::
  clone () const
  {
    return new typename AccessIteratorTT < A >::OuterHandle (*this);
  }


  template < class A > listSmartpointer__to__iteratorSTI < A >::
  listSmartpointer__to__iteratorSTI ( container_t & a)
   : _l (a),  _end( _l.end() ), _curr( _end )
  {
  }

  template < class A > listSmartpointer__to__iteratorSTI < A >::
  listSmartpointer__to__iteratorSTI (const listSmartpointer__to__iteratorSTI < A > & a)
    : _l (a._l), _end( _l.end() ), _curr(a._curr) {}

  template < class A > listSmartpointer__to__iteratorSTI < A >::~listSmartpointer__to__iteratorSTI () {
  }

  template < class A > void listSmartpointer__to__iteratorSTI < A >::first () {
    _curr = _l.begin ();
  }

  template < class A > void listSmartpointer__to__iteratorSTI < A >::next ()
  {
    ++_curr;
  }

  template < class A > int listSmartpointer__to__iteratorSTI < A >::done () const {
    return (_curr == _end) ? 1 : 0;
  }

  template < class A > int listSmartpointer__to__iteratorSTI < A >::size () {
    return _l.size ();
  }

  template < class A > A & listSmartpointer__to__iteratorSTI < A >::item () const {
    alugrid_assert (! done ());
    return *(*_curr);
  }

  template < class A > IteratorSTI < A > * listSmartpointer__to__iteratorSTI < A >::
  clone () const
  {
    return new listSmartpointer__to__iteratorSTI < A > (*this);
  }

  inline bool GitterPll::debugOption (int level) {
#ifdef ALUGRIDDEBUG
    return (getenv ("VERBOSE_PLL") ? ( atoi (getenv ("VERBOSE_PLL")) > level ? true : (level == 0)) : false);
#else
    return false ;
#endif
  }

  template <class StopRule_t>
  inline std::pair< IteratorSTI < GitterPll::hedge_STI > *, IteratorSTI < GitterPll::hedge_STI > * > GitterPll ::
    createEdgeIteratorTT(const StopRule_t * fake, int l) {

    AccessIteratorTT < hedge_STI >::InnerHandle mdi (containerPll (), l);
    AccessIteratorTT < hedge_STI >::OuterHandle mdo (containerPll (), l);

    Insert < AccessIteratorTT < hedge_STI >::InnerHandle, TreeIterator < hedge_STI, StopRule_t> > ei (mdi);
    Insert < AccessIteratorTT < hedge_STI >::OuterHandle, TreeIterator < hedge_STI, StopRule_t> > eo (mdo);

    AccessIteratorTT < hface_STI >::InnerHandle mfi (containerPll (), l);
    AccessIteratorTT < hface_STI >::OuterHandle mfo (containerPll (), l);

    Insert < AccessIteratorTT < hface_STI >::InnerHandle, TreeIterator < hface_STI, has_int_edge < hface_STI > > > fimi (mfi);
    Insert < AccessIteratorTT < hface_STI >::OuterHandle, TreeIterator < hface_STI, has_int_edge < hface_STI > > > fimo (mfo);

    Wrapper < Insert < AccessIteratorTT < hface_STI >::InnerHandle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > dfimi (fimi);
    Wrapper < Insert < AccessIteratorTT < hface_STI >::OuterHandle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > dfimo (fimo);

    Insert < Wrapper < Insert < AccessIteratorTT < hface_STI >::InnerHandle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, StopRule_t> > eifi (dfimi);

    Insert < Wrapper < Insert < AccessIteratorTT < hface_STI >::OuterHandle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, StopRule_t> > eifo (dfimo);

    return std::pair< IteratorSTI < hedge_STI > *, IteratorSTI < hedge_STI > * >
      (new AlignIterator < Insert < AccessIteratorTT < hedge_STI >::InnerHandle, TreeIterator < hedge_STI, StopRule_t> >,
    Insert < Wrapper < Insert < AccessIteratorTT < hface_STI >::InnerHandle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, StopRule_t> >, hedge_STI > (ei,eifi),
       new AlignIterator < Insert < AccessIteratorTT < hedge_STI >::OuterHandle, TreeIterator < hedge_STI, StopRule_t> >,
    Insert < Wrapper < Insert < AccessIteratorTT < hface_STI >::OuterHandle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, StopRule_t> >, hedge_STI > (eo, eifo));
  }

  template <class StopRule_t>
  inline std::pair< IteratorSTI < GitterPll::hface_STI > *, IteratorSTI < GitterPll::hface_STI > *>
    GitterPll::createFaceIteratorTT (const StopRule_t rule , int l)
  {
    AccessIteratorTT < hface_STI >::InnerHandle mif (containerPll () , l);
    AccessIteratorTT < hface_STI >::OuterHandle mof (containerPll () , l);
    return std::pair< IteratorSTI < hface_STI > *, IteratorSTI < hface_STI > * >
      (new Insert < AccessIteratorTT < hface_STI >::InnerHandle, TreeIterator < hface_STI, StopRule_t > > (mif,rule),
       new Insert < AccessIteratorTT < hface_STI >::OuterHandle, TreeIterator < hface_STI, StopRule_t > > (mof,rule));
  }


  template < class A > inline LeafIteratorTT < A >::LeafIteratorTT (GitterPll & g, int l) : _grd (g), _link (l), _a (0) {
    _p = _grd.iteratorTT (_a, _link);
  }

  template < class A > inline LeafIteratorTT < A >::LeafIteratorTT (const LeafIteratorTT & org)
    : _grd (org._grd), _link (org._link), _a (0)
    , _p( org._p.first->clone() , org._p.second->clone() )
  {
  }

  template < class A > inline LeafIteratorTT < A >::~LeafIteratorTT ()
  {
    delete _p.first;
    delete _p.second;
  }

  template < class A > inline IteratorSTI < A > & LeafIteratorTT < A >::inner ()
  {
    return * _p.first;
  }

  template < class A > const inline IteratorSTI < A > & LeafIteratorTT < A >::inner () const
  {
    return * _p.first;
  }

  template < class A > inline IteratorSTI < A > & LeafIteratorTT < A >::outer ()
  {
    return * _p.second;
  }

  template < class A > const inline IteratorSTI < A > & LeafIteratorTT < A >::outer () const
  {
    return * _p.second;
  }

} // namespace ALUGrid

#endif // #ifndef GITTER_PLL_STI_H_INCLUDED
