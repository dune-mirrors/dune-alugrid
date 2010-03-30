// (c) Bernhard Schupp 1997 - 1998
// modification for the dune interface 
// (c) Robert Kloefkorn 2004 - 2005 
#ifndef GITTER_IMPL_H_INCLUDED
#define GITTER_IMPL_H_INCLUDED

#include "gitter_sti.h"

#include "mapp_tetra_3d.h"  

#include "gitter_hexa_top.h"
#include "gitter_tetra_top.h"

// include of ghost_elements in line 959
 
class MacroGhostInfo; 

class GitterBasis : public virtual Gitter, public Gitter :: Geometric {
  public :
    class Objects 
    {
      public :
        class VertexEmpty : public VertexGeo 
        {
          public :
            inline VertexEmpty (int, double, double, double,IndexManagerType &im) ;
            inline VertexEmpty (int, double, double, double,VertexGeo & ) ;
           ~VertexEmpty () {}
            virtual inline int ident () const ;
        } ;

        class VertexEmptyMacro : public VertexEmpty {
          public :
            inline VertexEmptyMacro (double, double, double, int,IndexManagerType &im) ;
            ~VertexEmptyMacro () {}
            virtual inline int ident () const ;
          private :
            int _idn ;
        } ;

      class Dune_Hbnd3Default 
      {
        protected:
          enum { _dimvx = 1 }; 
          double _oppVx[3];

        public:
          inline Dune_Hbnd3Default ();
          inline void setOppPoint (int i, const double (&p)[3]);
          inline const double (& oppositeVertex (int i) const) [3];

          inline int dimVx () const; 
      };

        class Hbnd3Default : public hbndseg3_GEO 
#ifdef _DUNE_USES_ALU3DGRID_
           , public Dune_Hbnd3Default
#endif
        {
          protected :
           inline Hbnd3Default (myhface3_t *, int, ProjectVertex * ) ;
           virtual ~Hbnd3Default () {}
          public :
            typedef hbndseg3_GEO :: bnd_t bnd_t;
            virtual inline bnd_t bndtype () const ;
            virtual int ghostLevel () const ; 
            virtual bool ghostLeaf () const {return leaf();}
            
            // default implementation is doing nothing for these 3 methods
            // these methods are overloades just on HbndPll
            virtual const ghostpair_STI & getGhost () const
            { 
              static ghostpair_STI p( (helement_STI *)0, -1); 
              return p;
            }

            // default implementation returns 0
            virtual const MacroGhostInfo* buildGhostCell(ObjectStream&, int) { return 0; }
        };
        typedef Hbnd3Top < Hbnd3Default > hbndseg3_IMPL ;

      class Dune_Hbnd4Default //: public Dune_hbndDefault
      {
        protected:
          enum { _dimvx = 4 }; 
          double _oppVx[_dimvx][3];

        public:
          inline Dune_Hbnd4Default ();
          inline void setOppPoint (int i, const double (&p)[3]);
          inline const double (& oppositeVertex (int i) const) [3];

          inline int dimVx () const; 
      };

        class Hbnd4Default : public hbndseg4_GEO 
#ifdef _DUNE_USES_ALU3DGRID_
           , public Dune_Hbnd4Default
#endif
        {
          protected :
            inline Hbnd4Default (myhface4_t *, int, ProjectVertex * ) ;
            virtual ~Hbnd4Default () {}
          public :
            typedef hbndseg4_GEO :: bnd_t bnd_t;
            virtual inline bnd_t bndtype () const ;
            virtual int ghostLevel () const ;  
            virtual bool ghostLeaf () const {return leaf();}

            // default implementation is doing nothing for these 3 methods
            // these methods are overloades just on HbndPll
            virtual const ghostpair_STI & getGhost () const 
            { 
              static ghostpair_STI p( (helement_STI *)0, -1); 
              return p;
            }

            // default implementation returns 0
            virtual const MacroGhostInfo * buildGhostCell(ObjectStream&, int) { return 0; }
        };
        typedef Hbnd4Top < Hbnd4Default > hbndseg4_IMPL ;

