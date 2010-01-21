#include "grid.h"
#include "handle.h"
#include "triang.h"
#include "vmmap.h"

Hmesh::Hmesh() : _nconfDeg(-1), refinement_rule(Refco::none),
     _pro_el(0), _rest_el(0) {
}

Hmesh::Hmesh(const char *macroname,int pnconfDeg,Refco::tag_t pref_rule) :
  _nconfDeg(pnconfDeg), refinement_rule(pref_rule) {
  setup_grid(macroname);      
}

Hmesh::Hmesh(istream& macrofile, int pnconfDeg, Refco::tag_t pref_rule) :
  _nconfDeg(pnconfDeg), refinement_rule(pref_rule) 
{
  double time;
  long unsigned int nbr;
#ifndef NDEBUG 
  cerr << "\n  Hmesh::Hmesh reads istream: " << endl ;
#endif

  bool restart = setup_grid(macrofile, time, nbr);
  if( restart ) 
  { 
    cerr << "ERROR: Hmesh constructor with invalid istream called!";
    abort();
  }
}

Hmesh::Hmesh(const char *macroname,int pnconfDeg) :
  _nconfDeg(pnconfDeg), refinement_rule(Refco::quart) 
{
  setup_grid(macroname);    
}

Hmesh::Hmesh(const char *macroname, Refco::tag_t  pref_rule) :
  _nconfDeg(0), refinement_rule(pref_rule) 
{
  setup_grid(macroname);    
}

void Hmesh::setup_grid(const char* filename) 
{
#ifndef NDEBUG 
  cerr << "\n  Hmesh_basic::ascireadtriang(?) opens: " ;
  cerr << filename << "\n" << endl ;
#endif

  ifstream in;
  in.open(filename, ios::in) ;

  if (!in.good()) {
    in.clear();
    string macro(filename);
    macro+=".macro";
    cerr << "Warning: file " << filename << " not found, trying " << macro << endl;
    in.open(macro.c_str(), ios::in) ;
  }
  assert(in) ;

  double time;
  long unsigned int nbr;

  // call setup with istream 
  bool restart = setup_grid(in, time, nbr);

  /* END: set periodic neighbours of vertices */
  if (restart) {
    double time2 = time;
    long unsigned int nbr2 = nbr;
    assert( false );
    recoverGrid(filename,time2,nbr2);
    if (fabs(time2-time) + 
        fabs((double (nbr2-nbr)))> 1e-5) {
      cerr << "ERROR in Hmesh::setup_grid: "
           << "backup-file and macro-grid file not compatible" << endl;
      abort();
    }
  }
}

bool Hmesh::setup_grid(istream& macrofile, double& time, long unsigned int& nbr) 
{
  ncv=NULL;
  adp = new Multivertexadapter;
  _pro_el=0;  // new Prolong_basic;
  _rest_el=0; // new Restrict_basic;

  bool restart = ascireadtriang (macrofile,time,nbr);

  /* set periodic neighbours of vertices */
  {
    Listwalkptr < Hmesh_basic::hbndel_t > walkb(*this);
    for (walkb->first();!walkb->done();walkb->next())
    {
      Bndel_triang *bel = (Bndel_triang *)&(walkb->getitem());
      if (bel->type() == Bndel::periodic)
      {
        Bndel_triang *nbbel = ((Bndel_periodic *)bel)->periodic_nb;
        assert(nbbel);
        bel->vertex(0)->set_pernb(nbbel->vertex(1));
        bel->vertex(1)->set_pernb(nbbel->vertex(0));
      }
    }
  }
  // consider periodic vertices along diagonal
  {
    Listwalkptr < Vertex > walkv(*this);
    for (walkv->first();!walkv->done();walkv->next())
    {
      Vertex *v = (Vertex *)&walkv->getitem();
      if (v->get_nr_of_per_nbs() == 2)
      {
        int i,j;
        for (i=0;i<2;i++)
        {
          Vertex *pnv = v->get_pernb(i);
          for (j=0;j<pnv->get_nr_of_per_nbs();j++)
            v->set_pernb(pnv->get_pernb(j));
        }
        assert(v->get_nr_of_per_nbs() == 3);
      }
    }
  }

  return restart;
}

