#ifndef ALUGRID_XDISPLAY_IMP_INCLUDED
#define ALUGRID_XDISPLAY_IMP_INCLUDED

#include "grid.h"
#include "triang.h"

      
#if USE_ALUGRID_XDISPLAY 
inline void nb_draw(Xdisplay &xd,Element *el1,Element *el2) 
{
  double p1[2],p2[2];
  double bary[3]={1./3.,1./3.,1./3.};
  el1->fromlocal(bary,p1);
  el2->fromlocal(bary,p2);
  p1[0]+=0.01;
  p1[1]+=0.025;
  Fullvertex pp1(p1,-1);
  Fullvertex pp2(p2,-1);
  xd.linedraw(&pp1,&pp2,xd.bcol[12]);
}

inline void
Vtx_btree::draw(Xdisplay &xd,Element *el) {
  /*  
  if (lnb)
    nb_draw(xd,el,(Element*)lnb);
  if (rnb)
    nb_draw(xd,el,(Element*)rnb);
  */
  if (head)
    head->draw(xd,el);
}

inline void
Vtx_btree::Node::draw(Xdisplay &xd,Element *el) {
  /*
  if (lnb)
    nb_draw(xd,el,(Element*)lnb);
  if (rnb)
    nb_draw(xd,el,(Element*)rnb);
  */
  if (prev)
    prev->draw(xd,el);
  else {
    // assert(((Triang*)lnb)->leaf());
    nb_draw(xd,el,(Element*)lnb);
  }
  if (next)
    next->draw(xd,el);
  else {
    // assert(((Triang*)rnb)->leaf());
    nb_draw(xd,el,(Element*)rnb);
  }
}
#endif

#endif
