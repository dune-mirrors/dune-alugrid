#ifndef ALU2D_GRID_IMP_CC
#define ALU2D_GRID_IMP_CC

#include "handle.h"
#include <sstream>

static const double EPS = 1e-8;

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
inline Fullvertex < N >::Fullvertex(double (&p)[ncoord],int level) : Vertex < N >( level ) {
  for(int i = 0 ; i < ncoord ; i ++) 
    vcoord[i] = p[i] ;
}

// ***************************************************
// #begin(method)
// #method:
//   void Fullvertex::write(ostream & out) const
// #parameters:
//   \ ostream | &out | output--stream
// #description:
//   Ausgabe der Punktkoordinaten
// #end(method)
// ***************************************************

template < int N >
inline void Fullvertex < N >::write(ostream & out) const {

  for(int i = 0 ; i < ncoord ; i ++ ) out << vcoord[i] << "  " ;
   
  out << endl ;

}
// ***************************************************
// #begin(method)
// #method:
//   void Fullvertex::read(istream & in)
// #parameters:
//   \ istream | &in | input--stream
// #description:
//   Einlesen der Punktkoordinaten
// #end(method)
// ***************************************************

template < int N >
inline void Fullvertex < N >::read(istream & in) {

  for(int i = 0 ; i < ncoord ; i ++) 

    in >> vcoord[i] ;

}
// ***************************************************
// #begin(method)
// #method:
//   void Edge::write(ostream & out) const
// #parameters:
//   \ ostream | &out | output--stream
// #description:
//   Ausgabe der Punktkoordinaten
// #end(method)
// ***************************************************

inline void Edge::write(ostream & out) const {

  out << getIndex();
  out << endl ;

}
// ***************************************************
// #begin(method)
// #method:
//   void Edge::read(istream & in)
// #parameters:
//   \ istream | &in | input--stream
// #description:
//   Einlesen der Punktkoordinaten
// #end(method)
// ***************************************************

