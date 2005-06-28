// (c) Bernhard Schupp 1997 - 1998
// modification for the dun interface 
// (c) Robert Kloefkorn 2004 - 2005 
#ifndef GITTER_IMPL_H_INCLUDED
#define GITTER_IMPL_H_INCLUDED

#ifdef IBM_XLC
  #define _ANSI_HEADER
#endif

#ifdef _ANSI_HEADER
  using namespace std;
  #include <fstream>
  #include <vector>
  #include <utility>
#else
  #include <fstream.h>
  #include <vector.h>
  #include <pair.h>
#endif

#include "gitter_sti.h"

#include "mapp_tetra_3d.h"  

#include "gitter_hexa_top.h"
#include "gitter_tetra_top.h"

class GitterBasis : public virtual Gitter, public Gitter :: Geometric {
  public :
    class Objects {
      public :
        class VertexEmpty : public VertexGeo {
          public :
            inline VertexEmpty (int, double, double, double,
                IndexManagerType &im) ;
            inline VertexEmpty (int, double, double, double,
                VertexGeo & ) ;
     ~VertexEmpty () {}
            virtual inline int ident () const ;
        } ;

        class VertexEmptyMacro : public VertexEmpty {
          public :
            inline VertexEmptyMacro (double, double, double, int,
                IndexManagerType &im) ;
     ~VertexEmptyMacro () {}
            virtual inline int ident () const ;
          private :
            int _idn ;
        } ;

#if 0
      // organizes the indices for boundary faces and 
      // the opposite vertices for ghost cells 
      class Dune_hbndDefault 
      {
        protected:
          int _index;

        public:
          inline Dune_hbndDefault ();

          inline int getIndex () const; 
          inline void setIndex ( int idx ); 

        protected:  
          inline void splitGhost () {} 
          inline void setGhost (Gitter::helement_STI *) {}
      };
#endif

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
           inline Hbnd3Default (myhface3_t *, int, ProjectVertex * , Gitter * ) ;
           virtual ~Hbnd3Default () {}
          public :
            typedef hbndseg3_GEO :: bnd_t bnd_t;
            virtual inline bnd_t bndtype () const ;
            virtual int ghostLevel () const ; 
            
            // default implementation is doing nothing for these 3 methods
            // these methods are overloades just on HbndPll
            virtual helement_STI * getGhost () { return 0; }

            // points inside ghosts 
            void faceNormal( double * normal) const;

            inline int preCoarsening  () ;
            inline int postRefinement () ;
          protected:
            Gitter * _myGrid; 
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
            inline Hbnd4Default (myhface4_t *, int, ProjectVertex *, Gitter *) ;
            virtual ~Hbnd4Default () {}
          public :
            typedef hbndseg4_GEO :: bnd_t bnd_t;
            virtual inline bnd_t bndtype () const ;
            virtual int ghostLevel () const ;  

            // default implementation is doing nothing for these 3 methods
            // these methods are overloades just on HbndPll
            virtual helement_STI * getGhost () { return 0; }

            // points inside ghosts 
            void faceNormal( double * normal) const;

            inline int preCoarsening  () ; 
            inline int postRefinement () ;
          protected:
            Gitter * _myGrid; 
        };
        typedef Hbnd4Top < Hbnd4Default > hbndseg4_IMPL ;

        class Hedge1Empty : public hedge1_GEO {
          protected :
            typedef VertexEmpty innervertex_t ;
            inline Hedge1Empty (myvertex_t *,myvertex_t *) ;
     ~Hedge1Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectInnerVertex(const ProjectVertex &pv) ; 
        } ;
  
        typedef Hedge1Top < Hedge1Empty > hedge1_IMPL ;

        class Hface3Empty : public hface3_GEO {
          protected :
            typedef VertexEmpty   innervertex_t ;
            typedef hedge1_IMPL   inneredge_t ;
            inline Hface3Empty (myhedge1_t *,int, myhedge1_t *,int, myhedge1_t *,int) ;
     ~Hface3Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectVertex(const ProjectVertex &pv) ; 
  } ;
        typedef Hface3Top < Hface3Empty > hface3_IMPL ;

