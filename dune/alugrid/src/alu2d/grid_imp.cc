#ifndef ALU2D_GRID_IMP_CC
#define ALU2D_GRID_IMP_CC

#include "handle.h"
#include <sstream>

static const double EPS = 1e-8;

#ifdef PERIODIC_VERTICES
template < int N >
inline Vertex < N >*  Vertex < N >::get_pernb(int pidx)
{
  Vertex < N > *lret;

  if (pidx < nr_of_pernbs) 
    lret = pernb[pidx];
  else 
    lret = (Vertex < N > *)0;

  return lret; 
} 

template < int N >
inline void Vertex < N >::set_pernb(Vertex< N > *pv)
{
  int li, lnew = 1;

  for (li=0;li<nr_of_pernbs;li++)
  {
    if (pv == pernb[li]) lnew = 0;
  }

  if (pv == this) lnew = 0;

  if (lnew)
  {
    assert(nr_of_pernbs < 3);

    pernb[nr_of_pernbs] = pv;
    nr_of_pernbs++;
  }
}
#endif

// ***************************************************
// #begin(method)
// #method:
//   Fullvertex::Fullvertex(double (&p)[ncoord])
// #parameters:
//   \ double | (&p)[ncoord] | Koordinaten des Punktes
//                             als ncoord-Vektor
// #description:
//   allgemeiner Konstruktor
// #end(method)
// ***************************************************

template < int N >
inline Fullvertex < N >::Fullvertex(double (&p)[ncoord],int level) 
  : Vertex < N >( level ) 
{
  for(int i = 0 ; i < ncoord ; i ++) 
    vcoord[i] = p[i] ;
}

// ***************************************************
// #begin(method)
// #method:
//   double Bndel::area()
// #parameters:
// #description:
//   Gibt den Fl"acheninhalt der Geisterzelle wieder
// #end(method)
// ***************************************************
template < int N, int NV >
inline double Bndel < N,NV >::area() const 
{
  element_t *tr = (element_t *)(nbel(0));
  assert(tr);
  return tr->area();
}

template < int N >
inline void Vertex < N >::sethdl(IndexProvider *phdl) {
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(IndexProvider::IM_Vertices);
}
inline void Edge::sethdl(IndexProvider *phdl) {
  assert(_idx==-1);
  _idx=phdl->getIndex(IndexProvider::IM_Edges);
}

inline void Edge::freeIndex(IndexProvider* phdl)  
{
  assert(isfree());
  phdl->freeIndex(IndexProvider::IM_Edges,_idx);
}

