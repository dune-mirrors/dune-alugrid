#ifndef ALU2D_GRID_IMP_CC
#define ALU2D_GRID_IMP_CC

//#include <time.h>
//#include <stdio.h>	//  sprintf()
//#include <stdlib.h>	//  NULL
//#include <assert.h>	
//#include <math.h> 

//#ifdef _ANSI_HEADER
//  using namespace std;
//  #include <iostream>	//  ostream
//  #include <fstream.h> 
//#else
//  #include <iostream.h>	//  ostream
//  #include <fstream.h> 
//#endif

//#include "grid.h"
//#include "triang.h"
#include "handle.h"
//#include "vmmap.h"
//#include "vtx_btree.h"

static const double EPS = 1e-8;
    

// ***************************************************
// #begin(method)
// #method:
//   Fullvertex::Fullvertex(double x, double y)
// #parameters:
//   \ double | x | x--Koordinate
//   \ double | y | y--Koordinate
// #description:
//   2D--Konstruktor
// #end(method)
// ***************************************************

inline Fullvertex::Fullvertex(double x, double y, int level) {

  vcoord[0] = x;
 
  vcoord[1] = y;

  _level = level;
}


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

inline Fullvertex::Fullvertex(double (&p)[ncoord],int level) {

  for(int i = 0 ; i < ncoord ; i ++) 

    vcoord[i] = p[i] ;
  _level = level;
}

// ***************************************************
// #begin(method)
// #method:
//   void Fullvertex::write(ofstream & out) const
// #parameters:
//   \ ofstream | &out | output--stream
// #description:
//   Ausgabe der Punktkoordinaten
// #end(method)
// ***************************************************

inline void Fullvertex::write(ofstream & out) const {

  for(int i = 0 ; i < ncoord ; i ++ ) out << vcoord[i] << "  " ;
   
  out << endl ;

}
// ***************************************************
// #begin(method)
// #method:
//   void Fullvertex::read(ifstream & in)
// #parameters:
//   \ ifstream | &in | input--stream
// #description:
//   Einlesen der Punktkoordinaten
// #end(method)
// ***************************************************

inline void Fullvertex::read(ifstream & in) {

  for(int i = 0 ; i < ncoord ; i ++) 

    in >> vcoord[i] ;

}
// ***************************************************
// #begin(method)
// #method:
//   void Edge::write(ofstream & out) const
// #parameters:
//   \ ofstream | &out | output--stream
// #description:
//   Ausgabe der Punktkoordinaten
// #end(method)
// ***************************************************

inline void Edge::write(ofstream & out) const {

  out << getIndex();
  out << endl ;

}
// ***************************************************
// #begin(method)
// #method:
//   void Edge::read(ifstream & in)
// #parameters:
//   \ ifstream | &in | input--stream
// #description:
//   Einlesen der Punktkoordinaten
// #end(method)
// ***************************************************

inline void Edge::read(ifstream & in) {

  in >> setIndex();

}

// ***************************************************
// #begin(method)
// #method:
//   void Bndel::mirror_opposite(double (&pret)[2]) const
// #parameters:
//   \ double (& )[2] | pret | return value
// #description:
//   Spiegelt den ``Punkt'' opposite(0) an sich
// #end(method)
// ***************************************************
inline void Bndel::mirror_opposite(double (&pret)[2]) const
{
  // mirrors the ``point'' opposite(0) at ``this'' 
  Element* el = (Element*)nbel(0);
  mirror(el->vertex(opposite(0))->coord(),pret);
}

