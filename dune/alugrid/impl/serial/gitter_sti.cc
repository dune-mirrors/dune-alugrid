// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface
// (c) Robert Kloefkorn 2004 - 2005
#include <config.h>

#include <fstream>
#include <iostream>

#include "lock.h"
#include "gitter_sti.h"
#include "walk.h"

namespace ALUGrid
{
  Gitter::Makrogitter::MkGitName Gitter::Makrogitter::_msg( inMkGiter() );

#ifdef ALUGRIDDEBUG
#ifdef DEBUG_ALUGRID
  Refcount::Globalcount Refcount::_g;

  Refcount::Globalcount::~Globalcount () {
    alugrid_assert (_c ? (std::cerr << "**WARNING Refcount::Globalcount::~Globalcount() " << _c
             << " objects have not been removed correctly!" << std::endl, 1) : 1);
    return;
  }
#endif
#endif

  typedef Wrapper < AccessIterator < Gitter::vertex_STI >::Handle,
    Gitter::InternalVertex >              leaf_vertex__macro_vertex__iterator;

  typedef Insert < AccessIterator < Gitter::hedge_STI >::Handle,
    TreeIterator < Gitter::hedge_STI, is_leaf < Gitter::hedge_STI > > >   leaf_edge__macro_edge__iterator;

  typedef Insert < AccessIterator < Gitter::hface_STI >::Handle,
    TreeIterator < Gitter::hface_STI, is_leaf < Gitter::hface_STI > > >   leaf_face__macro_face__iterator;

  typedef Insert < AccessIterator < Gitter::hbndseg_STI >::Handle,
    TreeIterator < Gitter::hbndseg_STI, is_leaf < Gitter::hbndseg_STI> > >  leaf_bnd__macro_bnd__iterator;

  typedef Insert < AccessIterator < Gitter::helement_STI >::Handle,
    TreeIterator < Gitter::helement_STI, is_leaf < Gitter::helement_STI> > > leaf_element__macro_element__iterator;

