
#ifndef __HEADER__VTX_BTREE
#define __HEADER__VTX_BTREE

static volatile char RCSId_vtx_btree_h [] = "$Id$";

#include "xdisplay.h"

class Vertex;
class Thinelement;
class Element;

// ============================================================
// Klasse Vtx_btree: Binaerer Baum von Vertex-Instanzen.
// Ordnet die Knoten nach ihrem Abstand zu einem Referenzknoten,
// der dem Konstruktor uebergeben wird.
// ============================================================

void nb_draw(Xdisplay &xd,Element *el1,Element *el2);


class Vtx_btree {
 public:
    // Implementation eines Knotens des binaeren Baumes
    struct Node {
      Node* next;
      Node* prev;
      Vertex* vtx;
      Thinelement *lnb;
      Thinelement *rnb;
      int _lidx,_ridx;
      Node(Vertex* invtx,Thinelement *plnb,Thinelement *prnb)
	: vtx(invtx), lnb(plnb), rnb(prnb), next(0), prev(0)
      {assert(invtx);}

      ~Node() {
        if(next) delete next;
        if(prev) delete prev;
      }

      // Gibt die Anzahl von Knoten des laengsten "Zweiges" des Baumes zurueck
      int deepestLevel(int prevLvl = 0) const {
        int left  = (prev ? prev->deepestLevel(prevLvl+1) : prevLvl+1);
        int right = (next ? next->deepestLevel(prevLvl+1) : prevLvl+1);
        return left > right ? left : right;
      }

      // Gibt die Anzahl von Knoten unterhalb des Empfaengerknotens zurueck
      int count() const {
        return 1 + (next ? next->count() : 0) + (prev ? prev->count() : 0);
      }

      int remove(Vertex *pvtx);

      void nbconnect(int , Thinelement * , int ) ;
      void draw(Xdisplay &xd,Element *el);

    }* head;
 public:
    Vertex* rvtx;
    Thinelement *lnb;
    Thinelement *rnb;

    void insertNode(Node* node, Node* newNode);

    double dist(Vertex* invtx);

    Vtx_btree* left() const;

    Vtx_btree* right() const;

  public:

    Vtx_btree(Vertex* invtx,Thinelement *plnb,Thinelement *prnb)
      : rvtx(invtx), lnb(plnb), rnb(prnb), head(0){
      assert(rvtx);
      assert(plnb);
      assert(prnb);
    }
    
    ~Vtx_btree() {
      if( head ) delete head;
    }

    Vertex* getHead() { return head->vtx; }

    Thinelement *getlnb() { return head->lnb; }
    Thinelement *getrnb() { return head->rnb; }

    void insert(Vertex* invtx,Thinelement *plnb,Thinelement *prnb);
    
    void splitTree(Vtx_btree*& inleft, Vtx_btree*& inright);
		
    void merge(Vtx_btree* inleft, Vtx_btree* inright);

    void nbconnect(int , Thinelement * , int ) ;

    void draw(Xdisplay &xd,Element *el);

    int deepestLevel() {
      return (head ? head->deepestLevel() : 0);
    }

    int count() const {
      return head->count();
    }
    
    bool remove(Vertex *vtx) {
      assert(head->prev || head->next);
      return (head->remove(vtx)==1);
    }
};

#endif // VTXBTREE_H