        class Hface4Empty : public hface4_GEO {
          protected :
            typedef VertexEmpty innervertex_t ;
            typedef hedge1_IMPL     inneredge_t ;
            inline Hface4Empty (myhedge1_t *,int, myhedge1_t *,int, myhedge1_t *,int,myhedge1_t *,int) ;
     ~Hface4Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectVertex(const ProjectVertex &pv) ; 
        } ;
        typedef Hface4Top < Hface4Empty > hface4_IMPL ;

        class TetraEmpty : public tetra_GEO {
    protected :
            typedef hface3_IMPL innerface_t ;
            typedef hedge1_IMPL inneredge_t ;
            typedef VertexEmpty innervertex_t ;
            inline TetraEmpty (myhface3_t *,int,myhface3_t *,int,myhface3_t *,int,myhface3_t *,int, Gitter *) ;

           ~TetraEmpty () {}
            int preCoarsening  () ; 
            int postRefinement () ;

            Gitter * _myGrid;
            
       // for _myGrid     
       friend class TetraTop < TetraEmpty > ;     
  } ;
  typedef TetraTop < TetraEmpty > tetra_IMPL ;
  
  class Periodic3Empty : public periodic3_GEO {
    protected :
            typedef hface3_IMPL innerface_t ;
            typedef hedge1_IMPL inneredge_t ;
            typedef VertexEmpty innervertex_t ;
      inline Periodic3Empty (myhface3_t *,int,myhface3_t *,int) ;
     ~Periodic3Empty () {}
    public:
  } ;
  typedef Periodic3Top < Periodic3Empty > periodic3_IMPL ;

// Anfang - Neu am 23.5.02 (BS)
  class Periodic4Empty : public periodic4_GEO {
    protected :
            typedef hface4_IMPL innerface_t ;
            typedef hedge1_IMPL inneredge_t ;
            typedef VertexEmpty innervertex_t ;
      inline Periodic4Empty (myhface4_t *,int,myhface4_t *,int) ;
     ~Periodic4Empty () {}
    public:
  } ;
  typedef Periodic4Top < Periodic4Empty > periodic4_IMPL ;
// Ende - Neu am 23.5.02 (BS)

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

        Gitter* _myGrid;

        friend class HexaTop<HexaEmpty>;
      } ;
      typedef HexaTop < HexaEmpty > hexa_IMPL ;
    } ;
  public :
    class MacroGitterBasis : public virtual BuilderIF {
      protected :
        virtual inline VertexGeo     * insert_vertex (double, double, double, int,int = 0) ;
        virtual inline hedge1_GEO    * insert_hedge1 (VertexGeo *, VertexGeo *) ;
        virtual inline hface3_GEO    * insert_hface3 (hedge1_GEO *(&)[3], int (&)[3]) ;
        virtual inline hface4_GEO    * insert_hface4 (hedge1_GEO *(&)[4], int (&)[4]) ;
        virtual inline hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, Gitter :: hbndseg_STI :: bnd_t) ;
        // version with point , returns insert_hbnd3 here 
        virtual inline hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, Gitter :: hbndseg_STI :: bnd_t, const double (&p)[3]) ;
        virtual inline hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, Gitter :: hbndseg_STI :: bnd_t) ;
        virtual inline tetra_GEO     * insert_tetra (hface3_GEO *(&)[4], int (&)[4]) ;
        virtual inline periodic3_GEO * insert_periodic3 (hface3_GEO *(&)[2], int (&)[2]) ;

        virtual inline periodic4_GEO * insert_periodic4 (hface4_GEO *(&)[2], int (&)[2]) ;
        virtual inline hexa_GEO      * insert_hexa (hface4_GEO *(&)[6], int (&)[6]) ;
      public :
        // Gitter is a reference to our grid 
        inline MacroGitterBasis (Gitter * , istream &) ;
        inline MacroGitterBasis (Gitter * ) ;
        
        virtual ~MacroGitterBasis () {}

        // return index manager mostly for restore and backup 
        inline IndexManagerType & indexManager(int codim);
        
      protected: 
        // reference to our grid, for insert_tetra mainly 
        Gitter * _myGrid; 
    } ;
} ;

