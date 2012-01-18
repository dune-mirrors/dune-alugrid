// (c) mario ohlberger, 1998
// modifications for dune interface 
// (c) Robert Kloefkorn 2004 - 2005 
#ifndef GITTER_TETRATOP_H_INCLUDED
#define GITTER_TETRATOP_H_INCLUDED

#include "gitter_sti.h"
#include "gitter_hexa_top.h"
#include "mapp_tetra_3d.h"


// --checkFace
template < class A > 
inline bool checkFace ( const A* fce, const int child )            
{
  A* face = const_cast< A* > (fce);
  const int vxs[ 3 ][ 2 ] = { { 0, 1 }, { 1, 2 }, { 2, 0 } };
  bool found = true ;
  for( int i=0; i<3; ++i )
  {
    const int twst = face->twist( i );
    // check vertices of the face 
    for( int j=0; j<2; ++j ) 
    {
      bool foundVx = false ;
      for( int e=0; e<2; ++e ) 
      {
        if( face->myvertex( vxs[ i ][ j ] ) == face->myhedge1( i )->myvertex( e ) ) 
          foundVx = true ;
      }
      if( ! foundVx ) 
      {
        cout << "Edge inconsistency: " << face << endl;
        for( int e=0; e<3; ++e ) 
        {
          cout << "edge " << face->myhedge1( e )->myvertex( 0 ) << " " <<
            face->myhedge1( e )->myvertex( 1 ) << endl;
        }
        assert( false );
      }
    }

    if( ! ( face->myvertex( i ) == face->myhedge1( i )->myvertex( twst ) &&
            face->myvertex( (i+1)%3 ) == face->myhedge1( i )->myvertex( 1-twst ) ) )
      found = false;

    for( int j=1; j<3; ++j )
    {
      int f = (i+j)%3 ;
      if( face->myhedge1( i )->getIndex() == face->myhedge1( f )->getIndex() ) 
      {
        cout << "Edge " << i << "  " << face->myhedge1( i ) << endl;
        cout << "Edge " << f << "  " << face->myhedge1( f ) << endl;
        assert( false );
      }
    }
  }
  return found;
}

template < class A > class Hface3Top : public A 
{
  public :
    using A :: twist ;
    using A :: myhedge1 ;
    using A :: myvertex ;

    typedef Hface3Top < A >             innerface_t ;
    typedef typename A :: inneredge_t   inneredge_t;
    typedef typename A :: innervertex_t innervertex_t;
    typedef typename A :: myhedge1_t    myhedge1_t ;
    typedef typename A :: myvertex_t    myvertex_t ;
    typedef typename A :: myrule_t      myrule_t ;
    typedef InnerStorage< InnerEdgeStorage< innerface_t , false > > inner_t ;

    typedef pair< myhedge1_t*, myhedge1_t* > edgepair_t ;

  private :
    innerface_t * _bbb ;
    inner_t  * _inner ;

    const unsigned char _lvl ;
    const signed char _nChild;
    myrule_t _rule ;

  protected:  
    // we need this because TetraTop needs access to this indexManager 
    inline IndexManagerType & indexManager () { 
      return  this->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType :: IM_Faces ); }    

  private:
    inline myhedge1_t * subedge1 (int,int) ;
    inline const myhedge1_t * subedge1 (int,int) const ;
    void split_e01 () ;
    void split_e12 () ;
    void split_e20 () ;
    void split_iso4 () ;

  public :
    // constructor for macro elements 
    inline Hface3Top (int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int ) ;
    // constructor for refined elements 
    inline Hface3Top (int,myhedge1_t *,int,myhedge1_t *,int,myhedge1_t *,int, int nChild ) ;
    virtual inline ~Hface3Top () ;
    innervertex_t * subvertex (int) ;
    const innervertex_t * subvertex (int) const ;
    inneredge_t * subedge1 (int) ;
    const inneredge_t * subedge1 (int) const ;
    innerface_t * subface3 (int) ;
    const innerface_t * subface3 (int) const ;
    inline int level () const ;
    inline int nChild () const ; 
    innervertex_t * innerVertex () ;
    const innervertex_t * innerVertex () const ;
    inneredge_t * innerHedge () ;
    const inneredge_t * innerHedge () const ;
    innerface_t * down () ;
    const innerface_t * down () const ;
    innerface_t * next () ;
    const innerface_t * next () const ;
    void append (innerface_t * f) ;
  public:
    virtual myrule_t getrule () const ;
    virtual bool refine (myrule_t,int) ;
    virtual void refineImmediate (myrule_t) ;
    virtual bool coarse () ;
  public :
    virtual void backup (ostream &) const ;
    virtual void restore (istream &) ;
    
    virtual void backup (ObjectStream &) const ;
    virtual void restore (ObjectStream &) ;

  protected:
    myvertex_t* vertexNotOnSplitEdge( const int );
    edgepair_t subEdges( myhedge1_t* , const myvertex_t* , const myvertex_t*  ) ;

    // non-virtual methods of down and innerVertex 
    innerface_t* dwnPtr() ;
    const innerface_t* dwnPtr() const ;
    inneredge_t* inEd() ;
    const inneredge_t* inEd() const ;

    template <class OutStream_t> 
    void doBackup(OutStream_t &) const;
    
    template <class InStream_t> 
    void doRestore(InStream_t &);
} ;


