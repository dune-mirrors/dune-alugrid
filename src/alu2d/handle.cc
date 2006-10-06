
static volatile char RCSId_handle_cc [] = "$Id$";

#include <stdlib.h>
#include <assert.h>
#include <malloc.h>

#ifdef _ANSI_HEADER
  using namespace std;
  #include <iostream>
#else
  #include <iostream.h>
#endif

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
Hmesh::Hmesh(const char *macroname,int pnconfDeg) :
  _nconfDeg(pnconfDeg), refinement_rule(Refco::quart) {
  setup_grid(macroname);    
}
Hmesh::Hmesh(const char *macroname,Refco::tag_t pref_rule) :
  _nconfDeg(0), refinement_rule(pref_rule) {
  setup_grid(macroname);    
}

void Hmesh::setup_grid(const char *macroname) {
  ncv=NULL;
  adp = new Multivertexadapter;
  _pro_el=0;  // new Prolong_basic;
  _rest_el=0; // new Restrict_basic;

  double time;
  long unsigned int nbr;
  bool restart=ascireadtriang (macroname,time,nbr);
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
  /* END: set periodic neighbours of vertices */
  if (restart) {
    double time2 = time;
    long unsigned int nbr2 = nbr;
    recoverGrid(macroname,time2,nbr2);
    if (fabs(time2-time) + fabs(nbr2-nbr)>1e-5) {
      cerr << "ERROR in Hmesh::setup_grid: "
	   << "backup-file and macro-grid file not compatible" << endl;
      abort();
    }
  }
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

Hmesh *mesh;

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
  mesh=this;

  assert( ! mel.busy()) ;

  assert( ! mbl.busy()) ;

  assert( ! vl.busy()) ;

  int count = 0 ;

  Listwalk_impl <macroelement_t> walk(mel);
  for( walk.first() ; !walk.done() ; walk.next() )
    walk.getitem()->clearAllWas();  
  
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
  mesh=this;

  assert( ! mel.busy()) ;

  assert( ! mbl.busy()) ;

  assert( ! vl.busy()) ;

  {

    Listwalk_impl < macroelement_t > walk(mel) ;

    for(walk.first(); ! walk.done() ; walk.next()) {
      walk.getitem()->coarse(ncv,_nconfDeg,_rest_el) ;
    }
      
  }

  {	// sch.... konstruktion 

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

void get_memory(struct mallinfo &old_info)
{
  old_info = mallinfo();
}
void get_memory_diff(struct mallinfo &old_info)
{
  struct mallinfo meminfo = mallinfo();
  old_info.arena -= meminfo.arena;
  old_info.ordblks -= meminfo.ordblks;
  old_info.smblks -= meminfo.smblks;
  old_info.hblks -= meminfo.hblks;
  old_info.hblkhd -= meminfo.hblkhd;
  old_info.usmblks -= meminfo.usmblks;
  old_info.fsmblks -= meminfo.fsmblks;
  old_info.uordblks -= meminfo.uordblks;
  old_info.fordblks -= meminfo.fordblks;
  old_info.keepcost -= meminfo.keepcost;
}
void write_memory(ostream &out,char *header)
{
  struct mallinfo meminfo = mallinfo();
  out << "*********************************" << endl
      << header << endl
      << "space in small blocks in use:    " << meminfo.usmblks << endl
      << "space in ordinary blocks in use: " << meminfo.uordblks << endl
      << "space in free small blocks:      " << meminfo.fsmblks << endl
      << "space in free ordinary blocks    " << meminfo.fordblks << endl
      << "*********************************" << endl;
}
void write_memory(ostream &out,char *header,struct mallinfo &old_info)
{
  old_info = mallinfo();
  out << "*********************************" << endl
      << header << " (difference)" << endl
      << "space in small blocks in use:    " << old_info.usmblks << endl
      << "space in ordinary blocks in use: " << old_info.uordblks << endl
      << "space in free small blocks:      " << old_info.fsmblks << endl
      << "space in free ordinary blocks    " << old_info.fordblks << endl
      << "*********************************" << endl;
}
void write_memory_diff(ostream &out,char *header,struct mallinfo &old_info)
{
  struct mallinfo meminfo = mallinfo();
  old_info.arena -= meminfo.arena;
  old_info.ordblks -= meminfo.ordblks;
  old_info.smblks -= meminfo.smblks;
  old_info.hblks -= meminfo.hblks;
  old_info.hblkhd -= meminfo.hblkhd;
  old_info.usmblks -= meminfo.usmblks;
  old_info.fsmblks -= meminfo.fsmblks;
  old_info.uordblks -= meminfo.uordblks;
  old_info.fordblks -= meminfo.fordblks;
  old_info.keepcost -= meminfo.keepcost;
  out << "*********************************" << endl
      << header << " (difference)" << endl
      << "space in small blocks in use:    " << old_info.usmblks << endl
      << "space in ordinary blocks in use: " << old_info.uordblks << endl
      << "space in free small blocks:      " << old_info.fsmblks << endl
      << "space in free ordinary blocks    " << old_info.fordblks << endl
      << "*********************************" << endl;
}