  IteratorSTI < Gitter::vertex_STI > * Gitter::iterator (const Gitter::vertex_STI *) {

    std::vector< IteratorSTI < vertex_STI > * > _iterators;
    {
      _iterators.push_back ( new AccessIterator < vertex_STI >::Handle (container ()));
    }
    Insert < AccessIterator < hedge_STI >::Handle,
    TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > dw (container ());
    _iterators.push_back ( new Wrapper < Insert < AccessIterator < hedge_STI >::Handle,
    TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (dw));
    {
      Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_vertex < hface_STI > > > fw (container ());
      _iterators.push_back ( new Wrapper < Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (fw));
    }
    {
      Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_vertex < helement_STI > > > ew (container ());
      _iterators.push_back ( new Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_vertex < helement_STI > > >, InternalVertex > (ew));
    }
    {
      Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > > fw (container ());
      Wrapper < Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > df (fw);
      Insert < Wrapper < Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > > dif (df);
      _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > >, InternalVertex > (dif));
    }
    {
      Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_edge < helement_STI > > > ew (container ());
      Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge > de (ew);
      Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > > die (de);
      _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > >, InternalVertex > (die));
    }
    {
      Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > > ew (container ());
      Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fe (ew);
      Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_vertex < hface_STI > > > fie (fe);
      _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (fie));
    }
    {
      Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > > ew (container ());
      Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fe (ew);
      Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > > fie (fe);
      Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > dfie (fie);
      Insert < Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > difie (dfie);
      _iterators.push_back (new Wrapper < Insert < Wrapper < Insert < Wrapper <
    Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (difie));
    }
    return new VectorAlign < vertex_STI > (_iterators);
  }

  IteratorSTI < Gitter::hedge_STI > * Gitter::iterator (const hedge_STI * e)
  {
    is_leaf< hedge_STI > rule;
    return this->createIterator(e, rule);
  }

  IteratorSTI < Gitter::hface_STI > * Gitter::iterator (const hface_STI * f)
  {
    is_leaf< hface_STI > rule;
    return this->createIterator(f, rule);
  }

  IteratorSTI < Gitter::hbndseg_STI > * Gitter::iterator (const hbndseg_STI * bnd)
  {
    is_leaf <hbndseg_STI> rule;
    return this->createIterator(bnd, rule);
  }

  IteratorSTI < Gitter::helement_STI > * Gitter::iterator (const helement_STI *el)
  {
    is_leaf <helement_STI> rule;
    return this->createIterator(el, rule);
  }

  //**************************************************************************
  // all the level iterators
  //**************************************************************************
  IteratorSTI < Gitter::vertex_STI > * Gitter::
  levelIterator (const Gitter::vertex_STI * a, const any_has_level< vertex_STI > & vhl)
  {
    std::vector< IteratorSTI < vertex_STI > * > _iterators;
    {
      _iterators.push_back ( new AccessIterator < vertex_STI >::Handle (container ()));
    }
    Insert < AccessIterator < hedge_STI >::Handle,
    TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > dw (container ());
    _iterators.push_back ( new Wrapper < Insert < AccessIterator < hedge_STI >::Handle,
    TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (dw));
    {
      Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_vertex < hface_STI > > > fw (container ());
      _iterators.push_back ( new Wrapper < Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (fw));
    }
    {
      Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_vertex < helement_STI > > > ew (container ());
      _iterators.push_back ( new Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_vertex < helement_STI > > >, InternalVertex > (ew));
    }
    {
      Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > > fw (container ());
      Wrapper < Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > df (fw);
      Insert < Wrapper < Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > > dif (df);
      _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < hface_STI >::Handle,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > >, InternalVertex > (dif));
    }
    {
      Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_edge < helement_STI > > > ew (container ());
      Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge > de (ew);
      Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > > die (de);
      _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_edge < helement_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, unary_not < is_leaf < hedge_STI > > > >, InternalVertex > (die));
    }
    {
      Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > > ew (container ());
      Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fe (ew);
      Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_vertex < hface_STI > > > fie (fe);
      _iterators.push_back ( new Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_vertex < hface_STI > > >, InternalVertex > (fie));
    }
    {
      Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > > ew (container ());
      Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace > fe (ew);
      Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > > fie (fe);
      Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge > dfie (fie);
      Insert < Wrapper < Insert < Wrapper < Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > > difie (dfie);
      _iterators.push_back (new Wrapper < Insert < Wrapper < Insert < Wrapper <
    Insert < AccessIterator < helement_STI >::Handle,
    TreeIterator < helement_STI, has_int_face < helement_STI > > >, InternalFace >,
    TreeIterator < hface_STI, has_int_edge < hface_STI > > >, InternalEdge >,
    TreeIterator < hedge_STI, has_int_vertex < hedge_STI > > >, InternalVertex > (difie));
    }
    return new VectorAlign < vertex_STI > (_iterators);
  }

  // create level edge iterator
  IteratorSTI < Gitter::hedge_STI > * Gitter::levelIterator (const hedge_STI * e, const any_has_level<hedge_STI> & ahl)
  {
    any_has_level<hedge_STI> rule(ahl);
    return this->createIterator(e,rule);
  }

  // create level face iterator
  IteratorSTI < Gitter::hface_STI > * Gitter::
  levelIterator (const hface_STI * f , const any_has_level<hface_STI> & ahl )
  {
    any_has_level< hface_STI > rule(ahl);
    return this->createIterator(f, rule);
  }

  // create level element iterator
  IteratorSTI < Gitter::helement_STI > * Gitter::levelIterator (const helement_STI * el, const any_has_level<helement_STI> & ahl)
  {
    any_has_level <helement_STI> rule(ahl);
    return this->createIterator(el, rule);
  }

  IteratorSTI < Gitter::hbndseg_STI > * Gitter::levelIterator (const hbndseg_STI * bnd, const any_has_level<hbndseg_STI> & ahl)
  {
    any_has_level<hbndseg_STI> rule(ahl);
    return this->createIterator(bnd, rule);
  }

  //*******************************************
  //  other methods on class Gitter
  //*******************************************
  void Gitter::fullIntegrityCheck ()
  {
    const int start = clock();
    std::size_t count = 0;
    leaf_element__macro_element__iterator w( container () );
    for( w.first (); !w.done(); w.next() )
    {
      if( w.item().test() )
        std::cerr << "ERROR: Internal error in element " << count << std::endl;
      ++count;
    }
    if (debugOption (3)) {
      float used = (float)(clock () - start)/(float)(CLOCKS_PER_SEC);
      std::cout << "INFO: Gitter::fullIntegrityCheck() used " << used << " s." << std::endl;
    }
    return;
  }

  void Gitter::printsize ()
  {
    std::cout << std::endl << "Gitter::printSize():" << std::endl << std::endl;
    if( debugOption( 10 ) )
    {
      std::cout << " - Macro elements .... "  << AccessIterator < helement_STI >::Handle (container ()).size() << std::endl;
      std::cout << " - Macro boundary .... " << AccessIterator < hbndseg_STI >::Handle (container ()).size() << std::endl;
      std::cout << " - Macro faces ....... " << AccessIterator < hface_STI >::Handle (container ()).size() << std::endl;
      std::cout << " - Macro edges ....... "  << AccessIterator < hedge_STI >::Handle (container ()).size() << std::endl;
      std::cout << " - Makro vertices .... "  << AccessIterator < vertex_STI >::Handle (container ()).size() << std::endl;
      std::cout << std::endl;
    }
    std::cout << " - Elements ............ "  << LeafIterator < helement_STI > (*this)->size() << std::endl;
    std::cout << " - Boundaries .......... " << LeafIterator < hbndseg_STI > (*this)->size() << std::endl;
    std::cout << " - Faces  .............. " << LeafIterator < hface_STI > (*this)->size() << std::endl;
    std::cout << " - Edges ............... "  << LeafIterator < hedge_STI > (*this)->size() << std::endl;
    std::cout << " - Vertices ............ "  << LeafIterator < vertex_STI > (*this)->size() << std::endl;
    std::cout << std::endl;
  }

  int flagr = -1 ;