template < class A > class Hbnd3Top : public A {
  public:
    using A :: twist ;
    using A :: subface3 ;
    using A :: myhface3 ;

  protected :
    typedef Hbnd3Top < A >              innerbndseg_t ;
    typedef typename A :: myhface3_t    myhface3_t ;
    typedef typename A :: balrule_t     balrule_t ;
    typedef typename A :: myrule_t      myrule_t ;
    typedef typename A :: bnd_t         bnd_t;

    bool refineLikeElement (balrule_t) ;
    inline void append (innerbndseg_t *) ;

    // we need access to the indexManager 
    inline IndexManagerType & indexManager () { 
      return  myhface3(0)->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType :: IM_Bnd ); 
    }    

  private :
    innerbndseg_t * _bbb, * _dwn , * _up ;

    int _segmentIndex; // segment index of macro face 
    const bnd_t _bt; // type of boundary 
    unsigned char _lvl ;

    void split_e01 () ;
    void split_e12 () ;
    void split_e20 () ;
    void split_iso4 () ;
    inline bool coarse () ;
   
  public:
    // constructor for serial macro boundary elements  
    inline Hbnd3Top (int,myhface3_t *,int, const bnd_t b ) ;
    // constructor for children 
    inline Hbnd3Top (int, myhface3_t *,int, 
                     innerbndseg_t * up, const bnd_t b, 
                     typename Gitter::helement_STI * gh, int gFace ) ;

    inline virtual ~Hbnd3Top () ;
    bool refineBalance (balrule_t,int) ;
    bool bndNotifyCoarsen () ;
    void restoreFollowFace () ;
    inline int level () const ;
    inline int segmentIndex () const ;
    inline innerbndseg_t * next () ;
    inline innerbndseg_t * down () ;
    inline const innerbndseg_t * next () const ;
    inline const innerbndseg_t * down () const ;

    // for dune 
    inline innerbndseg_t * up () ;
    inline const innerbndseg_t * up () const ;
    
    inline bnd_t bndtype () const { return _bt; }
    
  protected:
    // set boundary id for all item connected to this hbnd
    void setBoundaryId( const int id ); 
} ;

