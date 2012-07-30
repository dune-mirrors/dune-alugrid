#include "grid.h"

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
void Fullvertex < N >::write(ostream & out) const {

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
void Fullvertex < N >::read(istream & in) {

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

void Edge::write(ostream & out) const {

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

void Edge::read(istream & in) {

  in >> setIndex();

}

template < int N, int NV >
Element < N, NV >::Element()
: _area(-1.0)
{
  int i,j;

  for (i=0;i<NV;i++)
  {
    for (j=0;j<ncoord;++j)
      _outernormal[i][j] =  0.0;
  }
}

template < int N, int NV >
Element < N, NV >::~Element() 
{
  assert(_idx>=0);
  IndexProvider* hdl = gethdl();
  hdl->freeIndex(IndexProvider::IM_Elements,_idx);

  for (int i=0;i<NV;++i) 
  {
    if (connect.edge[i]) 
    {
      connect.edge[i]->detach();
      if (connect.edge[i]->isfree())
      {
        connect.edge[i]->freeIndex( hdl );
        delete connect.edge[i];
      }
    }
  }

  // reset refcount, otherwise assertion in Basic 
  nvertices() = 0;
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
void Element < N, NV >::c::write(ostream &out) const {
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
int Element < N, NV >::c::read(istream & in, vertex_t ** v, const int l) {
  string line;
  while (in && line == "")
    getline( in, line );
  istringstream linein( line );
  int i;
  for (i = 0; ; ++i)
  {
    int vtxNumber = 0 ;
    linein >> vtxNumber ;
    if ( !linein ) 
    {
      // we want at least 3 vertices (also for NV = 4)
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
    if ( (vtxNumber < 0) || (vtxNumber >= l) )
    {
      cerr << "Wrong vertex number for element read from file" << endl;
      abort();
    }
    set((vertex_t *)v[vtxNumber], i) ;
  }
  return i;
}


template < int N, int NV >
int Element < N, NV >::get_splitpoint(int fce, double pos, double (&ppoint) [ncoord]) const
{
  const double (&c0)[ncoord] = vertex(fce+1)->coord();
  const double (&c1)[ncoord] = vertex(fce+2)->coord();
  double npos = (normaldir(fce) == 1 ? pos : 1.-pos);
  for (int i=0;i<ncoord;++i)
    ppoint[i] = (1.-npos)*c0[i] + npos*c1[i];

#ifndef ALU2D_OLD_BND_PROJECTION

  double lpos[2];
  static double rcs3[][2] = {{0.,0.}, {1.,0.}, {0.,1.}};
  static double rcs4[][2] = {{0.,0.}, {1.,0.}, {1.,1.}, {0.,1.}};
  const double (&rc0)[2] = (nv() == 3 ? rcs3[(fce+1)%3] : rcs4[(fce+1)%4]);
  const double (&rc1)[2] = (nv() == 3 ? rcs3[(fce+2)%3] : rcs4[(fce+2)%4]);
  for (int i=0;i<2;++i)
    lpos[i] = (1.-npos)*rc0[i] + npos*rc1[i];

  // apply vertex projection, if existent 
  dynamic_cast< Hmesh<N,NV>* >(gethdl())->projectVertex( this, lpos, ppoint );

#endif

  return 0;
}


template < int N, int NV >
int Element < N, NV >::get_splitpoint(const double (&pos) [2], double (&ppoint) [ncoord]) const
{
  const double (&c0)[ncoord] = vertex(0)->coord();
  const double (&c1)[ncoord] = vertex(1)->coord();
  const double (&c2)[ncoord] = vertex(2)->coord();
  if( nv() == 3 )
  {
    for (int i=0;i<ncoord;++i)
      ppoint[i] = (1.-pos[0]-pos[1])*c0[i] + pos[0]*c1[i] + pos[1]*c2[i];
  }
  else
  {
    const double (&c3)[ncoord] = vertex(3)->coord();
    for (int i=0;i<ncoord;++i)
    {
      const double x0 = (1.-pos[0])*c0[i] + pos[0]*c1[i];
      const double x1 = (1.-pos[0])*c3[i] + pos[0]*c2[i];
      ppoint[i] = (1.-pos[1])*x0 + pos[1]*x1;
    }
  }

#ifndef ALU2D_OLD_BND_PROJECTION

  // apply vertex projection, if existent 
  dynamic_cast< Hmesh<N,NV>* >(gethdl())->projectVertex( this, pos, ppoint );

#endif

  return 0;
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
void Element < N, NV >::setrefine(int fce) 
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
void Element < N, NV >::init()
{ 
  /* calculate area */
  if ( ncoord == 2 && NV == 3 )
  {
    const double (&vc0)[ncoord]=connect.vtx[0]->coord();
    const double (&vc1)[ncoord]=connect.vtx[1]->coord();
    const double (&vc2)[ncoord]=connect.vtx[2]->coord();

    _area =  0.5 * (  (vc0[0]-vc1[0])*(vc0[1]+vc1[1])
                    + (vc1[0]-vc2[0])*(vc1[1]+vc2[1])
                    + (vc2[0]-vc0[0])*(vc2[1]+vc0[1])) ;
    const double alpha = (_area < 0)?-1:1;
    _area *= alpha;
    /* calculate outer normal and sidelength */
    for (int i=0;i<numfaces();i++)
    {
      _outernormal[i][0]= alpha*(vertex(i+2)->coord()[1]-vertex(i+1)->coord()[1]);
      _outernormal[i][1]=-alpha*(vertex(i+2)->coord()[0]-vertex(i+1)->coord()[0]);
    }
  }
  else
  {
    double sides[NV][ncoord];
    double sidelen[ NV ];
    /* calculate outer normal and sidelength^2 */
    for (int i=0;i<numfaces();i++)
    {
      sidelen[i] = 0;
      for (int k=0;k<ncoord;++k)
      {
        sides[i][k]= (vertex(i+2)->coord()[k]-vertex(i+1)->coord()[k]);
        sidelen[i] += sides[i][k]*sides[i][k];
      }
    }
    _area = 0;
    for (int i=0;i<numfaces();i++)
    {
      double scp_ds = 0;
      for (int k=0;k<ncoord;++k)
      {
        if ( numvertices() == 3)
          _outernormal[i][k] = sides[mod(i+2)][k] - sides[mod(i+1)][k];
        else
          _outernormal[i][k] = sides[mod(i+3)][k] - sides[mod(i+2)][k] - sides[mod(i+1)][k];
        scp_ds += _outernormal[i][k] * sides[i][k];
      }
      double norm_n = 0;
      for (int k=0;k<ncoord;++k)
      {
        _outernormal[i][k] = sidelen[i]*_outernormal[i][k] - scp_ds*sides[i][k];
        norm_n += _outernormal[i][k] * _outernormal[i][k];
      }
      norm_n = sqrt( norm_n );
      sidelen[i] = sqrt( sidelen[i] );
      double fac = sidelen[i]/norm_n;
      for (int k=0;k<ncoord;++k)
        _outernormal[i][k] *= fac;
      _area += 1./(4.*fac);
    }
    _area /= ( numvertices()==3 ? 3.:2. );
  }

  assert(_area > 0.0);
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
int Element < N, NV >::setrefine()
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
void Element < N, NV >::setorientation()
{
  double o;
  const int nf = numfaces();
  if (ncoord==2) 
  {
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
    if (NV == 4) 
    { // test that element is convex
      // note: for a convex cube all four value of o will have the same
      //       sign, for a non convex cube one value will have a different
      //       sign from the others
      for (int i=1;i<4;++i)
      {
        const double (&v0)[ncoord]=connect.vtx[(i+0)%nf]->coord();
        const double (&v1)[ncoord]=connect.vtx[(i+1)%nf]->coord();
        const double (&v2)[ncoord]=connect.vtx[(i+2)%nf]->coord();
        double oi=(v1[0]-v0[0])*(v2[1]-v1[1])-(v1[1]-v0[1])*(v2[0]-v1[0]);
        if (oi*o<=1e-10)
        {
          // one of the 4 o values had the wrong sign -> not convex
          std::cout << "Error: one the cubes used to generate ALUGrid is not convex - Aborting."
                    << std::endl;
          std::cout << "The verticies of the offending element are:" << std::endl;
          for (int i=0;i<4;++i)
            std::cout << "p_" << i << " = ( " 
                      << connect.vtx[i]->coord()[0] << " "
                      << connect.vtx[i]->coord()[1] 
                      << std::endl;
          abort();
        }
      }
    }
    assert(o);  // Entartet!
    if (o<0)
    {
      switchorientation(1,nf-1);
    }
  }
}

template < int N, int NV >
void Element < N, NV >::switchorientation(int a,int b)
{
  swap(connect.vtx[a],connect.vtx[b]);
  swap(connect.nb[a],connect.nb[b]);
  swap(connect.edge[a],connect.edge[b]);
  swap(connect.bck[a],connect.bck[b]);
  swap(connect.normdir[a],connect.normdir[b]);
  for (int i=0;i<ncoord;++i)
    swap(_outernormal[a][i],_outernormal[b][i]);

  connect.nb[a]->nbconnect(connect.bck[a],this,a);
  connect.nb[b]->nbconnect(connect.bck[b],this,b);

  if (numvertices() == 4)
  {
    vertex_t *tmpv=connect.vtx[0];
    connect.vtx[0]=connect.vtx[1];
    connect.vtx[1]=connect.vtx[2];
    connect.vtx[2]=connect.vtx[3];
    connect.vtx[3]=tmpv;
  }
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
void Element < N, NV >::addhvtx(vertex_t *invtx, thinelement_t *lnb, thinelement_t *rnb,int fce)
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

template < int N, int NV >
void Element < N, NV >::removehvtx(int fce,vertex_t *vtx)
{
  if (connect.hvtx[fce]->count()==1) 
  {
    assert(connect.hvtx[fce]->getHead()==vtx);
    delete connect.hvtx[fce];
    connect.hvtx[fce] = 0;
  } 
  else 
  {
#ifndef NDEBUG
    // only used in assert 
    bool found=
#endif
      connect.hvtx[fce]->remove(vtx);
    assert(found);
  }
}

template < int N, int NV >
Bndel < N, NV >::~Bndel() 
{
  IndexProvider* hdl = gethdl();
  assert(_idx>=0);
  hdl->freeIndex(IndexProvider::IM_Bnd,_idx);

  if (connect.edge) 
  {
    connect.edge->detach();
    if (connect.edge->isfree())
    {
      connect.edge->freeIndex( hdl );
      delete connect.edge;
    }
  }
}

template < int N, int NV >
int Bndel < N,NV >::facevertex(int , int j) const {
 
  assert(0 <= j) ;
  
  assert(j < connect.nv) ;
  
  return j ;
  
}

template < int N, int NV >
void Bndel < N,NV >::edge_vtx(int e, vertex_t * (&v) [c::nv]) const {

  assert(e < connect.nv) ;

  v[0] = connect.vtx[e ++ ] ;

  v[1] = connect.vtx[e == connect.nv ? 0 : e] ;

}

template < int N, int NV >
typename Bndel < N,NV >::vertex_t * Bndel < N,NV >::vertex(int i) const {

  assert(0 <= i);

  i%=connect.nv;
 
  return connect.vtx[i] ;
  
}

template < int N, int NV >
void Bndel < N,NV >::nbconnect(int fce, thinelement_t * n, int b) {

  assert(!fce) ;
 
  connect.nb = n ; 

  connect.bck = b ; 

}
template < int N, int NV >
void Bndel < N,NV >::edgeconnect(int fce, Edge * n) { 
  assert(0 <= fce) ;
  connect.edge = n ; 
  n->attach();
}

template < int N, int NV >
Bndel < N,NV >::c::c()  {

  for(int i = 0 ; i < Basic::max_points ; i ++ ) vtx[i] = 0 ;

  nb = 0 ;

  bck = -1 ;

  edge = 0;

}

template < int N, int NV >
Bndel < N,NV >::c::~c() {

  for(int i = 0 ; i < nv ; i ++ ) 

    if(vtx[i]) vtx[i]->detach() ;

}

template < int N, int NV >
void Bndel < N,NV >::c::write(ostream &out) const {

  // out << nv << "  " ;

  for( int i = 0 ; i < nv ; i ++ )

    out << vtx[i]->Listagent < vertex_t > :: number() << "  " ;
    
}

template < int  N, int NV >
void Bndel < N,NV >::c::read(istream & in, vertex_t ** v, const int l) {

  int c ;

  for(int i = 0 ; i < nv ; i ++) {

    in >> c ;

    assert(-1 <= c && c < l) ;

    if(c != -1) set((vertex_t *)v[c], i) ;
 
  }

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
int Bndel < N, NV >::get_splitpoint(int fce, double pos, double (&ppoint) [ncoord]) const
{
  assert( fce == 0 );

  const double (&c0)[ncoord] = connect.vtx[0]->coord();
  const double (&c1)[ncoord] = connect.vtx[1]->coord();

  for (int i=0;i<ncoord;++i)
    ppoint[i] = (1.-pos)*c0[i] + pos*c1[i];

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
  dynamic_cast< Hmesh<N,NV>* >(gethdl())->projectVertex( this, pos, ppoint );

#endif

  return 0;
}

template < int N, int NV >
int Bndel < N, NV >::setorientation()
{
  int ret = (connect.vtx[0] != connect.nb->vertex(connect.bck+1));
  if(ret)
  {
    vertex_t *tmpv=connect.vtx[0];
    connect.vtx[0]=connect.vtx[1];
    connect.vtx[1]=tmpv;
  }
  return ret;
}


template < class A >
Hier < A >::~Hier()
{
  if(dwn) delete dwn ;
  if(nxt) delete nxt ;
}

template < class A >
int Hier < A >::deepestLevel()
{
  SubtreeIterator<A> iter = stIterator();
  int currLevel = iter->level();
  int result = currLevel;
  while( ++iter ) {
    if( iter->level() > result )
    result = iter->level();
  }
  return result - currLevel;
}

template < class A >
int Hier < A >::coarse(nconf_vtx_t *ncv,int nconfDeg,restrict_basic_t *rest_el)
{
  int count=0 ;
  if (dwn && (dwn->coarse(ncv,nconfDeg,rest_el) == numchild)) {
    if (this->docoarsen(ncv,nconfDeg,rest_el)) {
      this->deletesubtree();
      this->mysplit = this->unsplit;
    }
  }
  count=(this->is(Refco::crs) ? 1 : 0) ;
  this->clear(Refco::crs) ;

  if (nxt)
    count+=nxt->coarse(ncv,nconfDeg,rest_el) ;

  return count ;
}

template < class A >
int Hier < A >::refine_leaf(Listagency < vertex_t > * a,multivertexadapter_t * b ,
                            nconf_vtx_t *ncv,int nconfDeg,Refco::tag_t default_ref,
                            prolong_basic_t *pro_el)
{
  assert(leaf());

  if (this->is(Refco::ref))
    this->mark(default_ref);

  if(this->is(Refco::quart) || this->is(Refco::ref_1) || this->is(Refco::ref_2) || this->thinis(this->bndel_like)) {
    void * els[A::nparts] ;

    if (this->is(Refco::quart))
    {
      numchild = this->split(els,a,*b,ncv,this->triang_quarter,nconfDeg,default_ref,pro_el);
      this->clear(Refco::quart);
    }
    else if (this->is(Refco::ref_1))
    {
      numchild = this->split(els,a,*b,ncv,this->triang_conf2,nconfDeg,default_ref,pro_el);
      this->clear(Refco::ref_1);
    }
    else if (this->is(Refco::ref_2))
    {
      numchild = this->split(els,a,*b,ncv,this->triang_conf2,nconfDeg,default_ref,pro_el);
      this->clear(Refco::ref_2);
    }
    else
    {
      assert(this->thinis(this->bndel_like));
      numchild = this->split(els,a,*b,ncv,this->triang_bnd,nconfDeg,default_ref,pro_el);
    }

    dwn = (Hier *)els[0] ;

    const unsigned char newLevel = level() + 1 ;
    dwn->lvl() = newLevel ;

    dwn->up = this;
    dwn->writeToWas();
    dwn->childNr_ = 0;

    for (int i=1 ; i<numchild ; ++i) {
      ((Hier *)els[i])->lvl() = newLevel ;

      ((Hier *)els[i])->up = this ;
      ((Hier *)els[i])->writeToWas();
      ((Hier *)els[i])->childNr_ = i;

      ((Hier *)els[i-1])->nxt = (Hier *)els[i] ;
    }

    if (pro_el)
      pro_el->operator()(this);

    //this->check();

    return numchild;
  } else
    return 0;
}

template < class A >
int Hier < A >::refine(Listagency < vertex_t > * a,multivertexadapter_t * b,
                       nconf_vtx_t *ncv,int nconfDeg,Refco::tag_t default_ref,
                       prolong_basic_t * pro_el)
{
  int count = 0 ;

  if (nxt)
    count += nxt->refine(a,b,ncv,nconfDeg,default_ref,pro_el) ;

  if (dwn)
    count += dwn->refine(a,b,ncv,nconfDeg,default_ref,pro_el) ;
  else {
    // Wegen rek. Aufbau der Dreiecksverf. ist eine Funktion n"otig, die Verf.
    // aber nicht u"ber den Baum l"auft. 
    // Bei Rekursivem Verf. stimmt R"uckgabe sowieso nicht

    count += refine_leaf(a,b,ncv,nconfDeg,default_ref,pro_el) ; 
  }

  return count ;
}




// ------------------------------------------------------------
// Template Instantiation
// ------------------------------------------------------------
template class Fullvertex < 2 >;
template class Fullvertex < 3 >;

template class Element < 2,3 >;
template class Bndel < 2,3 >;
template class Hier < Element < 2,3 > >;
template class Hier < Bndel < 2,3 > >;

template class Element < 3,3 >;
template class Bndel < 3,3 >;
template class Hier < Element < 3,3 > >;
template class Hier < Bndel < 3,3 > >;

template class Element < 2,4 >;
template class Bndel < 2,4 >;
template class Hier < Element < 2,4 > >;
template class Hier < Bndel < 2,4 > >;

template class Element < 3,4 >;
template class Bndel < 3,4 >;
template class Hier < Element < 3,4 > >;
template class Hier < Bndel < 3,4 > >;