#ifdef ENABLE_ALUGRID_VTK_OUTPUT
  int adaptstep = 0;
  int stepnumber = 0;
#endif
  bool Gitter::refine ()
  {
    alugrid_assert (debugOption (20) ? (std::cout << "**INFO GitterDuneBasis::refine ()" << std::endl, 1) : 1);
    bool x = true;
    leaf_element__macro_element__iterator i (container ());
    // refine marked elements
    for( i.first(); ! i.done(); i.next()) x &= i.item ().refine ();
#ifdef ENABLE_ALUGRID_VTK_OUTPUT
    std::ostringstream ss;
    int filenr = adaptstep*100+stepnumber;
    ss << "ref-" << ZeroPadNumber(filenr) << ".vtu";
    tovtk(  ss.str() );
    ++stepnumber;
#endif
    return  x;
  }

  // returns true if no non-conforming element was found
  bool Gitter::markForConformingClosure()
  {
    bool needConformingClosure = false;
    // if bisection refinement was enabled we need to check
    // for the conforming closure
    if( conformingClosureNeeded() )
    {
      leaf_element__macro_element__iterator i ( container () );
      for( i.first(); ! i.done(); i.next())
      {
        // this should only be called for tetra
        // (although default impl for other elements exists and
        //  returns false )
        alugrid_assert ( i.item ().type() == tetra );
        // stores the result if it is true
        needConformingClosure |= i.item ().markForConformingClosure();
      }
    }
    return needConformingClosure;
  }

  bool Gitter::markEdgeCoarsening ()
  {
    if( conformingClosureNeeded() )
    {
      // reset all edge flags
      resetEdgeCoarsenFlags ();

      // now check for each tetra whether it could really be coarsened
      leaf_element__macro_element__iterator i (container ());
      for( i.first(); ! i.done(); i.next() )
      {
        // mark coarsening will unset some edge flags
        i.item().markEdgeCoarsening();
      }
      return true;
    }
    return false;
  }

  void Gitter::resetEdgeCoarsenFlags ()
  {
    // reset all edge flags
    {
      // iterate over all edges in the hierarchy
      is_def_true< hedge_STI > stoprule;
      IteratorSTI < hedge_STI >* edges = createIterator( (hedge_STI *) 0 , stoprule );

      // reset coarsening flag for all edges
      for( edges->first(); ! edges->done(); edges->next() )
      {
        edges->item().resetCoarsenFlag();
      }
      // delete iterator
      delete edges;
    }
  }


  void Gitter::doCoarse()
  {
    alugrid_assert (debugOption (20) ? (std::cout << "**INFO Gitter::coarse ()" << std::endl, 1) : 1);
    {
      AccessIterator < helement_STI >::Handle i (container ());
      for( i.first(); ! i.done(); i.next() )
      {
        i.item ().coarse ();
      }
    }

#ifdef ENABLE_ALUGRID_VTK_OUTPUT
    std::ostringstream ss;
    int filenr = adaptstep*100+stepnumber;
    ss << "crs-" << ZeroPadNumber(filenr) << ".vtu";
    tovtk(  ss.str() );
    ++stepnumber;
#endif

  }

  void Gitter::coarse()
  {
    markEdgeCoarsening();
    doCoarse();
  }

  template<class element_t, class bndseg>
  void Gitter::tovtkImpl( const std::string &fn,
                            const int elementVertices,
                            const element_t*, const bndseg* )
  {
    const bool showbnd = false;
    // only show faces for bisection grids
    const bool showface = conformingClosureNeeded();

    const int nFaceVertices = ( elementVertices == 4 ) ? 3 : 4;

    // openfile
    std::ofstream vtuFile;
    vtuFile.open( fn.c_str() );

    // header info
    vtuFile << "<?xml version=\"1.0\"?>" << std::endl;
    vtuFile << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"" << RestoreInfo::byteOrderString () << "\">" << std::endl;
    vtuFile << "  <UnstructuredGrid>" << std::endl;

    // vertex list
    typedef std::vector< double > Vertex;
    typedef std::map< int, std::pair<int,Vertex> > VertexList;
    VertexList vertexList;

    typedef LeafIterator < Gitter::helement_STI > Iterator;
    Iterator w (*this);
    typedef LeafIterator < Gitter::hbndseg_STI > BndIterator;
    BndIterator wbnd (*this);
    typedef LeafIterator < Gitter::hface_STI > FaceIterator;
    FaceIterator wface (*this);

    const int nCells = w->size();
    const int nBnd   = showbnd ? wbnd->size() : 0;
    const int nFaces = showface ? wface->size() : 0;

    typedef typename element_t::myvertex_t myvertex_t;
    typedef typename element_t::myhface_t  myhface_t;

    // loop to find vertexList and count cells
    {
      for (w->first (); ! w->done (); w->next ())
      {
        element_t* item = ((element_t *) &w->item ());
        // store vertices
        for (int i=0; i < elementVertices; ++i )
        {
          Vertex v(3);
          const myvertex_t* vx = item->myvertex( i );
          alugrid_assert ( vx );
          const alucoord_t (&coord)[ 3 ] = vx->Point();
          // copy coordinates
          for (int k=0;k<3;++k) v[k] = coord[ k ];
          vertexList[ vx->getIndex() ] = make_pair(-1,v);
        }
      }
    }

    vtuFile << "    <Piece NumberOfPoints=\"" << vertexList.size() << "\" "
                  << "NumberOfCells=\"" << nCells+nBnd+nFaces << "\">" << std::endl;

    // cell data
    {
      vtuFile << "      <CellData Scalars=\"cell-id\">" << std::endl;
      vtuFile << "        <DataArray type=\"Float32\" Name=\"cell-id\" NumberOfComponents=\"1\">" << std::endl;
      vtuFile << "          ";

      for (w->first (); ! w->done (); w->next ())
      {
        element_t* item = ((element_t *) &w->item ());
        // vtuFile << item->getIndex() << " ";
        bool ok = true;
        const int nFaces = item->nFaces();
        for (int k=0; k < nFaces; ++k )
          ok &= item->myneighbour( k ).first->isRealObject();

        if (!ok)
        {
          std::cout << "Problem: " << item << std::endl;
          for (int k=0; k<nFaces; ++k)
          {
            if (!item->myneighbour( k ).first->isRealObject())
            {
              std::cout << item->myhface(k) << std::endl;
              if ( item->myhface(k)->nb.front().first->isRealObject() )
              {
                std::cout << item->myhface(k)->nb.front().first << std::endl;
                std::cout << ((bndseg *) item->myhface(k)->nb.front().first)->myhface(0) << std::endl;
              }
              if ( item->myhface(k)->nb.rear().first->isRealObject() )
              {
                std::cout << item->myhface(k)->nb.rear().first << std::endl;
                std::cout << ((bndseg *) item->myhface(k)->nb.rear().first)->myhface(0) << std::endl;
              }
              std::cout << std::endl;
            }
          }
          std::cout << std::endl;
        }

        vtuFile << ((ok)?1:-1) << " ";
      }

      vtuFile << std::endl;
      if (showbnd)
      {
        for (wbnd->first (); ! wbnd->done (); wbnd->next ())
        {
          bndseg* item = ((bndseg *) &wbnd->item ());
          bool ok = true;
          ok &= item->myhface(0)->nb.front().first->isRealObject();
          ok &= item->myhface(0)->nb.rear().first->isRealObject();
          if (!ok)
          {
            if ( item->myhface(0)->nb.front().first->isRealObject() )
              alugrid_assert ( item->myhface(0)->nb.front().first == item );
            if ( item->myhface(0)->nb.rear().first->isRealObject() )
              alugrid_assert ( item->myhface(0)->nb.rear().first == item );
            std::cout << "Problem: " << item << std::endl;
            std::cout << item->myhface(0) << std::endl;
            std::cout << item->myhface(0)->nb.front().first << std::endl;
            std::cout << item->myhface(0)->nb.rear().first << std::endl;
            std::cout << std::endl;
          }
          vtuFile << ((ok)?1:-1)*item->myhface(0)->ref << " ";
        }
      }

      if (showface)
      {
        for (wface->first (); ! wface->done (); wface->next ())
        {
                myhface_t* item = ((myhface_t *) &wface->item ());
          bool ok = true;
          ok &= item->nb.front().first->isRealObject();
          ok &= item->nb.rear().first->isRealObject();
          // alugrid_assert (item->ref>0);
          vtuFile << ((ok)?1:-1)*item->ref << " ";
        }
      }

      vtuFile << std::endl;
      vtuFile << "        </DataArray>" << std::endl;
      vtuFile << "      </CellData>" << std::endl;
    }

    // points info
    {
      vtuFile << "      <Points>" << std::endl;
      vtuFile << "        <DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">" << std::endl;

      const VertexList::iterator end = vertexList.end();
      int index = 0;
      for( VertexList::iterator i = vertexList.begin(); i != end; ++i, ++ index )
      {
        vtuFile << "          " << (*i).second.second[ 0 ] << " " << (*i).second.second[ 1 ] << " " << (*i).second.second[ 2 ] << std::endl;
        (*i).second.first = index;
      }

      vtuFile << "        </DataArray>" << std::endl;
      vtuFile << "      </Points>" << std::endl;
    }

    // cell info
    {
      vtuFile << "      <Cells>" << std::endl;
      // connectivity
      vtuFile << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">" << std::endl;
      vtuFile << "         ";

      for (w->first (); ! w->done (); w->next ())
      {
        element_t* item = ((element_t *) &w->item ());
        for (int i=0; i<elementVertices; ++i)
        {
          vtuFile << " " << vertexList[ item->myvertex(i)->getIndex() ].first;
        }
      }

      if (showbnd)
      {
        for (wbnd->first (); ! wbnd->done (); wbnd->next ())
        {
          bndseg* item = ((bndseg *) &wbnd->item ());
          for (int i=0; i<nFaceVertices; ++i)
          {
            vtuFile << " " << vertexList[ item->myvertex(0,i)->getIndex() ].first;
          }
        }
      }

      if (showface)
      {
        for (wface->first (); ! wface->done (); wface->next ())
        {
          myhface_t* item = ((myhface_t *) &wface->item ());
          for (int i=0; i<nFaceVertices; ++i)
          {
            vtuFile << " " << vertexList[ item->myvertex(i)->getIndex() ].first;
          }
        }
      }
      vtuFile << std::endl;
      vtuFile << "        </DataArray>" << std::endl;

      // offsets
      vtuFile << "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">" << std::endl;
      vtuFile << "         ";

      for( int i = 0; i < nCells; ++i )
      {
        vtuFile << " " << (i+1)* elementVertices;
      }
      for( int i = 0; i < nBnd; ++i )
      {
        vtuFile << " " << nCells* elementVertices + (i+1)* nFaceVertices;
      }
      for( int i = 0; i < nFaces; ++i )
      {
        vtuFile << " " << nCells*elementVertices + nBnd*nFaceVertices + (i+1)*nFaceVertices;
      }
      vtuFile << std::endl;

      vtuFile << "        </DataArray>" << std::endl;

      // cell type
      vtuFile << "        <DataArray type=\"Int32\" Name=\"types\" format=\"ascii\">" << std::endl;
      vtuFile << "         ";

      // 10 for tetrahedra, 12 for hexahedron
      const int elemId = ( elementVertices == 4 ) ? 10 : 12;
      for( int i = 0; i < nCells; ++i )
      {
        vtuFile << " " << elemId;
      }
      // 5 for triangle, 9 for quadrilateral
      const int faceId = ( nFaceVertices == 3 ) ? 5 : 9;
      for( int i = 0; i < nBnd; ++i )
      {
        vtuFile << " " << faceId;
      }
      for( int i = 0; i < nFaces; ++i )
      {
        vtuFile << " " << faceId;
      }
      vtuFile << std::endl;

      vtuFile << "        </DataArray>" << std::endl;
    }
    vtuFile << "      </Cells>" << std::endl;
    vtuFile << "    </Piece>" << std::endl;
    vtuFile << "  </UnstructuredGrid>" << std::endl;
    vtuFile << "</VTKFile>" << std::endl;

    vtuFile.close();
    std::cout << "data written to " << fn << std::endl;
  }

  void Gitter::tovtk ( const std::string &filename )
  {
    typedef LeafIterator< Gitter::helement_STI > Iterator;
    Iterator w (*this);
    w->first();
    if( ! w->done() && w->item().type() == hexa )
    {
      tovtkImpl( filename, 8, (Geometric::hexa_GEO *) 0, (Geometric::hbndseg4_GEO * ) 0 );
    }
    else
    {
      tovtkImpl( filename, 4, (Geometric::tetra_GEO *) 0, (Geometric::tetra_GEO * ) 0 );
    }
  }

  bool Gitter::adapt ()
  {
    alugrid_assert (debugOption (20) ? (std::cout << "**INFO Gitter::adapt ()" << std::endl, 1) : 1);
    alugrid_assert (! iterators_attached ());

    bool needConformingClosure = false;
    bool refined = true;
    do {
      // refine the grid
      refined &= refine ();

      // check for conformity
      needConformingClosure = markForConformingClosure();
    }
    while( needConformingClosure );

    if( !refined )
      std::cerr << "WARNING (ignored): Incomplete refinement (This option should only be used by the parallel refiner)." << std::endl;

    // now call coarsen once
    coarse();

    // make sure that no non-conforming element are present in case of bisection
    alugrid_assert ( !markForConformingClosure() );

#ifdef ENABLE_ALUGRID_VTK_OUTPUT
    ++adaptstep;
#endif

    return refined;
  }

  bool Gitter::adaptWithoutLoadBalancing ()
  {
    return adapt();
  }

  bool Gitter::duneAdapt ( AdaptRestrictProlongType & )
  {
    std::cerr << "ERROR: Method Gitter::duneAdapt not overloaded." << std::endl;
    return adapt();
  }

  template <class stream_t>
  void Gitter::backupHierarchy ( stream_t& out )
  {
    char bisection = char(conformingClosureNeeded());
    // store whether we have bisection refinement
    out.put( bisection );

    char ghosts = char(ghostCellsEnabled());
    // store whether ghost cells are enabled
    out.put( ghosts );

    // backupHierarchy of edges
    {
      AccessIterator <hedge_STI>::Handle fw (container ());
      for (fw.first(); !fw.done(); fw.next()) fw.item ().backup (out);
    }
    // backupHierarchy of faces
    {
      AccessIterator <hface_STI>::Handle fw (container ());
      for (fw.first (); ! fw.done (); fw.next ()) fw.item().backup(out);
    }
    // backupHierarchy of elements
    {
      AccessIterator <helement_STI>::Handle ew (container ());
      for (ew.first (); ! ew.done (); ew.next ()) ew.item ().backup (out);
    }
    // backup periodic elements
    {
      AccessIterator <hperiodic_STI>::Handle ew (container ());
      for (ew.first (); ! ew.done (); ew.next ()) ew.item ().backup (out);
    }
  }

  template <class stream_t>
  void Gitter ::restoreHierarchy ( stream_t& in, const bool restoreBndFaces )
  {
    // store whether we have bisection refinement
    const char bisection = in.get();
    if( bisection ) enableConformingClosure();

    // store whether ghost cells are enabled
    const char ghostCells = in.get();
    if( ! ghostCells )
      disableGhostCells();

    // restoreHierarchy edges
    {
      AccessIterator < hedge_STI >::Handle ew (container ());
      for (ew.first (); !ew.done (); ew.next ()) ew.item ().restore (in);
    }
    // restoreHierarchy faces
    {
      AccessIterator < hface_STI >:: Handle fw(container());
      for ( fw.first(); !fw.done (); fw.next()) fw.item().restore (in);
    }
    // restoreHierarchy elements
    {
      AccessIterator < helement_STI >:: Handle ew(container());
      for ( ew.first(); !ew.done(); ew.next()) ew.item().restore (in);
    }
    // restoreHierarchy periodic elements
    {
      AccessIterator < hperiodic_STI >:: Handle ew(container());
      for ( ew.first(); !ew.done(); ew.next()) ew.item().restore (in);
    }

    // since the faces have been refined before the elements
    // the boundary faces might not habe benn refined at all
    if( restoreBndFaces )
    {
      AccessIterator < hbndseg_STI >::Handle bw (container ());
      for (bw.first (); ! bw.done (); bw.next ()) bw.item ().restoreFollowFace ();
    }
  }

  void Gitter::refineGlobal ()
  {
#ifdef ALUGRIDDEBUG
    if( debugOption( 20 ) )
      std::cout << "INFO: Gitter::refineGlobal()" << std::endl;
#endif // #ifdef ALUGRIDDEBUG
    const int start = clock ();
    {leaf_element__macro_element__iterator w (container ());
      for (w.first (); ! w.done (); w.next ()) w.item (). tagForGlobalRefinement (); }
    adapt ();
    if( debugOption( 2 ) )
      std::cout << "INFO: Gitter::refineGlobal() used " << (double)(clock () - start)/(double)(CLOCKS_PER_SEC) << " s." << std::endl;
  }

  void Gitter::refineRandom (double p)
  {
#ifdef ALUGRIDDEBUG
    if( debugOption( 20 ) )
      std::cout << "INFO: Gitter::refineRandom( p = " << p << " )." << std::endl;
#endif // #ifdef ALUGRIDDEBUG
    const int start = clock ();
    if( (p >= .0) || (p <= 1.) )
    {
      {
        leaf_element__macro_element__iterator w (container ());
        for (w.first (); ! w.done (); w.next ())
          if( drand48 () < p )  w.item ().tagForGlobalRefinement ();
      }
      adapt ();
      if( debugOption( 2 ) )
        std::cout << "*NFO: Gitter::refineRandom( p = " << p << " ) used " << (double)(clock () - start)/(double)(CLOCKS_PER_SEC) << " s." << std::endl;
    }
    else
      std::cerr << "WARNING (ignored): Argument p of Gitter::refineRandom( p = " << p << " ) must be between 0 and 1." << std::endl;
  }

  void Gitter::markForBallRefinement( const alucoord_t (&center)[3], double radius, int limit )
  {
    if( radius >= .0 )
    {
      const int start = clock ();
      {
        leaf_element__macro_element__iterator w (container ());
        for (w.first (); ! w.done (); w.next ())
          w.item (). tagForBallRefinement ( center, radius, limit );
      }
      if( debugOption( 2 ) )
        std::cout << "INFO: Gitter::refineBall() used " << (double)(clock () - start)/(double)(CLOCKS_PER_SEC) << " s." << std::endl;
    }
    else
      std::cerr << "WARNING (ignored) Gitter::refineBall ( center = ?, radius = " << radius << " ) radius must be non-negative." << std::endl;
  }

  void Gitter::notifyMacroGridChanges ()
  {
#ifdef ALUGRIDDEBUG
    if( debugOption( 20 ) )
      std::cout << "INFO: Gitter::notifyMacroGridChanges()." << std::endl;
#endif // #ifdef ALUGRIDDEBUG
  }

  Gitter::~Gitter ()
  {
    if( ref )
      std::cerr << "WARNING (ignored): Grid-Reference counter [" << ref << "] non-zero a point of removal." << std::endl;
  }

  int Gitter::Makrogitter::iterators_attached () const
  {
    return AccessIterator < vertex_STI >::ref + AccessIterator < hedge_STI >::ref +
     AccessIterator < hface_STI >::ref + AccessIterator < helement_STI >::ref +
     AccessIterator < hbndseg_STI >::ref;
  }

  Gitter::Makrogitter::~Makrogitter ()
  {
    if( iterators_attached() )
      std::cerr << "WARNING: (ignored) There are still iterators attached to the grid, remove them before removal of the grid to avoid errors." << std::endl;
  }

  template void Gitter::backupHierarchy( std::ostream& );
  template void Gitter::backupHierarchy( ObjectStream& );

  template void Gitter::restoreHierarchy( std::istream&, const bool);
  template void Gitter::restoreHierarchy( ObjectStream&, const bool);

} // namespace ALUGrid