        class Hedge1Empty : public hedge1_GEO {
          protected :
            typedef VertexEmpty innervertex_t ;
            inline Hedge1Empty (myvertex_t *,myvertex_t *) ;
            ~Hedge1Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectInnerVertex(const ProjectVertexPair &pv) ; 
        } ;
  
        typedef Hedge1Top < Hedge1Empty > hedge1_IMPL ;

        class Hface3Empty : public hface3_GEO {
          protected :
            typedef VertexEmpty   innervertex_t ;
            typedef hedge1_IMPL   inneredge_t ;
            inline Hface3Empty (myhedge1_t *,int, myhedge1_t *,int, myhedge1_t *,int) ;
           ~Hface3Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectVertex(const ProjectVertexPair &pv) ; 
  } ;
        typedef Hface3Top < Hface3Empty > hface3_IMPL ;
        

        class Hface4Empty : public hface4_GEO 
        {
         protected :
           typedef VertexEmpty innervertex_t ;
           typedef hedge1_IMPL     inneredge_t ;
           inline Hface4Empty (myhedge1_t *,int, myhedge1_t *,int, myhedge1_t *,int,myhedge1_t *,int) ;
           ~Hface4Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectVertex(const ProjectVertexPair &pv) ; 
        } ;
        typedef Hface4Top < Hface4Empty > hface4_IMPL ;


      class TetraEmpty : public tetra_GEO 
      {
    protected :
        typedef hface3_IMPL innerface_t ;
        typedef hedge1_IMPL inneredge_t ;
        typedef VertexEmpty innervertex_t ;
        inline TetraEmpty (myhface3_t *,int,myhface3_t *,int,myhface3_t *,int,myhface3_t *,int, Gitter *) ;

        ////////////////////////////////////////////////
        // read of data 
        ////////////////////////////////////////////////
        virtual void os2VertexData(ObjectStream & os, GatherScatterType & gs , int borderFace );
        virtual void os2EdgeData(ObjectStream & os, GatherScatterType & gs, int borderFace );
        virtual void os2FaceData(ObjectStream & os, GatherScatterType & gs, int borderFace );

        /////////////////////////////////////////
        //  writing of data 
        /////////////////////////////////////////
        virtual void VertexData2os(ObjectStream & os, GatherScatterType & gs, int borderFace );
        virtual void EdgeData2os(ObjectStream & os, GatherScatterType & gs, int borderFace);
        virtual void FaceData2os(ObjectStream & os, GatherScatterType & gs, int borderFace);

        /////////////////////////////////////////

        // declare this element and all parts leaf  
        virtual void attachleafs();
        
        // this element is not leaf anymore 
        virtual void detachleafs();

        // check that all indices are within range of index manager
        virtual void resetGhostIndices() ;
        
      protected:     
        ~TetraEmpty () {}
        
        int preCoarsening  () ; 
        int postRefinement () ;

        Gitter * _myGrid;
      public: 
        //ghost tetra gets indices of grid, to which it belongs actually
        virtual void setIndicesAndBndId (const hface_STI & f, int face_nr);
    
    private:     
        //ghost tetra gets indices of grid, to which it belongs actually
        void setGhostBoundaryIds();
        
        // for _myGrid     
        friend class TetraTop < TetraEmpty > ;     
    } ;
    typedef TetraTop < TetraEmpty > tetra_IMPL ;

  
  
  class Periodic3Empty : public periodic3_GEO {
    protected :
      typedef hface3_IMPL innerface_t ;
      typedef hedge1_IMPL inneredge_t ;
      typedef VertexEmpty innervertex_t ;
      typedef tetra_IMPL GhostElement_t;

