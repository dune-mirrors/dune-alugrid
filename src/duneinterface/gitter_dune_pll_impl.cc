#ifndef GITTER_DUNE_PLL_IMPL_CC_INCLUDED
#define GITTER_DUNE_PLL_IMPL_CC_INCLUDED

#include "gitter_dune_pll_impl.h"
#include "gitter_dune_pll_mgb.cc"

bool GitterDunePll :: duneNotifyNewGrid ()
{
  assert (debugOption (20) ? (cout << "**GitterDunePll :: duneNotifyNewGrid () " << endl, 1) : 1) ;
  const int np = mpAccess ().psize () ;
  LoadBalancer :: DataBase db ;
  {
    AccessIterator < hface_STI > :: Handle w (containerPll ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().ldbUpdateGraphEdge (db) ;
  }
  {
    AccessIterator < helement_STI > :: Handle w (containerPll ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().ldbUpdateGraphVertex (db) ;
  }
  bool neu = false ;
  {
    // Kriterium, wann eine Lastneuverteilung vorzunehmen ist:
    // 
    // load  - eigene ElementLast
    // mean  - mittlere ElementLast
    // nload - Lastverh"altnis

    double load = db.accVertexLoad () ;
    vector < double > v (mpAccess ().gcollect (load)) ;
    double mean = accumulate (v.begin (), v.end (), 0.0) / double (np) ;

    for (vector < double > :: iterator i = v.begin () ; i != v.end () ; i ++)
      neu |= (*i > mean ? (*i > (_ldbOver * mean) ? true : false) : (*i < (_ldbUnder * mean) ? true : false)) ;
  }
  return neu;
}



void GitterDunePll :: duneNotifyGridChanges ()
{
  Gitter :: notifyGridChanges () ;
  duneExchangeDynamicState () ;
  return ;
}


// done call notify and loadBalancer  
bool GitterDunePll :: duneAdapt ()   
{
  __STATIC_myrank = mpAccess ().myrank () ;
  __STATIC_turn ++ ;
  assert (debugOption (20) ? (cout << "**INFO GitterDunePll :: duneAdapt ()" << endl, 1) : 1) ;
  assert (! iterators_attached ()) ;
  int start = clock () ;
  bool refined = this->refine() ;
  int lap = clock () ;
  this->coarse ();
  int end = clock () ;
  if (debugOption (1))
  {
    float u1 = (float)(lap - start)/(float)(CLOCKS_PER_SEC) ;
    float u2 = (float)(end - lap)/(float)(CLOCKS_PER_SEC) ;
    float u3 = (float)(end - start)/(float)(CLOCKS_PER_SEC) ;
    cout << "**INFO GitterDunePll :: adapt () [ref (loops)|cse|all] " << u1 << " ("
         << _refineLoops << ") " << u2 << " " << u3 << endl ;
  }
  duneNotifyGridChanges () ;
  //balanceGrid_ = duneNotifyNewGrid();
  
  return refined;
}

// done call notify and loadBalancer  
bool GitterDunePll :: duneAdapt (AdaptRestrictProlongType & arp)   
{
  this->setAdaptRestrictProlongOp(arp);
  bool refined = this->duneAdapt();
  this->removeAdaptRestrictProlongOp ();
  return refined;
}

bool GitterDunePll :: duneLoadBalance () 
{
  loadBalancerGridChangesNotify () ;
  return true;
}

// returns true if grid was repartitioned 
bool GitterDunePll :: duneLoadBalance (GatherScatterType & gs) {
  assert (debugOption (20) ? (cout << "**GitterDunePll :: duneLoadBalance () " << endl, 1) : 1) ;
  const int np = mpAccess ().psize () ;
  LoadBalancer :: DataBase db ;
  {
    AccessIterator < hface_STI > :: Handle w (containerPll ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().ldbUpdateGraphEdge (db) ;
  }
  {
    AccessIterator < helement_STI > :: Handle w (containerPll ()) ;
    for (w.first () ; ! w.done () ; w.next ()) w.item ().accessPllX ().ldbUpdateGraphVertex (db) ;
  }
  bool neu = false ;
  {
  // Kriterium, wann eine Lastneuverteilung vorzunehmen ist:
  // 
  // load  - eigene ElementLast
  // mean  - mittlere ElementLast
  // nload - Lastverh"altnis
  
    double load = db.accVertexLoad () ;
    vector < double > v (mpAccess ().gcollect (load)) ;
    double mean = accumulate (v.begin (), v.end (), 0.0) / double (np) ;

    for (vector < double > :: iterator i = v.begin () ; i != v.end () ; i ++)
      neu |= (*i > mean ? (*i > (_ldbOver * mean) ? true : false) : (*i < (_ldbUnder * mean) ? true : false));
  }
  if (neu) 
  {
    if (mpAccess ().gmax (_ldbMethod)) 
    {
      assert (debugOption (5) ? (cout << "**GitterDunePll :: repartitioning macro grid! " << endl, 1) : 1) ;
      duneRepartitionMacroGrid (db, gs) ;
      notifyMacroGridChanges () ;
    }
  }
  return neu;
}

void GitterDunePll :: duneExchangeDynamicState () 
{
  // Die Methode wird jedesmal aufgerufen, wenn sich der dynamische
  // Zustand des Gitters ge"andert hat: Verfeinerung und alle Situationen
  // die einer "Anderung des statischen Zustands entsprechen. Sie wird in
  // diesem Fall NACH dem Update des statischen Zustands aufgerufen, und
  // kann demnach von einem korrekten statischen Zustand ausgehen. F"ur
  // Methoden die noch h"aufigere Updates erfordern m"ussen diese in der
  // Regel hier eingeschleift werden.
  {

  const int nl = mpAccess ().nlinks () ;
  
#ifndef NDEBUG 
  // if debug mode, then count time 
  const int start = clock () ;
#endif
  
  try 
  {
    typedef Insert < AccessIteratorTT < hface_STI > :: InnerHandle,
                TreeIterator < hface_STI, is_def_true < hface_STI > > > InnerIteratorType;
    typedef Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
                TreeIterator < hface_STI, is_def_true < hface_STI > > > OuterIteratorType;
                
    vector < ObjectStream > osv (nl) ;
    {
      for (int l = 0 ; l < nl ; l ++) 
      {
        {
          AccessIteratorTT < hface_STI > :: InnerHandle mif (this->containerPll (),l) ;
          AccessIteratorTT < hface_STI > :: OuterHandle mof (this->containerPll (),l) ;

          InnerIteratorType wi (mif);
          for (wi.first () ; ! wi.done () ; wi.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wi.item ().accessPllX ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
          }
        
          OuterIteratorType wo (mof);
          for (wo.first () ; ! wo.done () ; wo.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wo.item ().accessPllX ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
          }
        }
      } 
    }
    
    osv = mpAccess ().exchange (osv) ;
    
    { 
      for (int l = 0 ; l < nl ; l ++ ) 
      {
        {
          AccessIteratorTT < hface_STI > :: OuterHandle mof (this->containerPll (),l) ;
          AccessIteratorTT < hface_STI > :: InnerHandle mif (this->containerPll (),l) ;
        
          OuterIteratorType wo (mof) ;
          for (wo.first () ; ! wo.done () ; wo.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wo.item ().accessPllX ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
          }
        
          InnerIteratorType wi (mif);
          for (wi.first () ; ! wi.done () ; wi.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wi.item ().accessPllX ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
          }
        }
      } 
    }
  } 
  catch (Parallel ::  AccessPllException) 
  {
    cerr << "  FEHLER Parallel :: AccessPllException entstanden in: " << __FILE__ << " " << __LINE__ << endl ;
  }
  assert (debugOption (20) ? (cout << "**INFO GitterDunePll :: exchangeDynamicState () used " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec. " << endl, 1) : 1 ) ;
  }
}

#if 0
// reun only over leaf Level 
void GitterDunePll :: duneExchangeDataLeaf (GatherScatterType & gs) 
{
  // Die Methode wird jedesmal aufgerufen, wenn sich der dynamische
  // Zustand des Gitters ge"andert hat: Verfeinerung und alle Situationen
  // die einer "Anderung des statischen Zustands entsprechen. Sie wird in
  // diesem Fall NACH dem Update des statischen Zustands aufgerufen, und
  // kann demnach von einem korrekten statischen Zustand ausgehen. F"ur
  // Methoden die noch h"aufigere Updates erfordern m"ussen diese in der
  // Regel hier eingeschleift werden.
  {
  const int nl = mpAccess ().nlinks () ;
#ifndef NDEBUG
  const int start = clock () ;
#endif
  try 
  {
    typedef LeafIteratorTT< hface_STI > InnerIteratorType;
    typedef LeafIteratorTT< hface_STI > OuterIteratorType;
                
    vector < ObjectStream > osv (nl) ;
    {
      for (int l = 0 ; l < nl ; l ++) 
      {
        {
          LeafIteratorTT < hface_STI > w (*this,l);
          for (w.inner() .first () ; ! w.inner().done () ; w.inner().next ()) 
          {
            pair < ElementPllXIF_t *, int > p = w.inner().item ().accessPllX ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
            p.first->writeDynamicState (osv [l], gs) ;
          }
        
          for (w.outer().first () ; ! w.outer().done () ; w.outer().next ()) 
          {
            pair < ElementPllXIF_t *, int > p = w.outer().item ().accessPllX ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
            p.first->writeDynamicState (osv [l], gs) ;
          }
        }
      } 
    }
    
    osv = mpAccess ().exchange (osv) ;
    
    { 
      for (int l = 0 ; l < nl ; l ++ ) 
      {
        {
          LeafIteratorTT< hface_STI > w (*this,l) ;
          for (w.outer().first () ; ! w.outer().done () ; w.outer().next ()) 
          {
            pair < ElementPllXIF_t *, int > p = w.outer().item ().accessPllX ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
            p.first->readDynamicState (osv [l], gs) ;
          }
        
          for (w.inner().first () ; ! w.inner().done () ; w.inner().next ()) 
          {
            pair < ElementPllXIF_t *, int > p = w.inner().item ().accessPllX ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
            p.first->readDynamicState (osv [l], gs ) ;
          }
        }
      } 
    }
  } 
  catch (Parallel ::  AccessPllException) 
  {
    cerr << "  FEHLER Parallel :: AccessPllException entstanden in: " << __FILE__ << " " << __LINE__ << endl ;
  }
  assert (debugOption (20) ? (cout << "**INFO GitterDunePll :: duneExchangeData () used " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec. " << endl, 1) : 1 ) ;
  }
}
#endif

// go over all levels 
void GitterDunePll :: duneExchangeDataAll (GatherScatterType & gs) 
{
  // Die Methode wird jedesmal aufgerufen, wenn sich der dynamische
  // Zustand des Gitters ge"andert hat: Verfeinerung und alle Situationen
  // die einer "Anderung des statischen Zustands entsprechen. Sie wird in
  // diesem Fall NACH dem Update des statischen Zustands aufgerufen, und
  // kann demnach von einem korrekten statischen Zustand ausgehen. F"ur
  // Methoden die noch h"aufigere Updates erfordern m"ussen diese in der
  // Regel hier eingeschleift werden.
  {
  const int nl = mpAccess ().nlinks () ;
#ifndef NDEBUG
  const int start = clock () ;
#endif
  try 
  {
    typedef Insert < AccessIteratorTT < hface_STI > :: InnerHandle,
                TreeIterator < hface_STI, is_def_true < hface_STI > > > InnerIteratorType;
    typedef Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
                TreeIterator < hface_STI, is_def_true < hface_STI > > > OuterIteratorType;
                
    vector < ObjectStream > osv (nl) ;
    {
      for (int l = 0 ; l < nl ; l ++) 
      {
        {
          AccessIteratorTT < hface_STI > :: InnerHandle mif (this->containerPll (),l) ;
          AccessIteratorTT < hface_STI > :: OuterHandle mof (this->containerPll (),l) ;

          InnerIteratorType wi (mif);
          for (wi.first () ; ! wi.done () ; wi.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wi.item ().accessPllX ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
            p.first->writeDynamicState (osv [l], gs) ;
          }
        
          OuterIteratorType wo (mof);
          for (wo.first () ; ! wo.done () ; wo.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wo.item ().accessPllX ().accessInnerPllX () ;
            p.first->writeDynamicState (osv [l], p.second) ;
            p.first->writeDynamicState (osv [l], gs) ;
          }
        }
      } 
    }
    
    osv = mpAccess ().exchange (osv) ;
    
    { 
      for (int l = 0 ; l < nl ; l ++ ) 
      {
        {
          AccessIteratorTT < hface_STI > :: OuterHandle mof (this->containerPll (),l) ;
          AccessIteratorTT < hface_STI > :: InnerHandle mif (this->containerPll (),l) ;
        
          OuterIteratorType wo (mof) ;
          for (wo.first () ; ! wo.done () ; wo.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wo.item ().accessPllX ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
            p.first->readDynamicState (osv [l], gs) ;
          }
        
          InnerIteratorType wi (mif);
          for (wi.first () ; ! wi.done () ; wi.next ()) 
          {
            pair < ElementPllXIF_t *, int > p = wi.item ().accessPllX ().accessOuterPllX () ;
            p.first->readDynamicState (osv [l], p.second) ;
            p.first->readDynamicState (osv [l], gs ) ;
          }
        }
      } 
    }
  } 
  catch (Parallel ::  AccessPllException) 
  {
    cerr << "  FEHLER Parallel :: AccessPllException entstanden in: " << __FILE__ << " " << __LINE__ << endl ;
  }
  assert (debugOption (20) ? (cout << "**INFO GitterDunePll :: duneExchangeData () used " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec. " << endl, 1) : 1 ) ;
  }
}

void GitterDunePll :: duneExchangeData (GatherScatterType & gs, bool leaf) 
{
  //if(leaf) 
  //  this->duneExchangeDataLeaf(gs);
  //else 
  this->duneExchangeDataAll(gs);
  return; 
}

bool GitterDunePll :: refine () {
  assert (debugOption (5) ? (cout << "**INFO GitterDunePll :: refine () " << endl, 1) : 1) ;
  const int nl = mpAccess ().nlinks () ;
  bool state = false ;
  vector < vector < hedge_STI * > > innerEdges (nl), outerEdges (nl) ;
  vector < vector < hface_STI * > > innerFaces (nl), outerFaces (nl) ;
  {
  // Erst die Zeiger auf alle Fl"achen und Kanten mit paralleler
  // Mehrdeutigkeit sichern, da die LeafIteratorTT < . > nach dem 
  // Verfeinern auf gitter nicht mehr stimmen werden. Die Technik
  // ist zul"assig, da keine mehrfache Verfeinerung entstehen kann.
  
    {for (int l = 0 ; l < nl ; l ++) {
      //cout << "refinepll \n";
      LeafIteratorTT < hface_STI > fw (*this,l) ;
      LeafIteratorTT < hedge_STI > dw (*this,l) ;
      for (fw.outer ().first () ; ! fw.outer().done () ; fw.outer ().next ())
        outerFaces [l].push_back (& fw.outer ().item ()) ;
      for (fw.inner ().first () ; ! fw.inner ().done () ; fw.inner ().next ())
        innerFaces [l].push_back (& fw.inner ().item ()) ;
      for (dw.outer ().first () ; ! dw.outer().done () ; dw.outer ().next ())
        outerEdges [l].push_back (& dw.outer ().item ()) ;
      for (dw.inner ().first () ; ! dw.inner ().done () ; dw.inner ().next ())
        innerEdges [l].push_back (& dw.inner ().item ()) ;
    }}
      // jetzt normal verfeinern und den Status der Verfeinerung
  // [unvollst"andige / vollst"andige Verfeinerung] sichern.
    
    __STATIC_phase = 1 ;
    
    state = Gitter :: refine () ;
       
  // Phase des Fl"achenausgleichs an den Schnittfl"achen des
  // verteilten Gitters. Weil dort im sequentiellen Fall pseudorekursive
  // Methodenaufrufe vorliegen k"onnen, muss solange iteriert werden,
  // bis die Situation global station"ar ist.
  
    __STATIC_phase = 2 ;
  
    bool repeat (false) ;
    _refineLoops = 0 ;
    do {
      repeat = false ;
      {
        vector < ObjectStream > osv (nl) ;
        try {
      //cout << "refinepll 2 \n";
    for (int l = 0 ; l < nl ; l ++) {
            {for (vector < hface_STI * > :: const_iterator i = outerFaces [l].begin () ;
              i != outerFaces [l].end () ; (*i ++)->accessPllX ().accessOuterPllX ().first->getRefinementRequest (osv [l])) ; }
            {for (vector < hface_STI * > :: const_iterator i = innerFaces [l].begin () ;
              i != innerFaces [l].end () ; (*i ++)->accessPllX ().accessOuterPllX ().first->getRefinementRequest (osv [l])) ; }
          }
  } catch (Parallel :: AccessPllException) {
          cerr << "**FEHLER (FATAL) AccessPllException in " << __FILE__ << " " << __LINE__ << endl ; abort () ;
        }
  
        osv = mpAccess ().exchange (osv) ;
  
        try {
    for (int l = 0 ; l < nl ; l ++) {
            {for (vector < hface_STI * > :: const_iterator i = innerFaces [l].begin () ;
              i != innerFaces [l].end () ; repeat |= (*i ++)->accessPllX ().accessOuterPllX ().first->setRefinementRequest (osv [l])) ; }
            {for (vector < hface_STI * > :: const_iterator i = outerFaces [l].begin () ;
              i != outerFaces [l].end () ; repeat |= (*i ++)->accessPllX ().accessOuterPllX ().first->setRefinementRequest (osv [l])) ; }
          }
  } catch (Parallel :: AccessPllException) {
          cerr << "**FEHLER (FATAL) AccessPllException in " << __FILE__ << " " << __LINE__ << endl ; abort () ;
        }
      }

      _refineLoops ++ ;
    } while (mpAccess ().gmax (repeat ? 1 : 0)) ;

  // Jetzt noch die Kantensituation richtigstellen, es gen"ugt ein Durchlauf,
  // weil die Verfeinerung einer Kante keine Fernwirkungen hat. Vorsicht: Die
  // Kanten sind bez"uglich ihrer Identifikation sternf"ormig organisiert, d.h.
  // es muss die Verfeinerungsinformation einmal am Eigent"umer gesammelt und
  // dann wieder zur"ucktransportiert werden, eine einfache L"osung, wie bei
  // den Fl"achen (1/1 Beziehung) scheidet aus.

    __STATIC_phase = 3 ;

    {
      vector < ObjectStream > osv (nl) ;
      {for (int l = 0 ; l < nl ; l ++) 
        for (vector < hedge_STI * > :: const_iterator i = outerEdges [l].begin () ;
          i != outerEdges [l].end () ; (*i ++)->accessPllX ().getRefinementRequest (osv [l])) ;
      }
      osv = mpAccess ().exchange (osv) ;
      {for (int l = 0 ; l < nl ; l ++)
        for (vector < hedge_STI * > :: const_iterator i = innerEdges [l].begin () ;
          i != innerEdges [l].end () ; (*i ++)->accessPllX ().setRefinementRequest (osv [l])) ;
      }
    }   // ~vector < ObjectStream > ... 
    {
      vector < ObjectStream > osv (nl) ;
      {for (int l = 0 ; l < nl ; l ++)
        for (vector < hedge_STI * > :: const_iterator i = innerEdges [l].begin () ;
          i != innerEdges [l].end () ; (*i ++)->accessPllX ().getRefinementRequest (osv [l])) ;
      }
      osv = mpAccess ().exchange (osv) ;
      {for (int l = 0 ; l < nl ; l ++)
        for (vector < hedge_STI * > :: const_iterator i = outerEdges [l].begin () ;
          i != outerEdges [l].end () ; (*i ++)->accessPllX ().setRefinementRequest (osv [l])) ;
      }
    }   // ~vector < ObjectStream > ... 
  }
  
  __STATIC_phase = -1 ;
  
  return state ;
}

void GitterDunePll :: coarse () {
  assert (debugOption (20) ? (cout << "**INFO GitterDunePll :: coarse () " << endl, 1) : 1) ;
  const int nl = mpAccess ().nlinks () ;
  
  {
    vector < vector < hedge_STI * > > innerEdges (nl), outerEdges (nl) ;
    vector < vector < hface_STI * > > innerFaces (nl), outerFaces (nl) ;
  
    for (int l = 0 ; l < nl ; l ++) {
    
  // Zun"achst werden f"ur alle Links die Zeiger auf Gitterojekte mit
  // Mehrdeutigkeit gesichert, die an der Wurzel einer potentiellen
  // Vergr"oberungsoperation sitzen -> es sind die Knoten in der Hierarchie,
  // deren Kinder alle Bl"atter sind. Genau diese Knoten sollen gegen"uber
  // der Vergr"oberung blockiert werden und dann die Vergr"oberung falls
  // sie zul"assig ist, sp"ater durchgef"uhrt werden (pending) ;
    
      AccessIteratorTT < hface_STI > :: InnerHandle mfwi (containerPll (),l) ;
      AccessIteratorTT < hface_STI > :: OuterHandle mfwo (containerPll (),l) ;
      AccessIteratorTT < hedge_STI > :: InnerHandle mdwi (containerPll (),l) ;
      AccessIteratorTT < hedge_STI > :: OuterHandle mdwo (containerPll (),l) ;
      
  // Die inneren und a"usseren Iteratoren der potentiell vergr"oberungsf"ahigen
  // Fl"achen "uber den Grobgitterfl"achen. In den Elementen passiert erstmal
  // nichts, solange nicht mit mehrfachen Grobgitterelementen gearbeitet wird.
      
      Insert < AccessIteratorTT < hface_STI > :: InnerHandle, 
        TreeIterator < hface_STI, childs_are_leafs < hface_STI > > > fwi (mfwi) ;
      Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
        TreeIterator < hface_STI, childs_are_leafs < hface_STI > > > fwo (mfwo) ;
      
  // Die inneren und a"usseren Iteratoren der potentiell vergr"oberungsf"ahigen
  // Kanten "uber den Grobgitterkanten.
      
      Insert < AccessIteratorTT < hedge_STI > :: InnerHandle, 
        TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dwi (mdwi) ;
      Insert < AccessIteratorTT < hedge_STI > :: OuterHandle, 
        TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dwo (mdwo) ;

  // Die inneren und a"usseren Iteratoren der potentiell vergr"oberungsf"ahigen
  // Kanten "uber den Grobgitterfl"achen. Diese Konstruktion wird beim Tetraeder-
  // gitter notwendig, weil dort keine Aussage der Form:
  //

      Insert < AccessIteratorTT < hface_STI > :: InnerHandle, 
        TreeIterator < hface_STI, has_int_edge < hface_STI > > > efi (mfwi) ;
      Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
        TreeIterator < hface_STI, has_int_edge < hface_STI > > > efo (mfwo) ;
      Wrapper < Insert < AccessIteratorTT < hface_STI > :: InnerHandle, 
        TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > eifi (efi) ;
      Wrapper < Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
        TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > eifo (efo) ;
      Insert < Wrapper < Insert < AccessIteratorTT < hface_STI > :: InnerHandle, 
        TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dfi (eifi) ;
      Insert < Wrapper < Insert < AccessIteratorTT < hface_STI > :: OuterHandle, 
        TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
  TreeIterator < hedge_STI, childs_are_leafs < hedge_STI > > > dfo (eifo) ;

  // Die 'item ()' Resultatwerte (Zeiger) werden in Vektoren gesichert, weil die
  // Kriterien die zur Erzeugung der Iteratoren angewendet wurden (Filter) nach
  // einer teilweisen Vergr"oberung nicht mehr g"ultig sein werden, d.h. die 
  // Iterationsobjekte "andern w"ahrend der Vergr"oberung ihre Eigenschaften.
  // Deshalb werden sie auch am Ende des Blocks aufgegeben. Der Vektor 'cache'
  // ist zul"assig, weil kein Objekt auf das eine Referenz im 'cache' vorliegt
  // beseitigt werden kann. Sie sind alle ein Niveau darunter.

      for (fwi.first () ; ! fwi.done () ; fwi.next ()) innerFaces [l].push_back (& fwi.item ()) ;
      for (fwo.first () ; ! fwo.done () ; fwo.next ()) outerFaces [l].push_back (& fwo.item ()) ;
      for (dwo.first () ; ! dwo.done () ; dwo.next ()) outerEdges [l].push_back (& dwo.item ()) ;
      for (dfo.first () ; ! dfo.done () ; dfo.next ()) outerEdges [l].push_back (& dfo.item ()) ;
      for (dwi.first () ; ! dwi.done () ; dwi.next ()) innerEdges [l].push_back (& dwi.item ()) ;
      for (dfi.first () ; ! dfi.done () ; dfi.next ()) innerEdges [l].push_back (& dfi.item ()) ;
    }
    try {
  // Erstmal alles was mehrdeutig ist, gegen die drohende Vergr"oberung sichern.
  // Danach werden sukzessive die Fl"achenlocks aufgehoben, getestet und
  // eventuell vergr"obert, dann das gleiche Spiel mit den Kanten.

      for (int l = 0 ; l < nl ; l ++) {
        {for (vector < hedge_STI * > :: iterator i = outerEdges [l].begin () ;
            i != outerEdges [l].end () ; (*i ++)->accessPllX ().lockAndTry ()) ; }
        {for (vector < hedge_STI * > :: iterator i = innerEdges [l].begin () ;
            i != innerEdges [l].end () ; (*i ++)->accessPllX ().lockAndTry ()) ; }
        {for (vector < hface_STI * > :: iterator i = outerFaces [l].begin () ;
            i != outerFaces [l].end () ; (*i ++)->accessPllX ().accessOuterPllX ().first->lockAndTry ()) ; }
        {for (vector < hface_STI * > :: iterator i = innerFaces [l].begin () ;
            i != innerFaces [l].end () ; (*i ++)->accessPllX ().accessOuterPllX ().first->lockAndTry ()) ; }
      }
      
        // Gitter :: coarse () ist elementorientiert, d.h. die Vergr"oberung auf Fl"achen und
  // Kanten wird nur durch Vermittlung eines sich vergr"obernden Knotens in der Element-
  // hierarchie angestossen. In allen gegen Vergr"oberung 'gelockten' Fl"achen und Kanten
  // wird die angeforderte Operation zur"uckgewiesen, um erst sp"ater von aussen nochmals
  // angestossen zu werden.
      
      __STATIC_phase = 4 ;
      
      Gitter :: coarse () ;
      
    } catch (Parallel :: AccessPllException) {
      cerr << "**FEHLER (FATAL) AccessPllException beim Vergr\"obern der Elementhierarchie oder\n" ;
      cerr << "  beim locken der Fl\"achen- bzw. Kantenb\"aume aufgetreten. In " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
    try {
    
  // Phase des Fl"achenausgleichs des verteilten Vergr"oberungsalgorithmus
  // alle Schnittfl"achenpaare werden daraufhin untersucht, ob eine
  // Vergr"oberung in beiden Teilgittern durchgef"uhrt werden darf,
  // wenn ja, wird in beiden Teilgittern vergr"obert und der Vollzug
  // getestet.
  
      __STATIC_phase = 5 ;
    
      vector < vector < int > > clean (nl) ;
      {
        vector < vector < int > > inout (nl) ;
        {for (int l = 0 ; l < nl ; l ++)
          for (vector < hface_STI * > :: iterator i = outerFaces [l].begin () ; i != outerFaces [l].end () ; i ++)
            inout [l].push_back ((*i)->accessPllX ().accessOuterPllX ().first->lockAndTry ()) ;
        }
        inout = mpAccess ().exchange (inout) ;
        {for (int l = 0 ; l < nl ; l ++) {
          clean [l] = vector < int > (innerFaces [l].size (), long (true)) ;
    vector < int > :: iterator j = clean [l].begin (), k = inout [l].begin () ;
          for (vector < hface_STI * > :: iterator i = innerFaces [l].begin () ; i != innerFaces [l].end () ; i ++, j++, k++) {
      assert (j != clean [l].end ()) ; assert (k != inout [l].end ()) ;
      (*j) &= (*k) && (*i)->accessPllX ().accessOuterPllX ().first->lockAndTry () ;
    }
        }}
      }
      {
        vector < vector < int > > inout (nl) ;
        {for (int l = 0 ; l < nl ; l ++) {
          vector < int > :: iterator j = clean [l].begin () ;
    for (vector < hface_STI * > :: iterator i = innerFaces [l].begin () ; i != innerFaces [l].end () ; i ++, j++) {
      inout [l].push_back (*j) ;
      (*i)->accessPllX ().accessOuterPllX ().first->unlockAndResume (bool (*j)) ;
    }
        }}
      
        inout = mpAccess ().exchange (inout) ;
      
        {for (int l = 0 ; l < nl ; l ++) {
          vector < int > :: iterator j = inout [l].begin () ;
          for (vector < hface_STI * > :: iterator i = outerFaces [l].begin () ; i != outerFaces [l].end () ; i ++, j++) {
      assert (j != inout [l].end ()) ;
      (*i)->accessPllX ().accessOuterPllX ().first->unlockAndResume (bool (*j)) ;
    }
        }}
      }
    } catch (Parallel :: AccessPllException) {
      cerr << "**FEHLER (FATAL) AccessPllException beim Vergr\"obern der Fl\"achenb\"aume\n" ;
      cerr << "  aufgetreten. In " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
    try {
    
  // Phase des Kantenausgleichs im parallelen Vergr"oberungsalgorithmus:
  
      __STATIC_phase  = 6 ;
    
  // Weil hier jede Kante nur eindeutig auftreten darf, muss sie in einem
  // map als Adresse hinterlegt werden, dann k"onnen die verschiedenen
  // Refcounts aus den verschiedenen Links tats"achlich global miteinander
  // abgemischt werden. Dazu werden zun"achst alle eigenen Kanten auf ihre
  // Vergr"oberbarkeit hin untersucht und dieser Zustand (true = vergr"oberbar
  // false = darf nicht vergr"obert werden) im map 'clean' hinterlegt. Dazu
  // kommt noch ein zweiter 'bool' Wert, der anzeigt ob die Kante schon ab-
  // schliessend vergr"obert wurde oder nicht. 
    
      map < hedge_STI *, pair < bool, bool >, less < hedge_STI * > > clean ;
      
      {for (int l = 0 ; l < nl ; l ++)
        for (vector < hedge_STI * > :: iterator i = innerEdges [l].begin () ; i != innerEdges [l].end () ; i ++)
    if (clean.find (*i) == clean.end ()) clean [*i] = pair < bool, bool > ((*i)->accessPllX ().lockAndTry (), true) ;
      }
      {
        vector < vector < int > > inout (nl) ;
        {for (int l = 0 ; l < nl ; l ++)
          for (vector < hedge_STI * > :: iterator i = outerEdges [l].begin () ; i != outerEdges [l].end () ; i ++)
            inout [l].push_back ((*i)->accessPllX ().lockAndTry ()) ;
  }
        inout = mpAccess ().exchange (inout) ;
        {for (int l = 0 ; l < nl ; l ++) {
    vector < int > :: const_iterator j = inout [l].begin () ;
          for (vector < hedge_STI * > :: iterator i = innerEdges [l].begin () ; i != innerEdges [l].end () ; i ++, j++) {
      assert (j != inout [l].end ()) ;
            assert (clean.find (*i) != clean.end ()) ;
            if (*j == false) clean [*i] = pair < bool, bool > (false, clean[*i].second) ; 
    }
        }}
      }
      {
        vector < vector < int > > inout (nl) ;
        {for (int l = 0 ; l < nl ; l ++) {
    for (vector < hedge_STI * > :: iterator i = innerEdges [l].begin () ; i != innerEdges [l].end () ; i ++) {
      assert (clean.find (*i) != clean.end ()) ;
      pair < bool, bool > & a = clean [*i] ;
      inout [l].push_back (a.first) ;
      if (a.second) {
  
  // Wenn wir hier sind, kann die Kante tats"achlich vergr"obert werden, genauer gesagt,
  // sie wird es auch und der R"uckgabewert testet den Vollzug der Aktion. Weil aber nur
  // einmal vergr"obert werden kann, und die Iteratoren 'innerEdges [l]' aber eventuell
  // mehrfach "uber eine Kante hinweglaufen, muss diese Vergr"oberung im map 'clean'
  // vermerkt werden. Dann wird kein zweiter Versuch unternommen.
  
        a.second = false ;
#ifndef NDEBUG
        bool b = 
#endif
          (*i)->accessPllX ().unlockAndResume (a.first) ;
        assert (b == a.first) ;
      }
    }
        }}
        inout = mpAccess ().exchange (inout) ;
        {for (int l = 0 ; l < nl ; l ++) {
          vector < int > :: iterator j = inout [l].begin () ;
          for (vector < hedge_STI * > :: iterator i = outerEdges [l].begin () ; i != outerEdges [l].end () ; i ++, j++) {
      assert (j != inout [l].end ()) ;
      
  // Selbe Situation wie oben, aber der Eigent"umer der Kante hat mitgeteilt, dass sie
  // vergr"obert werden darf und auch wird auf allen Teilgebieten also auch hier. Der
  // Vollzug der Vergr"oberung wird durch den R"uckgabewert getestet.
      
#ifndef NDEBUG
      bool b = 
#endif
        (*i)->accessPllX ().unlockAndResume (bool (*j)) ;
      assert (b == bool (*j)) ;
    }
        }}
      }
    } catch (Parallel :: AccessPllException) {
      cerr << "**FEHLER (FATAL) AccessPllException beim Vergr\"obern der Kantenb\"aume\n" ;
      cerr << "  aufgetreten. In " << __FILE__ << " " << __LINE__ << endl ;
      abort () ;
    }
  }
  
  __STATIC_phase = -1 ;
  
  return ;
}

#endif
