#include "grid.h"
#include "handle.h"
#include "triang.h"
#include "vmmap.h"

template < int N, int NV >
Hmesh<N,NV>::Hmesh() : _nconfDeg(-1), refinement_rule(Refco::none),
     _pro_el(0), _rest_el(0) {
}

template < int N, int NV >
Hmesh<N,NV>::Hmesh(const std::string &macroname, int pnconfDeg, Refco::tag_t pref_rule) :
  _nconfDeg(pnconfDeg), refinement_rule(pref_rule)
{
  setup_grid(macroname);      
}

template < int N, int NV >
Hmesh<N,NV>::Hmesh(istream& gridfile, int pnconfDeg, Refco::tag_t pref_rule) 
  : _nconfDeg(pnconfDeg), refinement_rule(pref_rule) 
{
  double time;
  long unsigned int nbr;

  bool restart = setup_grid( gridfile, time, nbr);
  if( restart ) 
  { 
    recoverGrid( gridfile );
  }
}

template < int N, int NV >
Hmesh<N,NV>::Hmesh(const std::string &macroname,int pnconfDeg) :
  _nconfDeg(pnconfDeg), refinement_rule(Refco::quart) 
{
  setup_grid(macroname);    
}

template < int N, int NV >
Hmesh<N,NV>::Hmesh(const std::string &macroname, Refco::tag_t  pref_rule) :
  _nconfDeg(0), refinement_rule(pref_rule) 
{
  setup_grid(macroname);    
}

template < int N, int NV >
void Hmesh<N,NV>::setup_grid(const std::string &filename) 
{
#ifndef NDEBUG 
  cerr << "\n  Hmesh_basic::ascireadtriang(?) opens: " ;
  cerr << filename << "\n" << endl ;
#endif

  ifstream in;
  in.open(filename.c_str(), ios::in);
  if( ! in.good() )
  {
    in.clear();
    string macro = filename + ".macro";
    cerr << "Warning: file \"" << filename << "\" not found, trying \"" << macro
         << "\"." << endl;
    in.open(macro.c_str(), ios::in);
  }
  assert(in);

  double time;
  long unsigned int nbr;

  // call setup with istream 
  const bool restart = setup_grid(in, time, nbr);

  // if restart we have to read the hierarchy 
  if( restart )
  {
    recoverGrid( in );
  }
}

template < int N, int NV >
bool Hmesh<N,NV>::setup_grid(istream &macrofile, double &time, long unsigned int &nbr) 
{
  ncv=NULL;
  adp = new multivertexadapter_t;
  _pro_el=0;  // new Prolong_basic;
  _rest_el=0; // new Restrict_basic;

  bool restart = ascireadtriang (macrofile,time,nbr);

  /* set periodic neighbours of vertices */
#ifdef PERIODIC_VERTICES
  {
    Listwalkptr < hbndel_t > walkb(*this);
    for (walkb->first();!walkb->done();walkb->next())
    {
      bndel_triang_t *bel = (bndel_triang_t *)&(walkb->getitem());
      if (bel->type() == bndel_t::periodic)
      {
        assert(vertex_t::ncoord == 2);
        bndel_triang_t *nbbel = ((bndel_periodic_t *)bel)->periodic_nb;
        assert(nbbel);
        bel->vertex(0)->set_pernb(nbbel->vertex(1));
        bel->vertex(1)->set_pernb(nbbel->vertex(0));
      }
    }
  }

  // consider periodic vertices along diagonal
  {
    Listwalkptr < vertex_t > walkv(*this);
    for (walkv->first(); !walkv->done(); walkv->next())
    {
      vertex_t *v = (vertex_t *)&walkv->getitem();
      if (v->get_nr_of_per_nbs() == 2)
      {
        for (int i = 0; i<2; ++i)
        {
          vertex_t *pnv = v->get_pernb(i);
          for (int j=0; j < pnv->get_nr_of_per_nbs(); ++j)
            v->set_pernb(pnv->get_pernb(j));
        }
        assert(v->get_nr_of_per_nbs() == 3);
      }
    }
  }
#endif

  return restart;
}

template < int N, int NV >
Hmesh<N,NV>::~Hmesh() {

  delete _pro_el;
  delete _rest_el;
  delete adp ;

  assert(ncv==0);

}

template < int N, int NV >
void Hmesh<N,NV>::refresh() { 

  Listwalk_impl < macroelement_t > walk (mel) ;

  adp->refresh(walk) ;

} 

template < int N, int NV >
bool Hmesh<N,NV>::checkConf()
{
  bool elem_marked = false;
  Listwalkptr< helement_t > walk(*this); // Leafwalk
  for( walk->first() ; !walk->done() ; walk->next() ) {
    triang_t *item = (triang_t *)&walk->getitem();
    if( item->confLevelExceeded(_nconfDeg) ) {
      item->mark(refinement_rule);
    }
    if (item->is(Refco::quart) || item->is(Refco::ref_1) || item->is(Refco::ref_2) ) {
      elem_marked = true;
    }
  }
  return elem_marked;
}

