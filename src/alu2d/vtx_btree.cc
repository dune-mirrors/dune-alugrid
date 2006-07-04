#include <math.h>
#include "grid.h"
#include "triang.h"

// ------------------------------------------------------------
//  void insert(Vertex* invtx)                      - public -
// ------------------------------------------------------------
// "Verpackt" den uebergebenen Vertex in einer Instanz der
// privaten Klasse Node und fuegt jene in den Baum ein.

void
Vtx_btree::insert(Vertex* invtx,Thinelement *plnb,Thinelement *prnb)
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

void
Vtx_btree::insertNode(Node* node, Node* newNode)
{
  assert(node->vtx);
  assert(newNode->vtx);
  if( dist(node->vtx) < dist(newNode->vtx) ) {
    if( node->next != 0 )
      insertNode(node->next, newNode);
    else
      node->next = newNode;
  } else {
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

Vtx_btree*
Vtx_btree::left() const
{
  Vtx_btree* left = 0;
  if( head->prev ) {
    left = new Vtx_btree(rvtx,lnb,rnb);
    left->head = head->prev;
  }
  return left;
}

// ------------------------------------------------------------
//  Vtx_btree right() const                         - private -
// ------------------------------------------------------------
// Siehe 'left'

Vtx_btree*
Vtx_btree::right() const
{
  Vtx_btree* right = 0;
  if( head->next ) {
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

void
Vtx_btree::splitTree(Vtx_btree*& ioleft, Vtx_btree*& ioright)
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

void
Vtx_btree::merge(Vtx_btree* inleft, Vtx_btree* inright)
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

double
Vtx_btree::dist(Vertex* invtx)
{
  assert(rvtx);
  return   pow(invtx->coord()[0] - rvtx->coord()[0],2)
         + pow(invtx->coord()[1] - rvtx->coord()[1],2);
}

void 
Vtx_btree::nbconnect(int opp, Thinelement *el , int i) {
  if (head)
    head->nbconnect(opp,el,i);
}

void 
Vtx_btree::Node::nbconnect(int opp, Thinelement *el , int i) {
  if (lnb)
    lnb->nbconnect(opp,el,i);
  if (rnb)
    rnb->nbconnect(opp,el,i);
  if (prev)
    prev->nbconnect(opp,el,i);
  if (next)
    next->nbconnect(opp,el,i);  
}

int Vtx_btree::Node::remove(Vertex *pvtx) {
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