      inline Periodic3Empty (myhface3_t *,int,myhface3_t *,int) ;
      ~Periodic3Empty () {}
      // do nothing here 
      virtual void resetGhostIndices() {}
    public:
  } ;
  typedef Periodic3Top < Periodic3Empty > periodic3_IMPL ;


      class HexaEmpty : public hexa_GEO {
      protected :
        typedef hface4_IMPL innerface_t ;
        typedef hedge1_IMPL inneredge_t ;
        typedef VertexEmpty innervertex_t ;
        inline HexaEmpty (myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int, Gitter*) ;
        ~HexaEmpty () {}
      
        // Neu: burriad 29.4.05
        int preCoarsening(); 
        int postRefinement();

        ////////////////////////////////////////////////
        // read of data 
        ////////////////////////////////////////////////
        // scatter only on ghosts 
        virtual void os2VertexData(ObjectStream & os, GatherScatterType & gs, int borderFace );
        // scatter data on ghost edges  
        virtual void os2EdgeData(ObjectStream & os, GatherScatterType & gs, int borderFace );
        // scatter data on ghost faces 
        virtual void os2FaceData(ObjectStream & os, GatherScatterType & gs, int borderFace );

        //////////////////////////////////////////
        //  writing of data 
        //////////////////////////////////////////
        virtual void VertexData2os(ObjectStream & os, GatherScatterType & gs, int borderFace );
        virtual void EdgeData2os(ObjectStream & os, GatherScatterType & gs, int borderFace);
        virtual void FaceData2os(ObjectStream & os, GatherScatterType & gs, int borderFace);

        virtual void attachleafs();
        virtual void detachleafs();

        // check that all indices are within range of index manager
        virtual void resetGhostIndices();
        
        Gitter * _myGrid;
        friend class HexaTop<HexaEmpty>;
      public:
        //ghost hexa gets indices of grid, to which it belongs actually
        virtual void setIndicesAndBndId (const hface_STI & f, int face_nr);

      private:
        //ghost tetra gets indices of grid, to which it belongs actually
        void setGhostBoundaryIds();
      } ;
      typedef HexaTop < HexaEmpty > hexa_IMPL ;


      class Periodic4Empty : public periodic4_GEO {
        protected :
          typedef hface4_IMPL innerface_t ;
          typedef hedge1_IMPL inneredge_t ;
          typedef VertexEmpty innervertex_t ;
          typedef hexa_IMPL GhostElement_t;

          inline Periodic4Empty (myhface4_t *,int,myhface4_t *,int) ;
          ~Periodic4Empty () {}
         
          // so nothing here 
          virtual void resetGhostIndices() {}
        public:
      } ;
      typedef Periodic4Top < Periodic4Empty > periodic4_IMPL ;
 
    } ;
  public :
    class MacroGitterBasis : public virtual BuilderIF {
      protected :
        virtual VertexGeo     * insert_vertex (double, double, double, int) ;
        virtual VertexGeo     * insert_ghostvx (double, double, double, int) ;
        virtual hedge1_GEO    * insert_hedge1 (VertexGeo *, VertexGeo *) ;
        virtual hface3_GEO    * insert_hface3 (hedge1_GEO *(&)[3], int (&)[3]) ;
        virtual hface4_GEO    * insert_hface4 (hedge1_GEO *(&)[4], int (&)[4]) ;
        virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, ProjectVertex*, Gitter :: hbndseg_STI :: bnd_t)       ;
        // version with point , returns insert_hbnd3 here 
        virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, ProjectVertex*, Gitter :: hbndseg_STI :: bnd_t, MacroGhostInfoTetra* ) ;
        virtual hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, ProjectVertex*, Gitter :: hbndseg_STI :: bnd_t) ;
        virtual hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, ProjectVertex*, Gitter :: hbndseg_STI :: bnd_t, MacroGhostInfoHexa* ) ;
        virtual tetra_GEO     * insert_tetra (hface3_GEO *(&)[4], int (&)[4]) ;
        virtual periodic3_GEO * insert_periodic3 (hface3_GEO *(&)[2], int (&)[2]) ;

        virtual periodic4_GEO * insert_periodic4 (hface4_GEO *(&)[2], int (&)[2]) ;
        virtual hexa_GEO      * insert_hexa (hface4_GEO *(&)[6], int (&)[6]) ;
      public :
        // Gitter is a reference to our grid 
        MacroGitterBasis (Gitter * , istream &) ;
        MacroGitterBasis (Gitter * ) ;
        
        virtual ~MacroGitterBasis () {}

      protected: 
        // reference to our grid, for insert_tetra mainly 
        Gitter * _myGrid; 
    } ;
} ;