class GitterBasisImpl : public GitterBasis {
  MacroGitterBasis * _macrogitter ;
  public:
   //us fuer Globalmethode levelwalk
  inline Makrogitter & container () ;
  inline const Makrogitter & container () const ;
  public :
    inline IndexManagerType & indexManager(int codim);
        
    inline GitterBasisImpl () ;
    inline GitterBasisImpl (istream &) ;
    inline GitterBasisImpl (const char *) ;
    inline ~GitterBasisImpl () ;
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

inline void GitterBasis :: Objects :: Hedge1Empty :: projectInnerVertex(const ProjectVertex &pv) {
  if (innerVertex()) {
    assert(!leaf());
    innerVertex()->project(pv);
  }
}

inline GitterBasis :: Objects :: Hface3Empty :: Hface3Empty (myhedge1_t *e0, int s0, 
  myhedge1_t *e1, int s1, myhedge1_t *e2, int s2) : Gitter :: Geometric :: hface3_GEO (e0, s0, e1, s1, e2, s2) {
  return ;
}

inline void GitterBasis :: Objects :: Hface3Empty :: projectVertex(const ProjectVertex &pv) {
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

inline void GitterBasis :: Objects :: Hface4Empty :: projectVertex(const ProjectVertex &pv) {
  for (int e = 0; e < polygonlength; e++)
    myhedge1(e)->projectInnerVertex(pv);
  if (innerVertex())
    innerVertex()->project(pv);
}

//******************************************************************
// Dune hbnd Default , store the opposite vectices for generating 
// a ghost cell 
//******************************************************************
#if 0
inline GitterBasis :: Objects :: Dune_hbndDefault :: 
Dune_hbndDefault () : _index (-1) {}

inline int GitterBasis :: Objects :: Dune_hbndDefault :: getIndex () const 
{ 
  assert( _index >= 0 );
  return _index;
}

inline void  GitterBasis :: Objects :: Dune_hbndDefault :: setIndex ( int idx ) 
{
  _index = idx;
  return ;
}
#endif

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
  assert((i >= 0) && (i < _dimvx));
  return _oppVx;
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
  assert((i >= 0) && (i < _dimvx));
  return _oppVx[i];
}

inline int GitterBasis :: Objects :: Dune_Hbnd4Default :: dimVx () const { return _dimvx; }
//*************************************************************************
// end of Dune_hbndDefault 
//***************************************************************************

inline GitterBasis :: Objects :: Hbnd3Default :: Hbnd3Default (myhface3_t * f, int i, ProjectVertex *ppv, Gitter * grd ) :
  Gitter :: Geometric :: hbndseg3_GEO (f, i, ppv) , _myGrid(grd) 
{
  return ;
}

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: Hbnd3Default :: preCoarsening () 
{
  // grid pointer only exists in the case of internal bnds 
  if(_myGrid)
  {
    return _myGrid->preCoarsening(*this);
  }
  else 
  {
    return 0;
  }
}

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: Hbnd3Default :: postRefinement () 
{
  // grid pointer only exists in the case of internal bnds 
  if(_myGrid)
  {
    return _myGrid->postRefinement(*this);
  }
  else 
  {
    return 0;
  }
}

inline GitterBasis :: Objects ::Hbnd3Default :: bnd_t GitterBasis :: Objects :: Hbnd3Default :: bndtype () const {
  return undefined ;
}

inline int GitterBasis :: Objects :: Hbnd3Default :: ghostLevel () const {
  return level() ;
}

inline void GitterBasis :: Objects :: Hbnd3Default :: faceNormal( double * normal) const {

  hface3_GEO * face = this->myhface3(0);
  int tw = this->twist(0);
  BSGridLinearSurfaceMapping 
    LSM(face->myvertex( (tw < 0) ? 0 : 2 )->Point(),
        face->myvertex( 1                )->Point(),
        face->myvertex( (tw < 0) ? 2 : 0 )->Point()
       );
  LSM.normal(normal);
  return ;
}

inline GitterBasis :: Objects :: Hbnd4Default :: Hbnd4Default (myhface4_t * f, int i, ProjectVertex *ppv, Gitter * grd ) : 
  Gitter :: Geometric :: hbndseg4_GEO (f, i,ppv) , _myGrid(grd) 
{
  return ;
}

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: Hbnd4Default :: preCoarsening () 
{
  if(_myGrid)
    return _myGrid->preCoarsening(*this);
  else 
    return 0;
}

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: Hbnd4Default :: postRefinement () 
{
  if(_myGrid)
    return _myGrid->postRefinement(*this);
  else 
    return 0;
}

inline GitterBasis :: Objects ::Hbnd4Default :: bnd_t GitterBasis :: Objects :: Hbnd4Default :: bndtype () const {
  return undefined ;
}

inline int GitterBasis :: Objects :: Hbnd4Default :: ghostLevel () const {
  return level() ;
}

inline void GitterBasis :: Objects :: Hbnd4Default :: faceNormal( double * normal) const {
  cerr << "ERORR: Hbnd4Default :: faceNormal not implemented! " << __FILE__ << __LINE__ << endl;
  return ;
}

inline GitterBasis :: Objects :: TetraEmpty :: 
TetraEmpty (myhface3_t * f0, int t0, myhface3_t * f1, int t1,
            myhface3_t * f2, int t2, myhface3_t * f3, int t3,
            Gitter * mygrid ) : 
  Gitter :: Geometric :: Tetra (f0, t0, f1, t1, f2, t2, f3, t3) , 
  _myGrid(mygrid) {
  return ;
}

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: TetraEmpty :: preCoarsening () 
{
  assert( _myGrid );
  return _myGrid->preCoarsening(*this);
}

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: TetraEmpty :: postRefinement () 
{
  assert( _myGrid );
  return _myGrid->postRefinement(*this);
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
  _myGrid(mygrid) {
  return ;
}

inline int GitterBasis::Objects::HexaEmpty::preCoarsening() {
  assert(_myGrid);
  return _myGrid->preCoarsening(*this);
}

inline int GitterBasis::Objects::HexaEmpty::postRefinement() {
  assert(_myGrid);
  return _myGrid->postRefinement(*this);
}

// Ende - Neu 29.4.05

inline GitterBasisImpl :: GitterBasisImpl () : _macrogitter (0) {
  _macrogitter = new MacroGitterBasis ( this ) ;
  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

inline GitterBasisImpl :: GitterBasisImpl (istream & in) : _macrogitter (0) {
  _macrogitter = new MacroGitterBasis ( this , in) ;
  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

inline GitterBasisImpl :: GitterBasisImpl (const char * file) : _macrogitter (0) {
  ifstream in (file) ;
  if (!in) {
    cerr << "  GitterBasisImpl :: GitterBasisImpl (const char *) FEHLER (IGNORIERT) " ;
    cerr << "beim \"Offnen der Datei " << (file ? file : "\"null\"" ) << endl ;
    _macrogitter = new MacroGitterBasis ( this ) ;
  } else {
    _macrogitter = new MacroGitterBasis ( this, in) ;
  }
  assert (_macrogitter) ;
  notifyMacroGridChanges () ;
  return ;
}

inline GitterBasisImpl :: ~GitterBasisImpl () {
  delete _macrogitter ;
  return ;
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

inline GitterBasis :: MacroGitterBasis :: MacroGitterBasis (Gitter * mygrid, istream & in) : _myGrid(mygrid) {
  macrogridBuilder (in) ;
  return ;
}

inline GitterBasis :: MacroGitterBasis :: MacroGitterBasis (Gitter * mygrid) : _myGrid(mygrid) {
  return ;
}

inline GitterBasis :: VertexGeo * GitterBasis :: MacroGitterBasis :: insert_vertex (double x, double y, double z, int id,int) {
  return new Objects :: VertexEmptyMacro (x, y, z, id, indexManager(3)) ;
}

inline GitterBasis :: hedge1_GEO * GitterBasis :: MacroGitterBasis :: insert_hedge1 (VertexGeo * a, GitterBasis :: VertexGeo * b) {
  return new Objects :: hedge1_IMPL (0, a, b, indexManager(2) ) ;
}

inline GitterBasis :: hface3_GEO * GitterBasis :: MacroGitterBasis :: insert_hface3 (hedge1_GEO *(&e)[3], int (&s)[3]) {
  return new Objects :: hface3_IMPL (0,e[0],s[0],e[1],s[1],e[2],s[2], indexManager(1) ) ;
}

inline GitterBasis :: hface4_GEO * GitterBasis :: MacroGitterBasis :: insert_hface4 (hedge1_GEO *(&e)[4], int (&s)[4]) {
  return new Objects :: hface4_IMPL (0, e[0],s[0],e[1],s[1],e[2],s[2],e[3],s[3], indexManager(1) ) ;
}

inline GitterBasis :: tetra_GEO * GitterBasis :: MacroGitterBasis :: insert_tetra (hface3_GEO *(&f)[4], int (&t)[4]) {
  return new Objects :: tetra_IMPL (0,f[0],t[0],f[1],t[1],f[2],t[2],f[3],t[3], indexManager(0) , _myGrid ) ;
}

inline GitterBasis :: periodic3_GEO * GitterBasis :: MacroGitterBasis :: insert_periodic3 (hface3_GEO *(&f)[2], int (&t)[2]) {
  return new Objects :: periodic3_IMPL (0,f[0],t[0],f[1],t[1]) ;
}

inline GitterBasis :: periodic4_GEO * GitterBasis :: MacroGitterBasis :: insert_periodic4 (hface4_GEO *(&f)[2], int (&t)[2]) {
  return new Objects :: periodic4_IMPL (0,f[0],t[0],f[1],t[1]) ;
}

inline GitterBasis :: hexa_GEO * GitterBasis :: MacroGitterBasis :: insert_hexa (hface4_GEO *(&f)[6], int (&t)[6]) {
  return new Objects :: hexa_IMPL (0,f[0],t[0],f[1],t[1],f[2],t[2],f[3],t[3],f[4],t[4],f[5],t[5], indexManager(0), _myGrid ) ;
}

inline GitterBasis :: hbndseg3_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hbnd3 (hface3_GEO * f, int i, Gitter :: hbndseg_STI :: bnd_t b) {
  // the two last zeros are the following: 
  // the first pointer is pointer to grid which we dont need in the serial
  // case and the second is a pointer to ghost Element 
  return new Objects :: hbndseg3_IMPL ( 0,f,i,NULL, b, indexManager(4) ) ;
}

inline GitterBasis :: hbndseg3_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hbnd3 (hface3_GEO * f, int i, Gitter :: hbndseg_STI :: bnd_t b, const double (&p)[3]) {
  return insert_hbnd3(f,i,b); 
}

inline GitterBasis :: hbndseg4_GEO * GitterBasis :: MacroGitterBasis :: insert_hbnd4 (hface4_GEO * f, int i, Gitter :: hbndseg_STI :: bnd_t b) {
  return new Objects :: hbndseg4_IMPL ( 0,f,i,NULL, b,indexManager(4) );
}

inline IndexManagerType & GitterBasis :: MacroGitterBasis :: indexManager (int codim ) 
{ 
  assert((codim >= 0) && (codim < numOfIndexManager ));
  return _indexmanager[codim];
}

#endif  //  GITTER_IMPL_H_INCLUDED
