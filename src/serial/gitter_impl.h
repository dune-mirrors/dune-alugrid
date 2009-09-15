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

            inline int preCoarsening  () ;
            inline int postRefinement () ;

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
            inline int preCoarsening  () ; 
            inline int postRefinement () ;
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
        

        class Hface4Empty : public hface4_GEO 
        {
         protected :
           typedef VertexEmpty innervertex_t ;
           typedef hedge1_IMPL     inneredge_t ;
           inline Hface4Empty (myhedge1_t *,int, myhedge1_t *,int, myhedge1_t *,int,myhedge1_t *,int) ;
           ~Hface4Empty () {}
           // Methode um einen Vertex zu verschieben; f"ur die Randanpassung
           virtual inline void projectVertex(const ProjectVertex &pv) ; 
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
        virtual void os2VertexData(ObjectStream & os, GatherScatterType & gs , int borderFace ) 
        {
          // only one opposite vertex for tetras 
          gs.setData( os, *myvertex( borderFace ));
        }
        
        virtual void os2EdgeData(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
        {
          const vector<int> & edgesNotOnFace = 
            Gitter :: Geometric :: tetra_GEO :: edgesNotOnFace( borderFace );
          const int numEdges = edgesNotOnFace.size();
          assert( numEdges == 3 );
          for(int e = 0; e<numEdges; ++e)
          {
            gs.setData( os, *myhedge1( edgesNotOnFace[e] ) );
          }
        }
        virtual void os2FaceData(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
        {
          const vector<int> & facesNotOnFace = 
            Gitter :: Geometric :: tetra_GEO :: facesNotOnFace( borderFace );
          const int numFaces = facesNotOnFace.size();
          assert( numFaces == 3 );
          for (int i = 0; i <numFaces; ++i) 
          {
            gs.setData( os, *myhface3( facesNotOnFace[i] ) );
          }
        }

        /////////////////////////////////////////
        //  writing of data 
        /////////////////////////////////////////
        virtual void VertexData2os(ObjectStream & os, GatherScatterType & gs, int borderFace )
        {
          // only send one vertex
          gs.sendData( os, *myvertex(borderFace) );
        }
        
        virtual void EdgeData2os(ObjectStream & os, GatherScatterType & gs, int borderFace)
        {
          const vector<int> & edgesNotOnFace = 
            Gitter :: Geometric :: tetra_GEO :: edgesNotOnFace( borderFace );
          const int numEdges = edgesNotOnFace.size();
          assert( numEdges == 3 );
          for(int e=0; e<numEdges; ++e)
          {
            gs.sendData( os, *myhedge1( edgesNotOnFace[e] ) );
          }
        }
        
        virtual void FaceData2os(ObjectStream & os, GatherScatterType & gs, int borderFace) 
        {
          const vector<int> & facesNotOnFace = 
            Gitter :: Geometric :: tetra_GEO :: facesNotOnFace( borderFace );
          const int numFaces = facesNotOnFace.size();
          assert( numFaces == 3 );
          for (int i = 0; i <numFaces; ++i)
          {
            gs.sendData( os,  *myhface3( facesNotOnFace[i] ) );
          }
        }

        /////////////////////////////////////////

        // declare this element and all parts leaf  
        virtual void attachleafs() 
        {  
          addleaf();
          for (int i = 0; i < 4 ; ++i) myhface3(i)->addleaf();
          for (int i = 0; i < 6 ; ++i) myhedge1(i)->addleaf();
          for (int i = 0; i < 4 ; ++i) myvertex(i)->addleaf();
        }
        
        // this element is not leaf anymore 
        virtual void detachleafs() 
        { 
          removeleaf();
          for (int i = 0; i < 4 ; ++i) myhface3(i)->removeleaf();
          for (int i = 0; i < 6 ; ++i) myhedge1(i)->removeleaf();
          for (int i = 0; i < 4 ; ++i) myvertex(i)->removeleaf();
        }

        // check that all indices are within range of index manager
        virtual void resetGhostIndices() 
        {  
          // only set indices for macro level ghosts 
          if( this->level() > 0 ) return ;
          
          {
            typedef Gitter :: Geometric :: BuilderIF BuilderIF;
            // only call for ghosts 
            assert( this->isGhost() ); 

            // check my index first 
            resetGhostIndex(_myGrid->indexManager(BuilderIF :: IM_Elements ));
           
            {
              // get index manager of faces 
              IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Faces);
              for (int i = 0; i < 4 ; ++i) myhface3(i)->resetGhostIndex(im);
            }
            {
              // get index manager of edges 
              IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Edges);
              for (int i = 0; i < 6 ; ++i) myhedge1(i)->resetGhostIndex(im);
            }
            {
              // get index manager of vertices 
              IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Vertices);
              for (int i = 0; i < 4 ; ++i) myvertex(i)->resetGhostIndex(im);
            }
          }
        }
        
      protected:     
        ~TetraEmpty () {}
        
        int preCoarsening  () ; 
        int postRefinement () ;

        Gitter * _myGrid;
      public: 
        //ghost tetra gets indices of grid, to which it belongs actually
        virtual void setIndicesAndBndId (const hface_STI & f, int face_nr) 
        {
          // set all items to ghost bnd id 
          setGhostBoundaryIds();
          
          typedef Gitter :: Geometric :: BuilderIF BuilderIF;
            
          typedef Gitter :: Geometric :: vertex_GEO vertex_GEO; 
          typedef Gitter :: Geometric :: hedge1_GEO hedge1_GEO; 
      
          const myhface3_t & face = static_cast<const myhface3_t &> (f); 
          const bndid_t bndid = face.bndId ();

          myhface3_t & myface = *(myhface3(face_nr));

          // set index of face 
          myface.setIndex(_myGrid->indexManager(BuilderIF :: IM_Faces), face.getIndex());
          // set bnd id of face 
          myface.setGhostBndId( bndid );

          IndexManagerType & vxIm = _myGrid->indexManager(BuilderIF :: IM_Vertices);
          IndexManagerType & edIm = _myGrid->indexManager(BuilderIF :: IM_Edges);
          
          for (int i = 0; i < 3; ++i) 
          {
            // make sure we got the right face 
            assert(fabs(myface.myvertex(i)->Point()[0]-
                   face.myvertex(i)->Point()[0])<1e-8);
            assert(fabs(myface.myvertex(i)->Point()[1]-
                   face.myvertex(i)->Point()[1])<1e-8);
            assert(fabs(myface.myvertex(i)->Point()[2]-
                   face.myvertex(i)->Point()[2])<1e-8);

            vertex_GEO * vx = myface.myvertex(i); 
            vx->setIndex( vxIm , face.myvertex(i)->getIndex() );
            vx->setGhostBndId( bndid );
           
            hedge1_GEO * edge = myface.myhedge1(i);
            edge->setIndex( edIm , face.myhedge1(i)->getIndex() );
            edge->setGhostBndId( bndid );
          }
        }
    
    private:     
        //ghost tetra gets indices of grid, to which it belongs actually
        void setGhostBoundaryIds() 
        {
          const bndid_t bndid = Gitter :: hbndseg_STI :: ghost_closure ; 
          
          // value of ghost_closure 
          this->setGhostBndId( bndid );
          for( int i=0; i<4 ; ++i) myhface3(i)->setGhostBndId( bndid );
          for( int i=0; i<6 ; ++i) myhedge1(i)->setGhostBndId( bndid );
          for( int i=0; i<4 ; ++i) myvertex(i)->setGhostBndId( bndid );
        }
        
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
        virtual void os2VertexData(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
        {
          const vector<int> & verticesNotOnFace = 
            Gitter :: Geometric :: hexa_GEO :: verticesNotOnFace( borderFace );
          const int numVertices = verticesNotOnFace.size();
          assert( numVertices == 4 );
          for (int i = 0; i <numVertices; ++i) 
          {
            gs.setData( os, *myvertex( verticesNotOnFace[i] ) );
          }
        }
        
        // scatter data on ghost edges  
        virtual void os2EdgeData(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
        {
          const vector<int> & edgesNotOnFace = 
            Gitter :: Geometric :: hexa_GEO :: edgesNotOnFace( borderFace );
          const int numEdges = edgesNotOnFace.size();
          assert( numEdges == 8 );
          for(int e = 0; e<numEdges; ++e)
          {
            gs.setData( os, *myhedge1( edgesNotOnFace[e] ) );
          }
        }

        // scatter data on ghost faces 
        virtual void os2FaceData(ObjectStream & os, GatherScatterType & gs, int borderFace ) 
        {
          const vector<int> & facesNotOnFace = 
            Gitter :: Geometric :: hexa_GEO :: facesNotOnFace( borderFace );
          const int numFaces = facesNotOnFace.size();
          assert( numFaces == 5 );
          for (int i = 0; i <numFaces; ++i) 
          {
            gs.setData( os, *myhface4( facesNotOnFace[i] ) );
          }
        }

        //////////////////////////////////////////
        //  writing of data 
        //////////////////////////////////////////
        virtual void VertexData2os(ObjectStream & os, GatherScatterType & gs, int borderFace )
        {
          const vector<int> & verticesNotOnFace = 
            Gitter :: Geometric :: hexa_GEO :: verticesNotOnFace( borderFace );
          const int numVertices = verticesNotOnFace.size();
          assert( numVertices == 4 );
          for (int i = 0; i <numVertices; ++i)
          {
            gs.sendData( os, *myvertex( verticesNotOnFace[i] ) );
          }
        }
        
        virtual void EdgeData2os(ObjectStream & os, GatherScatterType & gs, int borderFace)
        {
          const vector<int> & edgesNotOnFace = 
            Gitter :: Geometric :: hexa_GEO :: edgesNotOnFace( borderFace );
          const int numEdges = edgesNotOnFace.size();
          assert( numEdges == 8 );
          for(int e=0; e<numEdges; ++e)
          {
            gs.sendData( os, *myhedge1( edgesNotOnFace[e] ) );
          }
        }
        
        virtual void FaceData2os(ObjectStream & os, GatherScatterType & gs, int borderFace) 
        {
          const vector<int> & facesNotOnFace = 
            Gitter :: Geometric :: hexa_GEO :: facesNotOnFace( borderFace );
          const int numFaces = facesNotOnFace.size();
          assert( numFaces == 5 );
          for (int i = 0; i < numFaces; ++i)
          {
            gs.sendData( os, *myhface4( facesNotOnFace[i] ) );
          }
        }

        virtual void attachleafs() {  
          assert(this->leafRefCount()==0);
          addleaf();
          for (int i = 0; i < 6 ; ++i) myhface4(i)->addleaf();
          for (int i = 0; i < 12; ++i) myhedge1(i)->addleaf();
          for (int i = 0; i < 8 ; ++i) myvertex(i)->addleaf();
        }
        virtual void detachleafs() 
        {
          assert(this->leafRefCount()==1);
          removeleaf();
          for (int i = 0; i < 6 ; ++i) myhface4(i)->removeleaf();
          for (int i = 0; i < 12; ++i) myhedge1(i)->removeleaf();
          for (int i = 0; i < 8 ; ++i) myvertex(i)->removeleaf();
        }

        // check that all indices are within range of index manager
        virtual void resetGhostIndices() 
        {  
          // only set indices for macro level ghosts 
          if( this->level() > 0 ) return ;
          
          {
            typedef Gitter :: Geometric :: BuilderIF BuilderIF;
            // only call for ghosts 
            assert( this->isGhost() ); 

            // check my index first 
            resetGhostIndex(_myGrid->indexManager(BuilderIF :: IM_Elements ));
           
            {
              // get index manager of faces 
              IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Faces);
              for (int i = 0; i < 6 ; ++i) myhface4(i)->resetGhostIndex(im);
            }
            {
              // get index manager of edges 
              IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Edges);
              for (int i = 0; i < 12 ; ++i) myhedge1(i)->resetGhostIndex(im);
            }
            {
              // get index manager of vertices 
              IndexManagerType & im = _myGrid->indexManager(BuilderIF :: IM_Vertices);
              for (int i = 0; i < 8 ; ++i) myvertex(i)->resetGhostIndex(im);
            }
          }
        }
        
        Gitter * _myGrid;
        friend class HexaTop<HexaEmpty>;
      public:
        //ghost hexa gets indices of grid, to which it belongs actually
        virtual void setIndicesAndBndId (const hface_STI & f, int face_nr)  
        {
           // set all items to ghost bnd id
           setGhostBoundaryIds();
          
           typedef Gitter :: Geometric :: BuilderIF BuilderIF;
            
           typedef Gitter :: Geometric :: vertex_GEO vertex_GEO; 
           typedef Gitter :: Geometric :: hedge1_GEO hedge1_GEO; 
          
           const myhface4_t & face = static_cast<const myhface4_t &> (f); 
           const bndid_t bndid = face.bndId();
           
           myhface4_t & myface = *(myhface4(face_nr));

           IndexManagerType & vxIm = _myGrid->indexManager(BuilderIF :: IM_Vertices);
           IndexManagerType & edIm = _myGrid->indexManager(BuilderIF :: IM_Edges);

           // set index of face 
           myface.setIndex( _myGrid->indexManager(BuilderIF :: IM_Faces) , face.getIndex ());
           // set bnd id of face 
           myface.setGhostBndId( bndid );
           
           for (int i = 0; i < 4; ++i) 
           {
             // make sure we got the right face 
             assert(fabs(myface.myvertex(i)->Point()[0]-
                    face.myvertex(i)->Point()[0])<1e-8);
             assert(fabs(myface.myvertex(i)->Point()[1]-
                    face.myvertex(i)->Point()[1])<1e-8);
             assert(fabs(myface.myvertex(i)->Point()[2]-
                    face.myvertex(i)->Point()[2])<1e-8);
             
             vertex_GEO * vx = myface.myvertex(i); 
             vx->setIndex(vxIm, face.myvertex(i)->getIndex());
             vx->setGhostBndId( bndid );
             
             hedge1_GEO * edge = myface.myhedge1(i);
             edge->setIndex(edIm, face.myhedge1(i)->getIndex());
             edge->setGhostBndId( bndid );
           }
        }
      private:
        //ghost tetra gets indices of grid, to which it belongs actually
        void setGhostBoundaryIds() 
        {
          const bndid_t bndid = Gitter :: hbndseg_STI :: ghost_closure ; 

          // value of ghost_closure 
          this->setGhostBndId( bndid );
          for( int i=0; i<6 ; ++i) myhface4(i)->setGhostBndId( bndid );
          for( int i=0; i<12; ++i) myhedge1(i)->setGhostBndId( bndid );
          for( int i=0; i<8 ; ++i) myvertex(i)->setGhostBndId( bndid );
        }
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
        virtual inline VertexGeo     * insert_vertex (double, double, double, int) ;
        virtual inline VertexGeo     * insert_ghostvx (double, double, double, int) ;
        virtual inline hedge1_GEO    * insert_hedge1 (VertexGeo *, VertexGeo *) ;
        virtual inline hface3_GEO    * insert_hface3 (hedge1_GEO *(&)[3], int (&)[3]) ;
        virtual inline hface4_GEO    * insert_hface4 (hedge1_GEO *(&)[4], int (&)[4]) ;
        virtual inline hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, ProjectVertex*, Gitter :: hbndseg_STI :: bnd_t) ;
        // version with point , returns insert_hbnd3 here 
        virtual inline hbndseg3_GEO  * insert_hbnd3 (hface3_GEO *, int, ProjectVertex*, Gitter :: hbndseg_STI :: bnd_t, MacroGhostInfoTetra* ) ;
        virtual inline hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, ProjectVertex*, Gitter :: hbndseg_STI :: bnd_t) ;
        virtual inline hbndseg4_GEO  * insert_hbnd4 (hface4_GEO *, int, ProjectVertex*, Gitter :: hbndseg_STI :: bnd_t, MacroGhostInfoHexa* ) ;
        virtual inline tetra_GEO     * insert_tetra (hface3_GEO *(&)[4], int (&)[4]) ;
        virtual inline periodic3_GEO * insert_periodic3 (hface3_GEO *(&)[2], int (&)[2]) ;

        virtual inline periodic4_GEO * insert_periodic4 (hface4_GEO *(&)[2], int (&)[2]) ;
        virtual inline hexa_GEO      * insert_hexa (hface4_GEO *(&)[6], int (&)[6]) ;
      public :
        // Gitter is a reference to our grid 
        inline MacroGitterBasis (Gitter * , istream &) ;
        inline MacroGitterBasis (Gitter * ) ;
        
        virtual ~MacroGitterBasis () {}

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

    virtual void printMemUsage ();
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

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: Hbnd3Default :: preCoarsening () 
{
  return 0;
}

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: Hbnd3Default :: postRefinement () 
{
  return 0;
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

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: Hbnd4Default :: preCoarsening () 
{
  return 0;
}

// calles method on grid which return 0 for default impl 
inline int GitterBasis :: Objects :: Hbnd4Default :: postRefinement () 
{
  return 0;
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

inline GitterBasis :: MacroGitterBasis :: MacroGitterBasis (Gitter * mygrid, istream & in) 
  : _myGrid(mygrid) 
{
  macrogridBuilder (in, _myGrid->vertexProjection() ) ;
  return ;
}

inline GitterBasis :: MacroGitterBasis :: MacroGitterBasis (Gitter * mygrid) : _myGrid(mygrid) {
  return ;
}

inline GitterBasis :: VertexGeo * GitterBasis :: MacroGitterBasis :: insert_vertex (double x, double y, double z, int id) {
  return new Objects :: VertexEmptyMacro (x, y, z, id, indexManager(IM_Vertices)) ;
}

inline GitterBasis :: VertexGeo * GitterBasis :: MacroGitterBasis :: insert_ghostvx (double x, double y, double z, int id) {
  return new Objects :: VertexEmptyMacro (x, y, z, id, indexManager(IM_Vertices)) ;
}

inline GitterBasis :: hedge1_GEO * GitterBasis :: MacroGitterBasis :: insert_hedge1 (VertexGeo * a, VertexGeo * b) {
  return new Objects :: hedge1_IMPL (0, a, b, indexManager(IM_Edges) ) ;
}

inline GitterBasis :: hface3_GEO * GitterBasis :: MacroGitterBasis :: insert_hface3 (hedge1_GEO *(&e)[3], int (&s)[3]) {
  return new Objects :: hface3_IMPL (0,e[0],s[0],e[1],s[1],e[2],s[2], indexManager(IM_Faces) ) ;
}

inline GitterBasis :: hface4_GEO * GitterBasis :: MacroGitterBasis :: insert_hface4 (hedge1_GEO *(&e)[4], int (&s)[4]) {
  return new Objects :: hface4_IMPL (0, e[0],s[0],e[1],s[1],e[2],s[2],e[3],s[3], indexManager(IM_Faces) ) ;
}

inline GitterBasis :: tetra_GEO * GitterBasis :: MacroGitterBasis :: 
insert_tetra (hface3_GEO *(&f)[4], int (&t)[4]) 
{
  return new Objects :: tetra_IMPL (0,f[0],t[0],f[1],t[1],f[2],t[2],f[3],t[3], 
                                    indexManager(IM_Elements) , _myGrid ) ;
}

// inlcudes implementation of MacroGhostTetra and MacroGhostHexa 
#include "ghost_elements.h"

inline GitterBasis :: periodic3_GEO * GitterBasis :: MacroGitterBasis :: insert_periodic3 (hface3_GEO *(&f)[2], int (&t)[2]) {

  Objects :: periodic3_IMPL * per3 = new Objects :: periodic3_IMPL (0,f[0],t[0],f[1],t[1]) ;
      
  double v[3]; //Verschiebungsvektor (von SW(myhface3(0) zu SW(myhface3(1))
  for (int i = 0; i < 3; i++) {
    v[i] = 0.3333333333*(f[1]->myvertex(0)->Point()[i] +
                         f[1]->myvertex(1)->Point()[i] +
                         f[1]->myvertex(2)->Point()[i] -
                         f[0]->myvertex(0)->Point()[i] -
                         f[0]->myvertex(1)->Point()[i] -
                         f[0]->myvertex(2)->Point()[i]); 
  }

  tetra_GEO * mytetra0 = static_cast<tetra_GEO * > (per3->myneighbour(0).first), //Tetra an Fl 0 (an dieser liegt Ghost0 an)
            * mytetra1 = static_cast<tetra_GEO * > (per3->myneighbour(1).first); //Tetra an Fl 1 ( - " -               1   )

  MacroGhostInfoTetra* allp0 = new MacroGhostInfoTetra(mytetra0, per3->myneighbour(0).second);
  MacroGhostInfoTetra* allp1 = new MacroGhostInfoTetra(mytetra1, per3->myneighbour(1).second);

  MacroGhostTetra * ghost0 = new MacroGhostTetra(*this, allp1, &(*mytetra1), v, -1.0);
  MacroGhostTetra * ghost1 = new MacroGhostTetra(*this, allp0, &(*mytetra0), v,  1.0);
  per3->setGhost(ghost0->getGhost(), 0);
  per3->setGhost(ghost1->getGhost(), 1);
  return per3;    
  //ohne Geister: return new Objects :: periodic3_IMPL (0,f[0],t[0],f[1],t[1]) ;
}

inline GitterBasis :: periodic4_GEO * GitterBasis :: MacroGitterBasis :: insert_periodic4 (hface4_GEO *(&f)[2], int (&t)[2]) {
  Objects :: periodic4_IMPL * per4 = new Objects :: periodic4_IMPL (0, f [0], t[0], f [1], t[1]) ;
  double v [3]; //Verschiebung von Schwerpunkt(myhface4(0)) zu Schwerpunkt(myhface4(1))
  for (int i = 0; i < 4; i++) {
    v[i] = 0.25*(f[1]->myvertex(0)->Point()[i] +
                 f[1]->myvertex(1)->Point()[i] +
                 f[1]->myvertex(2)->Point()[i] -
                 f[0]->myvertex(0)->Point()[i] -
                 f[0]->myvertex(1)->Point()[i] -
                 f[0]->myvertex(2)->Point()[i]);
  }

  /*
  hexa_GEO * myhexa0 = static_cast<hexa_GEO * > (per4->myneighbour(0).first), //Hexa an Fl 0 (an dieser liegt Ghost0 an)
           * myhexa1 = static_cast<hexa_GEO * > (per4->myneighbour(1).first); //Hexa an Fl 1 ( - " -               1   )

  const Hbnd4IntStoragePoints  allp0 (myhexa0, per4->myneighbour(0).second);
  const Hbnd4IntStoragePoints  allp1 (myhexa1, per4->myneighbour(1).second);
  MacroGhostHexa * ghost0 = new MacroGhostHexa(*this, allp1, &(*myhexa1), v, -1.0);
  MacroGhostHexa * ghost1 = new MacroGhostHexa(*this, allp0, &(*myhexa0), v,  1.0);
  per4->setGhost(ghost0->getGhost(), 0);
  per4->setGhost(ghost1->getGhost(), 1);  
  */
  return per4;   //  return new ObjectsPll :: Periodic4EmptyPllMacro (f [0], t[0], f [1], t[1]) ;  
}

inline GitterBasis :: hexa_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hexa (hface4_GEO *(&f)[6], int (&t)[6]) 
{
  return new Objects :: hexa_IMPL (0,f[0],t[0],f[1],t[1],f[2],t[2],f[3],t[3],f[4],t[4],f[5],t[5], 
                                   indexManager(IM_Elements), _myGrid ) ;
}

inline GitterBasis :: hbndseg3_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hbnd3 (hface3_GEO * f, int i, 
              ProjectVertex* ppv, 
              Gitter :: hbndseg_STI :: bnd_t b) 
{
  // the NULL pointer is the pointer to the father which does not exists 
  return new Objects :: hbndseg3_IMPL ( 0, f, i, ppv, b, indexManager(IM_Bnd) ) ;
}

inline GitterBasis :: hbndseg3_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hbnd3 (hface3_GEO * f, int i, ProjectVertex* ppv, 
              Gitter :: hbndseg_STI :: bnd_t b, MacroGhostInfoTetra* ) 
{
  return insert_hbnd3(f,i,ppv,b); 
}

inline GitterBasis :: hbndseg4_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hbnd4 (hface4_GEO * f, int i, ProjectVertex* ppv, Gitter :: hbndseg_STI :: bnd_t b) 
{
  return new Objects :: hbndseg4_IMPL ( 0, f, i, ppv, b, indexManager(IM_Bnd) );
}

inline GitterBasis :: hbndseg4_GEO * GitterBasis :: MacroGitterBasis :: 
insert_hbnd4 (hface4_GEO * f, int i, ProjectVertex* ppv,
              Gitter :: hbndseg_STI :: bnd_t b, MacroGhostInfoHexa* ) 
{
  return insert_hbnd4 (f,i,ppv,b); 
}

inline void GitterBasisImpl :: printMemUsage ()
{
  typedef GitterBasis :: DuneIndexProvider DuneIndexProvider; 
  typedef GitterBasis :: Objects :: tetra_IMPL tetra_IMPL ; 
  typedef GitterBasis :: Objects :: hexa_IMPL  hexa_IMPL ; 
  typedef GitterBasis :: Objects :: hbndseg3_IMPL hbndseg3_IMPL ; 
  typedef GitterBasis :: Objects :: hbndseg4_IMPL hbndseg4_IMPL ; 
  typedef GitterBasis :: Objects :: hface3_IMPL hface3_IMPL ; 
  typedef GitterBasis :: Objects :: hface4_IMPL hface4_IMPL ; 
  typedef GitterBasis :: Objects :: hedge1_IMPL hedge1_IMPL ; 
  typedef GitterBasis :: Objects :: VertexEmptyMacro VertexEmptyMacro; 
  typedef GitterBasis :: Objects :: VertexEmpty VertexEmpty; 
  typedef Gitter :: Geometric :: VertexGeo VertexGeo; 
  cout << "MyAlloc = " << sizeof(MyAlloc) << "\n";
  cout << "DuneIndexProvider = "<< sizeof(DuneIndexProvider) << "\n";
  cout << "HedgeRule = " << sizeof(Gitter :: Geometric :: Hedge1Rule) <<"\n";
  
  cout << "Tetrasize = " << sizeof(tetra_IMPL) << endl;
  cout << "Hexasize = " << sizeof(hexa_IMPL) << endl;
  cout << "Hface4 = " << sizeof(hface4_IMPL) << endl;
  cout << "Hface3 = " << sizeof(hface3_IMPL) << endl;
  cout << "Hface1 = " << sizeof(hedge1_IMPL) << endl;
  cout << "VertexMacro = " << sizeof(VertexEmptyMacro) << endl;
  cout << "VertexGeo   = " << sizeof(VertexGeo) << endl;
  cout << "Vertex = " << sizeof(VertexEmpty) << endl;
  cout << "Hbnd3  = " << sizeof(hbndseg3_IMPL) << endl;
  cout << "Hbnd4  = " << sizeof(hbndseg4_IMPL) << endl;

  {
    int totalSize = 0; 
    bool simplex = false;
    {
      AccessIterator < helement_STI > :: Handle iter (container ());
      int size = iter.size();
      iter.first(); 
      if( !iter.done() )
      {
        if( iter.item().type() == tetra )
        {
          simplex = true;
          size *= sizeof(tetra_IMPL);
        } 
        else
        {
          size *= sizeof(hexa_IMPL);
        } 
      } 
      totalSize += size;
      cout << "Macro elements: size = " << size/1024/1024 << " MB \n";
    } 
    
    {
      int size = AccessIterator < hbndseg_STI > :: Handle (container ()).size();
      size *= (simplex) ?  sizeof(hbndseg3_IMPL) : sizeof(hbndseg4_IMPL);
      cout << "Macro boundary : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    {
      int size = AccessIterator < hface_STI > :: Handle (container ()).size();
      size *= (simplex) ?  sizeof(hface3_IMPL) : sizeof(hface4_IMPL);
      cout << "Macro faces : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    {
      int size = AccessIterator < hedge_STI > :: Handle (container ()).size();
      size *= sizeof(hedge1_IMPL);
      cout << "Macro edges : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    {
      int size = AccessIterator < vertex_STI > :: Handle (container ()).size();
      size *= sizeof(VertexEmptyMacro);
      cout << "Macro vertices : size = " << size/1024/1024 << " MB \n";
      totalSize += size;
    }

    size_t build = container().memUsage();
    cout << "BuilderIF size = " << build/1024/1024 << " MB \n";
    totalSize += build;
    cout << "Overall size = " << totalSize/1024/1024 << " MB \n";
    cout << "\n" ;
  }
}
#endif  //  GITTER_IMPL_H_INCLUDED