inline void Edge::read(istream & in) {

  in >> setIndex();

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

// ***************************************************
// #begin(method)
// #method:
//   int Bndel::get_splitpoint(double (&ppoint) [2])
// #parameters:
//   \ double (& ) [2] | ppoint | splitpoint
// #description:
//   Calculate coordinates of splitpoint with
//   Newton's scheme
// #end(method)
// ***************************************************
template < int N, int NV >
inline int Bndel < N, NV >::get_splitpoint(double (&ppoint) [ncoord])
{ 
  const double (&c0)[ncoord] = connect.vtx[0]->coord();
  const double (&c1)[ncoord] = connect.vtx[1]->coord();

  for (int i=0;i<ncoord;++i)
    ppoint[i] = 0.5*(c0[i]+c1[i]);
  // ppoint[0] = 0.5 * ( c0[0] + c1[0] );//connect.vtx[0]->coord()[0]+connect.vtx[1]->coord()[0]);
  // ppoint[1] = 0.5 * ( c0[1] + c1[1] );//connect.vtx[0]->coord()[1]+connect.vtx[1]->coord()[1]);

  // old method, new method below 
#ifdef ALU2D_OLD_BND_PROJECTION
  if (lf && lDf)
  {
    const double EPS = 1e-8;
    const int lmax_iter = 1000;
    const double ltol   = 1e-12;

    int li=0,lret=0;

    double ldiv,lx,ly,lvx=0.0,lvy=0.0,lt=0.0;
    assert(fabs(  lf(connect.vtx[0]->coord()[0])
                - connect.vtx[0]->coord()[1]) <= 2.0 * ltol);
    assert(fabs(  lf(connect.vtx[1]->coord()[0])
    - connect.vtx[1]->coord()[1]) <= 2.0 * ltol);

    lvx = connect.vtx[0]->coord()[1] - connect.vtx[1]->coord()[1];
    lvy = connect.vtx[1]->coord()[0] - connect.vtx[0]->coord()[0];

    lx  = ppoint[0];
    ly  = ppoint[1];

    do
    {
      ldiv = lDf(lx) * lvx - lvy;
      if (fabs(ldiv) > EPS)
      {
        lt -= (lf(lx) - ly) / ldiv;

        lx = ppoint[0] + lt * lvx;
        ly = ppoint[1] + lt * lvy;
      }
      else
      {
        lret = -2;
      }
      li++;
    } while ((fabs(lf(lx) - ly) > ltol) && (li < lmax_iter) && (lret == 0));

    if ((li >= lmax_iter) || (lret != 0))
    {
      lt = 0.0;
      if (lret == 0) lret = -1;
    }
    else
    {
      lret = 1;
    }

    ppoint[0] += lt * lvx;
    ppoint[1] += lt * lvy;
  }
#else // use new method 

  // apply vertex projection, if existent 
  dynamic_cast< Hmesh<N,NV>* >(hdl)->projectVertex( _segmentIndex, ppoint );

#endif

  return 0;
}

template < int N >
inline void Fullvertex < N >::sethdl(IndexProvider *phdl) {
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(IndexProvider::IM_Vertices);
}
inline void Edge::sethdl(IndexProvider *phdl) {
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(IndexProvider::IM_Edges);
}
inline Edge::~Edge() {
  assert(isfree());
  hdl->freeIndex(IndexProvider::IM_Edges,_idx);
}
template < int N, int NV >
inline void Element < N, NV >::sethdl(IndexProvider *phdl) 
{
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(IndexProvider::IM_Elements);
}
template < int N, int NV >
inline void Bndel < N, NV >::sethdl(IndexProvider *phdl) 
{
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(IndexProvider::IM_Bnd);
}
template < int N >
inline Vertex < N >::~Vertex() {
  if (hdl) {
    assert(_idx>=0);
    hdl->freeIndex(IndexProvider::IM_Vertices,_idx);
  }
}
template < int N, int NV >
inline Element < N, NV >::~Element() 
{
  for (int i=0;i<NV;++i) 
  {
    if (connect.edge[i]) 
    {
      connect.edge[i]->detach();
      if (connect.edge[i]->isfree())
      delete connect.edge[i];
    }
  }
  assert(hdl);
  if (hdl) {
    assert(_idx>=0);
    hdl->freeIndex(IndexProvider::IM_Elements,_idx);
  }
}
template < int N, int NV >
inline Bndel < N, NV >::~Bndel() {
  if (connect.edge) {
    connect.edge->detach();
    if (connect.edge->isfree())
      delete connect.edge;
  }
  if (hdl) {
    assert(_idx>=0);
    hdl->freeIndex(IndexProvider::IM_Bnd,_idx);
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
//   void Element::c::write(ostream &out) const
// #parameters:
// #description:
//   Ausschreiben der connect-Daten (privat)
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::c::write(ostream &out) const {
  for(int i = 0 ; i < NV ; i ++ ) 
  {
    out << (vtx[i] ? vtx[i]->Listagent < vertex_t > :: number() : -1 ) << "  " ;
//    out << bck[i] << "  " ;
//    out << nb[i] << "  :  ";
//    vtx[i]-> write(out);
  }
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::c::read(istream & in, Vertex ** v, const int l) 
// #parameters:
//   Einlesen der connect-Daten (privat)
// #description:
// #end(method)
// ***************************************************
template < int N, int NV >
inline int Element < N, NV >::c::read(istream & in, vertex_t ** v, const int l) {
  int c ;
  string line;
  while (in && line == "")
    getline( in, line );
  istringstream linein( line );
  int i;
  for (i = 0; ; ++i)
  {
    linein >> c ;
    if ( !linein ) 
    {
      if ( i >= 3 )
        break;
      cerr << "Too few element vertices read from file" << endl;
      abort();
    }
    if ( i >= NV )
    {
      cerr << "Too many element vertices read from file" << endl;
      abort();
    }
    if ( 0 > c || c >= l) 
    {
      cerr << "Wrong vertex number for element read from file" << endl;
      abort();
    }
    set((vertex_t *)v[c], i) ;
  }
  return i;
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
inline void Element < N, NV >::outernormal(int fce,double (&n)[ncoord]) const
{
  for (int i=0;i<ncoord;++i)
    n[i]= _outernormal[mod(fce)][i];
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

#if 0
// ***************************************************
// #begin(method)
// #method:
//   void Element::fromlocal(const double (&bary)[3],double (&v)[ncoord]) const
// #parameters:
//   \ const double (&)[3] | bary | baryzentrische Koordinaten
//   \ double (&)[ncoord]  | v    | globale Koordinaten
// #description:
//   Rechnet die baryzentrischen Koordinaten bez"uglich des Dreiecks
//   in globale (x,y)-Koordinaten um
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::fromlocal(const double (&bary)[3],double (&v)[ncoord]) const
{
  const double (&v0)[ncoord]=connect.vtx[0]->coord();
  const double (&v1)[ncoord]=connect.vtx[1]->coord();
  const double (&v2)[ncoord]=connect.vtx[2]->coord();

  for (int i=0;i<ncoord;i++)
     v[i]=bary[0]*v0[i]+
          bary[1]*v1[i]+
          bary[2]*v2[i];  
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::midpoint(int fce, double (&bary)[3]) const
// #parameters:
//   \ int            | fce  | Nummer der Kante
//   \ double (&)[3]  | bary | baryzentrische Koordinaten
// #description:
//   Berechnet den Mittelpunkt der Seite fce
//   in baryzentrischen Koordinaten
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::midpoint(int fce, double (&bary)[3]) const
{
  for (int i=0;i<3;i++)
    bary[i]=((i==fce%3) ? 0.0 : 0.5);
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::facepoint(int fce, double pos, double (&bary)[3]) const
// #parameters:
//   \ int           | fce  | Nummer der Kante
//   \ double        | pos  | Position auf der Kante
//   \ double (&)[3] | bary | baryzentrische Koordinaten
// #description:
//   Verl"auft die Seite fce von P0 nach P1, so
//   gilt bary = P0 + pos * (P1 - P0).
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::facepoint(int fce, double pos, double (&bary)[3]) const
{
  bary[fce%3] = 0.0;

  if (normaldir(fce) == 1)
  {
    bary[(fce+1)%3] = 1.0 - pos;
    bary[(fce+2)%3] = pos;
  }
  else
  {
    bary[(fce+1)%3] = pos;
    bary[(fce+2)%3] = 1.0 - pos;
  }
}
#endif

// ***************************************************
// #begin(method)
// #method:
//   void Element::facepoint(int fce, double pos, double (&bary)[3]) const
// #parameters:
//   \ int           | fce  | Nummer der Kante
//   \ double        | pos  | Position auf der Kante
//   \ double (&)[3] | bary | baryzentrische Koordinaten
// #description:
//   Verl"auft die Seite fce von P0 nach P1, so
//   gilt bary = P0 + pos * (P1 - P0).
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::facepoint(int fce, double pos, double (&p)[ncoord]) const
{
  const double (&vc0)[ncoord]=vertex(fce+1)->coord();
  const double (&vc1)[ncoord]=vertex(fce+2)->coord();
  double lam = (normaldir(fce)==1)?pos:1.-pos;
  for (int i=0;i<ncoord;++i)
    p[i] = (1.-lam)*vc0[i]+lam*vc1[i];
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::setrefine(int fce) 
// #parameters:
//   \ int | fce | Lokale Kantennr.
// #description:
//   Macht die Kante mit der lokalen Nr. fce zur Verfeinerungskante
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::setrefine(int fce) 
{
  assert( numfaces() == 3 && numvertices() == 3 );
  assert( 0<=fce );
  fce=mod(fce);
  vertex_t *tmp_v[3]={connect.vtx[0],connect.vtx[1],connect.vtx[2]};
  thinelement_t *tmp_n[3]={connect.nb[0],connect.nb[1],connect.nb[2]};
  Edge *tmp_e[3]={connect.edge[0],connect.edge[1],connect.edge[2]};
  vtx_btree_t* tmp_btree[3] = {connect.hvtx[0], connect.hvtx[1], connect.hvtx[2] };
  short int tmp_b[3]={connect.bck[0],connect.bck[1],connect.bck[2]};
  short int tmp_no[3]={connect.normdir[0],connect.normdir[1],connect.normdir[2]};
  double tmp_sln[3]={_sidelength[0],_sidelength[1],_sidelength[2]};
  double tmp_on[3][ncoord];
  for (int i=0;i<3;++i)
    for (int j=0;j<ncoord;++j)
      tmp_on[i][j] = _outernormal[i][j];
  // double tmp_onx[3]={_outernormal[0][0],_outernormal[1][0],_outernormal[2][0]};
  // double tmp_ony[3]={_outernormal[0][1],_outernormal[1][1],_outernormal[2][1]};
  for (int j=0;j<numvertices();j++)
  {  
    connect.vtx[j]=tmp_v[mod(fce+j)];  
    connect.bck[j]=tmp_b[mod(fce+j)];  
    connect.nb[j]=tmp_n[mod(fce+j)];  
    connect.edge[j]=tmp_e[mod(fce+j)];  
    connect.normdir[j]=tmp_no[mod(fce+j)];  
    connect.nb[j]->nbconnect(connect.bck[j],this,j);
    connect.hvtx[j] = tmp_btree[mod(fce+j)];

    _sidelength[j]     = tmp_sln[mod(fce+j)];
    for (int k=0;k<ncoord;++k)
      _outernormal[j][k] = tmp_on[mod(fce+j)][k];
  }
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::init()
// #parameters:
// #description:
//   Initialisierung der Instanzvariablen
// #end(method)
// ***************************************************
template < int N, int NV >
inline void Element < N, NV >::init()
{ 
  /* calculate area */
  if ( ncoord == 2 && NV == 3 )
  {
    const double (&vc0)[ncoord]=connect.vtx[0]->coord();
    const double (&vc1)[ncoord]=connect.vtx[1]->coord();
    const double (&vc2)[ncoord]=connect.vtx[2]->coord();

    /* calculate outer normal and sidelength */
    for (int i=0;i<numfaces();i++)
    {
      _outernormal[i][0]= (vertex(i+2)->coord()[1]-vertex(i+1)->coord()[1]);
      _outernormal[i][1]=-(vertex(i+2)->coord()[0]-vertex(i+1)->coord()[0]);

      _sidelength[i] = 0;
      for (int k=0;k<ncoord;++k)
        _sidelength[i] += _outernormal[i][k]*_outernormal[i][k];
      _sidelength[i] = sqrt( _sidelength[i] );
    }
    _area =  fabs(0.5 * (  (vc0[0]-vc1[0])*(vc0[1]+vc1[1])
                         + (vc1[0]-vc2[0])*(vc1[1]+vc2[1])
                         + (vc2[0]-vc0[0])*(vc2[1]+vc0[1])) );
  }
  else
  {
    double sides[NV][ncoord];
    /* calculate outer normal and sidelength^2 */
    for (int i=0;i<numfaces();i++)
    {
      _sidelength[i] = 0;
      for (int k=0;k<ncoord;++k)
      {
        sides[i][k]= (vertex(i+2)->coord()[k]-vertex(i+1)->coord()[k]);
        _sidelength[i] += sides[i][k]*sides[i][k];
      }
    }
    _area = 0;
    for (int i=0;i<numfaces();i++)
    {
      double scp_ds = 0;
      for (int k=0;k<ncoord;++k)
      {
        _outernormal[i][k] = sides[mod(i+2)][k] - sides[mod(i+1)][k];
        scp_ds += _outernormal[i][k] * sides[i][k];
      }
      double norm_n = 0;
      for (int k=0;k<ncoord;++k)
      {
        _outernormal[i][k] = _sidelength[i]*_outernormal[i][k] - scp_ds*sides[i][k];
        norm_n += _outernormal[i][k] * _outernormal[i][k];
      }
      norm_n = sqrt( norm_n );
      _sidelength[i] = sqrt( _sidelength[i] );
      double fac = _sidelength[i]/norm_n;
      for (int k=0;k<ncoord;++k)
        _outernormal[i][k] *= fac;
      _area += 1./(4.*fac);
    }
    _area /= (numvertices()==3)?double(numfaces()):double(numfaces()/2.);
  }

  assert(_area > 0.0);

  /* calculate minheight */

  double maxlen = _sidelength[0];
  for (int i=1;i<numfaces();i++)
    maxlen = ((_sidelength[i] > maxlen) ? _sidelength[i] : maxlen);
  assert(maxlen > 0.0);

  _minheight = 2.0 * _area / maxlen;
}

// ***************************************************
// #begin(method)
// #method:
//   int Element::setrefine()
// #parameters:
// #description:
//   Macht die l"angste Kante zur Verfeinerungskante und 
//   gibt 0 zur"uck falls eine umsortierung n"otig war
//   1 sonst
// #end(method)
// ***************************************************
template < int N, int NV >
inline int Element < N, NV >::setrefine()
{
  double maxkantenlen=-1.0,kantenlen;
  int maxkante=-1;
  
  for (int i=0;i<numfaces();i++)
  {
    kantenlen=sidelength(i);
    if (kantenlen > maxkantenlen)
    {
      maxkantenlen=kantenlen;
      maxkante=i;
    } 
  }
  if (maxkante>0)
    setrefine(maxkante);
  return maxkante==0;
}

/****************************************************
// #begin(method)
// #method:
//   int Element::setorientation()
// #parameters:
// #description:\
//   Sorgt da"ur das das Dreieck gegen den Uhrzeigersinn orientiert wird
//   (wichtig z.B. f"ur outernormal). Liefert 0 zur"uck falls die Orientierung
//   vorher im Uhrzeigersinn war, 1 falls keine Umsortierung der Punkte
//   n"otig war. L"a"st die Verfeinerungskante umber"uhrt 
// #end(method)
***************************************************/
template < int N, int NV >
inline int Element < N, NV >::setorientation()
{
  // required ??
  if (ncoord>2) return 1;
  assert( numfaces() == 3 && numvertices() == 3 ); // TRIANG
  double o;
  const double (&v0)[ncoord]=connect.vtx[0]->coord();
  const double (&v1)[ncoord]=connect.vtx[1]->coord();
  const double (&v2)[ncoord]=connect.vtx[2]->coord();
  o=(v1[0]-v0[0])*(v2[1]-v1[1])-(v1[1]-v0[1])*(v2[0]-v1[0]);
  if (fabs(o)<1e-10) {
    cerr << o << " " 
   << v0[0] << "," << v0[1] << " "
   << v1[0] << "," << v1[1] << " "
   << v2[0] << "," << v2[1] << endl;
  }
  assert(o);  // Entartet!
  if (o<0)    // Orientierung im Uhrzeigersinn!
  {
    cerr << "Orienting" << endl;
    vertex_t *tmpv=connect.vtx[1];
    thinelement_t *tmpn=connect.nb[1];
    Edge *tmpe=connect.edge[1];
    short int tmpb=connect.bck[1];
    short int tmpd=connect.normdir[1];
    double tmponx=_outernormal[1][0];
    double tmpony=_outernormal[1][1];
    double tmpsln=_sidelength[1];

    connect.vtx[1]=connect.vtx[2];
    connect.nb[1]=connect.nb[2];
    connect.edge[1]=connect.edge[2];
    connect.bck[1]=connect.bck[2];
    connect.normdir[1]=connect.normdir[2];
    connect.nb[1]->nbconnect(connect.bck[1],this,1);
    connect.vtx[2]=tmpv;
    connect.nb[2]=tmpn;
    connect.edge[2]=tmpe;
    connect.bck[2]=tmpb;
    connect.normdir[2]=tmpd;
    connect.nb[2]->nbconnect(connect.bck[2],this,2);

    _outernormal[1][0] = _outernormal[2][0];
    _outernormal[1][1] = _outernormal[2][1];
    _sidelength[1]     = _sidelength[2];
    _outernormal[2][0] = tmponx;
    _outernormal[2][1] = tmpony;
    _sidelength[2]     = tmpsln;
  }
  return (o>0);
}

/***************************************************
// #begin(method)
// #method:
//   void Element::addhvtx(Vertex* invtx, int fce)
// #parameters:
//   \ Vertex* | invtx | Knoten
//   \ int     | fce   | lokale Kantennummer
// #description:\
//   Fuegt den uebergebenen Knoten als haengenden Knoten
//   in das Empfaengerelement auf der Seite fce ein.
// #end(method)
***************************************************/
template < int N, int NV >
inline void Element < N, NV >::addhvtx(vertex_t *invtx, thinelement_t *lnb, thinelement_t *rnb,int fce)
{
  if( invtx ) {
    assert(rnb);
    assert(lnb);
    if( !connect.hvtx[fce] )
      connect.hvtx[fce] = new vtx_btree_t(connect.vtx[mod(fce+1)],lnb,rnb);
    connect.hvtx[fce]->insert(invtx,lnb,rnb);
  } else {
    assert(!connect.hvtx[fce]); 
  }
}

#endif
