#include <config.h>

#include <cmath>
#include "grid.h"
#include "triang.h"

namespace ALU2DGrid
{

  // ------------------------------------------------------------
  //  void insert(Vertex* invtx)                      - public -
  // ------------------------------------------------------------
  // "Verpackt" den uebergebenen Vertex in einer Instanz der
  // privaten Klasse Node und fuegt jene in den Baum ein.

  template < int N, int NV >
  void
  Vtx_btree < N,NV >::insert(vertex_t *invtx, thinelement_t *plnb, thinelement_t *prnb)
  {
    Node* newNode = new Node(invtx,plnb,prnb);
    if( head == 0 )
      head = newNode;
    else
      insertNode(head, newNode);
  }

  // ------------------------------------------------------------
  //  void insertNode(Node* node, Node* newNode)     - private -
  // ------------------------------------------------------------
  // Fuegt neue Instanz der Klasse Node 'newNode' in den Baum
  // nach 'node' ein.

  template < int N, int NV >
  void
  Vtx_btree < N,NV >::insertNode(Node* node, Node* newNode)
  {
    assert(node->vtx);
    assert(newNode->vtx);
    if( dist(node->vtx) < dist(newNode->vtx) ) {
      if( node->next != 0 )
        insertNode(node->next, newNode);
      else
        node->next = newNode;
    } 
    else 
    {
      if( node->prev != 0 )
        insertNode(node->prev, newNode);
      else
        node->prev = newNode;
    }
  }

  // ------------------------------------------------------------
  //  Vtx_btree left() const                         - private -
  // ------------------------------------------------------------
  // Gibt den linken Teilbaum als neue Vtx_btree-Instanz zurueck.
  // Dabei ist zu beachten, dass nur die Referenzen und nicht
  // die Node-Instanzen kopiert werden.

  template < int N, int NV >
  Vtx_btree < N,NV > *
  Vtx_btree < N,NV >::left() const
  {
    Vtx_btree* left = 0;
    if( head->prev ) 
    {
      left = new Vtx_btree(rvtx,lnb,rnb);
      left->head = head->prev;
    }
    return left;
  }

  // ------------------------------------------------------------
  //  Vtx_btree right() const                         - private -
  // ------------------------------------------------------------
  // Siehe 'left'

  template < int N, int NV >
  Vtx_btree < N,NV >*
  Vtx_btree < N,NV >::right() const
  {
    Vtx_btree* right = 0;
    if( head->next ) 
    {
      right = new Vtx_btree(head->vtx,head->lnb,head->rnb);
      right->head = head->next;
    }
    return right;
  }

  // ------------------------------------------------------------
  //  void splitTree(Vtx_btree*& ioleft, Vtx_btree*& ioright)
  //                                                  - public -
  // ------------------------------------------------------------
  // Spaltet den Empfaenger in linken und rechten Teilbaum auf
  // und gibt jene zurueck. Hiernach besteht der Empfaenger nur
  // noch aus einem Element, dem Kopf der Baumes.

  template < int N, int NV >
  void
  Vtx_btree < N,NV >::splitTree(Vtx_btree < N,NV > *&ioleft, Vtx_btree < N,NV > *&ioright)
  {
    ioleft = left();
    ioright = right();
    head->prev = head->next = 0;
  }


  // ------------------------------------------------------------
  //  void merge(Vtx_btree* inleft, Vtx_btree* inright)
  //                                                  - public -
  // ------------------------------------------------------------
  // Umkehrung von 'split'. Dabei ist zu beachten, dass die
  // uebergebenen Baeume danach leer sind.

  template < int N, int NV >
  void
  Vtx_btree < N,NV >::merge(Vtx_btree < N,NV > *inleft, Vtx_btree < N,NV > *inright)
  {
    assert(count() == 1);
    head->prev = (inleft != NULL ? inleft->head : NULL);
    head->next = (inright != NULL ? inright->head : NULL);
    if( inleft )
            inleft->head = NULL;
    if( inright )
      inright->head = NULL;
  }

  // ------------------------------------------------------------
  //  double dist(Vertex* invtx)                     - private -
  // ------------------------------------------------------------
  // Gibt die Distanz zwischen dem uebergebenen und dem
  // Referenzvertex zurueck

  template < int N, int NV >
  double
  Vtx_btree < N,NV >::dist(vertex_t *invtx)
  {
    assert(rvtx);
    double ret=0;
    for (int i=0;i<vertex_t::ncoord;++i)
    {
      const double d = (invtx->coord()[i] - rvtx->coord()[i]);
      ret += d*d;
    }
    return ret;
  }

  template < int N, int NV >
  void
  Vtx_btree < N,NV >::nbconnect(int opp, thinelement_t *el , int i) {
    if (head)
      head->nbconnect(opp,el,i);
  }

  template < int N, int NV >
  void 
  Vtx_btree < N,NV >::Node::nbconnect(int opp, thinelement_t *el , int i) {
    if (lnb)
      lnb->nbconnect(opp,el,i);
    if (rnb)
      rnb->nbconnect(opp,el,i);
    if (prev)
      prev->nbconnect(opp,el,i);
    if (next)
      next->nbconnect(opp,el,i);  
  }

  template < int N, int NV >
  int Vtx_btree < N,NV >::Node::remove(vertex_t *pvtx) {
    if (vtx==pvtx) {
      assert(!prev && !next);
      return -1;
    }
    int left  = (prev ? prev->remove(pvtx) : 0);
    int right = (next ? next->remove(pvtx) : 0);
    if (left==-1) {
      delete prev;
      prev=0;
      return 1;
    }
    if (right==-1) {
      delete next;
      next=0;
      return 1;
    }
    return right+left;
  }


  // ------------------------------------------------------------
  // Template Instantiation
  // ------------------------------------------------------------
  template class Vtx_btree < 2,3 >;
  template class Vtx_btree < 3,3 >;

  template class Vtx_btree < 2,4 >;
  template class Vtx_btree < 3,4 >;

} // namespace ALU2DGrid