template < class A > class TetraTop : public A 
{
  public :
    using A :: twist ;
    using A :: myhface3 ;
    using A :: myvertex ;

    typedef TetraTop < A >    innertetra_t  ;
    typedef typename A :: innervertex_t innervertex_t ;
    typedef typename A :: inneredge_t   inneredge_t ;
    typedef typename A :: innerface_t   innerface_t ;
    typedef Gitter :: Geometric :: VertexGeo  myvertex_t ;
    typedef typename A :: myhedge1_t    myhedge1_t ;
    typedef typename A :: myhface3_t    myhface3_t ;
    typedef typename A :: myrule_t      myrule_t ;
    typedef typename A :: balrule_t     balrule_t ;
    typedef pair< myhface3_t *, myhface3_t * > facepair_t;

    typedef InnerStorage< InnerFaceStorage< innertetra_t , false > > inner_t ;
    typedef typename myhface3_t :: myrule_t face3rule_t;

  protected:  

    struct BisectionInfo 
    {
      struct CallSplitIF 
      {
        virtual ~CallSplitIF () {}
        virtual void splitEdge( innertetra_t* tetra ) const = 0;
      };

      template < typename myrule_t :: rule_enum rule > 
      struct CallSplitImpl : public CallSplitIF 
      {
        virtual ~CallSplitImpl () {}
        virtual void splitEdge( innertetra_t* tetra ) const 
        { 
          if( rule == myrule_t :: e01 ) 
            tetra->split_e01(); 
          else if ( rule == myrule_t :: e12 ) 
            tetra->split_e12(); 
          else if ( rule == myrule_t :: e20 ) 
            tetra->split_e20(); 
          else if ( rule == myrule_t :: e23 ) 
            tetra->split_e23(); 
          else if ( rule == myrule_t :: e30 ) 
            tetra->split_e30(); 
          else if ( rule == myrule_t :: e31 ) 
            tetra->split_e31(); 
          else 
          {
            cerr << "**ERROR (FATAL): wrong refinement rule! In " << __FILE__ << " " << __LINE__ << endl ;
            abort();
          }
        }
      };

      const CallSplitIF* _caller;
      unsigned char _faces[ 2 ];
      unsigned char _vertices[ 2 ];
      face3rule_t _faceRules[ 2 ];

    private:
      // constructor 
      BisectionInfo( myrule_t rule ) ;
      // no copying 
      BisectionInfo( const BisectionInfo& );

    public:
      ~BisectionInfo() { delete _caller; }

      static const BisectionInfo& instance( const myrule_t& rule ) 
      { 
        assert( rule == myrule_t :: e01 ||
                rule == myrule_t :: e12 || 
                rule == myrule_t :: e20 || 
                rule == myrule_t :: e23 || 
                rule == myrule_t :: e30 || 
                rule == myrule_t :: e31  );

        static const BisectionInfo bisectionInfo[ 6 ] = { 
            BisectionInfo( myrule_t :: e01 ),
            BisectionInfo( myrule_t :: e12 ),
            BisectionInfo( myrule_t :: e20 ),
            BisectionInfo( myrule_t :: e23 ),
            BisectionInfo( myrule_t :: e30 ),
            BisectionInfo( myrule_t :: e31 ) };
        return bisectionInfo[ int(rule) - 2 ];
      }

      static face3rule_t calculateRule( const myhface3_t* face, 
                                        const myvertex_t* vx0,
                                        const myvertex_t* vx1 )
      {
        static const face3rule_t rules[ 3 ] = { face3rule_t :: e01, face3rule_t :: e12, face3rule_t :: e20 };

        assert( checkFace( face, face->nChild() ) );

        for(int j=0; j<3; ++j ) 
        {
          for( int twist=0; twist<2; ++twist )
          {
            //cout << "Check edge " << face->myhedge1( j )->myvertex( twist ) << " " 
            //     << face->myhedge1( j )->myvertex( 1-twist ) << endl;
            if( face->myhedge1( j )->myvertex( twist ) == vx0  && 
                face->myhedge1( j )->myvertex( 1-twist ) == vx1  ) 
            {
              return rules[ j ];
            }
          }
        }

        assert( false );
        abort();
        return rules[ 0 ];
      }

      static bool refineFaces( innertetra_t* tetra, const myrule_t& rule )
      {
        const BisectionInfo& info = instance( rule );
        for( int i=0; i<2; ++i )
        {
          myhface3_t* face = tetra->myhface3( info._faces[ i ] );

          //cout << "Check rule for " << face << endl;
          //cout << "vx0 " << int(info._vertices[ 0 ]) << " " << tetra->myvertex( info._vertices[ 0 ] ) << endl;
          //cout << "vx1 " << int(info._vertices[ 1 ]) << " " << tetra->myvertex( info._vertices[ 1 ] ) << endl;
          const face3rule_t faceRule = calculateRule( face, 
              tetra->myvertex( info._vertices[ 0 ] ), tetra->myvertex( info._vertices[ 1 ] ) );

          //cout << "Got rule " << faceRule << endl;
          // check refinement of faces 
          if (! face->refine( faceRule, tetra->twist( info._faces[ i ] ) ) ) return false ;
        }
        return true ;
      }

      static void splitEdge( innertetra_t* tetra, const myrule_t& rule )
      {
        const BisectionInfo& info = instance( rule );

        for( int i=0; i<2; ++i )
        {
          myhface3_t* face = tetra->myhface3( info._faces[ i ] );

          const face3rule_t faceRule = calculateRule( face, 
              tetra->myvertex( info._vertices[ 0 ] ), tetra->myvertex( info._vertices[ 1 ] ) );

          face->refineImmediate ( faceRule );
        }
  
        // call correct split edge 
        info.caller().splitEdge( tetra );
      }

      const CallSplitIF& caller() const { assert( _caller ); return *_caller; }
    };

    void refineImmediate (myrule_t) ;
    inline void append (innertetra_t * h) ;

    bool checkTetra( const innertetra_t* tetra, const int  ) const ;
    int vertexTwist( const int, const int ) const ;
    int calculateFace2Twist( const int vx, const myhface3_t* ) const;
    int calculateFace3Twist( const int (&vx)[2], const myhface3_t*, const int ) const;
    // the element type is obtained from the level of the element 
    // under the assumption that on level 0 all elements have type 0
    unsigned char elementType () const { return (_lvl % 3); }

    void setNewMapping( innertetra_t*, innertetra_t*, innerface_t*, const int, const int ) ;

  private :
    innertetra_t * _bbb, * _up ; 
    inner_t * _inner ;
    const double _volume;

    const unsigned char _lvl ;  // 1 byte 
    signed char _nChild;        // 1 byte 
    unsigned char _vxMap[ 4 ] ; // 4 byte 
    myrule_t _req, _rule ;      // 2 byte   = 8 byte 
    
  private :
    bool checkRule( const myrule_t rule ) const
    {
      // this does not work, when children are fliped, see setNewMapping 
      static const myrule_t possibleRules0[ 6 ][ 2 ] = { 
          { myrule_t :: e20, myrule_t :: e30 }, // possible rules for e01 in child 0 
          { myrule_t :: e01, myrule_t :: e31 }, // possible rules for e12 in child 0 
          { myrule_t :: e30, myrule_t :: e01 }, // possible rules for e20 in child 0 
          { myrule_t :: e30, myrule_t :: e31 }, // possible rules for e23 in child 0 
          { myrule_t :: e20, myrule_t :: e01 }, // possible rules for e30 in child 0 
          { myrule_t :: e01, myrule_t :: e12 } // possible rules for e31 in child 0 
        };
      static const myrule_t possibleRules1[ 6 ][ 2 ] = { 
          { myrule_t :: e31, myrule_t :: e12 }, // possible rules for e01 in child 1 
          { myrule_t :: e23, myrule_t :: e20 }, // possible rules for e12 in child 1 
          { myrule_t :: e12, myrule_t :: e23 }, // possible rules for e12 in child 1 
          { myrule_t :: e12, myrule_t :: e20 }, // possible rules for e23 in child 1 
          { myrule_t :: e31, myrule_t :: e23 }, // possible rules for e30 in child 1 
          { myrule_t :: e23, myrule_t :: e30 } // possible rules for e31 in child 0 
        };

      if( _up ) 
      {
        if( _nChild == 0 ) 
        {
          return ( rule == possibleRules0[ int( _up->_rule )-2 ][ 0 ] ) || 
                 ( rule == possibleRules0[ int( _up->_rule )-2 ][ 1 ] ) ;
        }
        else 
        {
          return ( rule == possibleRules1[ int( _up->_rule )-2 ][ 0 ] ) || 
                 ( rule == possibleRules1[ int( _up->_rule )-2 ][ 1 ] ) ;
        }
      }
      else 
        return true ;
    }

    void splitInfo( const myrule_t rule ) const 
    {
#if 0
      cout << endl << "Split tetra " << this<< endl; 
      cout << " ( " << this->getIndex() << ", ch" << int( _nChild) << ") with rule " << rule << "  ";
      if( _up ) 
        cout << "father (" << _up->getIndex() << ", ch" << int( _up->_nChild) << ") rule = " << _up->_rule << endl;
      cout << endl;
      const bool chRule = checkRule( rule );
      if( ! chRule ) 
      {
        cout << "Map = ( " ;
        for(int i=0; i<4; ++i ) 
          cout << int(_vxMap[ i ]) << " " ;
        cout << " ) " << endl;

        cout << rule << " not valid " << endl;
        //assert( false );
      }
#endif
    }

    myrule_t suggestRule () const 
    {
      static const myrule_t rules [ 4 ][ 4 ] = {
        { myrule_t :: crs, myrule_t :: e01 , myrule_t :: e20, myrule_t :: e30 },
        { myrule_t :: e01 , myrule_t :: crs, myrule_t :: e12, myrule_t :: e31 },
        { myrule_t :: e20 , myrule_t :: e12, myrule_t :: crs, myrule_t :: e23 },
        { myrule_t :: e30 , myrule_t :: e31, myrule_t :: e23, myrule_t :: crs }
      };
      return rules[ int(_vxMap[ 0 ]) ][ int(_vxMap[ 3 ]) ]; 
    }

    inline IndexManagerType & indexManager() { 
      return this->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType :: IM_Elements ); }
    double calculateChildVolume(const double) const;
    
    void split_e01 () ;
    void split_e12 () ;
    void split_e20 () ;
    void split_e23 () ;
    void split_e30 () ;
    void split_e31 () ;

    void bisect () ;
    void splitISO8 () ;
  protected :
    myhedge1_t * subedge1 (int,int) ;
    const myhedge1_t * subedge1 (int,int) const ;
    facepair_t subFaces( const int );
    facepair_t subFaces( const int, const myvertex_t*, const myvertex_t* );
    myhface3_t * subface3 (int,int) ;
    const myhface3_t * subface3 (int i, int j) const ;
  public:
    // constructor for refined elements 
    TetraTop (int,myhface3_t *,int,myhface3_t *,int,myhface3_t *,int,
                  myhface3_t *,int,innertetra_t *up, int nChild, double vol) ;
    // constructor for macro elements 
    TetraTop (int,myhface3_t *,int,myhface3_t *,int,myhface3_t *,int,
                  myhface3_t *, int ) ;
    virtual ~TetraTop () ;
    inline innertetra_t * up () ;
    inline const innertetra_t * up () const;
    inline innertetra_t * down () ;
    inline const innertetra_t * down () const ;
    inline innertetra_t * next () ;
    inline const innertetra_t * next () const ;
    inline innervertex_t * innerVertex () ;
    inline const innervertex_t * innerVertex () const ;
    inline inneredge_t * innerHedge () ;
    inline const inneredge_t * innerHedge () const ;
    inline innerface_t * innerHface () ;
    inline const innerface_t * innerHface () const ;

    inline int level () const ;
    inline int nChild () const ;
    inline double volume () const ;
  public :
    myrule_t getrule () const ;
    myrule_t requestrule () const ;
    bool refine () ;
    void request (myrule_t) ;
    bool refineBalance (balrule_t,int) ;
    bool coarse () ;
    bool bndNotifyCoarsen () ;
    void backupCMode (ostream &) const ;
    
    void backup (ostream &) const ;
    void restore (istream &) ;
    
    void backup (ObjectStream &) const ;
    void restore (ObjectStream &) ;
    
    // backup and restore index 
    void backupIndex (ostream &) const ;
    void restoreIndex (istream &, vector<bool>(&)[4] ) ;
  protected:  
    // non-virtual methods of down and innerVertex 
    innertetra_t* dwnPtr() ;
    const innertetra_t* dwnPtr() const ;
    inneredge_t* inEd() ;
    const inneredge_t* inEd() const ;
    innerface_t* inFce() ;
    const innerface_t* inFce() const ;

    template <class OutStream_t> 
    void doBackup(OutStream_t &) const;
    
    template <class InStream_t> 
    void doRestore(InStream_t &);
};