class GitterBasisImpl : public GitterBasis {
  MacroGitterBasis * _macrogitter ;
  ProjectVertex*     _ppv;
  public:
   //us fuer Globalmethode levelwalk
  inline Makrogitter & container () ;
  inline const Makrogitter & container () const ;
  public :
    inline IndexManagerType & indexManager(int codim);
    inline size_t numMacroBndSegments() const;
        
    GitterBasisImpl () ;
    GitterBasisImpl (istream &, ProjectVertex* ) ;
    GitterBasisImpl (const char *, ProjectVertex* ) ;
    ~GitterBasisImpl () ;

    virtual void printMemUsage ();

    // return pointer to vertex projection 
    virtual ProjectVertex* vertexProjection() const;
} ;


  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //

inline GitterBasis :: Objects :: VertexEmpty :: VertexEmpty (int l, double x, double y, double z, IndexManagerType & im)
  : GitterBasis :: VertexGeo (l,x,y,z,im) {
  return ;
}

inline GitterBasis :: Objects :: VertexEmpty :: VertexEmpty (int l, double x, double y, double z, VertexGeo & vx )
  : GitterBasis :: VertexGeo (l,x,y,z,vx) {
  return ;
}

inline int GitterBasis :: Objects :: VertexEmpty :: ident () const {
  cerr << "**FEHLER (FATAL) vertex :: ident () nur f\"ur level-0 Vertices zul\"assig " << endl ;
  return (abort (), -1) ;
}

inline GitterBasis :: Objects :: VertexEmptyMacro :: VertexEmptyMacro (double x,double y,double z,int i, IndexManagerType &im) 
  : GitterBasis :: Objects :: VertexEmpty (0,x,y,z,im), _idn (i) {
  return ;
}

inline int GitterBasis :: Objects :: VertexEmptyMacro :: ident () const {
  return _idn ;
}

inline GitterBasis :: Objects :: Hedge1Empty :: Hedge1Empty (myvertex_t * a, myvertex_t * b) 
  : Gitter :: Geometric :: hedge1_GEO (a,b) {
  return ;
}

inline void GitterBasis :: Objects :: Hedge1Empty :: projectInnerVertex(const ProjectVertexPair &pv) 
{
  if (innerVertex()) {
    assert(!leaf());
    innerVertex()->project(pv);
  }
}

inline GitterBasis :: Objects :: Hface3Empty :: Hface3Empty (myhedge1_t *e0, int s0, 
  myhedge1_t *e1, int s1, myhedge1_t *e2, int s2) : Gitter :: Geometric :: hface3_GEO (e0, s0, e1, s1, e2, s2) {
  return ;
}

inline void GitterBasis :: Objects :: Hface3Empty :: projectVertex(const ProjectVertexPair &pv) 
{
  assert(!leaf());
  for (int e = 0; e < polygonlength; e++)
    myhedge1(e)->projectInnerVertex(pv);
  if (innerVertex())
    innerVertex()->project(pv);
}

