extern char ** g_argv;
extern int g_argc;

// ***************************************************
// #begin(header)
// #filename:
//   triang.cc
// #description:
//   Implementierung der Methoden auf Element und Bndel_triang
// #methodnames:
//   Element::Element
//   Element::edge
//   Element::vertex
//   Element::neighbour
//   Element::opposite
//   Element::facevertex
//   Element::normaldir
//   Element::write
//   Element::read
//   Element::nbconnect
//   Element::setnormdir
//   Element::c::c
//   Element::c::~c
//   Element::c::write
//   Element::c::read
//   Element::outernormal
//   Element::dirnormal
//   Element::tolocal
//   Element::fromlocal
//   Element::midpoint
//   Element::facepoint
//   Element::isecpoint
//   Element::isecpoint
//   Element::inside
//   Element::setrefine
//   Element::setrefine
//   Element::setorientation
//   Element::split
//   Element::docoarsen
//   Element::draw
//   Element::c::check
//   Element::check
//   Bndel_triang::Bndel_triang
//   Bndel::facevertex
//   Bndel::edge
//   Bndel::vertex
//   Bndel::write
//   Bndel::read
//   Bndel::nbconnect
//   Bndel::c::c
//   Bndel::c::~c
//   Bndel::c::write
//   Bndel::c::read
//   Bndel_triang::split
//   Bndel_triang::docoarsen
//   Bndel::draw
//   Bndel_periodic::split
//   Bndel_periodic::docoarsen
//   Bndel_slip::flux
//   Bndel_inflow::data
//   Bndel_inflow::reconstr_data
//   Bndel_outflow::reconstr_data
//   Bndel_periodic::reconstr_data
//   Bndel_absorbing::reconstr_data
// #end(header)
// ***************************************************

#include "grid.h"
#include "triang.h"
#include "handle.h"
#include "vmmap.h"
#include "vtx_btree.h"


// static const double EPS = 1e-8;

// int Element::_nconfDeg = 1;

// ***************************************************
// #begin(method)
// #method:
//   Triang::Triang()
// #parameters:
// #description:
//   Konstruktor f"ur ,,leeres'' Dreieck
// #end(method)
// ***************************************************
Triang::Triang() {
  Thinelement::nedges = -1 ;
  Thinelement::nfaces = connect.nf ;
  Thinelement::nvertices = connect.nv ;
}