template < class A > class Periodic3Top : public A {
  public:
    using A :: twist ;
    using A :: myhface3 ;

  protected :
    typedef Periodic3Top < A >          innerperiodic3_t  ;
    typedef typename A :: innervertex_t innervertex_t ;
    typedef typename A :: inneredge_t   inneredge_t ;
    typedef typename A :: innerface_t   innerface_t ;
    typedef typename A :: myhedge1_t    myhedge1_t ;
    typedef typename A :: myhface3_t    myhface3_t ;
    typedef typename A :: myrule_t      myrule_t ;
    typedef typename A :: balrule_t     balrule_t ;
    typedef typename A :: bnd_t         bnd_t; 

    void refineImmediate (myrule_t) ;
    inline void append (innerperiodic3_t * h) ;
    
  private :
    innerperiodic3_t * _dwn, * _bbb, * _up ; 
    // we need two indices since this pointer 
    // is available on the two periodic sides 
    int _segmentIndex[ 2 ]; 
    bnd_t _bt[ 2 ];
    const unsigned char _lvl ;
    const signed char _nChild; 
    myrule_t _rule ;

  private :
    void split_e01 () ;
    void split_e12 () ;
    void split_e20 () ;
    void split_iso4 () ;
  protected :
    myhedge1_t * subedge1 (int,int) ;
    const myhedge1_t * subedge1 (int,int) const ;
    myhface3_t * subface3 (int,int) ;
    const myhface3_t * subface3 (int i, int j) const ;

    // we need this for the boundary segment index 
    inline IndexManagerType & indexManager () { 
      return  this->myhface3(0)->myvertex(0)->indexManagerStorage().get( IndexManagerStorageType :: IM_Bnd ); 
    }    
  public:
    // constructor for macro elements 
    inline Periodic3Top (int,myhface3_t *,int,myhface3_t *,int, const bnd_t (&bnd)[2] ) ;
    // construtor for refined elements 
    inline Periodic3Top (int,myhface3_t *,int,myhface3_t *,int, innerperiodic3_t * up, int nChild ) ;
    virtual inline ~Periodic3Top () ;
    inline innerperiodic3_t * up () ;
    inline const innerperiodic3_t * up () const;
    inline innerperiodic3_t * down () ;
    inline const innerperiodic3_t * down () const ;
    inline innerperiodic3_t * next () ;
    inline const innerperiodic3_t * next () const ;
    inline innervertex_t * innerVertex () ;
    inline const innervertex_t * innerVertex () const ;
    inline inneredge_t * innerHedge () ;
    inline const inneredge_t * innerHedge () const ;
    inline innerface_t * innerHface () ;
    inline const innerface_t * innerHface () const ;
    inline int level () const ;
    inline int nChild () const ;
    inline int segmentIndex (const int) const ;
    inline bnd_t bndtype (const int i) const 
    {
      assert( i==0 || i==1 );
      return _bt[ i ];
    } 
  public :
    myrule_t getrule () const ;
    bool refine () ;
    void request (myrule_t) ;
    bool refineBalance (balrule_t,int) ;
    bool coarse () ;
    bool bndNotifyCoarsen () ;
  public:
    void backupCMode (ostream &) const ;
    void backup (ostream &) const ;
    void restore (istream &) ;
    
    void backup (ObjectStream &) const ;
    void restore (ObjectStream &) ;
  protected:  
    template <class OutStream_t> 
    void doBackup(OutStream_t &) const;
    
    template <class InStream_t> 
    void doRestore(InStream_t &);
};
  //
  //    #    #    #  #          #    #    #  ######
  //    #    ##   #  #          #    ##   #  #
  //    #    # #  #  #          #    # #  #  #####
  //    #    #  # #  #          #    #  # #  #
  //    #    #   ##  #          #    #   ##  #
  //    #    #    #  ######     #    #    #  ######
  //