inline GitterBasis :: Objects :: Hface4Empty :: Hface4Empty (myhedge1_t *e0, int s0, 
  myhedge1_t *e1, int s1, myhedge1_t *e2, int s2, myhedge1_t *e3, int s3)
  : Gitter :: Geometric :: hface4_GEO (e0, s0, e1, s1, e2, s2, e3, s3) {
  return ;
}

inline void GitterBasis :: Objects :: Hface4Empty :: projectVertex(const ProjectVertexPair &pv) {
  for (int e = 0; e < polygonlength; e++)
    myhedge1(e)->projectInnerVertex(pv);
  if (innerVertex())
    innerVertex()->project(pv);
}

inline GitterBasis :: Objects :: Dune_Hbnd3Default ::Dune_Hbnd3Default () 
{
  for(int j=0; j<3; j++) _oppVx[j] = 0.0;
}

inline void GitterBasis :: Objects :: Dune_Hbnd3Default 
:: setOppPoint (int i, const double (&p)[3]) 
{ 
  assert((i >= 0) && (i < _dimvx));
  _oppVx[0] = p[0];
  _oppVx[1] = p[1];
  _oppVx[2] = p[2];
  return;
}
           
inline const double (& GitterBasis :: Objects :: Dune_Hbnd3Default ::oppositeVertex (int i) const) [3] 
{
#ifdef __USE_INTERNAL_FACES__  
  assert((i >= 0) && (i < _dimvx));
  return _oppVx;
#else 
  cerr << "Dune_Hbnd3Default ::oppositeVertex is not supported when compiled without the '__USE_INTERNAL_FACES__' cpp variable! in: "<<__FILE__<<" line: " <<__LINE__<<endl; 
  abort();
  return _oppVx;
#endif
}

inline int GitterBasis :: Objects :: Dune_Hbnd3Default :: dimVx () const { return _dimvx; }

inline GitterBasis :: Objects :: Dune_Hbnd4Default ::Dune_Hbnd4Default () 
{
  for(int i=0; i<_dimvx; i++)
    for(int j=0; j<3; j++) _oppVx[i][j] = 0.0;
}

// for hexa internal boundaries 
inline void GitterBasis :: Objects :: Dune_Hbnd4Default 
:: setOppPoint (int i, const double (&p)[3]) 
{ 
  assert((i >= 0) && (i < _dimvx));
  _oppVx[i][0] = p[0];
  _oppVx[i][1] = p[1];
  _oppVx[i][2] = p[2];
  return;
}
           
inline const double (& GitterBasis :: Objects :: Dune_Hbnd4Default ::oppositeVertex (int i) const) [3] 
{
#ifdef __USE_INTERNAL_FACES__  
  assert((i >= 0) && (i < _dimvx));
  return _oppVx[i];
#else 
  cerr << "Dune_Hbnd4Default ::oppositeVertex is not supported when compiled without the '__USE_INTERNAL_FACES__' cpp variable! in: "<<__FILE__<<" line: " <<__LINE__<<endl; 
  abort();
  return _oppVx[i];
#endif
}

inline int GitterBasis :: Objects :: Dune_Hbnd4Default :: dimVx () const { return _dimvx; }
//*************************************************************************
// end of Dune_hbndDefault 
//***************************************************************************

inline GitterBasis :: Objects :: Hbnd3Default :: 
Hbnd3Default (myhface3_t * f, int i, ProjectVertex *ppv ) 
 : Gitter :: Geometric :: hbndseg3_GEO (f, i, ppv)
{
  return ;
}

inline GitterBasis :: Objects ::Hbnd3Default :: bnd_t GitterBasis :: Objects :: Hbnd3Default :: bndtype () const {
  return undefined ;
}

inline int GitterBasis :: Objects :: Hbnd3Default :: ghostLevel () const {
  return level() ;
}

inline GitterBasis :: Objects :: Hbnd4Default :: Hbnd4Default (myhface4_t * f, int i, ProjectVertex *ppv) : 
  Gitter :: Geometric :: hbndseg4_GEO (f, i,ppv)
{
  return ;
}

