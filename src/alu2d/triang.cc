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
//   Element::get_splitpoint
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
template < int N, int NV >
Triang < N,NV >::Triang() {
  element_t::nvertices() = 0;
}

// ***************************************************
// #begin(method)
// #method:
//   Triang::Triang(vertex_t * v0,vertex_t * v1, vertex_t * v2)
// #parameters:
// #description:
//   Konstruktor f"ur Dreiecke mit den Ecken v0,v1 und v2
// #end(method)
// ***************************************************
template < int N, int NV >
Triang < N,NV >::Triang(vertex_t * v0,vertex_t * v1, vertex_t * v2) {
  connect.set( v0, 0) ;
  connect.set( v1, 1) ;
  connect.set( v2, 2) ;
  element_t::nvertices() = 3 ;

  init();
}
template < int N, int NV >
Triang < N,NV >::Triang(vertex_t * v0,vertex_t * v1, vertex_t * v2, vertex_t * v3) {
  assert( NV == 4);
  if (NV == 4) 
  {
    connect.set( v0, 0) ;
    connect.set( v1, 1) ;
    connect.set( v2, 2) ;
    connect.set( v3, 3) ;
    element_t::nvertices() = 4 ;
  }
  init();
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::write(ostream & out) const
// #parameters:
// #description:
//   Rausschreiben des Dreicks (unvollst"andig)
// #end(method)
// ***************************************************
template < int N, int NV >
void Triang < N,NV >::write(ostream & out) const {
  element_t::connect.write(out) ;
  //  data.write(out) ;
  out << endl ;
}

// ***************************************************
// #begin(method)
// #method:
//   void Element::read(istream & in, vertex_t ** look, const int len)
// #parameters:
//   \ istream& | io    | Eingabestrom
//   \ vertex_t**  | look  | Liste von vertex_tpointern
//   \ const int | len   | Gr"o"se der Liste look
// #description:
//   Zum Einlesen eines Dreiecks. Es werden 3 Integerwerte eingelesen
//   und die entsprechenden Vetex* aus der Liste look als Eckpunkte
//   des Dreiecks gew"ahlt.
// #end(method)
// ***************************************************
template < int N, int NV >
void Triang < N,NV >::read(istream & in, vertex_t ** look, const int len) {
  helement_t::read(in) ;
  element_t::nvertices() = connect.read(in, look, len) ;
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
template < int N, int NV >
bool Triang < N,NV >::confLevelExceeded(int nconfDeg) const
{
  bool result = false;

  if( nconfDeg >= 0 ) {
    for( int i=0 ; i<numfaces() ; i++ )
      {
  bndel_triang_t *nb = nbbnd(i);

  if (nb)
    {
      if (nb->type()==bndel_t::periodic)
        {
    Triang* tr = ((bndel_periodic_t*)nb)->periodic_nb->nbel(0);
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
template < int N, int NV >
void Triang < N,NV >::newNeighbour(Triang* nb, int fce, int nbfce,
                          splitrule_t sr, bool setnormal)
{
  // TWIST
  assert(sr == thinelement_t::triang_conf2);
  assert(fce >= 0);
  assert(nbfce >= 0);
  fce = mod(fce);
  nbfce = mod(nbfce);

  this->nbconnect(fce, nb, nbfce);
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

#ifdef USE_ALUGRID_XDISPLAY
extern Hmesh *mesh;
#endif
/***************************************************
 // #begin(method)
 // #method:
 //   int Triang::split(void * (&e)[nparts], Listagency < vertex_t > * agnc,
 //                     Multivertexadapter & mva, splitrule_t)
 // #parameters:
 //   \ void* (&e)[nparts]  | e    | Array, welches die durch Verfeinerung neu
 //                                  entstehenden Elemente enthaelt
 //   \ Listagency<vertex_t>* | agnc | Liste, die neu entstehende Knoten aufnimmt
 //   \ Multivertexadapter& | mva  | Multivertexadapter (nicht benutzt)
 //   \ splitrule_t         | sr   | splitrule
 // #description:\
 //   (Nicht konformes) Verfeinern eines Elementes durch Bisektion.
 // #end(method)
 ***************************************************/

template < int N, int NV >
int Triang < N,NV >::split(void * (&e)[Basic::nparts], Listagency < vertex_t > * agnc,
                           multivertexadapter_t & mva, 
                           nconf_vtx_t *ncv,
                           splitrule_t sr,
                           int nconfDeg,
                           Refco::tag_t default_ref,
                           prolong_basic_t *pro_el)
{
  int ret = 0;

  assert(splitrule() == thinelement_t::unsplit);

  switch (sr)
  {
    case thinelement_t::triang_conf2:
      ret=split2tr(e,agnc,mva,ncv,nconfDeg,default_ref,pro_el);
      break;
    case thinelement_t::triang_quarter:
      ret=split4(e,agnc,mva,ncv,nconfDeg,default_ref,pro_el);
      break;
    default:                                                                         
      cerr << "ERROR (Triang::split()): unknown splitrule!" << endl;                 
      abort();                                                                       
  }
  return ret;
}
template < int N, int NV >
int Triang < N,NV >::split2tr(void * (&e)[Basic::nparts], Listagency < vertex_t > * agnc,
                              multivertexadapter_t & mva, 
                              nconf_vtx_t *ncv,
                              int nconfDeg,
                              Refco::tag_t default_ref,
                              prolong_basic_t *pro_el)
{
  assert( NV == 3);
  Triang* t1;
  Triang* t2;
  Edge *newedge[3];
  IndexProvider* hdl = this->gethdl();
  newedge[2]=new Edge( hdl );

  mysplit = thinelement_t::triang_conf2;

  if( connect.nb[0]->thinis(thinelement_t::bndel_like) ) 
  {
    // Verfeinerungsnachbar ist Randelement
    vertex_t* nvtx;
    double p[vertex_t::ncoord];

    // Erzeugen des neuen Knotens und der neuen Elemente
    connect.nb[0]->get_splitpoint(connect.bck[0], 0.5, p);
    agnc->insert(nvtx = new fullvertex_t(p,level()));
    t1 = new Triang(nvtx, connect.vtx[0], connect.vtx[1]);
    t2 = new Triang(nvtx, connect.vtx[2], connect.vtx[0]);
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
    if( neighbour(1)->thinis(thinelement_t::element_like) ) {
      if( !(((element_t*)neighbour(1))->hashvtx(opposite(1))) )
        ((Triang*)neighbour(1))->newNeighbour(t2, opposite(1), 0,
                                              thinelement_t::triang_conf2);
      else
        nbel(1)->connect.hvtx[opposite(1)]->head->rnb = t2;
    } 
    else {
      neighbour(1)->nbconnect(0, t2, 0);
    }

    if( neighbour(2)->thinis(thinelement_t::element_like) ) {
      if( !(((element_t*)neighbour(2))->hashvtx(opposite(2))) )
        ((Triang*)neighbour(2))->newNeighbour(t1, opposite(2),
                                              0, thinelement_t::triang_conf2);
      else
        nbel(2)->connect.hvtx[opposite(2)]->head->rnb = t1;
    } 
    else {
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
    ncv = new nconf_vtx_t(nvtx, t1, t2);
    hbndel_t *n = (hbndel_t *)connect.nb[0];
    n->refine_leaf(agnc, &mva,ncv,nconfDeg,default_ref,pro_el);
  } 
  else 
  {
    // Verfeinerungsnachbar ist "normales" Element
    vertex_t* nvtx;
    double p[ncoord];
    bool usehvtx = false;

    // Falls ein haengender Knoten existiert, jenen benutzen;
    // falls nicht, neuen Knoten erzeugen und jenen als haengenden in
    // entsprechenden Nachbarn einfuegen.
    if( connect.hvtx[0] ) {
      nvtx = connect.hvtx[0]->getHead();
      usehvtx = true;
      helement_t *tr = ((helement_t *)neighbour(0))->down()->next();
      if (tr->leaf()) 
        newedge[0]=connect.hvtx[0]->getrnb()->edge(2);
      else
        newedge[0]=connect.hvtx[0]->getrnb()->edge(0);
      tr = ((helement_t *)neighbour(0))->down();
      if (tr->leaf()) 
        newedge[1]=connect.hvtx[0]->getlnb()->edge(1);    
      else
        newedge[1]=connect.hvtx[0]->getlnb()->edge(0);
    } 
    else
    {
      this->get_splitpoint(0, 0.5, p);
      //connect.nb[0]->get_splitpoint(connect.bck[0], 0.5, p);
      agnc->insert(nvtx = new fullvertex_t(p,level()));
      newedge[0]=new Edge(hdl);
      newedge[1]=new Edge(hdl);
    }

    // Neue Elemente erzeugen
    t1 = new Triang(nvtx, connect.vtx[0], connect.vtx[1]);
    t2 = new Triang(nvtx, connect.vtx[2], connect.vtx[0]);
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
      ((element_t *)connect.nb[0])->addhvtx(nvtx, t1,t2, connect.bck[0]);
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
      tr->newNeighbour(t2, 1, 2, thinelement_t::triang_conf2);
      tr = (Triang*)(tr->next());
      tr->newNeighbour(t1, 2, 1, thinelement_t::triang_conf2);
    }
    // -- Nachbarschaft ueber Kante 1
    if( neighbour(1)->thinis(thinelement_t::element_like) ) {
      if( !(((Triang*)neighbour(1))->hashvtx(opposite(1))) ) {
        ((Triang*)neighbour(1))->newNeighbour(t2, opposite(1), 0,
                                              thinelement_t::triang_conf2);
      } 
      else {
        nbel(1)->connect.hvtx[opposite(1)]->head->lnb = t2;
      }
    } else {
      neighbour(1)->nbconnect(0, t2, 0);
    }
    // -- Nachbarschaft ueber Kante 2
    if( neighbour(2)->thinis(thinelement_t::element_like) ) {
      if( !(((Triang*)neighbour(2))->hashvtx(opposite(2))) ) {
        ((Triang*)neighbour(2))->newNeighbour(t1, opposite(2), 0,
                                              thinelement_t::triang_conf2);
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
      ((element_t *)neighbour(0))->setnormdir(opposite(0), -1);
      ((element_t *)this)->setnormdir(0, 1);
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

  if (this->is(Refco::ref_2)) 
  {
    t1->mark(Refco::ref_1);
    t2->mark(Refco::ref_1);
  }

  e[0] = t1;
  e[1] = t2;
  return 2;
}
template < int N, int NV >
int Triang < N,NV >::split4(void * (&e)[Basic::nparts], Listagency < vertex_t > * agnc,
                            multivertexadapter_t & mva, 
                            nconf_vtx_t *ncv,
                            int nconfDeg,
                            Refco::tag_t default_ref,
                            prolong_basic_t *pro_el)
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

  Triang *newtr[4];
  vertex_t *newvtx[NV];
  Edge *newedge[2*NV+NV];
  bool usehvtx[NV];
  int i;
  double p[ncoord];
  IndexProvider* hdl = this->gethdl();

  mysplit = thinelement_t::triang_quarter;

  // create new vertices and edges
  for (i=0;i<numfaces();i++)
  {
    usehvtx[i] = connect.hvtx[i];
    if (connect.nb[i]->thinis(thinelement_t::bndel_like))
    {
      assert(!usehvtx[i]);
      connect.nb[i]->get_splitpoint(connect.bck[i], 0.5, p);
      newvtx[i] = new fullvertex_t(p,level());
      agnc->insert(newvtx[i]);
      newedge[2*i]=new Edge(hdl);
      newedge[2*i+1]=new Edge(hdl);
    }
    else
    {
      // Use hanging node if it exists. Otherwise create
      // new vertex and insert it as hanging node in the
      // corresponding neighbour.

      if(usehvtx[i])
      {
        newvtx[i]  = connect.hvtx[i]->getHead();
        // TWIST
        newedge[2*i]=connect.hvtx[i]->getrnb()->edge(opposite(i));
        newedge[2*i+1]=connect.hvtx[i]->getlnb()->edge(opposite(i));
      }
      else
      {
        this->get_splitpoint(i, 0.5, p);
        // connect.nb[i]->get_splitpoint(connect.bck[i], 0.5, p);
        newvtx[i] = new fullvertex_t(p,level());
        agnc->insert(newvtx[i]);
        newedge[2*i]=new Edge(hdl);
        newedge[2*i+1]=new Edge(hdl);
      }
    }
    newedge[2*numfaces()+i]=new Edge(hdl);
  }

  // create new elements
  if (numfaces() == 3)
  {
    newtr[0] = new Triang(connect.vtx[0],newvtx[2],newvtx[1]);
    newtr[1] = new Triang(newvtx[2],connect.vtx[1],newvtx[0]);
    newtr[2] = new Triang(newvtx[1],newvtx[0],connect.vtx[2]);
    newtr[3] = new Triang(newvtx[0],newvtx[1],newvtx[2]);
    for (i=0;i<3;i++) {
      newtr[i]->edgeconnect(i,newedge[6+i]);
      newtr[i]->edgeconnect(i+1,newedge[2*((i+1)%3)+1]);
      newtr[i]->edgeconnect(i+2,newedge[2*((i+2)%3)]);
      newtr[3]->edgeconnect(i,newedge[6+i]);

      newtr[i]->setnormdir(i,-1);
      newtr[i]->setnormdir((i+1)%3,connect.normdir[(i+1)%3]);
      newtr[i]->setnormdir((i+2)%3,connect.normdir[(i+2)%3]);
      newtr[3]->setnormdir(i, 1);

      newtr[3]->nbconnect(i,newtr[i],i);
      newtr[i]->nbconnect(i,newtr[3],i);
    }
  }
  else 
  {
    const double l[2] = {0.5, 0.5};
    this->get_splitpoint( l, p );
    vertex_t* midvtx = new fullvertex_t(p,level());
    agnc->insert(midvtx);
    newtr[0] = new Triang(connect.vtx[0],newvtx[3],midvtx,newvtx[2]);
    newtr[1] = new Triang(newvtx[3],connect.vtx[1],newvtx[0],midvtx);
    newtr[2] = new Triang(midvtx,newvtx[0],connect.vtx[2],newvtx[1]);
    newtr[3] = new Triang(newvtx[2],midvtx,newvtx[1],connect.vtx[3]);
    for (i=0;i<4;i++) {
      newtr[i]->edgeconnect(i,newedge[8+i]);
      newtr[i]->edgeconnect(i+1,newedge[8+(i+3)%4]);
      newtr[i]->edgeconnect(i+2,newedge[2*((i+2)%4)+1]);
      newtr[i]->edgeconnect(i+3,newedge[2*((i+3)%4)]);

      newtr[i]->setnormdir(i,1);
      newtr[i]->setnormdir(i+1,-1);
      newtr[i]->setnormdir(i+2,connect.normdir[(i+2)%4]);
      newtr[i]->setnormdir(i+3,connect.normdir[(i+3)%4]);

      newtr[i]->nbconnect(i,newtr[(i+1)%4],(i+2)%4);
      newtr[i]->nbconnect(i+1,newtr[(i+3)%4],(i+3)%4);
    }
  }
  for (i=0;i<4;i++) {
    newtr[i]->sethdl(hdl);
    e[i] = newtr[i];
  }

  // distribute hanging nodes

  for (i=0;i<numfaces();i++)
  {
    if (usehvtx[i] == true)
    {
      connect.hvtx[i]->splitTree(newtr[mod(i+1)]->connect.hvtx[i],
                                 newtr[mod(i+2)]->connect.hvtx[i]);
      delete connect.hvtx[i];
      connect.hvtx[i] = 0;
    } else if (!connect.nb[i]->thinis(thinelement_t::bndel_like)) { // TWIST
      ((element_t*)connect.nb[i])->addhvtx(newvtx[i],newtr[mod(i+1)],newtr[mod(i+2)],connect.bck[i]);
    }
  }

  // set connectivity across outer boundaries
  for (i=0;i<numfaces();i++)
  {
    if (connect.nb[i]->thinis(thinelement_t::bndel_like))
    {
      // refine neighbouring boundary element
      ncv = new nconf_vtx_t(newvtx[i], newtr[mod(i+1)], newtr[mod(i+2)]);
      hbndel_t *hbel = (hbndel_t *)connect.nb[i];
      hbel->refine_leaf(agnc, &mva,ncv,nconfDeg,default_ref,pro_el);
    }
    else
    {
      // set connectivity if neighbour is already refined
      if (neighbour(i)->splitrule() == thinelement_t::triang_quarter)
      {
        Triang *trnb0 = (Triang *)neighbour(i);
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

        // TWIST
        Triang *trnb2 = (Triang *)(trnb0->down());
        assert(trnb2);
        for (int k=0;k<mod(opposite(i)+1);++k)
        {
          trnb2 = (Triang *)(trnb2->next());
          assert(trnb2);
        }

        Triang *trnb1 = (Triang *)( opposite(i)!=numfaces()-2 ? 
                        trnb2->next():trnb0->down() );
        assert( trnb1 );

        assert(trnb1->connect.vtx[mod(opposite(i)+1)] == newvtx[i]);
        assert(trnb2->connect.vtx[mod(opposite(i)+2)] == newvtx[i]);

        newtr[mod(i+1)]->nbconnect(i,trnb1,opposite(i));
        newtr[mod(i+2)]->nbconnect(i,trnb2,opposite(i));
        trnb1->nbconnect(opposite(i),newtr[mod(i+1)],i);
        trnb2->nbconnect(opposite(i),newtr[mod(i+2)],i);

        if (newtr[mod(i+1)]->connect.hvtx[i])
          newtr[mod(i+1)]->connect.hvtx[i]->nbconnect(opposite(i),newtr[mod(i+1)],i);
        if (newtr[mod(i+2)]->connect.hvtx[i])
          newtr[mod(i+2)]->connect.hvtx[i]->nbconnect(opposite(i),newtr[mod(i+2)],i);
      }
      else 
      {
        assert(neighbour(i)->splitrule() == thinelement_t::unsplit);
        newtr[mod(i+1)]->nbconnect(i,neighbour(i),opposite(i));
        newtr[mod(i+2)]->nbconnect(i,neighbour(i),opposite(i));
      }
    }
  }
  return 4;
}

template < int N, int NV >
bool Triang < N,NV >::canCoarsen(int nconfDeg) const
{
  bool ret;

  ret = ( (   (splitrule() == thinelement_t::triang_quarter)
        || (neighbour(0)->splitrule() == thinelement_t::triang_quarter)) 
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
template < int N, int NV >
int Triang < N,NV >::docoarsen(nconf_vtx_t *ncv,
                               int nconfDeg,restrict_basic_t *rest_el)
{
  int result = 0;
  if( nconfDeg == 0 && mysplit == thinelement_t::triang_conf2) 
  {
    int lcancoarsen=0;
    if ( connect.nb[0]->thinis(thinelement_t::bndel_like) )
    {
      if ( ((hbndel_t*)connect.nb[0])->docoarsen(ncv,nconfDeg,rest_el) )
      {
        helement_t *child=down();
        child->neighbour(0)->nbconnect(child->opposite(0),this,2);
        this->nbconnect(2,child->neighbour(0),child->opposite(0));
        child=child->next();
        child->neighbour(0)->nbconnect(child->opposite(0),this,1);
        this->nbconnect(1,child->neighbour(0),child->opposite(0));
        if( neighbour(1)->thinis(thinelement_t::element_like) ) {
          ((element_t*)neighbour(1))->setnormdir(opposite(1), 1);
          setnormdir(1, -1);
        } else
          setnormdir(1, 1);
        if( neighbour(2)->thinis(thinelement_t::element_like) ) {
          ((element_t*)neighbour(2))->setnormdir(opposite(2), 1);
          setnormdir(2, -1);
        } else
          setnormdir(2, 1);
        lcancoarsen=1;
      }
    }
    else if ( connect.bck[0] == 0 )
    {
      helement_t *neigh=(helement_t *)connect.nb[0];
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
        helement_t *child=down();
        ncv = new nconf_vtx_t(child->vertex(0),child,child->next());
        assert( ncv );
        child->neighbour(1)->nbconnect(2,this,-1);
        child->neighbour(0)->nbconnect(child->opposite(0),this,2);
        this->nbconnect(2,child->neighbour(0),child->opposite(0));
        if( child->neighbour(0)->thinis(thinelement_t::element_like) ) {
          ((element_t *)neighbour(2))->setnormdir(opposite(2), 1);
          setnormdir(2, -1);
        } else
          setnormdir(2, 1);

        child=child->next();
        child->neighbour(2)->nbconnect(1,this,-1);
        child->neighbour(0)->nbconnect(child->opposite(0),this,1);
        this->nbconnect(1,child->neighbour(0),child->opposite(0));
        if( child->neighbour(0)->thinis(thinelement_t::element_like) ) {
          ((element_t *)neighbour(1))->setnormdir(opposite(1), 1);
          setnormdir(1, -1);
        } else
          setnormdir(1, 1);
        setnormdir(0, -1);
              
#ifndef NDEBUG
        // only used in assert 
        bool didcoarse = 
#endif
          // do coarsen 
          ((helement_t *)connect.nb[0])->docoarsen(ncv,nconfDeg,rest_el);
          assert( didcoarse );
          lcancoarsen=1;
        }
      }
      else
      {
        helement_t *child=down();
        child->neighbour(0)->nbconnect(child->opposite(0),this,2);
        this->nbconnect(2,child->neighbour(0),child->opposite(0));
        if( child->neighbour(0)->thinis(thinelement_t::element_like) ) {
          ((element_t *)neighbour(2))->setnormdir(opposite(2), 1);
          setnormdir(2, -1);
        } else
          setnormdir(2, 1);

        child=child->next();
        child->neighbour(0)->nbconnect(child->opposite(0),this,1);
        this->nbconnect(1,child->neighbour(0),child->opposite(0));
        if( child->neighbour(0)->thinis(thinelement_t::element_like) ) {
          ((element_t*)neighbour(1))->setnormdir(opposite(1), 1);
          setnormdir(1, -1);
        } else
          setnormdir(1, 1);
        setnormdir(0, 1);

        delete ncv;
        ncv=0;
        element_t *e[2];
        e[0]=down();
        e[1]=down()->next();
        // rest_el->operator()(this,e,2);
        if (rest_el) 
          rest_el->operator()(this);
        lcancoarsen=2;
      }
    }
    if( lcancoarsen != 0 && rest_el) {
      element_t *e[2];
      e[0]=down();
      e[1]=down()->next();
      //rest_el->operator()(this,e,2);
      if (rest_el)
        rest_el->operator()(this);
    }
    if (lcancoarsen==2) {
      deletesubtree();
      mysplit = thinelement_t::unsplit;
    }
    result = lcancoarsen;
  } 
  // QUATERING
  else if (mysplit==thinelement_t::triang_quarter) 
  {
    int lcancoarsen=1;
    element_t* child[ NV ];
    child[ 0 ] = down();
    for(int nv=1; nv <numfaces(); ++nv )
    {
      assert( child[ nv - 1 ] );
      child[ nv ] = ((helement_t *) child[ nv - 1 ])->next();
      assert( child[ nv ] );
    }

    for (int i=0;i<numfaces();i++) {
      if (nbel(i)) {
        if (!(nbel(i)->leaf()) && child[mod(i+1)]->hashvtx(i)) {
          if (((Triang*)child[mod(i+1)])->connect.hvtx[i]->count()+1>nconfDeg)
            lcancoarsen=0;
        }
        if (!(nbel(i)->leaf()) && child[mod(i+2)]->hashvtx(i)) {
          if (((Triang*)child[mod(i+2)])->connect.hvtx[i]->count()+1>nconfDeg)
            lcancoarsen=0;
        }
      } 
    }
    if (lcancoarsen) {
      if (rest_el) 
        rest_el->operator()(this);
      // Nachbarschaften stimmen (hoffe ich)
      // Haengende Knoten verwalten
      for (int i=0;i<numfaces();i++) {
        assert(!hashvtx(i));
        if (nbel(i)) {
          if (!(nbel(i)->leaf())) { // Haengenden Knoten erzeugen 
            this->addhvtx(child[mod(i+1)]->vertex(i+2), 
                    child[mod(i+2)]->nbel(i),child[mod(i+1)]->nbel(i), i);
            connect.hvtx[i]->merge(((Triang*)child[mod(i+1)])->connect.hvtx[i],
                     ((Triang*)child[mod(i+2)])->connect.hvtx[i]);
            connect.hvtx[i]->nbconnect(opposite(i),this,i);
          } else { // Haengenden Knoten im Nachbarn entfernen 
            assert(nbel(i)->hashvtx(opposite(i)));
            nbel(i)->removehvtx(opposite(i),child[mod(i+1)]->vertex(i+2));
          }
        }
      }
      for (int i=0;i<numfaces();i++) {
        if (nbbnd(i)) {
          if( !(nbbnd(i)->docoarsen(ncv,nconfDeg,rest_el)) ) {
            lcancoarsen=0;
            cout << "Error in coarsening!" << endl;
            abort();
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

// **************************************************************

template < int N, int NV >
Bndel_triang < N,NV >::Bndel_triang(vertex_t * v0, vertex_t * v1, bnd_t type) : time(0.0) {

  connect.set(v0, 0) ;

  connect.set(v1, 1) ;

  bndel_t::typ = type ;

}

template < int N, int NV >
void Bndel_triang < N,NV >::write(ostream & out) const {

  out << (int) type() << "  " ;

  connect.write(out) ;

  out << endl ;
  
}

template < int N, int NV >
void Bndel_triang < N,NV >::read(istream & in, vertex_t ** v, const int nv) {

  int c ;

  in >> c ;

  bndel_t::typ = (typename bndel_t::bnd_t) c ;

  connect.read(in, v, nv) ;

}

template < int N, int NV >
void Bndel_triang < N,NV >::restrictLocal(bndel_t **pbel, int pnr)
{
  assert(   ((Bndel_triang *)(pbel[0]))->time
      == ((Bndel_triang *)(pbel[1]))->time);

  time = ((Bndel_triang *)(pbel[0]))->time; 
}

template < int N, int NV >
void Bndel_triang < N,NV >::prolongLocal(bndel_t **pbel, int pnr) const
{
  for (int i=0;i<pnr;i++)
    ((Bndel_triang*)(pbel[i]))->time = time;
}

template < int N, int NV >
int Bndel_triang < N,NV >::split(void * (&e)[Basic::nparts], Listagency < vertex_t > * agnc,
                                 multivertexadapter_t & mva, 
                                 nconf_vtx_t *ncv,
                                 splitrule_t sr,
                                 int nconfDeg,Refco::tag_t default_ref,
                                 prolong_basic_t *pro_el)
{
  int idx[2];
  //  nconf_vtx_t *ncv;
  IndexProvider* hdl = this->gethdl();

  assert(sr == thinelement_t::triang_bnd);
  assert(splitrule() == thinelement_t::unsplit);
  mysplit = thinelement_t::triang_bnd;

  // create new elements
  
  assert(ncv);
  bndel_t *t1=create(connect.vtx[0],ncv->vtx,bndel_t::typ);
  bndel_t *t2=create(ncv->vtx,connect.vtx[1],bndel_t::typ);

  // determine opposite vertices

  switch (nbel(0)->splitrule())
    {
    case thinelement_t::triang_conf2:
      idx[0] = 1;
      idx[1] = 2;
      break;
    case thinelement_t::triang_quarter:
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
  ((element_t *)ncv->el[0])->setnormdir(idx[0],1);
  ((element_t *)ncv->el[1])->setnormdir(idx[1],1);
  t1->nbconnect(0,ncv->el[0],idx[0]);
  t2->nbconnect(0,ncv->el[1],idx[1]);

#ifdef ALU2D_OLD_BND_PROJECTION  
  t1->set_bndfunctions(lf,lDf);
  t2->set_bndfunctions(lf,lDf);
#else 
  t1->copySegmentIndex( _segmentIndex );
  t2->copySegmentIndex( _segmentIndex );
#endif
  //      mva.delete(connect.vtx[1],connect.vtx[2]);    
  
  // Daten auf dem Rand prolongieren
  t1->sethdl( hdl );
  t2->sethdl( hdl );

  t1->edgeconnect(0,ncv->el[0]->edge(idx[0]));
  t2->edgeconnect(0,ncv->el[1]->edge(idx[1]));
  e[0]=t1;
  e[1]=t2;
  prolongLocal((bndel_t **)e,2);

  delete ncv;
  ncv=NULL;

  return 2;
}

template < int N, int NV >
int Bndel_triang < N,NV >::docoarsen(nconf_vtx_t *ncv,
          int nconfDeg,restrict_basic_t *rest_el)
{
  assert(splitrule() == thinelement_t::triang_bnd);

  // Daten auf dem Rand restringieren
  bndel_t *bel[2];
  bel[0]=down();
  bel[1]=down()->next();
  restrictLocal(bel,2);
  deletesubtree();
  mysplit = thinelement_t::unsplit;
  return 1;  
} 

int periodic_flag=0;


template < int N, int NV >
void Bndel_periodic < N,NV >::write(ostream &out) const
{
  assert( periodic_nb );
  out << bndel_t::general_periodic << "  ";
  connect.write(out);

  // write out index of periodic neighbor
  // (boundary segments are written in the order of their segment indices)
  out << "  " << periodic_nb->segmentIndex();
  out << endl;
}


template < int N, int NV >
void Bndel_periodic < N,NV >::read(istream &in, vertex_t ** v, const int nv)
{
  int typ;
  in >> typ;
  if( (typ != bndel_t::periodic) && (typ != bndel_t::general_periodic) )
  {
    cerr << "Error: Trying to read non-periodic boundary into a periodic one." << endl;
    abort();
  }
  connect.read(in, v, nv);
}


template < int N, int NV >
int Bndel_periodic < N,NV >::split(void * (&el)[Basic::nparts], Listagency < vertex_t > * agnc,
                                   multivertexadapter_t & mva, 
                                   nconf_vtx_t *ncv, 
                                   splitrule_t sr,
                                   int nconfDeg,Refco::tag_t default_ref,
                                   prolong_basic_t *pro_el)
{
  assert(sr == thinelement_t::triang_bnd);
  assert(splitrule() == thinelement_t::unsplit);

  if( nconfDeg == 0 ) {
    bndel_triang_t::split(el,agnc,mva,ncv,thinelement_t::triang_bnd,nconfDeg,default_ref,pro_el); 
    if (periodic_flag)
      return 2;
    periodic_flag=1;
    while (periodic_nb->leaf()) {
      switch (nbel(0)->splitrule())
      {
        case thinelement_t::triang_conf2:
          periodic_nb->nbel(0)->Refco_el::mark(Refco::ref_1) ;
          break;
        case thinelement_t::triang_quarter:
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

#ifdef PERIODIC_VERTICES
    ((Bndel_periodic*)(el[0]))->vertex(1)
      ->set_pernb(((Bndel_periodic*)(periodic_nb->down()))->vertex(1));

    ((Bndel_periodic*)(periodic_nb->down()))->vertex(1)
      ->set_pernb(((Bndel_periodic*)(el[0]))->vertex(1));
#endif
  } else {

    // split

    bndel_triang_t::split(el,agnc,mva,ncv,thinelement_t::triang_bnd,nconfDeg,default_ref,pro_el);

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

template < int N, int NV >
int Bndel_periodic < N,NV >::docoarsen(nconf_vtx_t *ncv,
                                       int nconfDeg,restrict_basic_t *rest_el)
{
  if(nconfDeg == 0 ) {
    if (periodic_nb->leaf())
      return bndel_triang_t::docoarsen(ncv,nconfDeg,rest_el);
    // Kann anderer Rand auch vergr"obert werden?  
    if (periodic_nb->opposite(0)!=0)
      return 0;
    assert(periodic_nb->nbel(0));
    assert(!(periodic_nb->nbel(0)->leaf()));    
    helement_t *h=periodic_nb->nbel(0)->down();

    while (h)
      {
  if (!h->is(Refco::crs))
    break;
  h=h->next();
      }
    if (h)
      return 0;
    // es kann verfeinert werden! 
    bndel_triang_t::docoarsen(ncv,nconfDeg,rest_el);  
    periodic_nb->nbel(0)->coarse(ncv,nconfDeg,rest_el);
    assert(periodic_nb->nbel(0)->leaf());
    assert(periodic_nb->leaf());  
  } else {
    cout << "nicht-konformes Vergroebern noch nicht implementiert!" << endl;
    abort();
  }
  
  return 1;  
}

// ------------------------------------------------------------
// Template Instantiation
// ------------------------------------------------------------
template class Triang < 2,3 >;
template class Bndel_triang < 2,3 >;
template class Bndel_periodic < 2,3 >;

template class Triang < 3,3 >;
template class Bndel_triang < 3,3 >;
template class Bndel_periodic < 3,3 >;

template class Triang < 2,4 >;
template class Bndel_triang < 2,4 >;
template class Bndel_periodic < 2,4 >;

template class Triang < 3,4 >;
template class Bndel_triang < 3,4 >;
template class Bndel_periodic < 3,4 >;
