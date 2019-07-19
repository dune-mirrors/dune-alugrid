// (c) Bernhard Schupp 1997 - 1998
// modification for the dune interface
// (c) Robert Kloefkorn 2004 - 2005
#ifndef GITTER_IMPL_H_INCLUDED
#define GITTER_IMPL_H_INCLUDED

#include "gitter_sti.h"

#include "mapp_tetra_3d.h"

#include "gitter_hexa_top.h"
#include "gitter_tetra_top.h"

namespace ALUGrid
{

  class MacroGhostInfo;

  class GitterBasis
  : public virtual Gitter,
    public Gitter::Geometric
  {
  public:
    class Objects
    {
      public :
        class VertexEmpty : public VertexGeo
        {
          public :
            inline VertexEmpty (int, double, double, double,IndexManagerStorageType &ims);
            inline VertexEmpty (int, double, double, double,VertexGeo & );
           ~VertexEmpty () {}
            virtual inline int ident () const;
        };

        class VertexEmptyMacro : public VertexEmpty {
          public :
            inline VertexEmptyMacro (double, double, double, int,IndexManagerStorageType &ims);
            ~VertexEmptyMacro () {}
            virtual inline int ident () const;
          private :
            int _idn;
        };

        class Hbnd3Default : public hbndseg3_GEO
        {
          protected :
           inline Hbnd3Default (myhface3_t *, int);
           virtual ~Hbnd3Default () {}
           //! return pointer to grid
           Gitter * myGrid() { return myhface(0)->myvertex(0)->myGrid(); }
           const Gitter * myGrid() const { return myhface(0)->myvertex(0)->myGrid(); }
          public :
            typedef hbndseg3_GEO::bnd_t bnd_t;
            virtual inline bnd_t bndtype () const;
            virtual int ghostLevel () const;
            virtual bool ghostLeaf () const;

            // default implementation is doing nothing for these 3 methods
            // these methods are overloades just on HbndPll
            virtual const ghostpair_STI & getGhost () const
            {
              static ghostpair_STI p( (helement_STI *)0, -1);
              return p;
            }

            virtual void projectGhostElement( const int face, const std::array< std::array<alucoord_t,3>, 8 >& newCoords, const double volume )
            {
              myhface3_t* f = myhface( 0 );
              for( int i=0; i<3; ++i )
              {
                f->myvertex( i )->setCoordinates( newCoords[ i ] );
              }

              ghostpair_STI gp = getGhost();
              // ghost element may not exists (i.e. bisection refined grids)
              if( gp.first )
              {
                gp.first->changeVertexCoordinates( face, newCoords, volume );
              }
            }

            // default implementation returns 0
            virtual const MacroGhostInfo* buildGhostCell(ObjectStream&, int) { return 0; }
        };
        typedef Hbnd3Top < Hbnd3Default > hbndseg3_IMPL;

        class Hbnd4Default : public hbndseg4_GEO
        {
          protected :
            inline Hbnd4Default (myhface4_t *, int);
            virtual ~Hbnd4Default () {}
            //! return pointer to grid
            Gitter * myGrid() { return myhface(0)->myvertex(0)->myGrid(); }
            const Gitter * myGrid() const { return myhface(0)->myvertex(0)->myGrid(); }
          public :
            typedef hbndseg4_GEO::bnd_t bnd_t;
            virtual inline bnd_t bndtype () const;
            virtual int ghostLevel () const;
            virtual bool ghostLeaf () const;

            // default implementation is doing nothing for these 3 methods
            // these methods are overloades just on HbndPll
            virtual const ghostpair_STI & getGhost () const
            {
              static ghostpair_STI p( (helement_STI *)0, -1);
              return p;
            }

            virtual void projectGhostElement( const int face, const std::array< std::array<alucoord_t,3>, 8 >& newCoords, const double volume )
            {
              myhface4_t* f = myhface( 0 );
              for( int i=0; i<4; ++i )
              {
                f->myvertex( i )->setCoordinates( newCoords[ i ] );
              }

              ghostpair_STI gp = getGhost();
              if( gp.first )
              {
                gp.first->changeVertexCoordinates( face, newCoords, volume );
              }
            }

            // default implementation returns 0
            virtual const MacroGhostInfo * buildGhostCell(ObjectStream&, int) { return 0; }
        };
        typedef Hbnd4Top < Hbnd4Default > hbndseg4_IMPL;

        class Hedge1Empty : public hedge1_GEO {
          protected :
            typedef VertexEmpty innervertex_t;
            inline Hedge1Empty (myvertex_t *,myvertex_t *);
            ~Hedge1Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectInnerVertex(const ProjectVertex &pv);
        };

        typedef Hedge1Top < Hedge1Empty > hedge1_IMPL;

        class Hface3Empty : public hface3_GEO {
          protected :
            typedef VertexEmpty   innervertex_t;
            typedef hedge1_IMPL   inneredge_t;
            inline Hface3Empty (myhedge1_t *,int, myhedge1_t *,int, myhedge1_t *,int);
           ~Hface3Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectVertex(const ProjectVertex &pv);
        };
        typedef Hface3Top < Hface3Empty > hface3_IMPL;


        class Hface4Empty : public hface4_GEO
        {
         protected :
           typedef VertexEmpty innervertex_t;
           typedef hedge1_IMPL     inneredge_t;
           inline Hface4Empty (myhedge1_t *,int, myhedge1_t *,int, myhedge1_t *,int,myhedge1_t *,int);
           ~Hface4Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectVertex(const ProjectVertex &pv);
        };
        typedef Hface4Top < Hface4Empty > hface4_IMPL;


      class TetraEmpty : public tetra_GEO
      {
    protected :
        typedef hface3_IMPL innerface_t;
        typedef hedge1_IMPL inneredge_t;
        typedef VertexEmpty innervertex_t;
        inline TetraEmpty (myhface3_t *,int,myhface3_t *,int,myhface3_t *,int,myhface3_t *,int);

      public:
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
      protected:
        // declare this element and all parts leaf
        virtual void attachleafs();

        // this element is not leaf anymore
        virtual void detachleafs();

        // check that all indices are within range of index manager
        virtual void resetGhostIndices();

      protected:
        ~TetraEmpty () {}

        int preCoarsening  ();
        int postRefinement ();

        //! return pointer to grid
        Gitter * myGrid() { return myvertex(0)->myGrid(); }
        const Gitter * myGrid() const { return myvertex(0)->myGrid(); }
      public:
        //ghost tetra gets indices of grid, to which it belongs actually
        virtual void setIndicesAndBndId (const hface_STI & f, int face_nr);
        // return MPI rank of master which is always the same as myrank
        int master() const { return myvertex(0)->indexManagerStorage ().myrank(); }

    private:
        //ghost tetra gets indices of grid, to which it belongs actually
        void setGhostBoundaryIds();

        // for _myGrid
        friend class TetraTop < TetraEmpty >;
    };
    typedef TetraTop < TetraEmpty > tetra_IMPL;


    class Periodic3Empty : public periodic3_GEO {
      protected :
        typedef hface3_IMPL innerface_t;
        typedef hedge1_IMPL inneredge_t;
        typedef VertexEmpty innervertex_t;
        typedef tetra_IMPL GhostElement_t;
        typedef periodic3_GEO::myneighbour_t myneighbour_t;

        inline Periodic3Empty (myhface3_t *,int,myhface3_t *,int);
        ~Periodic3Empty () {}
        // do nothing here
        virtual void resetGhostIndices() {}
        // return MPI rank of master which is always the same as myrank
        int master() const { return myvertex(0)->indexManagerStorage ().myrank(); }
      public:
    };
    typedef Periodic3Top < Periodic3Empty > periodic3_IMPL;


      class HexaEmpty : public hexa_GEO {
      protected :
        typedef hface4_IMPL innerface_t;
        typedef hedge1_IMPL inneredge_t;
        typedef VertexEmpty innervertex_t;
        inline HexaEmpty (myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int,myhface4_t *,int);
        ~HexaEmpty () {}

        // Neu: burriad 29.4.05
        int preCoarsening();
        int postRefinement();

        //! return pointer to grid
        Gitter * myGrid() { return myvertex(0)->myGrid(); }
        const Gitter * myGrid() const { return myvertex(0)->myGrid(); }

      public:
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

      protected:
        virtual void attachleafs();
        virtual void detachleafs();

        // check that all indices are within range of index manager
        virtual void resetGhostIndices();

        friend class HexaTop<HexaEmpty>;
      public:
        //ghost hexa gets indices of grid, to which it belongs actually
        virtual void setIndicesAndBndId (const hface_STI & f, int face_nr);
        // return MPI rank of master which is always the same as myrank
        int master() const { return myvertex(0)->indexManagerStorage ().myrank(); }

      private:
        //ghost tetra gets indices of grid, to which it belongs actually
        void setGhostBoundaryIds();
      };
      typedef HexaTop < HexaEmpty > hexa_IMPL;


      class Periodic4Empty : public periodic4_GEO
      {
      protected :
        typedef hface4_IMPL innerface_t;
        typedef hedge1_IMPL inneredge_t;
        typedef VertexEmpty innervertex_t;
        typedef hexa_IMPL GhostElement_t;

        inline Periodic4Empty (myhface4_t *,int,myhface4_t *,int);
        ~Periodic4Empty () {}

        // so nothing here
        virtual void resetGhostIndices() {}

        // return MPI rank of master which is always the same as myrank
        int master() const { return myvertex(0)->indexManagerStorage ().myrank(); }
      public:
      };
      typedef Periodic4Top < Periodic4Empty > periodic4_IMPL;
    };

  public :
    class MacroGitterBasis
    : public virtual BuilderIF
    {
    protected :
      virtual VertexGeo     * insert_vertex (double, double, double, int);
      virtual VertexGeo     * insert_ghostvx (double, double, double, int);
      virtual hedge1_GEO    * insert_hedge1 (VertexGeo *, VertexGeo *);
      virtual hface3_GEO    * insert_hface3 (hedge1_GEO *(&)[3], int (&)[3]);
      virtual hface4_GEO    * insert_hface4 (hedge1_GEO *(&)[4], int (&)[4]);
      virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, Gitter::hbndseg_STI::bnd_t)      ;
      // version with point , returns insert_hbnd3 here
      virtual hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, Gitter::hbndseg_STI::bnd_t, MacroGhostInfoTetra* );
      virtual hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, Gitter::hbndseg_STI::bnd_t);
      virtual hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, Gitter::hbndseg_STI::bnd_t, MacroGhostInfoHexa* );
      virtual tetra_GEO     * insert_tetra (hface3_GEO *(&)[4], int (&)[4], SimplexTypeFlag );
      virtual periodic3_GEO * insert_periodic3 (hface3_GEO *(&)[2], int (&)[2], const Gitter:: hbndseg_STI::bnd_t (&)[2]);

      virtual periodic4_GEO * insert_periodic4 (hface4_GEO *(&)[2], int (&)[2], const Gitter:: hbndseg_STI::bnd_t (&)[2]);
      virtual hexa_GEO      * insert_hexa (hface4_GEO *(&)[6], int (&)[6]);
    public :
      // Gitter is a reference to our grid
      // constructors creating macro grids from streams
      MacroGitterBasis ( const int, Gitter *, const ProjectVertexPtrPair& ptr, std::istream & );

      // constructor creating an empty macro grid
      MacroGitterBasis ( const int, Gitter *, const ProjectVertexPtrPair& ptr );

      virtual ~MacroGitterBasis () {}
    };
  };


  class GitterBasisImpl
  : public virtual GitterBasis
  {
    MacroGitterBasis*  _macrogitter;
  public:
    Makrogitter & container ();
    const Makrogitter & container () const;

    IndexManagerType & indexManager(int codim);
    IndexManagerStorageType& indexManagerStorage();

    std::size_t numMacroBndSegments() const;

    explicit GitterBasisImpl ( const int );
    GitterBasisImpl ( const int, std::istream &, const ProjectVertexPtrPair& );
    GitterBasisImpl ( const int, const char *, const ProjectVertexPtrPair& );
    ~GitterBasisImpl ();

    virtual void printMemUsage ();

    // return pointer to vertex projection
    // virtual ProjectVertex* vertexProjection() const;
  };


    //
    //    #    #    #  #          #    #    #  ######
    //    #    ##   #  #          #    ##   #  #
    //    #    # #  #  #          #    # #  #  #####
    //    #    #  # #  #          #    #  # #  #
    //    #    #   ##  #          #    #   ##  #
    //    #    #    #  ######     #    #    #  ######
    //

  inline GitterBasis::Objects::VertexEmpty::VertexEmpty (int l, double x, double y, double z, IndexManagerStorageType & ims)
    : GitterBasis::VertexGeo (l,x,y,z,ims)
  {
  }

  inline GitterBasis::Objects::VertexEmpty::VertexEmpty (int l, double x, double y, double z, VertexGeo & vx )
    : GitterBasis::VertexGeo (l,x,y,z,vx)
  {
    if( vx.is2d() ) this->set2dFlag();
  }

  inline int GitterBasis::Objects::VertexEmpty::ident () const
  {
    std::cerr << "ERROR (FATAL): vertex::ident() may only be called for macro vertices." << std::endl;
    abort();
    return -1;
  }

  inline GitterBasis::Objects::VertexEmptyMacro::VertexEmptyMacro (double x,double y,double z,int i, IndexManagerStorageType &ims)
  : GitterBasis::Objects::VertexEmpty (0,x,y,z,ims), _idn (i)
  {
    // if vertex identifier is odd then it's a 2d vertex if the grid is 2d
    if(ims.dimension() == 2 && i%2 ) this->set2dFlag();
    return;
  }

  inline int GitterBasis::Objects::VertexEmptyMacro::ident () const {
    return _idn;
  }

  inline GitterBasis::Objects::Hedge1Empty::Hedge1Empty (myvertex_t * a, myvertex_t * b)
    : Gitter::Geometric::hedge1_GEO (a,b) {
    return;
  }

  inline void GitterBasis::Objects::Hedge1Empty::projectInnerVertex(const ProjectVertex &pv)
  {
    if (innerVertex()) {
      alugrid_assert (!leaf());
      innerVertex()->project(pv);
    }
  }

  inline GitterBasis::Objects::Hface3Empty::Hface3Empty (myhedge1_t *e0, int s0,
    myhedge1_t *e1, int s1, myhedge1_t *e2, int s2) : Gitter::Geometric::hface3_GEO (e0, s0, e1, s1, e2, s2) {
    return;
  }

  inline void GitterBasis::Objects::Hface3Empty::projectVertex(const ProjectVertex &pv)
  {
    alugrid_assert (!leaf());
    for (int e = 0; e < polygonlength; e++)
      myhedge1(e)->projectInnerVertex(pv);
    if (innerVertex())
      innerVertex()->project(pv);
  }

  inline GitterBasis::Objects::Hface4Empty::Hface4Empty (myhedge1_t *e0, int s0,
    myhedge1_t *e1, int s1, myhedge1_t *e2, int s2, myhedge1_t *e3, int s3)
    : Gitter::Geometric::hface4_GEO (e0, s0, e1, s1, e2, s2, e3, s3) {
    return;
  }

  inline void GitterBasis::Objects::Hface4Empty::projectVertex(const ProjectVertex &pv) {
    for (int e = 0; e < polygonlength; e++)
      myhedge1(e)->projectInnerVertex(pv);
    if (innerVertex())
      innerVertex()->project(pv);
  }

  inline GitterBasis::Objects::Hbnd3Default::
  Hbnd3Default (myhface3_t * f, int i )
   : Gitter::Geometric::hbndseg3_GEO (f, i)
  {
    return;
  }

  inline GitterBasis::Objects ::Hbnd3Default::bnd_t GitterBasis::Objects::Hbnd3Default::bndtype () const {
    return undefined;
  }

  inline int GitterBasis::Objects::Hbnd3Default::ghostLevel () const {
    return level();
  }

  inline bool GitterBasis::Objects::Hbnd3Default::ghostLeaf () const {
    return leaf();
  }

  inline GitterBasis::Objects::Hbnd4Default::Hbnd4Default (myhface4_t * f, int i) :
    Gitter::Geometric::hbndseg4_GEO (f, i)
  {
    return;
  }

  inline GitterBasis::Objects ::Hbnd4Default::bnd_t GitterBasis::Objects::Hbnd4Default::bndtype () const {
    return undefined;
  }

  inline int GitterBasis::Objects::Hbnd4Default::ghostLevel () const {
    return level();
  }

  inline bool GitterBasis::Objects::Hbnd4Default::ghostLeaf () const {
    return leaf();
  }

  inline GitterBasis::Objects::TetraEmpty::
  TetraEmpty (myhface3_t * f0, int t0, myhface3_t * f1, int t1,
              myhface3_t * f2, int t2, myhface3_t * f3, int t3) :
    Gitter::Geometric::Tetra (f0, t0, f1, t1, f2, t2, f3, t3)
  {
    attachleafs();
    return;
  }

  // calles method on grid which return 0 for default impl
  inline int GitterBasis::Objects::TetraEmpty::preCoarsening ()
  {
    // only call preCoarsening on non ghost elements
    return ((this->isGhost()) ? 0 : myGrid()->preCoarsening(*this));
  }

  // calles method on grid which return 0 for default impl
  inline int GitterBasis::Objects::TetraEmpty::postRefinement ()
  {
    // NOTE: this method is called after an element is refined removing the Refined Tag
    // from the parent element - for bisection this is wrong since the parent has
    // possibly also been generated in the same refinement cycle

    // OLD: // reset refined tag of this element because no leaf anymore
    // OLD: this->resetRefinedTag();

    // only call postRefinement on non ghost elements
    return ((this->isGhost()) ? 0 : myGrid()->postRefinement(*this));
  }

  inline GitterBasis::Objects::Periodic3Empty::Periodic3Empty (myhface3_t * f0, int t0, myhface3_t * f1, int t1)
    : Gitter::Geometric::Periodic3 (f0, t0, f1, t1) {
    return;
  }

  inline GitterBasis::Objects::Periodic4Empty::Periodic4Empty (myhface4_t * f0, int t0, myhface4_t * f1, int t1)
    : Gitter::Geometric::Periodic4 (f0, t0, f1, t1) {
    return;
  }

  // Neu: burriad 29.4.05
  inline GitterBasis::Objects::HexaEmpty::
  HexaEmpty (myhface4_t * f0, int t0, myhface4_t * f1, int t1,
             myhface4_t * f2, int t2, myhface4_t * f3, int t3,
             myhface4_t * f4, int t4, myhface4_t * f5, int t5) :
    Gitter::Geometric::hexa_GEO(f0, t0, f1, t1, f2, t2, f3, t3, f4, t4, f5, t5)
  {
    attachleafs();
    return;
  }

  inline int GitterBasis::Objects::HexaEmpty::preCoarsening()
  {
    // only call preCoarsening on non ghost elements
    return ((this->isGhost()) ? 0 : myGrid()->preCoarsening(*this));
  }

  inline int GitterBasis::Objects::HexaEmpty::postRefinement()
  {
    // NOTE: this method is called after an element is refined removing the Refined Tag
    // from the parent element - for bisection this is wrong since the parent has
    // possibly also been generated in the same refinement cycle
    // This is only a problem for bisection, i.e., on TetraEmpty but the reset method
    // is also removed here for symmetry (and the reset is apparently not needed...)

    // OLD: // reset refined tag of this element because no leaf anymore
    // OLD: this->resetRefinedTag();

    // only call postRefinement on non ghost elements
    return ((this->isGhost()) ? 0 : myGrid()->postRefinement(*this));
  }

  ////////////////////////////////////////////////////////////////
  //
  //  --GitterBasisImpl
  //
  ////////////////////////////////////////////////////////////////
  inline Gitter::Makrogitter & GitterBasisImpl::container () { return *_macrogitter; }

  inline const Gitter::Makrogitter & GitterBasisImpl::container () const { return *_macrogitter; }

  inline IndexManagerType & GitterBasisImpl::indexManager ( int codim )
  {
    return _macrogitter->indexManager( codim );
  }

  inline IndexManagerStorageType&  GitterBasisImpl::indexManagerStorage()
  {
    return _macrogitter->indexManagerStorage();
  }

  inline std::size_t GitterBasisImpl::numMacroBndSegments() const
  {
    return _macrogitter->numMacroBndSegments();
  }

} // namespace ALUGrid

#endif // #ifndef GITTER_IMPL_H_INCLUDED