inline GitterBasis :: Objects ::Hbnd4Default :: bnd_t GitterBasis :: Objects :: Hbnd4Default :: bndtype () const {
  return undefined ;
}

inline int GitterBasis :: Objects :: Hbnd4Default :: ghostLevel () const {
  return level() ;
}

inline GitterBasis :: Objects :: TetraEmpty :: 
TetraEmpty (myhface3_t * f0, int t0, myhface3_t * f1, int t1,
            myhface3_t * f2, int t2, myhface3_t * f3, int t3,
            Gitter * mygrid) : 
  Gitter :: Geometric :: Tetra (f0, t0, f1, t1, f2, t2, f3, t3) , 
  _myGrid(mygrid) 
{
  attachleafs();
  return ;
}

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: TetraEmpty :: preCoarsening () 
{
  assert( _myGrid );
  // only call preCoarsening on non ghost elements 
  return ((this->isGhost()) ? 0 : _myGrid->preCoarsening(*this));
}

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: TetraEmpty :: postRefinement () 
{
  // reset refined tag of this element because no leaf anymore 
  this->resetRefinedTag();

  assert( _myGrid );
  // only call postRefinement on non ghost elements 
  return ((this->isGhost()) ? 0 : _myGrid->postRefinement(*this));
}

inline GitterBasis :: Objects :: Periodic3Empty :: Periodic3Empty (myhface3_t * f0, int t0, myhface3_t * f1, int t1) 
  : Gitter :: Geometric :: Periodic3 (f0, t0, f1, t1) {
  return ;
}

inline GitterBasis :: Objects :: Periodic4Empty :: Periodic4Empty (myhface4_t * f0, int t0, myhface4_t * f1, int t1) 
  : Gitter :: Geometric :: Periodic4 (f0, t0, f1, t1) {
  return ;
}

// Neu: burriad 29.4.05
inline GitterBasis :: Objects :: HexaEmpty :: 
HexaEmpty (myhface4_t * f0, int t0, myhface4_t * f1, int t1,
           myhface4_t * f2, int t2, myhface4_t * f3, int t3, 
           myhface4_t * f4, int t4, myhface4_t * f5, int t5,
           Gitter* mygrid) : 
  Gitter::Geometric::hexa_GEO(f0, t0, f1, t1, f2, t2, f3, t3, f4, t4, f5, t5),
  _myGrid(mygrid) 
{
  attachleafs();
  return ;
}

inline int GitterBasis::Objects::HexaEmpty::preCoarsening() {
  assert(_myGrid);
  // only call preCoarsening on non ghost elements 
  return ((this->isGhost()) ? 0 : _myGrid->preCoarsening(*this));
}

inline int GitterBasis::Objects::HexaEmpty::postRefinement() 
{
  // reset refined tag of this element because no leaf anymore 
  this->resetRefinedTag();
  
  assert(_myGrid);
  // only call postRefinement on non ghost elements 
  return ((this->isGhost()) ? 0 : _myGrid->postRefinement(*this));
}

////////////////////////////////////////////////////////////////
//
//  --GitterBasisImpl
//
////////////////////////////////////////////////////////////////
inline ProjectVertex*  GitterBasisImpl :: vertexProjection() const 
{
  return _ppv;
}

inline Gitter :: Makrogitter & GitterBasisImpl :: container () {
  return * _macrogitter ;
}

inline const Gitter :: Makrogitter & GitterBasisImpl :: container () const {
  return * _macrogitter ;
}

inline IndexManagerType & GitterBasisImpl :: indexManager (int codim) 
{ 
  return _macrogitter->indexManager(codim);
}

inline size_t GitterBasisImpl :: numMacroBndSegments() const
{ 
  return _macrogitter->numMacroBndSegments();
}

#endif  //  GITTER_IMPL_H_INCLUDED