// ***************************************************
// #begin(method)
// #method:
//   Triang::Triang(Vertex * v0,Vertex * v1, Vertex * v2)
// #parameters:
// #description:
//   Konstruktor f"ur Dreiecke mit den Ecken v0,v1 und v2
// #end(method)
// ***************************************************
Triang::Triang(Vertex * v0,Vertex * v1, Vertex * v2) {
  connect.set( v0, 0) ; connect.set( v1, 1) ; connect.set( v2, 2) ;
  Thinelement::nedges = -1 ;
  Thinelement::nfaces = connect.nf ;
  Thinelement::nvertices = connect.nv ;

  init();
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

/*
  void Element::edge_vtx(int e, Vertex *(&v) [2]) const {
  assert(0 <= e) ;
  v[0] = connect.vtx[(e+1)%connect.nv] ;
  v[1] = connect.vtx[(e+2)%connect.nv] ;
  }
*/

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
/*
  Vertex * Element::vertex(int i) const {
  assert(0 <= i) ;
  return connect.vtx[i%connect.nv] ; 
  }
*/

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
/*
  Thinelement * Element::neighbour(int fce) const {
  assert(0 <= fce) ;
  return connect.nb[fce%connect.nf] ;
  }
*/

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
/*
  int Element::opposite(int fce) const {
  assert(0 <= fce) ;
  return connect.bck [fce%connect.nf] ;
  }
  int Element::edge_idx(int fce) const {
  assert(0 <= fce) ;
  return connect.edge[fce%connect.nf]->getIndex() ;
  }
  Edge *Element::edge(int fce) const {
  assert(0 <= fce) ;
  return connect.edge[fce];
  }
*/

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
/*
  int Element::facevertex(int fce, int loc) const { 
  assert(0 <= fce) ;
  assert(0 <= loc) ;
  fce %= connect.nf ;
  loc %= connect.pv ;
  return (fce+loc+1)%connect.nv ; 
  }
*/
/*
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
//   multipliziert mit dem R"uckgabewert dieser Funktion. \\
//   Auf dem Nachbardreieck an diese Kante ergibt diese Funktion immer 
//   den entsprechend anderen Wert, au"ser: \\
//   1.) Bei einer nichtkonfornen Verfeinerung zeigt die gerichtete Normale
//   an Grenzen immer von den kleinen in das gro"se Dreieck \\
//   2.) Am Rand zeigt die gerichtet Normale immer nach aussnn \\
//   Dieser kann auch benutzt werden, um in der Nummerik den Flu"s "uber die
//   Kanten gerichtet zu berechnen, d.h. man berechnet ihn nur f"ur Kanten
//   die eine 1 als R"uckgabewert haben.
// #end(method)
// ***************************************************
*/
/*
  int Element::normaldir(int fce) const
  {
  assert(0 <= fce) ;
  fce%=connect.nf;
  return connect.normdir[fce] ;
  }
*/
// ***************************************************
// #begin(method)
// #method:
//   void Element::write(ofstream & out) const
// #parameters:
// #description:
//   Rausschreiben des Dreicks (unvollst"andig)
// #end(method)
// ***************************************************
void Triang::write(ofstream & out) const {
  connect.write(out) ;
  //  data.write(out) ;
  out << endl ;
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::read(ifstream & in, Vertex ** look, const int len)
// #parameters:
//   \ ifstream& | io    | Eingabestrom
//   \ Vertex**  | look  | Liste von Vertexpointern
//   \ const int | len   | Gr"o"se der Liste look
// #description:
//   Zum Einlesen eines Dreiecks. Es werden 3 Integerwerte eingelesen
//   und die entsprechenden Vetex* aus der Liste look als Eckpunkte
//   des Dreiecks gew"ahlt.
// #end(method)
// ***************************************************
void Triang::read(ifstream & in, Vertex ** look, const int len) {
  Hier < Element > :: read(in) ;
  connect.read(in, look, len) ;
  init() ;
}

// ***************************************************
// #begin(method)
// #method:
//   bool Triang::confLevelExceeded() const
// #parameters:
// #description:
//   Prueft, ob _nconfDeg, d.h. der maximale Grad der Nichtkonformitaet
//   fuer das Empfaengerelement ueberschritten ist.
// #end(method)
// ***************************************************
bool Triang::confLevelExceeded(int nconfDeg) const
{
  bool result = false;

  if( nconfDeg >= 0 ) {
    for( int i=0 ; i<connect.nf ; i++ )
      {
	Bndel_triang *nb = nbbnd(i);

	if (nb)
	  {
	    if (nb->type()==Bndel::periodic)
	      {
		Triang* tr = ((Bndel_periodic*)nb)->periodic_nb->nbel(0);
		int dl = (tr->hashvtx(i) ? tr->connect.hvtx[i]->deepestLevel() : 0);
		if( dl > nconfDeg )
		  // if (((Bndel_periodic*)nb)->periodic_nb->deepestLevel() > nconfDeg)
		  {
		    result = true;
		    break;
		  }
	      }
	  }
	else
	  {
	    int dl = (hashvtx(i) ? connect.hvtx[i]->deepestLevel() : 0);
	    if( dl > nconfDeg )
	      {
		result = true;
		break;
	      }
	  }
      }
  }
  return result;
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
/*
  void Element::nbconnect(int fce, Thinelement * n, int b) { 
  assert(0 <= fce) ;
  fce %= connect.nf ;

  connect.nb[fce] = n ; 
  connect.bck[fce] = b ;
  }
  void Element::edgeconnect(int fce, Edge * n) { 
  assert(0 <= fce) ;
  fce %= connect.nf ;

  connect.edge[fce] = n ; 
  n->attach();
  }
*/

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
/*void Element::setnormdir(int fce, int dir) {
  assert( 0 <= fce );
  fce %= connect.nf ;
  assert( dir == 1 || dir == -1);
  connect.normdir[fce] = dir;
  }
*/
// ***************************************************
// #begin(method)
// #method:
//   Element::c::c()
// #parameters:
// #description:
//   Konstruktor f"ur connect Daten (privat)
// #end(method)
// ***************************************************
/*
  Element::c::c() {
  for( int i = 0 ; i < nv ; i ++ ) { vtx[i] = 0; hvtx[i] = 0; }
  for( int j = 0 ; j < nf ; j ++ ) { nb[j] = 0 ; bck[j] = -1 ; normdir[j]=0 ; edge[j] = 0;}  
  }
*/

// ***************************************************
// #begin(method)
// #method:
//   Element::c::~c()
// #parameters:
// #description:
//   Destruktor f"ur connect Daten (privat)
// #end(method)
// ***************************************************
/*
  Element::c::~c() {
  for(int i = 0 ; i < nv ; i ++ ) {
  if(vtx[i]) vtx[i]->detach() ;
  if(hvtx[i]) delete hvtx[i];
  }
  }
*/

// ***************************************************
// #begin(method)
// #method:
//   void Element::c::write(ofstream &out) const
// #parameters:
// #description:
//   Ausschreiben der connect-Daten (privat)
// #end(method)
// ***************************************************
/*
  void Element::c::write(ofstream &out) const {
  for(int i = 0 ; i < nv ; i ++ ) 
  {
  out << (vtx[i] ? vtx[i]->Listagent < Vertex > :: number() : -1 ) << "  " ;
  //    out << bck[i] << "  " ;
  //    out << nb[i] << "  :  ";
  //    vtx[i]-> write(out);
  }
  }
*/

// ***************************************************
// #begin(method)
// #method:
//   void Element::c::read(ifstream & in, Vertex ** v, const int l) 
// #parameters:
//   Einlesen der connect-Daten (privat)
// #description:
// #end(method)
// ***************************************************
/*
  void Element::c::read(ifstream & in, Vertex ** v, const int l) {
  int c ;
  for(int i = 0 ; i < nv ; i ++ ) {
  in >> c ;
  assert(-1 <= c && c < l) ;
  if(c != -1) set((Vertex *)v[c], i) ;
  }
  }
*/

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
/*
  void Element::outernormal(int fce,double (&n)[ncoord]) const
  {
  n[0]= _outernormal[fce%connect.nf][0];
  n[1]= _outernormal[fce%connect.nf][1];
  }
*/
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
/*
  void Element::dirnormal(int fce,double (&n)[ncoord]) const
  {
  outernormal(fce,n);
  for (int i=0;i<ncoord;i++)
  n[i]*=normaldir(fce);
  }
*/
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
/*
  void Element::tolocal(const double (&v)[ncoord], double (&bary)[3]) const
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
*/
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
/*
  void Element::fromlocal(const double (&bary)[3],double (&v)[ncoord]) const
  {
  const double (&v0)[ncoord]=connect.vtx[0]->coord();
  const double (&v1)[ncoord]=connect.vtx[1]->coord();
  const double (&v2)[ncoord]=connect.vtx[2]->coord();

  for (int i=0;i<ncoord;i++)
  v[i]=bary[0]*v0[i]+
  bary[1]*v1[i]+
  bary[2]*v2[i];  
  }
*/
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
/*
  void Element::midpoint(int fce, double (&bary)[3]) const
  {
  for (int i=0;i<3;i++)
  bary[i]=((i==fce%3) ? 0.0 : 0.5);
  }
*/
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
/*
  void Element::facepoint(int fce, double pos, double (&bary)[3]) const
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
*/

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
/*
  int Element::isecpoint(int fce, const double (&pa)[ncoord],
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
*/
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
/*
  int Element::isecpoint(int fce, const double (&pa)[ncoord],
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
*/
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
/*
  int Element::inside(Vertex &v) const
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
*/

// ***************************************************
// #begin(method)
// #method:
//   int Element::getAllNb(Vtx_btree* vtx, vector<Thinelement *> vec) const
// #parameters: 
// #description:
//   will return all neighbours associated with the element in a vector
// #end(method)
// ***************************************************
/*
  void Element::getAllNb(Vtx_btree::Node* node, stack<Thinelement *> vec) 
  {
  if(!node->leftNode())
  vec.push(node->leftElement());
  else {
  getAllNb(node->leftNode(), vec);
  }
  if(!node->rightNode())
  vec.push(node->rightElement());
  else {
  getAllNb(node->rightNode(), vec);
  }    
  //if(!vtx->right())
  //  vec.push_back(vtx->getrnb());
  //else {    
  //  getAllNb(vtx->right(), vec);        
  //}
  }
*/
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
/*
  int Element::inside(const double (&point)[2]) const
  {
  int i,ret = 1;
  double bary[3];

  tolocal(point,bary);

  for (i=0;i<3;i++)
  if ((bary[i] < -EPS) || (bary[i] > 1.0 + EPS))
  ret = 0;

  return ret;
  }
*/
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
/*
  void Element::setrefine(int fce) 
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
*/
// ***************************************************
// #begin(method)
// #method:
//   void Element::init()
// #parameters:
// #description:
//   Initialisierung der Instanzvariablen
// #end(method)
// ***************************************************
/*
  void Element::init()
  {
  // calculate area 

  const double (&vc0)[ncoord]=connect.vtx[0]->coord();
  const double (&vc1)[ncoord]=connect.vtx[1]->coord();
  const double (&vc2)[ncoord]=connect.vtx[2]->coord();

  _area =  fabs(0.5 * (  (vc0[0]-vc1[0])*(vc0[1]+vc1[1])
  + (vc1[0]-vc2[0])*(vc1[1]+vc2[1])
  + (vc2[0]-vc0[0])*(vc2[1]+vc0[1])) );

  assert(_area > 0.0);

  //calculate outer normal and sidelength

  int i;

  for (i=0;i<3;i++)
  {
  _outernormal[i][0]= (vertex(i+2)->coord()[1]-vertex(i+1)->coord()[1]);
  _outernormal[i][1]=-(vertex(i+2)->coord()[0]-vertex(i+1)->coord()[0]);

  _sidelength[i] = sqrt(  _outernormal[i][0]*_outernormal[i][0]
  + _outernormal[i][1]*_outernormal[i][1]);
  }

  // calculate minheight

  double maxlen = _sidelength[0];
  for (i=1;i<3;i++)
  maxlen = ((_sidelength[i] > maxlen) ? _sidelength[i] : maxlen);
  assert(maxlen > 0.0);

  _minheight = 2.0 * _area / maxlen;

  }
*/

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
/*
  int Element::setrefine()
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
*/

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
/*
  int Element::setorientation()
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
*/

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
/*
  void Element::addhvtx(Vertex* invtx, Thinelement *lnb,Thinelement *rnb,int fce)
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
*/

/***************************************************
 // #begin(method)
 // #method:
 //   void Triang::newNeighbour(Triang* nb, int fce, int nbfce, splitrule_t sr, bool setnormal)
 // #parameters:
 //   \ Triang*     | nb        | neues Nachbardreieck
 //   \ int         | fce       | lokale Nummer der Kante des Empfaengerelementes,
 //                               an welche nb grenzt
 //   \ int         | nbfce     | lokale Nummer der Kante von nb, an welche das
 //                               Empfaengerelement grenzt
 //   \ splitrule_t | sr        | METHODE FUNKTIONIERT NUR BEI BISEKTION,
 //                               deshalb splitrule als Parameter
 //   \ bool        | setnormal | Legt fest, ob die Normalenrichtung des Empfaengers
 //                               und dessen Kinder entsprechend gesetzt werden
 // #description:\
 //   Legt nach Verfeinerung eines Elementes die Nachbarschaften
 //   ueber die nicht verfeinerten Kanten rekursiv fest.
 // #end(method)
 ***************************************************/

void Triang::newNeighbour(Triang* nb, int fce, int nbfce,
                          splitrule_t sr, bool setnormal)
{
  assert(sr == triang_conf2);
  assert(fce >= 0);
  assert(nbfce >= 0);
  fce %= connect.nf;
  nbfce %= connect.nf;

  nbconnect(fce, nb, nbfce);
  if( setnormal )
    setnormdir(fce, 1);
  if( !leaf() ) {
    if( fce == 0 ) {
      // connect t1, t2
      ((Triang*)down())->newNeighbour(nb, 1, nbfce, sr);
      ((Triang*)down()->next())->newNeighbour(nb, 2, nbfce, sr);
    } else if( fce == 1 ) {
      // connect t2
      ((Triang*)(down()->next()))->newNeighbour(nb, 0, nbfce, sr);
    } else {
      // connect t1
      ((Triang*)down())->newNeighbour(nb, 0, nbfce, sr);
    }
  }
}

extern Hmesh *mesh;
/***************************************************
 // #begin(method)
 // #method:
 //   int Triang::split(void * (&e)[nparts], Listagency < Vertex > * agnc,
 //                     Multivertexadapter & mva, splitrule_t)
 // #parameters:
 //   \ void* (&e)[nparts]  | e    | Array, welches die durch Verfeinerung neu
 //                                  entstehenden Elemente enthaelt
 //   \ Listagency<Vertex>* | agnc | Liste, die neu entstehende Knoten aufnimmt
 //   \ Multivertexadapter& | mva  | Multivertexadapter (nicht benutzt)
 //   \ splitrule_t         | sr   | splitrule
 // #description:\
 //   (Nicht konformes) Verfeinern eines Elementes durch Bisektion.
 // #end(method)
 ***************************************************/

int Triang::split(void * (&e)[nparts], Listagency < Vertex > * agnc,
                  Multivertexadapter & mva, 
		  nconf_vtx_t *ncv,
		  splitrule_t sr,
		  int nconfDeg,Refco::tag_t default_ref,
		  Prolong_basic *pro_el)
{
  int ret = 0;

  assert(splitrule() == unsplit);

  switch (sr)
    {
    case triang_conf2:
      {
        Triang* t1;
        Triang* t2;
	Edge *newedge[3];
	newedge[2]=new Edge(hdl);

        mysplit = triang_conf2;

        if( connect.nb[0]->thinis(bndel_like) ) {
          // Verfeinerungsnachbar ist Randelement
          Vertex* nv;
          double p[2];

          // Erzeugen des neuen Knotens und der neuen Elemente
          ((Bndel*)connect.nb[0])->get_splitpoint(p);
          agnc->insert(nv = new Fullvertex(p,level()));
          t1 = new Triang(nv, connect.vtx[0], connect.vtx[1]);
          t2 = new Triang(nv, connect.vtx[2], connect.vtx[0]);
	  t1->sethdl(hdl);
	  t2->sethdl(hdl);
          t1->connect.hvtx[0] = connect.hvtx[2];
          t2->connect.hvtx[0] = connect.hvtx[1];
          connect.hvtx[1] = connect.hvtx[2] = NULL;
	  newedge[0]=new Edge(hdl);
	  newedge[1]=new Edge(hdl);
	  
          // Nachbarschaftsinfo der Nachbarn "uber die Kanten
	  // 1 und 2 neu setzen (aber nur, falls die Nachbarn nicht
          // gr"ober sind als Empfaengerelement)
          if( neighbour(1)->thinis(element_like) ) {
            if( !(((Element*)neighbour(1))->hashvtx(opposite(1))) )
              ((Triang*)neighbour(1))->newNeighbour(t2, opposite(1), 0,
                                                    triang_conf2);
	    else
	      nbel(1)->connect.hvtx[opposite(1)]->head->rnb = t2;
          } else {
            neighbour(1)->nbconnect(0, t2, 0);
          }

          if( neighbour(2)->thinis(element_like) ) {
            if( !(((Element*)neighbour(2))->hashvtx(opposite(2))) )
              ((Triang*)neighbour(2))->newNeighbour(t1, opposite(2),
                                                    0, triang_conf2);
	    else
	      nbel(2)->connect.hvtx[opposite(2)]->head->rnb = t1;
          } else {
            neighbour(2)->nbconnect(0, t1, 0);
          }
	
          // Nachbarschaft der neu entstandenen Elemente t1 und t2 regeln
          t1->nbconnect(0, connect.nb[2], connect.bck[2]);
          t1->nbconnect(2, t2, 1);
          t2->nbconnect(0, connect.nb[1], connect.bck[1]);
          t2->nbconnect(1, t1, 2);

          // Richtung der Normalen
          t1->setnormdir(0, connect.normdir[2]);
          t1->setnormdir(1, connect.normdir[0]);
          t1->setnormdir(2, 1);
          t2->setnormdir(0, connect.normdir[1]);
          t2->setnormdir(1, -1);
          t2->setnormdir(2, connect.normdir[0]);
	  t1->edgeconnect(0,connect.edge[2]);
	  t1->edgeconnect(1,newedge[0]);
	  t1->edgeconnect(2,newedge[2]);
	  t2->edgeconnect(0,connect.edge[1]);
	  t2->edgeconnect(1,newedge[2]);
	  t2->edgeconnect(2,newedge[1]);

          // Nachbarelement (=Randelement) verfeinern
          ncv = new nconf_vtx_t(nv, t1, t2);
          Hier<Bndel>* n = (Hier<Bndel>*)connect.nb[0];
          n->refine_leaf(agnc, &mva,ncv,nconfDeg,default_ref,pro_el);
        } else {
          // Verfeinerungsnachbar ist "normales" Element
          Vertex* nv;
          double b[3],p[2];
          bool usehvtx = false;

          // Falls ein haengender Knoten existiert, jenen benutzen;
          // falls nicht, neuen Knoten erzeugen und jenen als haengenden in
          // entsprechenden Nachbarn einfuegen.
          if( connect.hvtx[0] ) {
            nv = connect.hvtx[0]->getHead();
            usehvtx = true;
	    Hier<Element> *tr;
	    tr = ((Hier<Element>*)neighbour(0))->down()->next();
	    if (tr->leaf()) 
	      newedge[0]=connect.hvtx[0]->getrnb()->edge(2);
	    else
	      newedge[0]=connect.hvtx[0]->getrnb()->edge(0);
	    tr = ((Hier<Element>*)neighbour(0))->down();
	    if (tr->leaf()) 
	      newedge[1]=connect.hvtx[0]->getlnb()->edge(1);	  
	    else
	      newedge[1]=connect.hvtx[0]->getlnb()->edge(0);
          } else {
            midpoint(0, b);
            fromlocal(b,p);
            agnc->insert(nv = new Fullvertex(p,level()));
	    newedge[0]=new Edge(hdl);
	    newedge[1]=new Edge(hdl);
          }

          // Neue Elemente erzeugen
          t1 = new Triang(nv, connect.vtx[0], connect.vtx[1]);
          t2 = new Triang(nv, connect.vtx[2], connect.vtx[0]);
          // Nachbarschaft von t1...
          t1->nbconnect(0, neighbour(2), opposite(2));
          if( usehvtx ) {
            Triang* tr = (Triang*)neighbour(0);
            tr = (Triang*)(tr->down()->next());
            if( tr->leaf() ) {
              t1->nbconnect(1, tr, 2);
	      assert(connect.hvtx[0]->getrnb()==tr);
            } else {
              tr = (Triang*)(tr->down());
              t1->nbconnect(1, tr, 0);
	      assert(connect.hvtx[0]->getrnb()==tr);
            }
          } else {
            t1->nbconnect(1, neighbour(0), opposite(0));
          }
          t1->nbconnect(2, t2, 1);

          // ... und t2 setzen
          t2->nbconnect(0, neighbour(1), opposite(1));
          t2->nbconnect(1, t1, 2);
          if( usehvtx ) {
            Triang* tr = (Triang*)neighbour(0);
            tr = (Triang*)(tr->down());
            if( tr->leaf() ) {
              t2->nbconnect(2, tr, 1);
	      assert(connect.hvtx[0]->getlnb()==tr);
            } else {
              tr = (Triang*)(tr->down()->next());
              t2->nbconnect(2, tr, 0);
	      assert(connect.hvtx[0]->getlnb()==tr);
            }
          } else {
            t2->nbconnect(2, neighbour(0), opposite(0));
          }
          if( usehvtx == true ) {
            connect.hvtx[0]->splitTree(t1->connect.hvtx[1],
                                       t2->connect.hvtx[2]);
          }
          else {
            ((Element*)connect.nb[0])->addhvtx(nv, t1,t2, connect.bck[0]);
          }
          if (connect.hvtx[0]) {
            delete connect.hvtx[0];
            connect.hvtx[0] = 0;
          }
          t1->connect.hvtx[0] = connect.hvtx[2];
          t2->connect.hvtx[0] = connect.hvtx[1];
          connect.hvtx[0] = connect.hvtx[1] = connect.hvtx[2] = 0;


          // Nachbarschaft der Nachbarn (neu) setzen
          // -- Nachbarschaft ueber Kante 0
          if( usehvtx ) {
            Triang* tr = (Triang*)neighbour(0);
            tr = (Triang*)(tr->down());
            tr->newNeighbour(t2, 1, 2, triang_conf2);
            tr = (Triang*)(tr->next());
            tr->newNeighbour(t1, 2, 1, triang_conf2);
          }
          // -- Nachbarschaft ueber Kante 1
          if( neighbour(1)->thinis(element_like) ) {
            if( !(((Triang*)neighbour(1))->hashvtx(opposite(1))) ) {
              ((Triang*)neighbour(1))->newNeighbour(t2, opposite(1), 0,
                                                    triang_conf2);
            } else {
	      nbel(1)->connect.hvtx[opposite(1)]->head->lnb = t2;
	    }
          } else {
            neighbour(1)->nbconnect(0, t2, 0);
          }
          // -- Nachbarschaft ueber Kante 2
          if( neighbour(2)->thinis(element_like) ) {
            if( !(((Triang*)neighbour(2))->hashvtx(opposite(2))) ) {
              ((Triang*)neighbour(2))->newNeighbour(t1, opposite(2), 0,
                                                    triang_conf2);
	    } else {
	      nbel(2)->connect.hvtx[opposite(2)]->head->rnb = t1;
	    }
          } else {
            neighbour(2)->nbconnect(0, t1, 0);
          }

          // Normalenrichtung
          t1->setnormdir(2, 1);
          t2->setnormdir(1, -1);
          if( usehvtx ) {
            t1->setnormdir(1, -1);
            t2->setnormdir(2, -1);
          } else {
            t1->setnormdir(1, 1);
            t2->setnormdir(2, 1);
            ((Element*)neighbour(0))->setnormdir(opposite(0), -1);
            ((Element*)this)->setnormdir(0, 1);
          }
          t1->setnormdir(0, normaldir(2));
          t2->setnormdir(0, normaldir(1));
	  t1->sethdl(hdl);
	  t2->sethdl(hdl);
	  t1->edgeconnect(0,connect.edge[2]);
	  t1->edgeconnect(1,newedge[0]);
	  t1->edgeconnect(2,newedge[2]);
	  t2->edgeconnect(0,connect.edge[1]);
	  t2->edgeconnect(1,newedge[2]);
	  t2->edgeconnect(2,newedge[1]);
        }
	if (this->is(Refco::ref_2)) {
	  t1->mark(Refco::ref_1);
	  t2->mark(Refco::ref_1);
	}
        e[0] = t1;
        e[1] = t2;
	/*
	  if (pro_el)
	  pro_el->operator()(this, (Element**)e, 2);
	*/
        ret = 2;
      }
      break;
    case triang_quarter:
      {
	// Every triangle is split into four triangles #0, #1, #2,
	// and #3. Three new vertices (0), (1), and (2) are created.
	// If the edge 1->2 is the longest edge and the orientation
	// is correct, then the same holds for the four children.
	// Thus no additional calls to setrefine() or setorientation()
	// are required. Note that this splitrule is only intended for
	// the global refinements performed at the beginning of a
	// simulation. Coarsening of triangles created using
	// ``triang_quarter'' is not possible and thus prohibited by
	// Triang::canCoarsen() below!
	//
	//                 1*                        1* 
	//                 /|                        /| 
	//                / |                       /1| 
	//               /  |                      /  | 
	//              /   |                     /   | 
	//             /    |                    / #1 |
	//            /     |                   /     | 
	//           /      |                  /2    0| 
	//          /       |              (0)*-------*(2)
	//         /        |     ==>        /|0    2/| 
	//        /         |               /1|     /1| 
	//       /          |              /  | #3 /  | 
	//      /           |             /   |   /   | 
	//     /            |            / #2 |  / #0 | 
	//    /             |           /     |1/     | 
	//   /              |          /2    0|/2    0| 
	// 2*---------------*0       2*-------*-------*0
        //                                   (1)

	if (0) {
	  //Xdisplay disp(g_argc,g_argv,"a","Grid",
	  //		      350.,450.,450.);
	  double p1[2],p2[2];
	  double bary[3]={1./3.,1./3.,1./3.};
#ifdef USE_ALUGRID_XDISPLAY
	  mesh->draw(disp);
#endif
	  {
	    this->fromlocal(bary,p1);
	    this->fromlocal(bary,p2);
	    p1[0]-=0.05;
	    p1[1]-=0.05;
	    p2[0]+=0.05;
	    p2[1]+=0.05;
	    Fullvertex pp1(p1,-1);
	    Fullvertex pp2(p2,-1);
	    //disp.linedraw(&pp1,&pp2,disp.bcol[17]);
	  }
	  {
	    this->fromlocal(bary,p1);
	    this->fromlocal(bary,p2);
	    p1[0]-=0.05;
	    p1[1]+=0.05;
	    p2[0]+=0.05;
	    p2[1]-=0.05;
	    Fullvertex pp1(p1,-1);
	    Fullvertex pp2(p2,-1);
	    //disp.linedraw(&pp1,&pp2,disp.bcol[17]);
	  }
	}
        Triang *newtr[4];
        Vertex *newvtx[3];
	Edge *newedge[9];
        bool usehvtx[3];
        int i;

        mysplit = triang_quarter;

        // create new vertices
        for (i=0;i<3;i++)
	  {
	    double b[3],p[2];

	    usehvtx[i] = false;
	    if (connect.nb[i]->thinis(bndel_like))
	      {
		assert(!connect.hvtx[i]);
		((Bndel*)connect.nb[i])->get_splitpoint(p);
		newvtx[i] = new Fullvertex(p,level());
		agnc->insert(newvtx[i]);
		newedge[2*i]=new Edge(hdl);
		newedge[2*i+1]=new Edge(hdl);
	      }
	    else
	      {
		// Use hanging node if it exists. Otherwise create
		// new vertex and insert it as hanging node in the
		// corresponding neighbour.

		if(connect.hvtx[i])
		  {
		    usehvtx[i] = true;
		    newvtx[i]  = connect.hvtx[i]->getHead();
		    newedge[2*i]=connect.hvtx[i]->getrnb()->edge(opposite(i));
		    newedge[2*i+1]=connect.hvtx[i]->getlnb()->edge(opposite(i));
		  }
		else
		  {
		    midpoint(i,b);
		    fromlocal(b,p);
		    newvtx[i] = new Fullvertex(p,level());
		    agnc->insert(newvtx[i]);
		    newedge[2*i]=new Edge(hdl);
		    newedge[2*i+1]=new Edge(hdl);
		  }
	      }
	    newedge[6+i]=new Edge(hdl);
	  }

        // create new elements

        newtr[0] = new Triang(connect.vtx[0],newvtx[2],newvtx[1]);
        newtr[1] = new Triang(newvtx[2],connect.vtx[1],newvtx[0]);
        newtr[2] = new Triang(newvtx[1],newvtx[0],connect.vtx[2]);
        newtr[3] = new Triang(newvtx[0],newvtx[1],newvtx[2]);
        for (i=0;i<4;i++) {
	  newtr[i]->sethdl(hdl);
	  e[i] = newtr[i];
	}
	for (i=0;i<3;i++) {
	  newtr[i]->edgeconnect(i,newedge[6+i]);
	  newtr[i]->edgeconnect(i+1,newedge[2*((i+1)%3)+1]);
	  newtr[i]->edgeconnect(i+2,newedge[2*((i+2)%3)]);
	  newtr[3]->edgeconnect(i,newedge[6+i]);
	}

        // distribute hanging nodes

        for (i=0;i<3;i++)
	  {
	    if (usehvtx[i] == true)
	      {
		connect.hvtx[i]->splitTree(newtr[(i+1)%3]->connect.hvtx[i],
					   newtr[(i+2)%3]->connect.hvtx[i]);
		delete connect.hvtx[i];
		connect.hvtx[i] = 0;
	      } else if (!connect.nb[i]->thinis(bndel_like)) {
	      ((Element*)connect.nb[i])->addhvtx(newvtx[i],newtr[(i+1)%3],newtr[(i+2)%3],connect.bck[i]);
	    }
	  }

	// set connectivity for element #3 and normal directions

        for (i=0;i<3;i++)
	  {
	    newtr[3]->nbconnect(i,newtr[i],i);
	    newtr[i]->nbconnect(i,newtr[3],i);

	    newtr[3]->setnormdir(i, 1);
	    newtr[i]->setnormdir(i,-1);
	    newtr[i]->setnormdir((i+1)%3,connect.normdir[(i+1)%3]);
	    newtr[i]->setnormdir((i+2)%3,connect.normdir[(i+2)%3]);
	  }

	// set connectivity across outer boundaries

        for (i=0;i<3;i++)
	  {
	    if (connect.nb[i]->thinis(bndel_like))
	      {
		// refine neighbouring boundary element

		ncv = new nconf_vtx_t(newvtx[i], newtr[(i+1)%3], newtr[(i+2)%3]);
		Hier<Bndel>* hbel = (Hier<Bndel>*)connect.nb[i];
		hbel->refine_leaf(agnc, &mva,ncv,nconfDeg,default_ref,pro_el);
	      }
	    else
	      {
		// set connectivity if neighbour is already refined

		if (neighbour(i)->splitrule() == triang_quarter)
		  {
		    Triang *trnb0 = (Triang *)neighbour(i);
		    Triang *trnb1 = (Triang *)(trnb0->down());
		    Triang *trnb2 = trnb1;

		    if (trnb0->is(Refco::quart))
		      {
			// This case should only occur if periodic boundaries
			// are used and the macrogrid is too coarse, i.e., if
			// there are two elements which are direct and periodic
			// neighbours at the same time.

			cerr << "ERROR (Triang::split()): "
			     << "refinement of neighbour not yet finished!" << endl
			     << "                         "
			     << "Please try a finer macrogrid!" << endl;
			abort();
		      }

		    assert(trnb1);
		    assert(trnb2);

		    switch (opposite(i))
		      {
		      case 0:
			trnb2 = (Triang *)(trnb2->next());
			trnb1 = (Triang *)(trnb2->next());
			break;
		      case 1:
			trnb2 = (Triang *)(trnb2->next());
			trnb2 = (Triang *)(trnb2->next());
			break;
		      case 2:
			trnb1 = (Triang *)(trnb1->next());
			break;
		      default:
			abort();
		      }

		    assert(trnb1->connect.vtx[(opposite(i)+1)%3] == newvtx[i]);
		    assert(trnb2->connect.vtx[(opposite(i)+2)%3] == newvtx[i]);

		    newtr[(i+1)%3]->nbconnect(i,trnb1,opposite(i));
		    newtr[(i+2)%3]->nbconnect(i,trnb2,opposite(i));
		    trnb1->nbconnect(opposite(i),newtr[(i+1)%3],i);
		    trnb2->nbconnect(opposite(i),newtr[(i+2)%3],i);

		    if (newtr[(i+1)%3]->connect.hvtx[i])
		      newtr[(i+1)%3]->connect.hvtx[i]->nbconnect(opposite(i),newtr[(i+1)%3],i);
		    if (newtr[(i+2)%3]->connect.hvtx[i])
		      newtr[(i+2)%3]->connect.hvtx[i]->nbconnect(opposite(i),newtr[(i+2)%3],i);
		  }
		else {
		  assert(neighbour(i)->splitrule() == unsplit);
		  newtr[(i+1)%3]->nbconnect(i,neighbour(i),opposite(i));
		  newtr[(i+2)%3]->nbconnect(i,neighbour(i),opposite(i));
		}
	      }
	  }

	/*
	  if (pro_el)
	  pro_el->operator()(this, (Element**)newtr, 4);
	*/
        ret = 4;
      }
      break;
    default:
      cerr << "ERROR (Triang::split()): unknown splitrule!" << endl;
      abort();
    }

  return ret;
}

bool Triang::canCoarsen(int nconfDeg) const
{
  bool ret;

  ret = ( (   (splitrule() == triang_quarter)
	      || (neighbour(0)->splitrule() == triang_quarter)) 
	  ? false : true);

  if (ret && nbel(0))
    ret = (nbel(0)->deepestLevel() <= nconfDeg); /* &&
						    (down()->next()->neighbour(0)->thinis(element_like) ?
						    !(((Element*)(down()->next()->neighbour(0)))->
						    hashvtx(down()->next()->opposite(0))) : true) &&
						    (down()->neighbour(0)->thinis(element_like) ?
						    !(((Element*)(down()->neighbour(0)))->hashvtx(down()->opposite(0)))
						    : true); */

  ret = ret && !(down()->hashvtx(0)) && !(down()->next()->hashvtx(0));

  return ret;
}

// ***************************************************
// #begin(method)
// #method:
//   int Triang::docoarsen() 
// #parameters:
// #description:
//   KONFORMES Vergroebern von Elementen.
//   (Nicht konformes Vergroebern funktioniert noch nicht!)
// #end(method)
// ***************************************************
int Triang::docoarsen(nconf_vtx_t *ncv,
		      int nconfDeg,Restrict_basic *rest_el)
{
  int result = 0;
  if( nconfDeg == 0 && mysplit == triang_conf2) {
    int lcancoarsen=0;
    if ( connect.nb[0]->thinis(bndel_like) )
      {
	if ( ((Hier<Bndel>*)connect.nb[0])->docoarsen(ncv,nconfDeg,rest_el) )
	  {
	    Hier<Element> *child=down();
	    child->neighbour(0)->nbconnect(child->opposite(0),this,2);
	    nbconnect(2,child->neighbour(0),child->opposite(0));
	    child=child->next();
	    child->neighbour(0)->nbconnect(child->opposite(0),this,1);
	    nbconnect(1,child->neighbour(0),child->opposite(0));
	    if( neighbour(1)->thinis(element_like) ) {
	      ((Element*)neighbour(1))->setnormdir(opposite(1), 1);
	      setnormdir(1, -1);
	    } else
	      setnormdir(1, 1);
	    if( neighbour(2)->thinis(element_like) ) {
	      ((Element*)neighbour(2))->setnormdir(opposite(2), 1);
	      setnormdir(2, -1);
	    } else
	      setnormdir(2, 1);
	    lcancoarsen=1;
	  }
      }
    else if ( connect.bck[0] == 0 )
      {
	Hier<Element> *neigh=(Hier<Element>*)connect.nb[0];
	if ( !ncv )
	  {
	    int cancoarsen=1;
	    neigh=neigh->down();
	    do
	      {
	      } while (   (cancoarsen=(neigh->is(Refco::crs)))
			  && (neigh=neigh->next()) );
	    if (cancoarsen)
	      {
		Hier<Element> *child=down();
		ncv = new nconf_vtx(child->vertex(0),child,child->next());
		assert( ncv );
		child->neighbour(1)->nbconnect(2,this,-1);
		child->neighbour(0)->nbconnect(child->opposite(0),this,2);
		nbconnect(2,child->neighbour(0),child->opposite(0));
		if( child->neighbour(0)->thinis(element_like) ) {
		  ((Element*)neighbour(2))->setnormdir(opposite(2), 1);
		  setnormdir(2, -1);
		} else
		  setnormdir(2, 1);

		child=child->next();
		child->neighbour(2)->nbconnect(1,this,-1);
		child->neighbour(0)->nbconnect(child->opposite(0),this,1);
		nbconnect(1,child->neighbour(0),child->opposite(0));
		if( child->neighbour(0)->thinis(element_like) ) {
		  ((Element*)neighbour(1))->setnormdir(opposite(1), 1);
		  setnormdir(1, -1);
		} else
		  setnormdir(1, 1);
		setnormdir(0, -1);
          
#ifndef NDEBUG
		// only used in assert 
		bool didcoarse = 
#endif
		  // do coarsen 
		  ((Hier<Element>*)connect.nb[0])->docoarsen(ncv,nconfDeg,rest_el);
		assert( didcoarse );
		lcancoarsen=1;
	      }
	  }
	else
	  {
	    Hier<Element> *child=down();
	    child->neighbour(0)->nbconnect(child->opposite(0),this,2);
	    nbconnect(2,child->neighbour(0),child->opposite(0));
	    if( child->neighbour(0)->thinis(element_like) ) {
	      ((Element*)neighbour(2))->setnormdir(opposite(2), 1);
	      setnormdir(2, -1);
	    } else
	      setnormdir(2, 1);

	    child=child->next();
	    child->neighbour(0)->nbconnect(child->opposite(0),this,1);
	    nbconnect(1,child->neighbour(0),child->opposite(0));
	    if( child->neighbour(0)->thinis(element_like) ) {
	      ((Element*)neighbour(1))->setnormdir(opposite(1), 1);
	      setnormdir(1, -1);
	    } else
	      setnormdir(1, 1);
	    setnormdir(0, 1);

	    delete ncv;
	    ncv=0;
	    Element *e[2];
	    e[0]=down();
	    e[1]=down()->next();
	    // rest_el->operator()(this,e,2);
	    if (rest_el) 
	      rest_el->operator()(this);
	    lcancoarsen=2;
	  }
      }
    if( lcancoarsen != 0 && rest_el) {
      Element *e[2];
      e[0]=down();
      e[1]=down()->next();
      //rest_el->operator()(this,e,2);
      if (rest_el)
	rest_el->operator()(this);
    }
    if (lcancoarsen==2) {
      deletesubtree();
      mysplit = unsplit;
    }
    result = lcancoarsen;
  } else if (mysplit==triang_quarter) {
    int lcancoarsen=1;
    Element *child[3]={down(),down()->next(),down()->next()->next()};
    for (int i=0;i<3;i++) {
      if (nbel(i)) {
	if (!(nbel(i)->leaf()) && child[(i+1)%3]->hashvtx(i)) {
	  if (((Triang*)child[(i+1)%3])->connect.hvtx[i]->count()+1>nconfDeg)
	    lcancoarsen=0;
	}
	if (!(nbel(i)->leaf()) && child[(i+2)%3]->hashvtx(i)) {
	  if (((Triang*)child[(i+2)%3])->connect.hvtx[i]->count()+1>nconfDeg)
	    lcancoarsen=0;
	}
      } 
    }
    if (lcancoarsen) {
      if (rest_el) 
	rest_el->operator()(this);
      if (0) {
	//Xdisplay disp(g_argc,g_argv,"a","Grid",
	//	      350.,450.,450.);
	double p1[2],p2[2];
	double bary[3]={1./3.,1./3.,1./3.};
#ifdef USE_ALUGRID_XDISPLAY
	mesh->draw(disp);
#endif
	{
	  this->fromlocal(bary,p1);
	  this->fromlocal(bary,p2);
	  p1[0]-=0.05;
	  p1[1]-=0.05;
	  p2[0]+=0.05;
	  p2[1]+=0.05;
	  Fullvertex pp1(p1,-1);
	  Fullvertex pp2(p2,-1);
	  //disp.linedraw(&pp1,&pp2,disp.bcol[17]);
	}
	{
	  this->fromlocal(bary,p1);
	  this->fromlocal(bary,p2);
	  p1[0]-=0.05;
	  p1[1]+=0.05;
	  p2[0]+=0.05;
	  p2[1]-=0.05;
	  Fullvertex pp1(p1,-1);
	  Fullvertex pp2(p2,-1);
	  //disp.linedraw(&pp1,&pp2,disp.bcol[17]);
	}
      }
      // Nachbarschaften stimmen (hoffe ich)
      // Haengende Knoten verwalten
      for (int i=0;i<3;i++) {
	assert(!hashvtx(i));
	if (nbel(i)) {
	  if (!(nbel(i)->leaf())) { // Haengenden Knoten erzeugen 
	    addhvtx(child[(i+1)%3]->vertex((i+2)%3), 
		    child[(i+2)%3]->nbel(i),child[(i+1)%3]->nbel(i), i);
	    connect.hvtx[i]->merge(((Triang*)child[(i+1)%3])->connect.hvtx[i],
	    			   ((Triang*)child[(i+2)%3])->connect.hvtx[i]);
	    connect.hvtx[i]->nbconnect(opposite(i),this,i);
	  } else { // Haengenden Knoten im Nachbarn entfernen 
	    assert(nbel(i)->hashvtx(opposite(i)));
	    nbel(i)->removehvtx(opposite(i),child[(i+1)%3]->vertex((i+2)%3));
	  }
	}
      }
      for (int i=0;i<3;i++) {
	if (nbbnd(i)) {
	  if( !(nbbnd(i)->docoarsen(ncv,nconfDeg,rest_el)) ) {
	    lcancoarsen=0;
	    assert(0);
	  }
	}
      }
    } 
    result = lcancoarsen;
  } else {
    cerr << "COARSENING: not implemented for non-conform refinemnet and bisectionm" << endl;
    abort();
  }

  return result;
}

// ***************************************************
// #begin(method)
// #method:
//   int Element::c::check()
// #parameters:
// #description:
//   F"uhrt einen check der connect-Info durch (privat).
//   Steigt mit einem assert aus falls nicht alles stimmt!
// #end(method)
// ***************************************************
int Element::c::check()
{
  for (int i=0;i<nv;i++)
    assert( vtx[i] );
  for (int i=0;i<nf;i++)
    {
      if (nb[i]->thinis(element_like))
	{
	  assert( 0<=bck[i] && bck[i]<=nf );
	}
      else
	{
	  assert( bck[i] == 0 );
	  assert( normdir[i] == 1 );
	}
      assert( normdir[i]==1 || normdir[i]==-1 );
      assert(edge[i]);
      assert(edge[i]->getIndex()>=0);
    }
  return 1;
}

// ***************************************************
// #begin(method)
// #method:
//   int Element::check()
// #parameters:
// #description:
//   Checkt alle Dreieksinfos. Steigt aus falls es eine Unstimmigkeit findet!
// #end(method)
// ***************************************************
int Element::check()
{
  assert(getIndex()>=0);
  // char* whichCheck = 0;
  // std::string whichCheck = "reason: ";
  bool connectFailure = false;
  // 1. Bin ich ein richtiges Dreieck
  assert( connect.check() );
  assert( setorientation() );
  // 2. Meine Nachbarschaft stimmt
  for (int i=0;i<3;i++)
    {
      if(  connect.nb[i]->thinis(element_like) )
	{
	  if(  !connect.hvtx[i] &&
	       !(((Triang*)neighbour(i))->hashvtx(opposite(i))) ) { // Konform
	    assert( connect.nb[i]->opposite(connect.bck[i])==i );
	    assert( connect.nb[i]->neighbour(connect.bck[i])==this );
	    assert( connect.nb[i]->edge(connect.bck[i])==connect.edge[i]);
	    assert( connect.nb[i]->vertex(connect.bck[i]+1)==vertex(i+2) );
	    assert( connect.nb[i]->vertex(connect.bck[i]+2)==vertex(i+1) );
	  } else if (connect.hvtx[i]) {
	    assert(!(((Triang*)neighbour(i))->hashvtx(opposite(i))));
	    assert( connect.hvtx[i]->getlnb()->neighbour(opposite(i))==this);
	    assert( connect.hvtx[i]->getrnb()->neighbour(opposite(i))==this);
	  }
	  assert(((Element*)neighbour(i))->normaldir(opposite(i)) == -normaldir(i));
	}
      else
	{
	  assert( connect.nb[i]->neighbour(0)==this );
	  assert( connect.nb[i]->vertex(0)==vertex(i+1) );
	  assert( connect.nb[i]->vertex(1)==vertex(i+2) );
	  assert( connect.nb[i]->opposite(0)==i );
	  assert( connect.nb[i]->edge(connect.bck[i])==connect.edge[i]);
	}
    }
  if ( ((Hier<Element>*)this)->leaf()) {
  }
  return (connectFailure ? 0 : 1);
}

// **************************************************************

Bndel_triang::Bndel_triang(Vertex * v0, Vertex * v1, bnd_t type) : time(0.0) {

  connect.set(v0, 0) ;

  connect.set(v1, 1) ;

  Bndel::typ = type ;

  Thinelement::nedges = connect.nv ;

  Thinelement::nfaces = connect.nf ;

  Thinelement::nvertices = connect.nv ;
  
}

int Bndel::facevertex(int , int j) const {
 
  assert(0 <= j) ;
  
  assert(j < connect.nv) ;
  
  return j ;
  
}

void Bndel::edge_vtx(int e, Vertex * (&v) [2]) const {

  assert(e < connect.nv) ;

  v[0] = connect.vtx[e ++ ] ;

  v[1] = connect.vtx[e == connect.nv ? 0 : e] ;

}

Vertex * Bndel::vertex(int i) const {

  assert(0 <= i);

  i%=connect.nv;
 
  return connect.vtx[i] ;
  
}

void Bndel_triang::write(ofstream & out) const {

  out << (int) type() << "  " ;

  connect.write(out) ;

  out << endl ;
  
}

void Bndel_triang::read(ifstream & in, Vertex ** v, const int nv) {

  int c ;

  in >> c ;

  Bndel::typ = (Bndel::bnd_t) c ;

  connect.read(in, v, nv) ;

  Thinelement::nedges = connect.nv ;

  Thinelement::nfaces = connect.nf ;

  Thinelement::nvertices = connect.nv ;

}

void Bndel::nbconnect(int fce, Thinelement * n, int b) {

  assert(!fce) ;
 
  connect.nb = n ; 

  connect.bck = b ; 

}
void Bndel::edgeconnect(int fce, Edge * n) { 
  assert(0 <= fce) ;
  connect.edge = n ; 
  n->attach();
}

Bndel::c::c()  {

  for(int i = 0 ; i < max_points ; i ++ ) vtx[i] = 0 ;

  nb = 0 ;

  bck = -1 ;

  edge = 0;

}

Bndel::c::~c() {

  for(int i = 0 ; i < nv ; i ++ ) 

    if(vtx[i]) vtx[i]->detach() ;

}

void Bndel::c::write(ofstream &out) const {

  // out << nv << "  " ;

  for( int i = 0 ; i < nv ; i ++ )

    out << vtx[i]->Listagent < Vertex > :: number() << "  " ;
    
}

void Bndel::c::read(ifstream & in, Vertex ** v, const int l) {

  int c ;

  for(int i = 0 ; i < nv ; i ++) {

    in >> c ;

    assert(-1 <= c && c < l) ;

    if(c != -1) set((Vertex *)v[c], i) ;
 
  }

}

void Bndel_triang::restrictLocal(Bndel **pbel, int pnr)
{
  assert(   ((Bndel_triang*)(pbel[0]))->time
	    == ((Bndel_triang*)(pbel[1]))->time);

  time = ((Bndel_triang*)(pbel[0]))->time; 
}

void Bndel_triang::prolongLocal(Bndel **pbel, int pnr) const
{
  for (int i=0;i<pnr;i++)
    ((Bndel_triang*)(pbel[i]))->time = time;
}

int Bndel_triang::split(void * (&e)[nparts], Listagency < Vertex > * agnc,
                        Multivertexadapter & mva, 
			nconf_vtx_t *ncv,
			splitrule_t sr,
			int nconfDeg,Refco::tag_t default_ref,
			Prolong_basic *pro_el)
{
  int idx[2];
  //  nconf_vtx_t *ncv;

  assert(sr == triang_bnd);
  assert(splitrule() == unsplit);
  mysplit = triang_bnd;

  // create new elements
  
  assert(ncv);
  Bndel *t1=create(connect.vtx[0],ncv->vtx,typ);
  Bndel *t2=create(ncv->vtx,connect.vtx[1],typ);

  // determine opposite vertices

  switch (nbel(0)->splitrule())
    {
    case Thinelement::triang_conf2:
      idx[0] = 1;
      idx[1] = 2;
      break;
    case Thinelement::triang_quarter:
      idx[0] = opposite(0);
      idx[1] = opposite(0);
      break;
    default:
      cerr << "ERROR (Bndel_triang::split()): "
	   << "illegal splitrule!" << endl;
      abort();
    }

  // set connectivity

  ncv->el[0]->nbconnect(idx[0],t1,0);
  ncv->el[1]->nbconnect(idx[1],t2,0); 
  ((Element*)ncv->el[0])->setnormdir(idx[0],1);
  ((Element*)ncv->el[1])->setnormdir(idx[1],1);
  t1->nbconnect(0,ncv->el[0],idx[0]);
  t2->nbconnect(0,ncv->el[1],idx[1]);
  t1->set_bndfunctions(lf,lDf);
  t2->set_bndfunctions(lf,lDf);
  //      mva.delete(connect.vtx[1],connect.vtx[2]);    
  
  // Daten auf dem Rand prolongieren
  t1->sethdl(hdl);
  t2->sethdl(hdl);
  t1->edgeconnect(0,ncv->el[0]->edge(idx[0]));
  t2->edgeconnect(0,ncv->el[1]->edge(idx[1]));
  e[0]=t1;
  e[1]=t2;
  prolongLocal((Bndel **)e,2);

  delete ncv;
  ncv=NULL;

  return 2;
}

int Bndel_triang::docoarsen(nconf_vtx_t *ncv,
			    int nconfDeg,Restrict_basic *rest_el)
{
  assert(splitrule() == triang_bnd);

  // Daten auf dem Rand restringieren
  Bndel *bel[2];
  bel[0]=down();
  bel[1]=down()->next();
  restrictLocal(bel,2);
  deletesubtree();
  mysplit = unsplit;
  return 1;  
} 

int periodic_flag=0;

int Bndel_periodic::split(void * (&el)[nparts], Listagency < Vertex > * agnc,
                          Multivertexadapter & mva, 
			  nconf_vtx_t *ncv,
			  splitrule_t sr,
			  int nconfDeg,Refco::tag_t default_ref,
			  Prolong_basic *pro_el)
{
  assert(sr == triang_bnd);
  assert(splitrule() == unsplit);

  if( nconfDeg == 0 ) {
    Bndel_triang::split(el,agnc,mva,ncv,triang_bnd,nconfDeg,default_ref,pro_el); 
    if (periodic_flag)
      return 2;
    periodic_flag=1;
    while (periodic_nb->leaf())
      {
	switch (nbel(0)->splitrule())
	  {
	  case Thinelement::triang_conf2:
	    periodic_nb->nbel(0)->Refco_el::mark(Refco::ref_1) ;
	    break;
	  case Thinelement::triang_quarter:
	    periodic_nb->nbel(0)->Refco_el::mark(Refco::quart) ;
	    break;
	  default:
	    cerr << "ERROR (Bndel_periodic::split()): "
		 << "illegal splitrule!" << endl;
	    abort();
	  }
	periodic_nb->nbel(0)->refine_leaf(agnc,&mva,ncv,nconfDeg,default_ref,pro_el);
      }
    periodic_flag=0;
    ((Bndel_periodic*)(el[0]))
      ->set_pnb((Bndel_periodic*)(periodic_nb->down()->next()));
    ((Bndel_periodic*)(periodic_nb->down()->next()))
      ->set_pnb((Bndel_periodic*)(el[0]));
    ((Bndel_periodic*)(el[1]))
      ->set_pnb((Bndel_periodic*)(periodic_nb->down()));
    ((Bndel_periodic*)(periodic_nb->down()))
      ->set_pnb((Bndel_periodic*)(el[1]));

    ((Bndel_periodic*)(el[0]))->vertex(1)
      ->set_pernb(((Bndel_periodic*)(periodic_nb->down()))->vertex(1));
    ((Bndel_periodic*)(periodic_nb->down()))->vertex(1)
      ->set_pernb(((Bndel_periodic*)(el[0]))->vertex(1));
  } else {

    // split

    Bndel_triang::split(el,agnc,mva,ncv,triang_bnd,nconfDeg,default_ref,pro_el);

    // update connectivity

    if( periodic_nb->leaf() ) {
      ((Bndel_periodic*)el[0])->set_pnb(periodic_nb);
      ((Bndel_periodic*)el[1])->set_pnb(periodic_nb);
    } else {
      ((Bndel_periodic*)el[0])
	->set_pnb((Bndel_periodic*)(periodic_nb->down()->next()));
      ((Bndel_periodic*)(periodic_nb->down()->next()))
	->set_pnb((Bndel_periodic*)el[0]);
      ((Bndel_periodic*)el[1])
	->set_pnb((Bndel_periodic*)(periodic_nb->down()));
      ((Bndel_periodic*)(periodic_nb->down()))
	->set_pnb((Bndel_periodic*)el[1]);
    }

  }

  return 2;
}

int Bndel_periodic::docoarsen(nconf_vtx_t *ncv,
			      int nconfDeg,Restrict_basic *rest_el)
{
  if(nconfDeg == 0 ) {
    if (periodic_nb->leaf())
      return Bndel_triang::docoarsen(ncv,nconfDeg,rest_el);
    // Kann anderer Rand auch vergr"obert werden?  
    if (periodic_nb->opposite(0)!=0)
      return 0;
    assert(periodic_nb->nbel(0));
    assert(!(periodic_nb->nbel(0)->leaf()));    
    Hier<Element> *h=periodic_nb->nbel(0)->down();

    while (h)
      {
	if (!h->is(Refco::crs))
	  break;
	h=h->next();
      }
    if (h)
      return 0;
    // es kann verfeinert werden! 
    Bndel_triang::docoarsen(ncv,nconfDeg,rest_el);  
    periodic_nb->nbel(0)->coarse(ncv,nconfDeg,rest_el);
    assert(periodic_nb->nbel(0)->leaf());
    assert(periodic_nb->leaf());  
  } else {
    cout << "nicht-konformes Vergroebern noch nicht implementiert!" << endl;
    abort();
  }
  
  return 1;  
}