// #     #                                  #####  #######
// #     #  ######    ##     ####   ###### #     #    #      ####   #####
// #     #  #        #  #   #    #  #            #    #     #    #  #    #
// #######  #####   #    #  #       #####   #####     #     #    #  #    #
// #     #  #       ######  #       #            #    #     #    #  #####
// #     #  #       #    #  #    #  #      #     #    #     #    #  #
// #     #  #       #    #   ####   ######  #####     #      ####   #


template < class A > inline typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: dwnPtr () {
  return (_inner) ? _inner->dwn() : 0 ;
}

template < class A > inline const typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: dwnPtr () const {
  return (_inner) ? _inner->dwn() : 0 ;
}

template < class A > inline typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: inEd () {
  return (_inner) ? _inner->ed() : 0 ;
}

template < class A > inline const typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: inEd () const {
  return (_inner) ? _inner->ed() : 0 ;
}
template < class A > inline typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: down () {
  return dwnPtr() ;
}

template < class A > inline const typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: down () const {
  return dwnPtr() ;
}

template < class A > inline typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: next () {
  return _bbb ;
}

template < class A > const typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: next () const {
  return _bbb ;
}

template < class A > inline int Hface3Top < A > :: level () const {
  return _lvl ;
}

template < class A > inline int Hface3Top < A > :: nChild () const {
  assert( _nChild >= 0 && _nChild < 4 );
  return _nChild ;
}

template < class A > inline typename Hface3Top < A > :: innervertex_t * Hface3Top < A > :: innerVertex () {
  return 0 ;
}

template < class A > inline const typename Hface3Top < A > :: innervertex_t * Hface3Top < A > :: innerVertex () const {
  return 0 ;
}

template < class A > inline typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: innerHedge () {
  return inEd();
}

template < class A > inline const typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: innerHedge () const {
  return inEd();
}

template < class A > inline typename Hface3Top < A > :: innervertex_t * Hface3Top < A > :: subvertex (int) {
  assert (getrule() == myrule_t :: iso4) ;
  return 0 ;
}

template < class A > inline const typename Hface3Top < A > :: innervertex_t * Hface3Top < A > :: subvertex (int) const {
  assert (getrule() == myrule_t :: iso4) ;
  return 0 ;
}

template < class A > inline typename Hface3Top < A > :: myhedge1_t * Hface3Top < A > :: subedge1 (int i,int j) {
  assert(j == 0 || j == 1) ;
  return myhedge1 (i)->subedge1 (j ? 1 - twist(i) : twist(i)) ;
}

