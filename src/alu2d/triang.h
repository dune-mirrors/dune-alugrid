#ifndef __HEADER__TRIANG
#define __HEADER__TRIANG

#include "grid.h"

// ***************************************************
// #begin(header)
// #filename:
//   triang.h
// #description:
//   Headerfile f"ur 2D--Dreiecksgitter
// #classes:
//   class Triang : public Hier < Element >
//   class Bndel_triang   : public Hier < Bndel >
// #end(header)
// *************************************************** 

// ***************************************************
// #begin(class)
// #description:
//   2D--Dreieckselemente
// #definition:
template < int N, int NV >
class Triang : public Hier < Element < N,NV > > {

  public:

    enum { ncoord = N };

    typedef Vertex < ncoord > vertex_t;
    typedef Fullvertex < ncoord > fullvertex_t;
    typedef Multivertexadapter < ncoord, NV > multivertexadapter_t;
    typedef nconf_vtx < ncoord, NV > nconf_vtx_t;

    typedef Thinelement < ncoord, NV > thinelement_t;
    typedef typename thinelement_t::splitrule_t splitrule_t;

    typedef Element < ncoord,NV > element_t;
    typedef Bndel < ncoord, NV > bndel_t;

    typedef Hier < element_t > helement_t;
    typedef Hier < bndel_t > hbndel_t;

    typedef Bndel_triang < ncoord, NV > bndel_triang_t;
    typedef Bndel_periodic < ncoord, NV > bndel_periodic_t;

    typedef Prolong_basic < ncoord, NV > prolong_basic_t;
    typedef Restrict_basic < ncoord, NV > restrict_basic_t;

  protected:
    using element_t::mod;

    using helement_t::connect;
    using helement_t::hdl;
    using helement_t::level;
    using helement_t::mysplit;

  public:
    using element_t::numvertices;
    using element_t::numfaces;

    using helement_t::deletesubtree;
    using helement_t::down;
    using helement_t::hashvtx;
    using helement_t::init;
    using helement_t::leaf;
    using helement_t::nbbnd;
    using helement_t::nbel;
    using helement_t::neighbour;
    using helement_t::normaldir;
    using helement_t::opposite;
    using helement_t::setnormdir;
    using helement_t::splitrule;

    Triang();

    Triang(vertex_t *v1, vertex_t *v2, vertex_t *v3 );

    Triang(vertex_t *v1, vertex_t *v2, vertex_t *v3, vertex_t *v4 );

   ~Triang() { }


    void write(ostream & ) const ;

    void read(istream &, vertex_t ** , const int ) ;


  private:

    Triang(const Triang &);
    
    Triang & operator = (const Triang & ) ;

  protected:

    void newNeighbour(Triang *, int, int, splitrule_t, bool = false);

    bool canCoarsen(int) const;

    int split2tr(void *(&)[Basic::nparts], Listagency < vertex_t > *,
                 multivertexadapter_t &, nconf_vtx_t *ncv,
	               int,Refco::tag_t,prolong_basic_t *pro_el);
    int split4(void *(&)[Basic::nparts], Listagency < vertex_t > *,
               multivertexadapter_t &, nconf_vtx_t *ncv,
	             int,Refco::tag_t,prolong_basic_t *pro_el);
  public:

    int split(void *(&)[Basic::nparts], Listagency < vertex_t > *,
              multivertexadapter_t &, nconf_vtx_t *ncv,splitrule_t,
	            int,Refco::tag_t,prolong_basic_t *pro_el);
     
    int docoarsen(nconf_vtx_t *ncv,int,restrict_basic_t *rest_el);

    bool confLevelExceeded(int) const;

} ;
// #end(class)
// ***************************************************

// ***************************************************
// #begin(class)
// #description:
//   Randelemente f"ur 2D--Triangulierung
// #definition:
template < int N, int NV >
class Bndel_triang : public Hier < Bndel < N, NV > > {

  public:

    typedef Vertex < N > vertex_t;
    typedef nconf_vtx < N, NV > nconf_vtx_t;
    typedef Multivertexadapter < N, NV > multivertexadapter_t;

    typedef Thinelement < N, NV > thinelement_t;
    typedef typename thinelement_t::splitrule_t splitrule_t; 

    typedef Element < N,NV > element_t;

    typedef Bndel < N,NV > bndel_t;
    typedef typename bndel_t::bnd_t bnd_t;

    typedef Hier < element_t > helement_t;
    typedef Hier < bndel_t > hbndel_t;