// ***************************************************
// #begin(method)
// #method:
//   void Bndel::mirror(const double (&ppoint)[2], double (&pret)[2]) const
// #parameters:
//   \ const double (& )[2] | ppoint | point to be mirrored
//   \ double (& )[2]       | pret   | return value
// #description:
//   Spiegelt den ``Punkt'' ppoint an sich
// #end(method)
// ***************************************************
inline void Bndel::mirror(const double (&ppoint)[2],
                   double (&pret)[2]) const
{
  // If (a,b) is the vector from vertex(0) to vertex(1), then
  // (a,b) and (-b,a) define a local orthogonal coordinate
  // system with origin vertex(0). Let (c,d) denote the vector
  // from vertex(0) to ppoint. Then the local coordinates (l,m)
  // of the point ppoint mirrored at the line through vertex(0)
  // and vertex(1) are given by:
  // 
  // ( l )       1     ( a  b ) ( c )
  // (   ) = --------- (      ) (   )
  // ( m )   a^2 + b^2 (-b  a ) ( d )
  //
  // Thus the point ppoint mirrored at pel has the cartesian
  // coordinates (x,y) with 
  //
  // ( x )                 ( a )     (-b )
  // (   ) = vertex(0) + l (   ) - m (   )
  // ( y )                 ( b )     ( a ).

  double a = vertex(1)->coord()[0] - vertex(0)->coord()[0];
  double b = vertex(1)->coord()[1] - vertex(0)->coord()[1];
  double c = ppoint[0] - vertex(0)->coord()[0];
  double d = ppoint[1] - vertex(0)->coord()[1];
  double mult = 1.0 / (a * a + b * b);
  double l,m;

  l = mult * (  a * c + b * d);
  m = mult * ((-b)* c + a * d);

  pret[0] = vertex(0)->coord()[0] + l * a + m * b;
  pret[1] = vertex(0)->coord()[1] + l * b - m * a;
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
inline double Bndel::area() const 
{
  Element *tr = (Element*)(nbel(0));
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

inline int Bndel::get_splitpoint(double (&ppoint) [2])
{
  const double EPS = 1e-8;
  const int lmax_iter = 1000;
  const double ltol   = 1e-12;

  double ldiv,lx,ly,lcx,lcy,lvx=0.0,lvy=0.0,lt=0.0;
  int li=0,lret=0;

  lcx = 0.5*(connect.vtx[0]->coord()[0]+connect.vtx[1]->coord()[0]);
  lcy = 0.5*(connect.vtx[0]->coord()[1]+connect.vtx[1]->coord()[1]);

  if (0 && lf && lDf)
  {
    assert(fabs(  lf(connect.vtx[0]->coord()[0])
                - connect.vtx[0]->coord()[1]) <= 2.0 * ltol);
    assert(fabs(  lf(connect.vtx[1]->coord()[0])
		- connect.vtx[1]->coord()[1]) <= 2.0 * ltol);

    lvx = connect.vtx[0]->coord()[1] - connect.vtx[1]->coord()[1];
    lvy = connect.vtx[1]->coord()[0] - connect.vtx[0]->coord()[0];

    lx  = lcx;
    ly  = lcy;

    do
    {
      ldiv = lDf(lx) * lvx - lvy;
      if (fabs(ldiv) > EPS)
      {
        lt -= (lf(lx) - ly) / ldiv;

        lx = lcx + lt * lvx;
        ly = lcy + lt * lvy;
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
  }

  ppoint[0] = lcx + lt * lvx;
  ppoint[1] = lcy + lt * lvy;

  return 0;
}


inline void Fullvertex::sethdl(Hmesh_basic *phdl) {
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(Hmesh::IM_Vertices);
}
inline void Edge::sethdl(Hmesh_basic *phdl) {
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(Hmesh::IM_Edges);
}
inline Edge::~Edge() {
  assert(isfree());
  hdl->freeIndex(Hmesh::IM_Edges,_idx);
}
inline void Element::sethdl(Hmesh_basic *phdl) 
{
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(Hmesh::IM_Elements);
}
inline void Bndel::sethdl(Hmesh_basic *phdl) 
{
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(Hmesh::IM_Bnd);
}
inline Vertex::~Vertex() {
  if (hdl) {
    assert(_idx>=0);
    hdl->freeIndex(Hmesh::IM_Vertices,_idx);
  }
}
inline Element::~Element() 
{
  for (int i=0;i<3;++i) 
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
    hdl->freeIndex(Hmesh::IM_Elements,_idx);
  }
}
inline Bndel::~Bndel() {
  if (connect.edge) {
    connect.edge->detach();
    if (connect.edge->isfree())
      delete connect.edge;
  }
  if (hdl) {
    assert(_idx>=0);
    hdl->freeIndex(Hmesh::IM_Bnd,_idx);
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
inline void Element::edge_vtx(int e, Vertex *(&v) [2]) const {
  assert(0 <= e) ;
  v[0] = connect.vtx[(e+1)%connect.nv] ;
  v[1] = connect.vtx[(e+2)%connect.nv] ;
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
inline Fullvertex * Element::getVertex(int i) const {
  assert(0 <= i) ;
  return (Fullvertex*)(connect.vtx[i%connect.nv]) ; 
}

inline Vertex * Element::vertex(int i) const {
  assert(0 <= i) ;
  return connect.vtx[i%connect.nv] ; 
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
inline Thinelement * Element::neighbour(int fce) const {
  assert(0 <= fce) ;
  return connect.nb[fce%connect.nf] ;
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

inline int Element::opposite(int fce) const {
  assert(0 <= fce) ;
  return connect.bck [fce%connect.nf] ;
}
inline int Element::edge_idx(int fce) const {
  assert(0 <= fce) ;
  return connect.edge[fce%connect.nf]->getIndex() ;
}
inline Edge *Element::edge(int fce) const {
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
inline int Element::facevertex(int fce, int loc) const { 
  assert(0 <= fce) ;
  assert(0 <= loc) ;
  fce %= connect.nf ;
  loc %= connect.pv ;
  return (fce+loc+1)%connect.nv ; 
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
inline int Element::normaldir(int fce) const
{
  assert(0 <= fce) ;
  fce%=connect.nf;
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
inline void Element::nbconnect(int fce, Thinelement * n, int b) { 
  assert(0 <= fce) ;
  fce %= connect.nf ;

  connect.nb[fce] = n ; 
  connect.bck[fce] = b ;
  }
inline void Element::edgeconnect(int fce, Edge * n) { 
  assert(0 <= fce) ;
  fce %= connect.nf ;

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
inline void Element::setnormdir(int fce, int dir) {
  assert( 0 <= fce );
  fce %= connect.nf ;
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
inline Element::c::c() {
  for( int i = 0 ; i < nv ; i ++ ) { vtx[i] = 0; hvtx[i] = 0; }
  for( int j = 0 ; j < nf ; j ++ ) { nb[j] = 0 ; bck[j] = -1 ; normdir[j]=0 ; edge[j] = 0;}  
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
inline Element::c::~c() {
  for(int i = 0 ; i < nv ; i ++ ) {
    if(vtx[i]) vtx[i]->detach() ;
    if(hvtx[i]) delete hvtx[i];
  }
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::c::write(ofstream &out) const
// #parameters:
// #description:
//   Ausschreiben der connect-Daten (privat)
// #end(method)
// ***************************************************
inline void Element::c::write(ofstream &out) const {
  for(int i = 0 ; i < nv ; i ++ ) 
  {
    out << (vtx[i] ? vtx[i]->Listagent < Vertex > :: number() : -1 ) << "  " ;
//    out << bck[i] << "  " ;
//    out << nb[i] << "  :  ";
//    vtx[i]-> write(out);
  }
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::c::read(ifstream & in, Vertex ** v, const int l) 
// #parameters:
//   Einlesen der connect-Daten (privat)
// #description:
// #end(method)
// ***************************************************
inline void Element::c::read(ifstream & in, Vertex ** v, const int l) {
  int c ;
  for(int i = 0 ; i < nv ; i ++ ) {
    in >> c ;
    assert(-1 <= c && c < l) ;
    if(c != -1) set((Vertex *)v[c], i) ;
  }
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
inline void Element::outernormal(int fce,double (&n)[ncoord]) const
{
  n[0]= _outernormal[fce%connect.nf][0];
  n[1]= _outernormal[fce%connect.nf][1];
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
inline void Element::dirnormal(int fce,double (&n)[ncoord]) const
{
  outernormal(fce,n);
  for (int i=0;i<ncoord;i++)
    n[i]*=normaldir(fce);
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::tolocal(const double (&v)[ncoord], double (&bary)[3]) const
// #parameters:
//   \ double (&)[ncoord]  | v    | globale Koordinaten
//   \ double (&)[3]       | bary | baryzentrische Koordinaten
// #description:
//   Rechnet globale (x,y)-Koordinaten in baryzentrische
//   Koordinaten bez"uglich des Dreiecks um. Bezeichnen
//   v0,v1 und v2 die Koordinaten der Eckpunkte, so
//   gilt wegen l0 + l1 + l2 = 1:
//
//           l0 v0 + l1 v1 + l2 v2 = v
//
//     <==>  l0 (v0-v0) + l1 (v1-v0) + l2 (v2-v0) = v-v0
//
//     <==>  l1 (v1-v0) + l2 (v2-v0) = v-v0
//
//             (a  b) (l1)   (e)
//     <==>:   (    ) (  ) = ( )
//             (c  d) (l2)   (f)
//
//
//             (l1)     1   ( d  -b) (e)
//     <==>    (  ) = ----- (      ) ( )
//             (l2)   ad-bc (-c   a) (f)
// #end(method)
// ***************************************************
inline void Element::tolocal(const double (&v)[ncoord], double (&bary)[3]) const
{
  const double (&v0)[ncoord]=connect.vtx[0]->coord();
  const double (&v1)[ncoord]=connect.vtx[1]->coord();
  const double (&v2)[ncoord]=connect.vtx[2]->coord();
  double a   = v1[0] - v0[0];
  double b   = v2[0] - v0[0];
  double c   = v1[1] - v0[1];
  double d   = v2[1] - v0[1];
  double e   =  v[0] - v0[0];
  double f   =  v[1] - v0[1];
  double det = a * d - b * c;

  assert(ncoord == 2);
  assert(fabs(det) > EPS);

  bary[1] = (d * e - b * f) / det;
  bary[2] = (a * f - c * e) / det;
  bary[0] = 1.0 - bary[1] - bary[2];
}

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
inline void Element::fromlocal(const double (&bary)[3],double (&v)[ncoord]) const
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
inline void Element::midpoint(int fce, double (&bary)[3]) const
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
inline void Element::facepoint(int fce, double pos, double (&bary)[3]) const
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

// ***************************************************
// #begin(method)
// #method:
//   int Element::isecpoint(int fce, const double (&p)[ncoord],
//                          double (&bary)[3]) const
// #parameters:
//   \ int                     | fce  | Nummer der Kante
//   \ const double(&)[ncoord] | p    | Punktkoordinaten
//   \ double (&)[3]           | bary | baryzentrische Koordinaten
//                                      des Schnittpunktes
// #description:
//   Berechnet den Schnittpunkt der Verbindungsgeraden
//   des Elementschwerpunktes mit dem Punkt p
//   und der Kante fce. Existiert ein Schnittpunkt,
//   so wird 1 zur"uckgegeben, ansonsten 0. 
// #end(method)
// ***************************************************
inline int Element::isecpoint(int fce, const double (&pa)[ncoord],
                       double (&bary)[3]) const
{
  int ret = 1;
  double bary_cog[3] = {1.0/3.0, 1.0/3.0, 1.0/3.0};
  double v[ncoord],cog[2],pb[2],pc[2],pd[2],det,s;

  assert(ncoord == 2);

  fromlocal(bary_cog,cog);
  pb[0] = cog[0] - pa[0];
  pb[1] = cog[1] - pa[1];
  pc[0] = vertex(fce+1)->coord()[0];
  pc[1] = vertex(fce+1)->coord()[1];
  pd[0] = vertex(fce+2)->coord()[0] - pc[0];
  pd[1] = vertex(fce+2)->coord()[1] - pc[1];

  det = pb[1] * pd[0] - pb[0] * pd[1];

  if (fabs(det) <= EPS)
  {
    ret = 0;
  }
  else
  {
    s = (pd[0] * (pc[1] - pa[1]) - pd[1] * (pc[0] - pa[0])) / det;

    v[0] = pa[0] + s * pb[0];
    v[1] = pa[1] + s * pb[1];
    tolocal(v,bary);
  }

  return ret;
}

// ***************************************************
// #begin(method)
// #method:
//   int Element::isecpoint(int fce, const double (&p)[ncoord],
//                          double (&bary)[3]) const
// #parameters:
//   \ int                     | fce  | Nummer der Kante
//   \ const double(&)[ncoord] | p    | Punktkoordinaten
//   \ double (&)[ncoord]      | pret | Koordinaten
//                                      des Schnittpunktes
// #description:
//   Berechnet den Schnittpunkt der Verbindungsgeraden
//   des Elementschwerpunktes mit dem Punkt p
//   und der Kante fce. Existiert ein Schnittpunkt,
//   so wird 1 zur"uckgegeben, ansonsten 0. 
// #end(method)
// ***************************************************
inline int Element::isecpoint(int fce, const double (&pa)[ncoord],
                       double (&pret)[ncoord]) const
{
  int ret = 1;
  double bary_cog[3] = {1.0/3.0, 1.0/3.0, 1.0/3.0};
  double cog[2],pb[2],pc[2],pd[2],det,s;

  assert(ncoord == 2);

  fromlocal(bary_cog,cog);
  pb[0] = cog[0] - pa[0];
  pb[1] = cog[1] - pa[1];
  pc[0] = vertex(fce+1)->coord()[0];
  pc[1] = vertex(fce+1)->coord()[1];
  pd[0] = vertex(fce+2)->coord()[0] - pc[0];
  pd[1] = vertex(fce+2)->coord()[1] - pc[1];

  det = pb[1] * pd[0] - pb[0] * pd[1];

  if (fabs(det) <= EPS)
  {
    ret = 0;
  }
  else
  {
    s = (pd[0] * (pc[1] - pa[1]) - pd[1] * (pc[0] - pa[0])) / det;

    pret[0] = pa[0] + s * pb[0];
    pret[1] = pa[1] + s * pb[1];
  }

  return ret;
}

// ***************************************************
// #begin(method)
// #method:
//   int Element::inside(Vertex &v) const
// #parameters:
// #description:
//   Liefert 0 zur"uck falls v au"serhalb des Dreiecks,
//   1 falls innerhalb und -1 falls v auf einer Kante liegt
// #end(method)
// ***************************************************
inline int Element::inside(Vertex &v) const
{
  double n[ncoord];
  double wert,test=0.0;
  int ret=1;
  for (int i=0;i<connect.nf;i++)
  {
    outernormal(i,n);
    wert=   n[0]*(v.coord()[0]-vertex(i+1)->coord()[0])
          + n[1]*(v.coord()[1]-vertex(i+1)->coord()[1]);
    if (wert==0.0)
    {
      ret=-1;
      continue;
    }
    if (test==0.0)
      test=wert;
    else
      if (wert*test<0)
      {
        ret=0;
        break;
      }
  }
  return ret;  
}


// ***************************************************
// #begin(method)
// #method:
//   int Element::inside(const double (&)[2]) const
// #parameters:
//   \ const double (&)[2] | point | Punktkoordinaten
// #description:
//   Liefert 1 zur"uck, falls point innerhalb des
//   Dreiecks liegt, 0 sonst.
// #end(method)
// ***************************************************
inline int Element::inside(const double (&point)[2]) const
{
  int i,ret = 1;
  double bary[3];

  tolocal(point,bary);

  for (i=0;i<3;i++)
    if ((bary[i] < -EPS) || (bary[i] > 1.0 + EPS))
      ret = 0;

  return ret;
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
inline void Element::setrefine(int fce) 
{
  assert( 0<=fce );
  fce%=connect.nv;
  Vertex *tmp_v[3]={connect.vtx[0],connect.vtx[1],connect.vtx[2]};
  Thinelement *tmp_n[3]={connect.nb[0],connect.nb[1],connect.nb[2]};
  Edge *tmp_e[3]={connect.edge[0],connect.edge[1],connect.edge[2]};
  Vtx_btree* tmp_btree[3] = {connect.hvtx[0], connect.hvtx[1], connect.hvtx[2] };
  short int tmp_b[3]={connect.bck[0],connect.bck[1],connect.bck[2]};
  short int tmp_no[3]={connect.normdir[0],connect.normdir[1],connect.normdir[2]};
  double tmp_sln[3]={_sidelength[0],_sidelength[1],_sidelength[2]};
  double tmp_onx[3]={_outernormal[0][0],_outernormal[1][0],_outernormal[2][0]};
  double tmp_ony[3]={_outernormal[0][1],_outernormal[1][1],_outernormal[2][1]};
  for (int j=0;j<connect.nv;j++)
  {  
    connect.vtx[j]=tmp_v[(fce+j)%connect.nv];  
    connect.bck[j]=tmp_b[(fce+j)%connect.nv];  
    connect.nb[j]=tmp_n[(fce+j)%connect.nv];  
    connect.edge[j]=tmp_e[(fce+j)%connect.nv];  
    connect.normdir[j]=tmp_no[(fce+j)%connect.nv];  
    connect.nb[j]->nbconnect(connect.bck[j],this,j);
    connect.hvtx[j] = tmp_btree[(fce+j)%connect.nv];

    _sidelength[j]     = tmp_sln[(fce+j)%connect.nv];
    _outernormal[j][0] = tmp_onx[(fce+j)%connect.nv];
    _outernormal[j][1] = tmp_ony[(fce+j)%connect.nv];
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
inline void Element::init()
{
  /* calculate area */

  const double (&vc0)[ncoord]=connect.vtx[0]->coord();
  const double (&vc1)[ncoord]=connect.vtx[1]->coord();
  const double (&vc2)[ncoord]=connect.vtx[2]->coord();

  _area =  fabs(0.5 * (  (vc0[0]-vc1[0])*(vc0[1]+vc1[1])
                       + (vc1[0]-vc2[0])*(vc1[1]+vc2[1])
                       + (vc2[0]-vc0[0])*(vc2[1]+vc0[1])) );

  assert(_area > 0.0);

  /* calculate outer normal and sidelength */

  int i;

  for (i=0;i<3;i++)
  {
    _outernormal[i][0]= (vertex(i+2)->coord()[1]-vertex(i+1)->coord()[1]);
    _outernormal[i][1]=-(vertex(i+2)->coord()[0]-vertex(i+1)->coord()[0]);

    _sidelength[i] = sqrt(  _outernormal[i][0]*_outernormal[i][0]
                          + _outernormal[i][1]*_outernormal[i][1]);
  }

  /* calculate minheight */

  double maxlen = _sidelength[0];
  for (i=1;i<3;i++)
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
inline int Element::setrefine()
{
  double maxkantenlen=-1.0,kantenlen;
  int maxkante=-1;
  
  for (int i=0;i<connect.nf;i++)
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
inline int Element::setorientation()
{
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
    Vertex *tmpv=connect.vtx[1];
    Thinelement *tmpn=connect.nb[1];
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
inline void Element::addhvtx(Vertex* invtx, Thinelement *lnb,Thinelement *rnb,int fce)
{
  if( invtx ) {
    assert(rnb);
    assert(lnb);
    if( !connect.hvtx[fce] )
      connect.hvtx[fce] = new Vtx_btree(connect.vtx[(fce+1)%connect.nf],lnb,rnb);
    connect.hvtx[fce]->insert(invtx,lnb,rnb);
  } else {
    assert(!connect.hvtx[fce]); 
  }
}

#endif