template < class A > inline const typename Hface3Top < A > :: myhedge1_t * Hface3Top < A > :: subedge1 (int i,int j) const {
  assert(j == 0 || j == 1) ;
  return myhedge1 (i)->subedge1 (j ? 1 - twist(i) : twist(i)) ;
}

template < class A > inline typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: subedge1 (int n) {
  inneredge_t * e = inEd() ;
  for (int i = 0 ; i < n ; ++i ) e = e ? e->next () : 0 ;
  assert (e) ;
  return e ;
}

template < class A > inline const typename Hface3Top < A > :: inneredge_t * Hface3Top < A > :: subedge1 (int n) const {
  const inneredge_t * e = inEd();
  for (int i = 0 ; i < n ; ++i ) e = e ? e->next () : 0 ;
  assert (e) ;
  return e ;
}

template < class A > inline typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: subface3 (int n) {
  innerface_t * f = dwnPtr() ;
  for (int i = 0 ; i < n ; i++ ) f = f ? f->next () : 0 ;
  assert (f) ;
  return f ;
}

template < class A > inline const typename Hface3Top < A > :: innerface_t * Hface3Top < A > :: subface3 (int n) const {
  const innerface_t * f = dwnPtr();
  for (int i = 0 ; i < n ; i++ ) f = f ? f->next () : 0 ;
  assert (f) ;
  return f ;
}

template < class A > inline void Hface3Top < A > :: append (innerface_t * f) {
  assert (!_bbb) ;
  _bbb = f ;
  return ;
}

template < class A > inline typename Hface3Top < A > :: myrule_t Hface3Top < A > :: getrule () const {
  return myrule_t (_rule) ;
}

// constructor called during refinement 
template < class A > inline Hface3Top < A > :: 
Hface3Top (int l, myhedge1_t * e0, 
  int t0, myhedge1_t * e1, int t1, myhedge1_t * e2, int t2,
  int nChild ) : 
  A (e0, t0, e1, t1, e2, t2), 
  _bbb (0), _inner(0) ,
  _lvl (l),
  _nChild (nChild),
  _rule (myrule_t :: nosplit)
{
  this->setIndex( indexManager().getIndex() );

#ifndef NDEBUG 
  double n[ 3 ];
  LinearSurfaceMapping( myvertex( 0 )->Point(), 
                        myvertex( 1 )->Point(),
                        myvertex( 2 )->Point() ).normal( n );

  //if( (n[0]*n[0] + n[1]*n[1] + n[2]*n[2] ) < 1e-8 ) 
  //{
  //  cout << "Determinant of " << this << " is wrong" <<endl;
  //}
  assert( (n[0]*n[0] + n[1]*n[1] + n[2]*n[2] ) >= 1e-8 );
#endif
  assert( checkFace( this, nChild ) );
  return ;
}

// constructor called while creating macro face 
template < class A > inline Hface3Top < A > :: 
Hface3Top (int l, myhedge1_t * e0, 
  int t0, myhedge1_t * e1, int t1, myhedge1_t * e2, int t2) : 
  A (e0, t0, e1, t1, e2, t2), 
  _bbb (0), _inner (0), 
  _lvl (l),
  _nChild (0),
  _rule (myrule_t :: nosplit)
{
  this->setIndex( indexManager().getIndex() );
  assert( checkFace( this, nChild() ) );
}

template < class A > inline Hface3Top < A > :: ~Hface3Top () 
{
  this->freeIndex( indexManager() );
  if (_bbb) delete _bbb ;
  if (_inner) delete _inner ;
  return ;
}

// #     #                          #####  #######
// #     #  #####   #    #  #####  #     #    #      ####   #####
// #     #  #    #  ##   #  #    #       #    #     #    #  #    #
// #######  #####   # #  #  #    #  #####     #     #    #  #    #
// #     #  #    #  #  # #  #    #       #    #     #    #  #####
// #     #  #    #  #   ##  #    # #     #    #     #    #  #
// #     #  #####   #    #  #####   #####     #      ####   #

// serial macro bnd constructor 
template < class A > inline Hbnd3Top < A > :: 
Hbnd3Top (int l, myhface3_t * f, int i, const bnd_t bt) :
  A (f, i ), 
  _bbb (0), _dwn (0), _up (0) , 
  _bt( bt ),
  _lvl (l)
{
  // set index of boundary segment 
  this->setIndex( indexManager().getIndex() );

  // for macro bnd faces store current index as segment index 
  _segmentIndex = this->getIndex();

  // set boundary id 
  setBoundaryId( _bt ); 
  return ;
}

template < class A > inline Hbnd3Top < A > :: 
Hbnd3Top (int l, myhface3_t * f, 
          int i,  
          innerbndseg_t * up, bnd_t bt, 
          Gitter::helement_STI * gh, int gFace ) : 
  A (f, i ), _bbb (0), _dwn (0), _up (up) , 
  _bt (bt),
  _lvl (l) 
{
  // store ghost element 
  typedef Gitter :: ghostpair_STI ghostpair_STI;
  this->setGhost ( ghostpair_STI (gh , gFace) );

  // set index of boundary segment 
  this->setIndex( indexManager().getIndex() );

  // get segment index from father if existent 
  _segmentIndex = (_up) ? _up->_segmentIndex : this->getIndex() ;
 
  setBoundaryId( _bt ); 
  return ;
}

template < class A > inline Hbnd3Top < A > :: ~Hbnd3Top () 
{
  // free index  
  indexManager().freeIndex( this->getIndex() );

  // detach leaf entities 
  if (this->isLeafEntity()) this->detachleafs();
  // delete down and next 
  if (_bbb) delete _bbb ; 
  if (_dwn) delete _dwn ;
  return ; 
}

