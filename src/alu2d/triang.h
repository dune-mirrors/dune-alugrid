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
class Triang : public Hier < Element > {

  public:

    Triang();

    Triang(Vertex *v1, Vertex *v2, Vertex *v3 );

   ~Triang() { }


    void write(ofstream & ) const ;

    void read(ifstream &, Vertex ** , const int ) ;


  private:

    Triang(const Triang &);
    
    Triang & operator = (const Triang & ) ;

  protected:

    void newNeighbour(Triang*, int, int, splitrule_t, bool = false);

    bool canCoarsen(int) const;

  public:

    int split(void *(&)[nparts], Listagency < Vertex > *,
              Multivertexadapter &, nconf_vtx_t *ncv,splitrule_t,
	      int,Refco::tag_t,Prolong_basic *pro_el);
     
    int docoarsen(nconf_vtx_t *ncv,int,Restrict_basic *rest_el);

    bool confLevelExceeded(int) const;

} ;
// #end(class)
// ***************************************************

// ***************************************************
// #begin(class)
// #description:
//   Randelemente f"ur 2D--Triangulierung
// #definition:
class Bndel_triang : public Hier < Bndel > {

  protected:

    double time;

    virtual void restrictLocal(Bndel **, int);

    virtual void prolongLocal(Bndel **, int) const;

  public :

    Bndel_triang(const int segmentIndex, 
                 Bndel::bnd_t t) 
      : time(0.0) 
    {
      typ = t ;
      this->copySegmentIndex( segmentIndex );
    }

    // Bndel_triang() : time(0.0) {typ=-111;}

    Bndel_triang(Vertex * ,Vertex * , bnd_t ) ;

   ~Bndel_triang() { }

   virtual Bndel *create(Vertex *v1 , Vertex *v2, int ptyp) const
    {
      return new Bndel_triang(v1,v2,ptyp);
    }

    void write(ofstream &) const ;

    void read(ifstream &, Vertex ** , const int ) ;

    int split(void *(&)[nparts], Listagency < Vertex > *,
              Multivertexadapter &, nconf_vtx_t *ncv,splitrule_t,
	      int,Refco::tag_t,Prolong_basic *pro_el);

    int  docoarsen(nconf_vtx_t *ncv,int,Restrict_basic *rest_el);

} ;
// #end(class)
// ***************************************************
// ***************************************************
// #begin(class)
// #description:
//   Randelelemente f"ur periodischen Rand
// #definition:
class Bndel_periodic : public Bndel_triang
{
  public:

    Bndel_periodic *periodic_nb;

    Bndel_periodic()
      : Bndel_triang(-1,periodic), periodic_nb(0)
      { }

    Bndel_periodic(const int segmentIndex)
      : Bndel_triang(segmentIndex, periodic), periodic_nb(0)
      { }

    Bndel_periodic(Vertex *v1 , Vertex *v2)
      : Bndel_triang(v1,v2,periodic),periodic_nb(0)
      { }
    
    virtual double area() const {
      assert(periodic_nb);
      return periodic_nb->nbel(0)->area();
    }

    virtual Bndel *create(Vertex *v1 , Vertex *v2, int) const
    {
      return new Bndel_periodic(v1,v2);
    }

    void set_pnb(Bndel *pnb)
    {
      assert(pnb->type()==periodic);
      periodic_nb=(Bndel_periodic*)pnb;

      if( !leaf() ) {
        ((Bndel_periodic*)down())->set_pnb(pnb);
        ((Bndel_periodic*)(down()->next()))->set_pnb(pnb);
      }
    }

    virtual int split(void * (&el)[nparts], Listagency < Vertex > * agnc,
                      Multivertexadapter & mva, nconf_vtx_t *ncv,splitrule_t,
		      int,Refco::tag_t,Prolong_basic *pro_el);

    virtual int docoarsen(nconf_vtx_t *ncv,int,Restrict_basic *rest_el);
 
};
// #end(class)
// ***************************************************

#endif