#ifdef USE_ALUGRID_XDISPLAY
// for displaying during refinement and coarsening
Hmesh *mesh;
#endif

template < int N, int NV >
class RestrictDune : public Restrict_basic < N, NV >
{
  AdaptRestrictProlong2d < N, NV > &restop;
  public:
  typedef Hier < Element < N, NV > > helement_t;
  typedef Hier < Bndel < N, NV > > hbndel_t;

  RestrictDune(AdaptRestrictProlong2d < N, NV > &arp) : restop(arp) {}
  virtual ~RestrictDune() {}
  virtual void operator ()(helement_t *parent) {
    restop.preCoarsening(*parent);
  }
  virtual void operator ()(hbndel_t *parent) {
  }
};

template < int N, int NV >
class ProlongDune : public Prolong_basic < N,NV >
{
  AdaptRestrictProlong2d < N, NV > &restop;
  public:
  typedef Hier < Element < N, NV > > helement_t;
  typedef Hier < Bndel < N, NV > > hbndel_t;

  ProlongDune(AdaptRestrictProlong2d < N,NV > &arp) : restop(arp) {}
  virtual ~ProlongDune () {}
  virtual void operator ()(helement_t *parent) {
    restop.postRefinement(*parent);
  }
  virtual void operator ()(hbndel_t *parent) {
  }
};

template < int N, int NV >
bool Hmesh<N,NV>::duneAdapt(AdaptRestrictProlong2dType & arp) {
  ProlongDune < ncoord,nvtx > produne(arp);
  RestrictDune < ncoord,nvtx > restdune(arp);
  prolong_basic_t *pro_el_old = _pro_el;
  restrict_basic_t *rest_el_old = _rest_el;
  _pro_el=&produne;
  _rest_el=&restdune;
  this->refine ();
  this->coarse () ;
  _pro_el=pro_el_old;
  _rest_el=rest_el_old;
  return true;
}

template < int N, int NV >
void Hmesh<N,NV>::refine() {
#ifdef USE_ALUGRID_XDISPLAY
  mesh=this;
#endif

  assert( ! mel.busy()) ;
  assert( ! mbl.busy()) ;
  assert( ! vl.busy()) ;

  int count = 0 ;

  //Listwalk_impl <macroelement_t> walk(mel);
  //for( walk.first() ; !walk.done() ; walk.next() )
  //  walk.getitem()->clearAllWas();  
  
  do {
    int lcount = 0;
    Listwalk_impl <macroelement_t> walk(mel);
    for (walk.first() ; !walk.done() ; walk.next())
      lcount += walk.getitem()->refine(&vl, adp,ncv,_nconfDeg,refinement_rule,_pro_el) ;
    count += lcount ;
    for (walk.first() ; !walk.done() ; walk.next())
      lcount += walk.getitem()->refine(&vl, adp,ncv,_nconfDeg,refinement_rule,_pro_el) ;
    count += lcount ;
   } while( checkConf() ) ;

  // renumber vertices
  vl.renumber() ;
}

template < int N, int NV >
void Hmesh<N,NV>::coarse() {
#ifdef USE_ALUGRID_XDISPLAY
  mesh=this;
#endif

  assert(!mel.busy()) ;
  assert(!mbl.busy()) ;
  assert(!vl.busy()) ;

  // walk over all macro elements and call hierarchic coarseining procedure
  {
    Listwalk_impl < macroelement_t > walk(mel) ;

    for(walk.first() ; !walk.done() ; walk.next()) {
      walk.getitem()->coarse(ncv,_nconfDeg,_rest_el) ;
    }
  }

  // remove all unused vertices
  {
    Listwalk_impl < vertex_t > walk (vl) ;

    for(walk.first() ; !walk.done() ; ) {
      vertex_t * v = & walk.getitem() ;
      walk.next() ;

      if (v->Basic::isfree()) {
        vl.detach(v) ;
        delete v ;
      }
    }
  }

  // renumber vertices
  vl.renumber() ;
}

template < int N, int NV >
void Hmesh<N,NV>::setdata(void (*f)(element_t &)) 
{
  Leafwalk < element_t > walk(mel) ;
  for (walk.first();walk.done();walk.next())
    f(walk.getitem());
}

// ------------------------------------------------------------
// Template Instantiation
// ------------------------------------------------------------
template class Hmesh_basic < 2,3 >;
template class Hmesh < 2,3 >;
template class Hmesh_basic < 3,3 >;
template class Hmesh < 3,3 >;

template class Hmesh_basic < 2,4 >;
template class Hmesh < 2,4 >;
template class Hmesh_basic < 3,4 >;
template class Hmesh < 3,4 >;