template < class A > inline int Hbnd3Top < A > :: segmentIndex () const {
  return _segmentIndex;
}

template < class A > inline int Hbnd3Top < A > :: level () const {
  return _lvl ;
}

template < class A > inline typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: next () { 
  return _bbb ;
}

template < class A > inline const typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: next () const { 
  return _bbb ;
}

template < class A > inline typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: down () { 
  return _dwn ;
}

template < class A > inline const typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: down () const { 
  return _dwn ;
}

template < class A > inline typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: up () { 
  return _up ;
}

template < class A > inline const typename Hbnd3Top < A > :: innerbndseg_t * Hbnd3Top < A > :: up () const { 
  return _up ;
}

template < class A > inline void Hbnd3Top < A > :: append (innerbndseg_t * b) {
  assert (_bbb == 0) ;
  _bbb = b ;
  return ;
}

// #######                                 #######
//    #     ######   #####  #####     ##      #      ####   #####
//    #     #          #    #    #   #  #     #     #    #  #    #
//    #     #####      #    #    #  #    #    #     #    #  #    #
//    #     #          #    #####   ######    #     #    #  #####
//    #     #          #    #   #   #    #    #     #    #  #
//    #     ######     #    #    #  #    #    #      ####   #

template < class A > inline typename TetraTop < A > :: innertetra_t * TetraTop < A > :: dwnPtr() {
  return (_inner) ? _inner->dwn() : 0 ;
}

template < class A > inline const typename TetraTop < A > :: innertetra_t * TetraTop < A > :: dwnPtr() const {
  return (_inner) ? _inner->dwn() : 0 ;
}

template < class A > inline typename TetraTop < A > :: inneredge_t * TetraTop < A > :: inEd() {
  return (_inner) ? _inner->ed() : 0 ;
}

template < class A > inline const typename TetraTop < A > :: inneredge_t * TetraTop < A > :: inEd() const {
  return (_inner) ? _inner->ed() : 0 ;
}

template < class A > inline typename TetraTop < A > :: innerface_t * TetraTop < A > :: inFce() {
  return (_inner) ? _inner->fce() : 0 ;
}

template < class A > inline const typename TetraTop < A > :: innerface_t * TetraTop < A > :: inFce() const {
  return (_inner) ? _inner->fce() : 0 ;
}

template < class A > inline double TetraTop < A > :: calculateChildVolume (const double childVolume) const 
{
  // if vertex projection is available on a neighbor 
  // volume has to be recalculated 
  return ( this->myGrid()->vertexProjection() ) ? -1.0 : childVolume; 
}

template < class A > inline int TetraTop < A > :: level () const {
  return _lvl ;
}

template < class A > inline double TetraTop < A > :: volume () const {
  return _volume ;
}

template < class A > inline int TetraTop < A > :: nChild () const {
  assert( _nChild >= 0 && _nChild < 8 );
  return _nChild ;
}

template < class A > inline typename TetraTop < A > :: innertetra_t * TetraTop < A > :: up () {
  return _up ;
}
template < class A > inline const typename TetraTop < A > :: innertetra_t * TetraTop < A> :: up () const {
  return _up ;
} 

template < class A > inline typename TetraTop < A > :: innertetra_t * TetraTop < A > :: down () {
  return dwnPtr() ;
}

template < class A > inline const typename TetraTop < A > :: innertetra_t * TetraTop < A > :: down () const {
  return dwnPtr() ;
}

template < class A > inline typename TetraTop < A > :: innertetra_t * TetraTop < A > :: next () {
  return _bbb ;
}

template < class A > inline const typename TetraTop < A > :: innertetra_t * TetraTop < A > :: next () const {
  return _bbb ;
}

template < class A > inline typename TetraTop < A > :: innervertex_t * TetraTop < A > :: innerVertex () {
  return 0 ;
}

template < class A > inline const typename TetraTop < A > :: innervertex_t * TetraTop < A > :: innerVertex () const {
  return 0 ;
}

template < class A > inline typename TetraTop < A > :: inneredge_t * TetraTop < A > :: innerHedge () {
  return inEd();
}

template < class A > inline const typename TetraTop < A > :: inneredge_t * TetraTop < A > :: innerHedge () const {
  return inEd();
}

template < class A > inline typename TetraTop < A > :: innerface_t * TetraTop < A > :: innerHface () {
  return inFce();
}

template < class A > inline const typename TetraTop < A > :: innerface_t * TetraTop < A > :: innerHface () const {
  return inFce();
}

template < class A > inline void TetraTop < A > :: append (TetraTop < A > * h) {
  assert (_bbb == 0) ;
  _bbb = h ;
  return ;
}

template < class A > inline typename TetraTop < A > :: myrule_t TetraTop < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > inline typename TetraTop < A > :: myrule_t TetraTop < A > :: requestrule () const {
  return myrule_t (_req) ;
}

// --request 
template < class A > inline void TetraTop < A > :: request (myrule_t r) 
{
  assert (r.isValid ()) ;

  if( r == myrule_t :: bisect )
  {
    // we always split edge 0 and 3 following 
    // the idea of Stevenson, Nochetto, Veser, Siebert 
    // suggestRule returns the correct splitting edge 
    // according to ALUGrid's rules 
    _req = suggestRule();
  }
  else 
  {
    // all other cases 
    _req = r ;
  }

  //cout << "Request rule " <<  _req << " for tetra " << this << endl;
  return ;
}

