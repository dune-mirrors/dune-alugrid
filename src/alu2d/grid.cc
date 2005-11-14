
static volatile char RCSId_grid_cc [] = "$Id$";

// ***************************************************
// #begin(header)
// #filename:
//   grid.cc
// #description:
//   Implementierungen zu Hexa, Fullvertex und Bndel_impl
// #methods:
//   Fullvertex::Fullvertex()
//   Fullvertex::write()
//   Fullvertex::read()
//   Bndel::flux()
//   Bndel::get_splitpoint()
// #copyright:
//   IAM Freiburg
// #end(header)
// ***************************************************

#include <time.h>
#include <stdio.h>	//  sprintf()
#include <stdlib.h>	//  NULL
#include <assert.h>	

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>	//  ostream
#else
  #include <iostream.h>	//  ostream
#endif

#include "grid.h"
#include "triang.h"
#include "handle.h"

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

Fullvertex::Fullvertex(double x, double y, int level) {

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

Fullvertex::Fullvertex(double (&p)[ncoord],int level) {

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

void Fullvertex::write(ofstream & out) const {

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

void Fullvertex::read(ifstream & in) {

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

void Edge::write(ofstream & out) const {

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

void Edge::read(ifstream & in) {

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
void Bndel::mirror_opposite(double (&pret)[2]) const
{
  // mirrors the ``point'' opposite(0) at ``this'' 

  mirror(nbel(0)->vertex(opposite(0))->coord(),pret);
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
void Bndel::mirror(const double (&ppoint)[2],
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
double Bndel::area() const 
{
  Element *tr = nbel(0);
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

int Bndel::get_splitpoint(double (&ppoint) [2])
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


void Fullvertex::sethdl(Hmesh_basic *phdl) {
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(0);
}
void Edge::sethdl(Hmesh_basic *phdl) {
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(1);
}
Edge::~Edge() {
  assert(isfree());
  hdl->freeIndex(1,_idx);
}
void Element::sethdl(Hmesh_basic *phdl) {
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(2);
}
void Bndel::sethdl(Hmesh_basic *phdl) {
  hdl=phdl;
  assert(_idx==-1);
  _idx=hdl->getIndex(3);
}
Vertex::~Vertex() {
  if (hdl) {
    assert(_idx>=0);
    hdl->freeIndex(0,_idx);
  }
}
Element::~Element() {
  for (int i=0;i<3;i++) {
    if (connect.edge[i]) {
      connect.edge[i]->detach();
      if (connect.edge[i]->isfree())
	delete connect.edge[i];
    }
  }
  assert(hdl);
  if (hdl) {
    assert(_idx>=0);
    hdl->freeIndex(2,_idx);
  }
}
Bndel::~Bndel() {
  if (connect.edge) {
    connect.edge->detach();
    if (connect.edge->isfree())
      delete connect.edge;
  }
  if (hdl) {
    assert(_idx>=0);
    hdl->freeIndex(3,_idx);
  }
}
