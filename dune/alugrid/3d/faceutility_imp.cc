#ifndef DUNE_FACEUTILITY_IMP_HH
#define DUNE_FACEUTILITY_IMP_HH

namespace Dune
{


  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline ALU3dGridFaceInfo< dim, dimworld, type, Comm >::
  ALU3dGridFaceInfo( const bool levelIntersection ) :
    face_(0),
    innerElement_(0),
    outerElement_(0),
    innerFaceNumber_(-1),
    outerFaceNumber_(-1),
    innerTwist_(-665),
    outerTwist_(-665),
    segmentId_( -1 ),
    bndId_( -1 ),
    bndType_( noBoundary ),
    conformanceState_(UNDEFINED),
    conformingRefinement_( false ),
    ghostCellsEnabled_( false ),
    levelIntersection_( levelIntersection )
  {
  }

  // points face from inner element away?
  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline void
  ALU3dGridFaceInfo< dim, dimworld, type, Comm >::
  setFlags(const bool conformingRefinement,
           const bool ghostCellsEnabled )
  {
    conformingRefinement_ = conformingRefinement;
    ghostCellsEnabled_    = ghostCellsEnabled;
  }

  // points face from inner element away?
  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline void
  ALU3dGridFaceInfo< dim, dimworld, type, Comm >::
  updateFaceInfo(const GEOFaceType& face,
                 int innerLevel,
                 int innerTwist)
  {
    face_ = &face;

    innerElement_ = 0;
    outerElement_ = 0;
    innerFaceNumber_ = -1;
    outerFaceNumber_ = -1;
    bndType_ = noBoundary;
    segmentId_ = -1;
    bndId_ = 0; // inner face

    // points face from inner element away?
    if (innerTwist < 0)
    {
      innerElement_    = face.nb.rear().first;
      innerFaceNumber_ = face.nb.rear().second;
      outerElement_    = face.nb.front().first;
      outerFaceNumber_ = face.nb.front().second;
    }
    else
    {
      innerElement_    = face.nb.front().first;
      innerFaceNumber_ = face.nb.front().second;
      outerElement_    = face.nb.rear().first;
      outerFaceNumber_ = face.nb.rear().second;
    } // end if

    // if not true we are accessing a fake bnd
    alugrid_assert ( innerElement_->isRealObject() );
    // if not true we are accessing a fake bnd
    alugrid_assert ( outerElement_->isRealObject() );

    // we only have to do this in parallel runs
    if( parallel() && innerElement_->isboundary() )
    {
      bndType_ = innerGhostBoundary;
      alugrid_assert ( ! dynamic_cast< const GEOPeriodicType* > ( innerElement_ ) );
    }

    if( parallel() && innerBoundary() )
    {
      // check for ghosts
      // this check is only need in the parallel case
      const BNDFaceType * bnd = static_cast<const BNDFaceType *> (innerElement_);

      if(bnd->bndtype() == ALU3DSPACE ProcessorBoundary_t)
      {
        // if nonconformity occurs then go up one level
        if( bnd->level () != bnd->ghostLevel() && !conformingRefinement_)
        {
          bnd = static_cast<const BNDFaceType *>(bnd->up());
          alugrid_assert ( bnd );
          innerElement_ = static_cast<const HasFaceType*> (bnd);
        }

        // get ghost and internal number
        GhostPairType p  = bnd->getGhost();

        // get face number
        innerFaceNumber_ = p.second;

        // this doesn't count as outer boundary
        const GEOElementType* ghost = static_cast<const GEOElementType*> (p.first);
        alugrid_assert (ghost);

        innerTwist_ = ghost->twist(innerFaceNumber_);
      }
      else
      {
        innerTwist_ = innerFace().twist(innerALUFaceIndex());
      }
    }
    else
    {
      // set inner twist
      alugrid_assert (innerTwist == innerEntity().twist(innerFaceNumber_));
      innerTwist_ = innerTwist;
    }

    //in the case of a levelIntersectionIterator and conforming elements
    //we assume the macro grid view. So we go up to level 0
    //after that we have to get new twist and facenumbers
    if(levelIntersection_ && conformingRefinement_ && ! (innerElement_->isboundary() ) )
    {
      const GEOElementType * inner = static_cast<const GEOElementType *> (innerElement_);
      while( inner -> up () ) inner = static_cast<const GEOElementType *> ( inner ->up() );
      innerElement_ = static_cast<const HasFaceType *> (inner);
      innerTwist_ = innerEntity().twist(innerFaceNumber_);
    }

    if( outerElement_->isboundary() )
    {
      alugrid_assert ( ! innerBoundary() );
      // set to default boundary (with domain boundary)
      bndType_ = domainBoundary ;

      // check for ghosts
      // this check is only need in the parallel case
      // if this cast fails we have a periodic element
      const BNDFaceType * bnd = dynamic_cast<const BNDFaceType *> (outerElement_);
      const bool periodicBnd = ( bnd == nullptr ) ;

      if( periodicBnd ) // the periodic case
      {
        bndType_ = periodicBoundary ;
        alugrid_assert ( dynamic_cast< const GEOPeriodicType* > ( outerElement_ ) );
        const GEOPeriodicType* periodicClosure = static_cast< const GEOPeriodicType* > ( outerElement_ ) ;

        // previously, the segmentId( 1 - outerFaceNumber_ ) was used, why?
        // compute segment already since it's complicated to obtain
        segmentId_ = periodicClosure->segmentId( outerFaceNumber_ );
        bndId_  = periodicClosure->bndtype( outerFaceNumber_ );

        const GEOFaceType* face = ImplTraits::getFace( *periodicClosure, 1 - outerFaceNumber_ );
        alugrid_assert ( (face->nb.front().first == periodicClosure) || (face->nb.rear().first == periodicClosure) );
        if( face->nb.rear().first == periodicClosure )
        {
          alugrid_assert ( dynamic_cast< const GEOPeriodicType * >( face->nb.rear().first ) );
          outerElement_    = face->nb.front().first ;
          outerFaceNumber_ = face->nb.front().second ;
        }
        else
        {
          alugrid_assert ( dynamic_cast< const GEOPeriodicType * >( face->nb.front().first ) );
          outerElement_    = face->nb.rear().first ;
          outerFaceNumber_ = face->nb.rear().second ;
        }

        alugrid_assert ( outerElement_->isRealObject() );
        if( outerElement_->isboundary() )
        {
          alugrid_assert ( dynamic_cast< const BNDFaceType * >( outerElement_ ) );
          bnd = static_cast< const BNDFaceType * >( outerElement_ );
        }
        else
          outerTwist_ = outerEntity().twist( outerFaceNumber_ );
      }
      if ( bnd ) // the boundary case
      {
        alugrid_assert ( bnd );

        // if this cast is valid we have either
        // a boundary or a ghost element
        // the ghost element case
        if( parallel() && bnd->bndtype() == ALU3DSPACE ProcessorBoundary_t)
        {
          // if nonconformity occurs then go up one level
          if( bnd->level () != bnd->ghostLevel() && !conformingRefinement_)
          {
            bnd = static_cast<const BNDFaceType *>(bnd->up());
            alugrid_assert ( bnd );
            outerElement_ = static_cast<const HasFaceType*> (bnd);
          }

          // set boundary type to ghost boundary
          bndType_ = outerGhostBoundary ;

          if(conformingRefinement_)
            outerTwist_ = boundaryFace().twist(outerALUFaceIndex());

          // access ghost only when ghost cells are enabled
          if( ghostCellsEnabled_ )
          {
            // get ghost and internal number
            GhostPairType p  = bnd->getGhost();
            outerFaceNumber_ = p.second;

            const GEOElementType* ghost = static_cast<const GEOElementType*> (p.first);
            alugrid_assert ( ghost );
            outerTwist_ = ghost->twist(outerFaceNumber_);
          }
        }
        else // the normal boundary case
        {
          // get outer twist
          outerTwist_ = boundaryFace().twist(outerALUFaceIndex());
          // compute segment index when needed
          segmentId_ = boundaryFace().segmentId();
          bndId_ = boundaryFace().bndtype();
        }
      }
    } // if outerElement_->isboundary
    else
    {
      // get outer twist
      outerTwist_ = outerEntity().twist(outerALUFaceIndex());
    }

    //in the case of a levelIntersectionIterator and conforming elements
    //we assume the macro grid view. So we go up to level 0
    //after that we have to get new twist and facenumbers
    if(levelIntersection_ && conformingRefinement_ && !  (outerElement_->isboundary() ) )
    {
      const GEOElementType * outer = static_cast<const GEOElementType *> (outerElement_);
      while( outer -> up () ) outer = static_cast<const GEOElementType *> ( outer ->up() );
      outerElement_ = static_cast<const HasFaceType *> (outer);
      outerTwist_ = outerEntity().twist(outerFaceNumber_);
    }

    // make sure we got boundary id correctly
    alugrid_assert ( bndType_ == periodicBoundary || bndType_ == domainBoundary ? bndId_ > 0 : bndId_ == 0 );

    //make sure twists are set
    alugrid_assert( innerTwist_ != -665);
    alugrid_assert( outerTwist_ != -665);

    // set conformance information
    conformanceState_ = getConformanceState(innerLevel);
  }