template < class A > inline bool TetraTop < A > :: bndNotifyCoarsen () {
  return true ;
}

// ######                                                           #####  #######
// #     #  ######  #####      #     ####   #####      #     ####  #     #   #      ####   #####
// #     #  #       #    #     #    #    #  #    #     #    #    #       #   #     #    #  #    #
// ######   #####   #    #     #    #    #  #    #     #    #       #####    #     #    #  #    #
// #        #       #####      #    #    #  #    #     #    #            #   #     #    #  #####
// #        #       #   #      #    #    #  #    #     #    #    # #     #   #     #    #  #
// #        ######  #    #     #     ####   #####      #     ####   #####    #      ####   #
   
template < class A > inline Periodic3Top < A > :: 
Periodic3Top (int l, myhface3_t * f0, int t0,
  myhface3_t * f1, int t1, const bnd_t (&bt)[2] ) 
 : A (f0, t0, f1, t1)
 , _dwn (0), _bbb (0), _up(0)
 , _lvl (l) 
 , _nChild(0)
 , _rule (myrule_t :: nosplit)
{ 
  IndexManagerType& im = indexManager();
  // get index 
  this->setIndex( im.getIndex() );
  
  // take macro index as segment index 
  _segmentIndex[ 0 ] = this->getIndex() ;
  _segmentIndex[ 1 ] = im.getIndex();

  // store boundary ids 
  _bt[ 0 ] = bt[ 0 ];
  _bt[ 1 ] = bt[ 1 ];
}

template < class A > inline Periodic3Top < A > :: 
Periodic3Top (int l, myhface3_t * f0, int t0, myhface3_t * f1, int t1, innerperiodic3_t * up, int nChild ) 
  : A (f0, t0, f1, t1)
  , _dwn (0), _bbb (0), _up(up)
  , _lvl (l) 
  , _nChild (nChild) 
  , _rule (myrule_t :: nosplit)
{
  // get index 
  this->setIndex( indexManager().getIndex() );
  
  // get segment index from father
  assert( _up );
  _segmentIndex[ 0 ] = _up->_segmentIndex[ 0 ];
  _segmentIndex[ 1 ] = _up->_segmentIndex[ 1 ];

  // store boundary ids 
  _bt[ 0 ] = _up->_bt[ 0 ];
  _bt[ 1 ] = _up->_bt[ 1 ];
}

template < class A > inline Periodic3Top < A > :: ~Periodic3Top () 
{
  // free index 
  IndexManagerType& im = indexManager();
  // free indices 
  im.freeIndex( this->getIndex() );
  // only on macro boundary free segment index 
  if( level() == 0 ) im.freeIndex( _segmentIndex[ 1 ] );

  // delete down and next 
  if (_bbb) delete _bbb ;
  if (_dwn) delete _dwn ;
}

template < class A > inline int Periodic3Top < A > :: level () const {
  return _lvl ;
}

template < class A > inline int Periodic3Top < A > :: segmentIndex (const int fce) const {
  assert( fce == 0  || fce == 1 );
  return _segmentIndex[ fce ];
}

template < class A > inline int Periodic3Top < A > :: nChild () const {
  assert( _nChild >= 0 && _nChild < 4 );
  return _nChild ;
}

template < class A > inline typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A > :: up () {
  return _up ;
}
template < class A > inline const typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A> :: up () const {
  return _up ;
}

template < class A > inline typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A > :: down () {
  return _dwn ;
}

template < class A > inline const typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A > :: down () const {
  return _dwn ;
}

template < class A > inline typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A > :: next () {
  return _bbb ;
}

template < class A > inline const typename Periodic3Top < A > :: innerperiodic3_t * Periodic3Top < A > :: next () const {
  return _bbb ;
}

template < class A > inline typename Periodic3Top < A > :: innervertex_t * Periodic3Top < A > :: innerVertex () {
  return 0 ;
}

template < class A > inline const typename Periodic3Top < A > :: innervertex_t * Periodic3Top < A > :: innerVertex () const {
  return 0 ;
}

template < class A > inline typename Periodic3Top < A > :: inneredge_t * Periodic3Top < A > :: innerHedge () {
  return 0 ;
}

template < class A > inline const typename Periodic3Top < A > :: inneredge_t * Periodic3Top < A > :: innerHedge () const {
  return 0 ;
}

template < class A > inline typename Periodic3Top < A > :: innerface_t * Periodic3Top < A > :: innerHface () {
  return 0 ;
}

template < class A > inline const typename Periodic3Top < A > :: innerface_t * Periodic3Top < A > :: innerHface () const {
  return 0 ;
}

template < class A > inline void Periodic3Top < A > :: append (Periodic3Top < A > * h) {
  assert (_bbb == 0) ;
  _bbb = h ;
  return ;
}

template < class A > inline typename Periodic3Top < A > :: myrule_t Periodic3Top < A > :: getrule () const {
  return myrule_t (_rule) ;
}

template < class A > inline void Periodic3Top < A > :: request (myrule_t) {

  // Einen Request zur Verfeinerung zu setzen, ist vorl"aufig inhaltlich nicht
  // vorgesehen und wird deshalb ignoriert (leise).

  return ;
}
#endif  //  GITTER_TetraTop_H_INCLUDED