Hmesh::~Hmesh() {

  delete _pro_el;
  delete _rest_el;
  delete adp ;

  assert(ncv==0);

}

void Hmesh::refresh() { 

  Listwalk_impl < Macro < Element > > walk (mel) ;

  adp->refresh(walk) ;

} 

bool Hmesh::checkConf()
{
  bool elem_marked = false;
  Listwalkptr<Hmesh_basic::helement_t> walk(*this); // Leafwalk
  for( walk->first() ; !walk->done() ; walk->next() ) {
    if( ((Triang*)&walk->getitem())->confLevelExceeded(_nconfDeg) ) {
      walk->getitem().mark(refinement_rule);
    }
    if (((Triang*)&walk->getitem())->is(Refco::quart) ||
  ((Triang*)&walk->getitem())->is(Refco::ref_1) ||
  ((Triang*)&walk->getitem())->is(Refco::ref_2) ) {
      elem_marked = true;
    }
  }
  return elem_marked;
}

#ifdef USE_ALUGRID_XDISPLAY
// for displaying during refinement and coarsening
Hmesh *mesh;
#endif

class RestrictDune : public Restrict_basic
{
  AdaptRestrictProlong2dType & restop;
  public: 
  RestrictDune(AdaptRestrictProlong2dType & arp) : restop(arp) {}
  virtual ~RestrictDune() {}
  virtual void operator ()(Hier<Element> *parent) {
    restop.preCoarsening(*parent);
  }
  virtual void operator ()(Hier<Bndel> *parent) {
  }
};

class ProlongDune : public Prolong_basic
{
  AdaptRestrictProlong2dType & restop;
  public:
  ProlongDune(AdaptRestrictProlong2dType & arp) : restop(arp) {}
  virtual ~ProlongDune () {}
  virtual void operator ()(Hier<Element> *parent) {
    restop.postRefinement(*parent);
  }
  virtual void operator ()(Hier<Bndel> *parent) {
  }
};

bool Hmesh::duneAdapt(AdaptRestrictProlong2dType & arp) {
  ProlongDune produne(arp);
  RestrictDune restdune(arp);
  Prolong_basic *pro_el_old = _pro_el;
  Restrict_basic *rest_el_old = _rest_el;
  _pro_el=&produne;
  _rest_el=&restdune;
  this->refine ();
  this->coarse () ;
  _pro_el=pro_el_old;
  _rest_el=rest_el_old;
  return true;
}

void Hmesh::refine() {
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
    for( walk.first() ; !walk.done() ; walk.next() )
      lcount += walk.getitem()->refine(&vl, adp,ncv,_nconfDeg,refinement_rule,_pro_el);
    count += lcount;
    for( walk.first() ; !walk.done() ; walk.next() )
      lcount += walk.getitem()->refine(&vl, adp,ncv,_nconfDeg,refinement_rule,_pro_el);
    count += lcount;
   } while( checkConf() );

  vl.renumber() ;
  
  //cerr << "  refine() Anzahl der Elemente / Vertices : " ;

  //cerr << Leafwalk < Element > (mel).size() << "  " ;

  //cerr << Listwalk_impl < Vertex > (vl).size() << endl ;

}

void Hmesh::coarse() {
#ifdef USE_ALUGRID_XDISPLAY
  mesh=this;
#endif

  assert( ! mel.busy()) ;

  assert( ! mbl.busy()) ;

  assert( ! vl.busy()) ;

  {

    Listwalk_impl < macroelement_t > walk(mel) ;

    for(walk.first(); ! walk.done() ; walk.next()) {
      walk.getitem()->coarse(ncv,_nconfDeg,_rest_el) ;
    }
      
  }

  { // sch.... konstruktion 

    Listwalk_impl < Vertex > walk (vl) ;

    walk.first() ; 

    while(! walk.done()) {

      Vertex * curr = & walk.getitem() ;

      if(curr->Basic::isfree()) {

        walk.next() ;

        vl.detach(curr) ;

        delete curr ;

      }

      else walk.next() ;

    }

  }

  vl.renumber() ;
}

void Hmesh::setdata(void (*f)(Element &)) 
{
  Leafwalk < Element > walk(mel) ;
  for (walk.first();walk.done();walk.next())
    f(walk.getitem());
}