template < int N, int NV >
inline void Element < N, NV >::sethdl(IndexProvider *phdl) 
{
  assert(_idx==-1);
  _idx = phdl->getIndex(IndexProvider::IM_Elements);
}
template < int N, int NV >
inline void Bndel < N, NV >::sethdl(IndexProvider *phdl) 
{
  assert(_idx==-1);
  _idx=phdl->getIndex(IndexProvider::IM_Bnd);
}
template < int N >
inline Vertex < N >::~Vertex() {
  if (hdl) {
    assert(_idx>=0);
    hdl->freeIndex(IndexProvider::IM_Vertices,_idx);
  }
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::edge_vtx(int e, Vertex *(&v) [2]) const
// #parameters:
//   \ int        | e | Lokale Kantennr. >=0
//   \ Vertex*[2] | v | Randpkt. der Kante e
// #description:
//   Liefert die Punkte auf der eten Kante modulo 3
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::edge_vtx(int e, vertex_t *(&v) [2]) const {
  assert(0 <= e) ;
  v[0] = connect.vtx[mod(e+1)] ;
  v[1] = connect.vtx[mod(e+2)] ;
}

// ***************************************************
// #begin(method)
// #method:
//   Vertex * Element::vertex(int i) const
// #parameters:
//   \ int i | Lokale Punktnr. >= 0
// #description:
//   Liefert iten Punkt zur"uck modulo 3
// #end(method)
// ***************************************************
template < int N, int NV >
inline typename Element < N, NV >::fullvertex_t *
Element < N,NV >::getVertex(int i) const {
  assert(0 <= i) ;
  return (fullvertex_t *)(connect.vtx[mod(i)]) ; 
}

template < int N, int NV >
inline typename Element < N, NV >::vertex_t *
Element < N, NV >::vertex(int i) const {
  assert(0 <= i) ;
  return connect.vtx[mod(i)] ; 
}

// ***************************************************
// #begin(method)
// #method:
//   Thinelement * Element::neighbour(int fce) const
// #parameters:
//   \ int | fce | Lokale Kantennr. >= 0
// #description:
//   Liefert Nachbaren gegen"uber der Kante mit Nr. fce modulo 3
// #end(method)
// ***************************************************
template < int N, int NV >
inline typename Element < N, NV >::thinelement_t *
Element < N, NV >::neighbour(int fce) const {
  assert(0 <= fce) ;
  return connect.nb[mod(fce)] ;
}

// ***************************************************
// #begin(method)
// #method:
//   int Element::opposite(int fce) const
// #parameters:
//   \ int | fce | Lokale Kantennr. >=0
// #description:
//   Liefert die lokale Nr. der Kante die der Nachbar mit der Nr. fce
//   mit this gemeinsamm hat (wird auch modulo 3 gerechnet)
// #end(method)
// ***************************************************
template < int N, int NV >
inline int Element < N, NV >::opposite(int fce) const {
  assert(0 <= fce) ;
  return connect.bck [mod(fce)] ;
}
template < int N, int NV >
inline int Element < N, NV >::edge_idx(int fce) const {
  assert(0 <= fce) ;
  return connect.edge[mod(fce)]->getIndex() ;
}
template < int N, int NV >
inline Edge *Element < N, NV >::edge(int fce) const {
  assert(0 <= fce) ;
  return connect.edge[fce];
}


// ***************************************************
// #begin(method)
// #method:
//   int Element::facevertex(int fce, int loc) const
// #parameters:
//   \ int | fce | Lokale Kantennr. >=0
//   \ int | loc | 0 oder 1
// #description:
//   Liefert lokale Nr. des 0ten bzw. 1ten Punktes auf der Kante mit der
//   lokalen Nr. fce
// #end(method)
// ***************************************************
template < int N, int NV >
inline int Element < N, NV >::facevertex(int fce, int loc) const { 
  assert(0 <= fce) ;
  assert(0 <= loc) ;
  fce = mod(fce); 
  loc %= connect.pv ;
  return mod(fce+loc+1); 
}

// ***************************************************
// #begin(method)
// #method:
//   int Element::normaldir(int fce) const
// #parameters:
//   \ int | fce | Lokale Kantennr. >=0
// #description:
//   Gibt -1 oder 1 zur"uck abh"anig davon, ob die gerichtete Normale an
//   die Kante moit der Nr. fce nach innen oder nach au"ssen zeigt.
//   D.h. die gerichtete Normale ergibt sich aus der "ausseren Normale
//   multipliziert mit dem R"uckgabewert dieser Funktion. 
//   Auf dem Nachbardreieck an diese Kante ergibt diese Funktion immer 
//   den entsprechend anderen Wert, au"ser: 
//   1.) Bei einer nichtkonfornen Verfeinerung zeigt die gerichtete Normale
//   an Grenzen immer von den kleinen in das gro"se Dreieck 
//   2.) Am Rand zeigt die gerichtet Normale immer nach aussnn 
//   Dieser kann auch benutzt werden, um in der Nummerik den Flu"s "uber die
//   Kanten gerichtet zu berechnen, d.h. man berechnet ihn nur f"ur Kanten
//   die eine 1 als R"uckgabewert haben.
// #end(method)
// ***************************************************
template < int N, int NV >
inline int Element < N, NV >::normaldir(int fce) const
{
  assert(0 <= fce) ;
  fce=mod(fce);
  return connect.normdir[fce] ;
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::nbconnect(int fce, Thinelement * n, int b)  
// #parameters:
//   \ int           | fce | Lokale Kantennr. >=0
//   \ Thinelement*  | n   | Pointer auf Nachbar 
//   \ int           | b   | Lokale Nr. der gemeinsammen Kante auf dem Nachbarelement >=0
// #description:
//   Legt alle Nachbarschaftsinformationen auf der Kante mit der lokalen Nr. fce
//   an.
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::nbconnect(int fce, thinelement_t * n, int b) { 
  assert(0 <= fce) ;
  fce = mod(fce) ;

  connect.nb[fce] = n ; 
  connect.bck[fce] = b ;
  }
template < int N, int NV >
inline void Element < N, NV >::edgeconnect(int fce, Edge * n) { 
  assert(0 <= fce) ;
  fce = mod(fce) ;

  connect.edge[fce] = n ; 
  n->attach();
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::setnormdir(int fce, int dir)  
// #parameters:
//   \ int | fce | Lokale Kantennr. >= 0
//   \ int | dor | Normalenrichtung (-1 oder 1)
// #description:
//   Legt die Richtung der gerichteten Normalen an der Kante mit der Nr. fce fest
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::setnormdir(int fce, int dir) {
  assert( 0 <= fce );
  fce = mod(fce) ;
  assert( dir == 1 || dir == -1);
  connect.normdir[fce] = dir;
}

// ***************************************************
// #begin(method)
// #method:
//   Element::c::c()
// #parameters:
// #description:
//   Konstruktor f"ur connect Daten (privat)
// #end(method)
// ***************************************************
template < int N, int NV >
inline Element < N, NV >::c::c() { 
  for( int i = 0 ; i < NV ; i ++ ) vtx[i] = 0; 
  for( int j = 0 ; j < NV ; j ++ ) { hvtx[j] = 0 ; nb[j] = 0 ; bck[j] = -1 ; normdir[j]=0 ; edge[j] = 0;}  
}

// ***************************************************
// #begin(method)
// #method:
//   Element::c::~c()
// #parameters:
// #description:
//   Destruktor f"ur connect Daten (privat)
// #end(method)
// ***************************************************
template < int N, int NV >
inline Element < N, NV >::c::~c() {
  for(int i = 0 ; i < NV ; i ++ ) 
    if(vtx[i]) vtx[i]->detach() ;
  for(int i = 0 ; i < NV ; i ++ ) 
    if(hvtx[i]) delete hvtx[i];  
  // edges are detached in the Element destructor
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::outernormal(int fce,double (&n)[ncoord]) const
// #parameters:
//   \ int                 | fce | Lokale Kantennr.
//   \ double (&)[ncoord]  | n   | R"uckgabe der Normalen
// #description:
//   Liefert unskalierte "au"sere Normale in n zur"uck
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::outernormal(int fce, double (&n)[ncoord]) const
{
  for (int i=0; i<ncoord; ++i)
    n[i] = _outernormal[mod(fce)][i];
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::dirnormal(int fce,double (&n)[ncoord]) const
// #parameters:
//   \ int                 | fce | Lokale Kantennr.
//   \ double (&)[ncoord]  | n   | R"uckgabe der Normalen 
// #description:
//   Liefert unskalierte gerichtete Normale in n zur"uck
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::dirnormal(int fce,double (&n)[ncoord]) const
{
  outernormal(fce,n);
  for (int i=0;i<ncoord;i++)
    n[i]*=normaldir(fce);
}

template < int N, int NV >
template <class O>
void Element < N, NV >::setorientation(vector<O> &str)
{
  assert( N == 3 );
  const int nf = numfaces();
  const double (&v0)[ncoord]=connect.vtx[0]->coord();
  const double (&v1)[ncoord]=connect.vtx[1]->coord();
  const double (&v2)[ncoord]=connect.vtx[nf-1]->coord();
  double e0[3];
  double e1[3];
  for (int i=0;i<3;++i)
  {
    e0[i] = v1[i]-v0[i];
    e1[i] = v2[i]-v0[i];
  }
  O add;
  add.el = this;
  add.nextNb = 0;
  add.n[0] = e0[1]*e1[2]-e1[1]*e0[2];
  add.n[1] = e0[2]*e1[0]-e1[2]*e0[0];
  add.n[2] = e0[0]*e1[1]-e1[0]*e0[1];
  if ( ! str.empty() )
  { // have comparison normal...
    O &old = str.back();
    if ( add.n[0]*old.n[0]+add.n[1]*old.n[1]+add.n[2]*old.n[2] < 0 )
    {
      switchorientation(1,nf-1);
      add.n[0] *= -1.;
      add.n[1] *= -1.;
      add.n[2] *= -1.;
    }
  }
  str.push_back(add);
}

#endif