    typedef Prolong_basic < N,NV > prolong_basic_t;
    typedef Restrict_basic < N,NV > restrict_basic_t;

  protected:
    using hbndel_t::connect;
    using hbndel_t::hdl;
    using hbndel_t::mysplit;
    using hbndel_t::_segmentIndex;

    double time;

    virtual void restrictLocal(bndel_t **, int);

    virtual void prolongLocal(bndel_t **, int) const;

  public :

    using hbndel_t::deletesubtree;
    using hbndel_t::down;
    using hbndel_t::nbel;
    using hbndel_t::opposite;
    using hbndel_t::splitrule;
    using hbndel_t::type;

    Bndel_triang(const int segmentIndex, bnd_t t)
    : time(0.0) 
    {
      bndel_t::typ = t;
      this->copySegmentIndex( segmentIndex );
    }

    // Bndel_triang() : time(0.0) {typ=-111;}

    Bndel_triang(vertex_t *, vertex_t *, bnd_t ) ;

   ~Bndel_triang() { }

   virtual bndel_t *create(vertex_t *v1 , vertex_t *v2, int ptyp) const
    {
      return new Bndel_triang(v1,v2,ptyp);
    }

    void write(ostream &) const ;

    void read(istream &, vertex_t ** , const int ) ;

    int split(void *(&)[Basic::nparts], Listagency < vertex_t > *,
              multivertexadapter_t &, nconf_vtx_t *ncv,splitrule_t,
	      int,Refco::tag_t,prolong_basic_t *pro_el);

    int  docoarsen(nconf_vtx_t *ncv,int,restrict_basic_t *rest_el);

} ;
// #end(class)
// ***************************************************
// ***************************************************
// #begin(class)
// #description:
//   Randelelemente f"ur periodischen Rand
// #definition:
template < int N, int NV >
class Bndel_periodic : public Bndel_triang < N,NV >
{
  public:

    typedef Vertex < N > vertex_t;
    typedef Multivertexadapter < N, NV > multivertexadapter_t;
    typedef nconf_vtx < N,NV > nconf_vtx_t;

    typedef Thinelement < N,NV > thinelement_t;
    typedef typename thinelement_t::splitrule_t splitrule_t;

    typedef Element < N,NV > element_t;
    typedef Bndel < N,NV > bndel_t;
    typedef Bndel_triang < N,NV > bndel_triang_t;

    typedef Hier < element_t > helement_t;

    typedef Prolong_basic < N,NV > prolong_basic_t;
    typedef Restrict_basic < N,NV > restrict_basic_t;

  protected:
    using bndel_t::connect;

  public:
    using bndel_triang_t::deletesubtree;
    using bndel_triang_t::down;
    using bndel_triang_t::leaf;
    using bndel_triang_t::nbel;
    using bndel_triang_t::opposite;
    using bndel_triang_t::splitrule;
    using bndel_triang_t::type;

    Bndel_periodic *periodic_nb;

    Bndel_periodic()
      : bndel_triang_t(-1,bndel_t::periodic), periodic_nb(0)
      { }

    Bndel_periodic(const int segmentIndex)
      : bndel_triang_t(segmentIndex, bndel_t::periodic), periodic_nb(0)
      { }

    Bndel_periodic(vertex_t *v1 , vertex_t *v2)
      : bndel_triang_t(v1,v2,bndel_t::periodic),periodic_nb(0)
      { }
    
    virtual double area() const {
      assert(periodic_nb);
      return periodic_nb->nbel(0)->area();
    }

    virtual bndel_t *create(vertex_t *v1 , vertex_t *v2, int) const
    {
      return new Bndel_periodic(v1,v2);
    }

    void set_pnb(bndel_t *pnb)
    {
      assert(pnb->type()==bndel_t::periodic);
      periodic_nb=(Bndel_periodic*)pnb;

      if( !leaf() ) {
        ((Bndel_periodic*)down())->set_pnb(pnb);
        ((Bndel_periodic*)(down()->next()))->set_pnb(pnb);
      }
    }

    void write(ostream &) const;
    void read(istream &, vertex_t ** , const int);

    virtual int split(void * (&el)[Basic::nparts], Listagency < vertex_t > * agnc,
                      multivertexadapter_t & mva, nconf_vtx_t *ncv,splitrule_t,
		      int,Refco::tag_t,prolong_basic_t *pro_el);

    virtual int docoarsen(nconf_vtx_t *ncv,int,restrict_basic_t *rest_el);
 
};
// #end(class)
// ***************************************************

#endif