  // points face from inner element away?
  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline ALU3dGridFaceInfo< dim, dimworld, type, Comm >::
  ALU3dGridFaceInfo(const GEOFaceType& face,
                    int innerTwist)
  {
    updateFaceInfo(face,innerTwist);
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline ALU3dGridFaceInfo< dim, dimworld, type, Comm >::~ALU3dGridFaceInfo() {}

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  ALU3dGridFaceInfo< dim, dimworld, type, Comm >::
  ALU3dGridFaceInfo ( const ALU3dGridFaceInfo &orig )
  : face_(orig.face_),
    innerElement_(orig.innerElement_),
    outerElement_(orig.outerElement_),
    innerFaceNumber_(orig.innerFaceNumber_),
    outerFaceNumber_(orig.outerFaceNumber_),
    innerTwist_(orig.innerTwist_),
    outerTwist_(orig.outerTwist_),
    segmentId_( orig.segmentId_ ),
    bndId_( orig.bndId_ ),
    bndType_( orig.bndType_ ),
    conformanceState_(orig.conformanceState_),
    conformingRefinement_( orig.conformingRefinement_ ),
    ghostCellsEnabled_( orig.ghostCellsEnabled_ ),
    levelIntersection_( orig.levelIntersection_ )
  {}

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline bool ALU3dGridFaceInfo< dim, dimworld, type, Comm >::isElementLike() const {
    return bndType_ < domainBoundary;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline bool ALU3dGridFaceInfo< dim, dimworld, type, Comm >::innerBoundary() const {
    return bndType_ == innerGhostBoundary;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline bool ALU3dGridFaceInfo< dim, dimworld, type, Comm >::outerBoundary() const {
    return bndType_ == domainBoundary;
  }

  template< int dim, int dimworld,  ALU3dGridElementType type, class Comm >
  inline bool ALU3dGridFaceInfo< dim, dimworld, type, Comm >::boundary() const {
    return outerBoundary() || (bndType_ == periodicBoundary);
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline bool ALU3dGridFaceInfo< dim, dimworld, type, Comm >::neighbor() const
  {
    return isElementLike() || ( ghostBoundary() && ghostCellsEnabled_ );
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline bool ALU3dGridFaceInfo< dim, dimworld, type, Comm >::ghostBoundary () const
  {
    // when communicator is No_Comm there is no ghost boundary
    return parallel() ? ( bndType_ == outerGhostBoundary ) : false ;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline const typename ALU3dGridFaceInfo< dim, dimworld, type, Comm >::GEOFaceType&
  ALU3dGridFaceInfo< dim, dimworld, type, Comm >::face() const
  {
    return *face_;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline const typename ALU3dGridFaceInfo< dim, dimworld, type, Comm >::GEOElementType&
  ALU3dGridFaceInfo< dim, dimworld, type, Comm >::innerEntity() const
  {
    alugrid_assert ( ! innerElement_->isboundary() );
    return static_cast<const GEOElementType&>(*innerElement_);
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline const typename ALU3dGridFaceInfo< dim, dimworld, type, Comm >::GEOElementType&
  ALU3dGridFaceInfo< dim, dimworld, type, Comm >::outerEntity() const
  {
    alugrid_assert ( isElementLike() );
    return static_cast<const GEOElementType&>(*outerElement_);
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline const typename ALU3dGridFaceInfo< dim, dimworld, type, Comm >::BNDFaceType&
  ALU3dGridFaceInfo< dim, dimworld, type, Comm >::innerFace() const
  {
    alugrid_assert ( innerElement_->isboundary() );
    return static_cast<const BNDFaceType&>(*innerElement_);
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline const typename ALU3dGridFaceInfo< dim, dimworld, type, Comm >::BNDFaceType&
  ALU3dGridFaceInfo< dim, dimworld, type, Comm >::boundaryFace() const {
    alugrid_assert ( ! isElementLike() );
    return static_cast<const BNDFaceType&>(*outerElement_);
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline int ALU3dGridFaceInfo< dim, dimworld, type, Comm >::outsideLevel() const
  {
    alugrid_assert ( outerElement_ );
    alugrid_assert ( !isElementLike() || outerEntity().level() == outerElement_->nbLevel() );
    alugrid_assert ( isElementLike() || boundaryFace().level() == outerElement_->nbLevel() );
    return outerElement_->nbLevel();
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline int ALU3dGridFaceInfo< dim, dimworld, type, Comm >::segmentId() const
  {
    alugrid_assert ( segmentId_ >= 0 );
    return segmentId_;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline int ALU3dGridFaceInfo< dim, dimworld, type, Comm >::boundaryId() const
  {
    return bndId_;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline int ALU3dGridFaceInfo< dim, dimworld, type, Comm >::innerTwist() const
  {
    // don't check ghost boundaries here
    alugrid_assert ( ( ! innerBoundary() ) ?
        innerEntity().twist(innerALUFaceIndex()) == innerTwist_ : true );
    return innerTwist_;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline int ALU3dGridFaceInfo< dim, dimworld, type, Comm >::duneTwist(const int faceIdx, const int aluTwist) const
  {
    typedef ElementTopologyMapping<type> ElementTopo;
    typedef FaceTopologyMapping<type> FaceTopo;

    const int mappedZero =
      FaceTopo::twist(ElementTopo::dune2aluFaceVertex( faceIdx, 0), aluTwist);

    const int twist =
      (ElementTopo::faceOrientation( faceIdx ) * sign(aluTwist) < 0 ?
       mappedZero : -mappedZero-1);

    // see topology.* files for aluTwistMap
    if constexpr ( dim == 2 )
    {
      // in 2d twists are either 0 or 1, but because
      // of the underlying 3d alu grid they could be different
      // therefore we adjust to the right range
      const int duneTwst = FaceTopo :: aluTwistMap( twist );
      return (duneTwst == 0) ? 0 : 1;
    }
    else
    {
      return FaceTopo :: aluTwistMap( twist );
    }
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline int ALU3dGridFaceInfo< dim, dimworld, type, Comm >::outerTwist() const
  {
    // don't check ghost boundaries here
    //alugrid_assert ( (outerBoundary_) ?
    //          (outerTwist_ == boundaryFace().twist(0)) :
    //          (! ghostBoundary_) ?
    //          (outerTwist_ == outerEntity().twist(outerALUFaceIndex())) : true
    //      );
    return outerTwist_;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline int ALU3dGridFaceInfo< dim, dimworld, type, Comm >::innerALUFaceIndex() const {
    return innerFaceNumber_;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline int ALU3dGridFaceInfo< dim, dimworld, type, Comm >::outerALUFaceIndex() const {
    return outerFaceNumber_;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  typename ALU3dGridFaceInfo< dim, dimworld, type, Comm >::ConformanceState
  inline ALU3dGridFaceInfo< dim, dimworld, type, Comm >::conformanceState() const
  {
    alugrid_assert ( conformanceState_ != UNDEFINED );
    return conformanceState_;
  }

  // calculate conformance state
  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  typename ALU3dGridFaceInfo< dim, dimworld, type, Comm >::ConformanceState
  inline ALU3dGridFaceInfo< dim, dimworld, type, Comm >::getConformanceState(const int innerLevel) const
  {
    ConformanceState result = CONFORMING;

    // in case of non-conforming refinement check level difference
    if( ! conformingRefinement_ )
    {
      // A boundary is always unrefined
      int levelDifference = 0 ;
      if ( isElementLike() )
        levelDifference = innerLevel - outerEntity().level();
      else
        levelDifference = innerLevel - boundaryFace().level();

      if (levelDifference < 0) {
        result = REFINED_OUTER;
      }
      else if (levelDifference > 0) {
        result = REFINED_INNER;
      }
    }

    return result;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline ALU3dGridGeometricFaceInfoBase< dim, dimworld, type, Comm >::
  ALU3dGridGeometricFaceInfoBase(const ConnectorType& connector) :
    connector_(connector),
    coordsSelfLocal_(-1.0),
    coordsNeighborLocal_(-1.0),
    generatedGlobal_(false),
    generatedLocal_(false)
  {
    const auto& refFace = (type == tetra) ?
      Dune::ReferenceElements< alu3d_ctype, 2 >::simplex() :
      Dune::ReferenceElements< alu3d_ctype, 2 >::cube() ;

    // do the mappings
    const int numCorners = refFace.size( 2 );
    alugrid_assert( numCorners == int(childLocal_.size()) );
    for( int i = 0; i < numCorners; ++i )
    {
      childLocal_[ i ] = refFace.position( i, 2 );
    }
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline void
  ALU3dGridGeometricFaceInfoBase< dim, dimworld, type, Comm >::
  resetFaceGeom()
  {
    generatedGlobal_ = false;
    generatedLocal_  = false;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline const typename ALU3dGridGeometricFaceInfoBase< dim, dimworld, type, Comm >::CoordinateType&
  ALU3dGridGeometricFaceInfoBase< dim, dimworld, type, Comm >::intersectionSelfLocal() const {
    generateLocalGeometries();
    alugrid_assert (generatedLocal_);
    return coordsSelfLocal_;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline const typename ALU3dGridGeometricFaceInfoBase< dim, dimworld, type, Comm >::CoordinateType&
  ALU3dGridGeometricFaceInfoBase< dim, dimworld, type, Comm >::intersectionNeighborLocal() const {
    alugrid_assert (!connector_.outerBoundary());
    generateLocalGeometries();
    alugrid_assert (generatedLocal_);
    return coordsNeighborLocal_;
  }


  //sepcialisation for tetra and hexa
  template< int dim, int dimworld, class Comm >
  inline ALU3dGridGeometricFaceInfoTetra< dim, dimworld, Comm >::
  ALU3dGridGeometricFaceInfoTetra(const ConnectorType& connector)
  : Base( connector ), normalUp2Date_( false )
  {}

  template< int dim, int dimworld, class Comm >
  inline void ALU3dGridGeometricFaceInfoTetra< dim, dimworld, Comm >::
  resetFaceGeom()
  {
    Base::resetFaceGeom();
    normalUp2Date_ = false;
  }

  template< int dim, int dimworld, class Comm >
  inline ALU3dGridGeometricFaceInfoTetra< dim, dimworld, Comm >::
  ALU3dGridGeometricFaceInfoTetra(const ALU3dGridGeometricFaceInfoTetra& orig)
  : Base( orig ), normalUp2Date_( orig.normalUp2Date_ )
  {}

  template< int dim, int dimworld, class Comm >
  template <class GeometryImp>
  inline void
  ALU3dGridGeometricFaceInfoTetra< dim, dimworld, Comm >::
  buildGlobalGeom(GeometryImp& geo) const
  {
    if (! this->generatedGlobal_)
    {
      // calculate the normal
      const GEOFaceType & face = this->connector_.face();

      geo.buildGeom( face.myvertex(FaceTopo::dune2aluVertex(0))->Point() ,
                     face.myvertex(FaceTopo::dune2aluVertex(1))->Point() ,
                     face.myvertex(FaceTopo::dune2aluVertex(2))->Point() );

      this->generatedGlobal_ = true ;
    }
  }

  template< int dim, int dimworld, class Comm >
  inline FieldVector<alu3d_ctype, 3> &
  ALU3dGridGeometricFaceInfoTetra< dim, dimworld, Comm >::
  outerNormal(const FieldVector<alu3d_ctype, 2>& local) const
  {
    // if geomInfo was not reseted then normal is still correct
    if(!normalUp2Date_)
    {
      // calculate the normal
      const GEOFaceType & face = this->connector_.face();
      const alu3d_ctype (&_p0)[3] = face.myvertex(0)->Point();
      const alu3d_ctype (&_p1)[3] = face.myvertex(1)->Point();
      const alu3d_ctype (&_p2)[3] = face.myvertex(2)->Point();

      // change sign if face normal points into inner element
      // factor is 1.0 to get integration outer normal and not volume outer normal
      const double factor = (this->connector_.innerTwist() < 0) ? 1.0 : -1.0;

      // see mapp_tetra_3d.h for this piece of code
      outerNormal_[0] = factor * ((_p1[1]-_p0[1]) *(_p2[2]-_p1[2]) - (_p2[1]-_p1[1]) *(_p1[2]-_p0[2]));
      outerNormal_[1] = factor * ((_p1[2]-_p0[2]) *(_p2[0]-_p1[0]) - (_p2[2]-_p1[2]) *(_p1[0]-_p0[0]));
      outerNormal_[2] = factor * ((_p1[0]-_p0[0]) *(_p2[1]-_p1[1]) - (_p2[0]-_p1[0]) *(_p1[1]-_p0[1]));

      normalUp2Date_ = true;
    } // end if mapp ...

    return outerNormal_;
  }

  //-sepcialisation for and hexa
  template< int dim, int dimworld, class Comm >
  inline ALU3dGridGeometricFaceInfoHexa< dim, dimworld, Comm >::
  ALU3dGridGeometricFaceInfoHexa(const ConnectorType& connector)
  : Base( connector )
    , mappingGlobal_()
    , mappingGlobalUp2Date_(false)
  {}

  template< int dim, int dimworld, class Comm >
  inline void ALU3dGridGeometricFaceInfoHexa< dim, dimworld, Comm >::
  resetFaceGeom()
  {
    Base::resetFaceGeom();
    mappingGlobalUp2Date_ = false;
  }

  template< int dim, int dimworld, class Comm >
  inline ALU3dGridGeometricFaceInfoHexa< dim, dimworld, Comm >::
  ALU3dGridGeometricFaceInfoHexa(const ALU3dGridGeometricFaceInfoHexa& orig)
  : Base( orig )
    , mappingGlobal_(orig.mappingGlobal_)
    , mappingGlobalUp2Date_(orig.mappingGlobalUp2Date_)
  {}

  template< int dim, int dimworld, class Comm >
  template <class GeometryImp>
  inline void
  ALU3dGridGeometricFaceInfoHexa< dim, dimworld, Comm >::
  buildGlobalGeom(GeometryImp& geo) const
  {
    if (! this->generatedGlobal_)
    {
      // calculate the normal
      const GEOFaceType & face = this->connector_.face();

      geo.buildGeom( face.myvertex(FaceTopo::dune2aluVertex(0))->Point() ,
                     face.myvertex(FaceTopo::dune2aluVertex(1))->Point() ,
                     face.myvertex(FaceTopo::dune2aluVertex(2))->Point() ,
                     face.myvertex(FaceTopo::dune2aluVertex(3))->Point() );
      this->generatedGlobal_ = true ;
    }
  }

  template< int dim, int dimworld, class Comm >
  inline FieldVector<alu3d_ctype, 3> &
  ALU3dGridGeometricFaceInfoHexa< dim, dimworld, Comm >::
  outerNormal(const FieldVector<alu3d_ctype, 2>& local) const
  {
    // if mapping calculated and affine, nothing more to do
    if ( mappingGlobal_.affine () && mappingGlobalUp2Date_ )
      return outerNormal_ ;

    // update surface mapping
    if(! mappingGlobalUp2Date_ )
    {
      const GEOFaceType & face = connector_.face();
      // update mapping to actual face
      mappingGlobal_.buildMapping(
        face.myvertex( FaceTopo::dune2aluVertex(0) )->Point(),
        face.myvertex( FaceTopo::dune2aluVertex(1) )->Point(),
        face.myvertex( FaceTopo::dune2aluVertex(2) )->Point(),
        face.myvertex( FaceTopo::dune2aluVertex(3) )->Point()
        );
      mappingGlobalUp2Date_ = true;
    }

    // calculate the normal
    // has to be calculated every time normal called, because
    // depends on local
    if (connector_.innerTwist() < 0)
      mappingGlobal_.negativeNormal(local,outerNormal_);
    else
      mappingGlobal_.normal(local,outerNormal_);

    // end if
    return outerNormal_;
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline void ALU3dGridGeometricFaceInfoBase< dim, dimworld, type, Comm >::
  generateLocalGeometries() const
  {
    if (!generatedLocal_)
    {
      // Get the coordinates of the face in the reference element of the
      // adjoining inner and outer elements and initialise the respective
      // geometries
      switch (connector_.conformanceState())
      {
      case (ConnectorType::CONFORMING) :
        referenceElementCoordinatesRefined(INNER, coordsSelfLocal_);
        // generate outer local geometry only when not at boundary
        // * in the parallel case, this needs to be altered for the ghost cells
        if (!connector_.outerBoundary()) {
          referenceElementCoordinatesRefined(OUTER, coordsNeighborLocal_);
        } // end if
        break;
      case (ConnectorType::REFINED_INNER) :
        referenceElementCoordinatesRefined(INNER, coordsSelfLocal_);
        referenceElementCoordinatesUnrefined(OUTER, coordsNeighborLocal_);
        break;
      case (ConnectorType::REFINED_OUTER) :
        referenceElementCoordinatesUnrefined(INNER, coordsSelfLocal_);
        referenceElementCoordinatesRefined(OUTER, coordsNeighborLocal_);
        break;
      default :
        std::cerr << "ERROR: Wrong conformanceState in generateLocalGeometries! in: " << __FILE__ << " line: " << __LINE__<< std::endl;
        alugrid_assert (false);
        exit(1);
      } // end switch

      generatedLocal_ = true;
    } // end if
  }

  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline int ALU3dGridGeometricFaceInfoBase< dim, dimworld, type, Comm >::
  globalVertexIndex(const int duneFaceIndex,
                    const int aluFaceTwist,
                    const int duneFaceVertexIndex) const
  {
    const int localALUIndex =
      FaceTopo::dune2aluVertex(duneFaceVertexIndex,
                               aluFaceTwist);

    // get local ALU vertex number on the element's face
    const int localDuneIndex = ElementTopo::
        alu2duneFaceVertex(ElementTopo::dune2aluFace(duneFaceIndex),
                           localALUIndex);

    return getReferenceElement().subEntity(duneFaceIndex, 1, localDuneIndex, 3);
  }


  template< int dim, int dimworld, ALU3dGridElementType type, class Comm >
  inline void ALU3dGridGeometricFaceInfoBase< dim, dimworld, type, Comm >::
  referenceElementCoordinatesRefined(SideIdentifier side,
                                     CoordinateType& result) const
  {
    // this is a dune face index
    const int faceIndex =
      (side == INNER ?
       ElementTopo::alu2duneFace(connector_.innerALUFaceIndex()) :
       ElementTopo::alu2duneFace(connector_.outerALUFaceIndex()));
    const int faceTwist =
      (side == INNER ?
       connector_.innerTwist() :
       connector_.outerTwist());

    const ReferenceElementType& refElem = getReferenceElement();

    for (int i = 0; i < numVerticesPerFace; ++i)
    {
      int duneVertexIndex = globalVertexIndex(faceIndex, faceTwist, i);
      result[i] = refElem.position(duneVertexIndex, 3);
    }
  }

  template<  int dimworld, ALU3dGridElementType type, class Comm >
  inline ALU3dGridGeometricFaceInfoBase< 2, dimworld, type, Comm >::
  ALU3dGridGeometricFaceInfoBase(const ConnectorType& connector) :
    connector_(connector),
    coordsSelfLocal_(-1.0),
    coordsNeighborLocal_(-1.0),
    generatedGlobal_(false),
    generatedLocal_(false)
  {}

  template<  int dimworld, ALU3dGridElementType type, class Comm >
  inline void
  ALU3dGridGeometricFaceInfoBase< 2, dimworld, type, Comm >::
  resetFaceGeom()
  {
    generatedGlobal_ = false;
    generatedLocal_  = false;
  }

  template< int dimworld, ALU3dGridElementType type, class Comm >
  inline ALU3dGridGeometricFaceInfoBase< 2, dimworld, type, Comm >::
  ALU3dGridGeometricFaceInfoBase ( const ALU3dGridGeometricFaceInfoBase &orig )
  : connector_(orig.connector_),
    coordsSelfLocal_(orig.coordsSelfLocal_),
    coordsNeighborLocal_(orig.coordsNeighborLocal_),
    generatedGlobal_(orig.generatedGlobal_),
    generatedLocal_(orig.generatedLocal_)
  {}

  template<  int dimworld, ALU3dGridElementType type, class Comm >
  inline const typename ALU3dGridGeometricFaceInfoBase< 2, dimworld, type, Comm >::LocalCoordinateType&
  ALU3dGridGeometricFaceInfoBase< 2, dimworld, type, Comm >::intersectionSelfLocal() const {
    generateLocalGeometries();
    alugrid_assert (generatedLocal_);
    return coordsSelfLocal_;
  }

  template<  int dimworld, ALU3dGridElementType type, class Comm >
  inline const typename ALU3dGridGeometricFaceInfoBase< 2, dimworld, type, Comm >::LocalCoordinateType&
  ALU3dGridGeometricFaceInfoBase< 2, dimworld, type, Comm >::intersectionNeighborLocal() const {
    alugrid_assert (!connector_.outerBoundary());
    generateLocalGeometries();
    alugrid_assert (generatedLocal_);
    return coordsNeighborLocal_;
  }


  //sepcialisation for tetra and hexa
  template< int dimworld, class Comm >
  inline ALU3dGridGeometricFaceInfoTetra< 2, dimworld, Comm >::
  ALU3dGridGeometricFaceInfoTetra(const ConnectorType& connector)
  : Base( connector ), normalUp2Date_( false )
  {}

  template<  int dimworld, class Comm >
  inline void ALU3dGridGeometricFaceInfoTetra< 2, dimworld, Comm >::
  resetFaceGeom()
  {
    Base::resetFaceGeom();
    normalUp2Date_ = false;
  }

  template<  int dimworld, class Comm >
  inline ALU3dGridGeometricFaceInfoTetra< 2, dimworld, Comm >::
  ALU3dGridGeometricFaceInfoTetra(const ALU3dGridGeometricFaceInfoTetra& orig)
  : Base( orig ), normalUp2Date_( orig.normalUp2Date_ )
  {}

  template<  int dimworld, class Comm >
  template <class GeometryImp>
  inline void
  ALU3dGridGeometricFaceInfoTetra< 2, dimworld, Comm >::
  buildGlobalGeom(GeometryImp& geo) const
  {
        //could be wrong in twist sense
    if (! this->generatedGlobal_)
    {
      // calculate the normal
      const GEOFaceType & face = this->connector_.face();

      geo.buildGeom( face.myvertex(FaceTopo::dune2aluVertex(1))->Point() ,
                     face.myvertex(FaceTopo::dune2aluVertex(2))->Point() );

      this->generatedGlobal_ = true ;
    }
  }

  template< int dimworld, class Comm >
  inline FieldVector<alu3d_ctype, dimworld> &
  ALU3dGridGeometricFaceInfoTetra< 2, dimworld, Comm >::
  outerNormal(const FieldVector<alu3d_ctype, 1>& local) const
  {
    // if geomInfo was not reseted then normal is still correct
    if(!normalUp2Date_)
    {
      // calculate the normal
      const GEOFaceType & face = this->connector_.face();
      const alu3d_ctype (&_p1)[3] = face.myvertex(1)->Point();
      const alu3d_ctype (&_p2)[3] = face.myvertex(2)->Point();

      // change sign if face normal points into inner element
      // factor is 1.0 to get integration outer normal and not volume outer normal
      const double factor = (this->connector_.innerTwist() < 0) ? 1.0 : -1.0;


      if(dimworld == 2)
      {
        // we want the outer normal orhtogonal to the intersection and  with length of the intersection
        outerNormal_[0] = factor * (_p2[1]-_p1[1]);
        outerNormal_[1] = factor * (_p1[0]-_p2[0]);
      }
      //implemented in iterator_imp.cc
      //else if(dimworld == 3)

      normalUp2Date_ = true;
    } // end if mapp ...

    return outerNormal_;
  }

  //-sepcialisation for and hexa
  template<  int dimworld, class Comm >
  inline ALU3dGridGeometricFaceInfoHexa< 2, dimworld, Comm >::
  ALU3dGridGeometricFaceInfoHexa(const ConnectorType& connector)
  : Base( connector )
    , normalUp2Date_(false)
  {}

  template< int dimworld, class Comm >
  inline void ALU3dGridGeometricFaceInfoHexa< 2, dimworld, Comm >::
  resetFaceGeom()
  {
    Base::resetFaceGeom();
    normalUp2Date_ = false;
  }

  template<  int dimworld, class Comm >
  inline ALU3dGridGeometricFaceInfoHexa< 2, dimworld, Comm >::
  ALU3dGridGeometricFaceInfoHexa(const ALU3dGridGeometricFaceInfoHexa& orig)
  : Base( orig )
    , normalUp2Date_(orig.normalUp2Date_)
  {}

  template<  int dimworld, class Comm >
  template <class GeometryImp>
  inline void
  ALU3dGridGeometricFaceInfoHexa< 2, dimworld, Comm >::
  buildGlobalGeom(GeometryImp& geo) const
  {
    //could be wrong in twist sense
    if (! this->generatedGlobal_)
    {
      // calculate the normal
      const GEOFaceType & face = this->connector_.face();

      geo.buildGeom( face.myvertex(FaceTopo::dune2aluVertex(0))->Point() ,
                     face.myvertex(FaceTopo::dune2aluVertex(1))->Point() );
      this->generatedGlobal_ = true ;
    }
  }

  template<  int dimworld, class Comm >
  inline FieldVector<alu3d_ctype, dimworld> &
  ALU3dGridGeometricFaceInfoHexa< 2, dimworld, Comm >::
  outerNormal(const FieldVector<alu3d_ctype, 1>& local) const
  {
    // if geomInfo was not reseted then normal is still correct
    if(!normalUp2Date_)
    {
      // calculate the normal
      const GEOFaceType & face = this->connector_.face();
      const alu3d_ctype (&_p0)[3] = face.myvertex(0)->Point();
      const alu3d_ctype (&_p3)[3] = face.myvertex(3)->Point();

      // change sign if face normal points into inner element
      // factor is 1.0 to get integration outer normal and not volume outer normal
      const double factor = (this->connector_.innerTwist() < 0) ? -1.0 : 1.0;

      if(dimworld == 2)
      {
        // we want the length of the intersection and orthogonal to it
        outerNormal_[0] = factor * (_p0[1] - _p3[1]);
        outerNormal_[1] = factor * (_p3[0] - _p0[0]);
      }
      //implemented in iterator_imp.cc
      //else if(dimworld == 3)

      normalUp2Date_ = true;
    } // end if mapp ...

    return outerNormal_;
  }

  template<  int dimworld, ALU3dGridElementType type, class Comm >
  inline void ALU3dGridGeometricFaceInfoBase< 2, dimworld, type, Comm >::
  generateLocalGeometries() const
  {
    if (!generatedLocal_)
    {
      // Get the coordinates of the face in the reference element of the
      // adjoining inner and outer elements and initialise the respective
      // geometries
      switch (connector_.conformanceState())
      {
      case (ConnectorType::CONFORMING) :
        referenceElementCoordinatesRefined(INNER, coordsSelfLocal_);
        // generate outer local geometry only when not at boundary
        // * in the parallel case, this needs to be altered for the ghost cells
        if (!connector_.outerBoundary() && !(connector_.conformingRefinement() && connector_.ghostBoundary())) {
          referenceElementCoordinatesRefined(OUTER, coordsNeighborLocal_);
        } // end if
        break;
      case (ConnectorType::REFINED_INNER) :
        referenceElementCoordinatesRefined(INNER, coordsSelfLocal_);
        referenceElementCoordinatesUnrefined(OUTER, coordsNeighborLocal_);
        break;
      case (ConnectorType::REFINED_OUTER) :
        referenceElementCoordinatesUnrefined(INNER, coordsSelfLocal_);
        referenceElementCoordinatesRefined(OUTER, coordsNeighborLocal_);
        break;
      default :
        std::cerr << "ERROR: Wrong conformanceState in generateLocalGeometries! in: " << __FILE__ << " line: " << __LINE__<< std::endl;
        alugrid_assert (false);
        exit(1);
      } // end switch

      generatedLocal_ = true;
    } // end if
  }

  template<  int dimworld, ALU3dGridElementType type, class Comm >
  inline int ALU3dGridGeometricFaceInfoBase< 2, dimworld, type, Comm >::
  globalVertexIndex(const int duneFaceIndex,
                    const int aluFaceTwist,
                    const int duneFaceVertexIndex) const
  {
    //we want vertices 1,2 of the real 3d DUNE face for tetras and 0,1 for hexas
    const  int localALUIndex =
      FaceTopo::dune2aluVertex(type == tetra ? duneFaceVertexIndex + 1 : duneFaceVertexIndex,
                               aluFaceTwist);

    // get local DUNE vertex number on the element's face - for tetra map  1,2 of real 3d face back to 0,1 by subtracting 1
    const int localDuneIndex = (type == tetra) ?
                             ElementTopo::alu2duneFaceVertex(ElementTopo::dune2aluFace(duneFaceIndex), localALUIndex) - 1
                             :
                             ElementTopo::alu2duneFaceVertex(ElementTopo::dune2aluFace(duneFaceIndex), localALUIndex)
                             ;

   /* std::cout << "duneFaceIndex: " << duneFaceIndex << std::endl;
    std::cout << "aluFaceTwist: " << aluFaceTwist << std::endl;
    std::cout << "duneFaceVertexIndex: " << duneFaceVertexIndex << std::endl;
    std::cout << "localALUIndex: " << localALUIndex << std::endl;
    std::cout << "localDuneIndex: " << localDuneIndex << std::endl;
    std ::cout << "ReferenceElementindex: " << getReferenceElement().subEntity(duneFaceIndex, 1, localDuneIndex, 2) << std::endl << std::endl; */
    assert( localDuneIndex == 0 || localDuneIndex == 1 );
    return getReferenceElement().subEntity(duneFaceIndex, 1, localDuneIndex, 2);
  }


  template<  int dimworld, ALU3dGridElementType type, class Comm >
  inline void ALU3dGridGeometricFaceInfoBase< 2, dimworld, type, Comm >::
  referenceElementCoordinatesRefined(SideIdentifier side,
                                    LocalCoordinateType& result) const
  {
    // this is a dune face index
    const int faceIndex =
      (side == INNER ?
       ElementTopo::alu2duneFace(connector_.innerALUFaceIndex()) :
       ElementTopo::alu2duneFace(connector_.outerALUFaceIndex()));
    const int faceTwist =
      (side == INNER ?
       connector_.innerTwist() :
       connector_.outerTwist());

    const ReferenceElementType& refElem = getReferenceElement();

    for (int i = 0; i < numVerticesPerFace; ++i)
    {
      int duneVertexIndex = globalVertexIndex(faceIndex, faceTwist, i);
      result[i] = refElem.position(duneVertexIndex, 2);
    }
  }
} //end namespace Dune
#endif
