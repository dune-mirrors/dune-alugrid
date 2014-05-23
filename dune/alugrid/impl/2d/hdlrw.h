#include <config.h>

#include <dune/alugrid/common/alugrid_assert.hh>
#include <fstream>
#include <iostream>
#include <vector>

#include "grid.h"
#include "triang.h"
#include "handle.h"
#include "vmmap.h"

namespace ALU2DGrid
{

  // typdef these stream because this code uses a lot strstream
  typedef std::basic_stringbuf<char> strstreambuf_t;

  template< int N, int NV >
  bool Hmesh< N, NV >::asciireadtriang ( std::istream &in, double &time, unsigned long int &nbr )
  {
    bool isbackup = false; // Wiederaufsetzen?
    int c = in.peek();
    alugrid_assert (in.good());
    // Beginnt die erste Zeile mit einem Kommentarzeichen?
    if( c == int('!') )
    {
      in.get(); // Kommentarzeichen entfernen

      // Erste Zeile in einen strstreambuf lesen und auf 'Backup' untersuchen.
      strstreambuf_t buf;
      in.get(buf);
      std::istream is(&buf);
      std::string str;
      is >> str;
      if( str == std::string( "Backup" ) )
      {
        isbackup = true;
        is >> time >> nbr;
        int rrule;
        is >> _nconfDeg >> rrule;
        refinement_rule = (Refco::tag_t)rrule;
        if( _nconfDeg < 0 )
        {
          std::cerr << "Error in Hmesh :: asciireadtriang: "
                    << "Negative degree of nonconformity encountered."
                    << std::endl;
          abort();
        }
        if( (_nconfDeg > 0) && (refinement_rule != Refco::quart) )
        {
          std::cerr << "Error in Hmesh :: asciireadtriang: "
                    << "Nonconform grids must use quartering as refinement rule."
                    << std::endl;
          abort();
        }
      } 
      else if( str != std::string( "Triangles" ) )
      {
        std::cerr << "Error in Hmesh :: asciireadtriang: "
                  << "Wrong macrogrid format: " << std::endl;
        std::cerr << "file with !-line but command not recognized" << std::endl;
        abort();
      }
    }
    hmesh_basic_t::asciireadtriang(in, isbackup);
    return isbackup;
  }
   
  template< int N, int NV >
  void Hmesh_basic< N, NV >::asciireadtriang ( std::istream &in, const bool verbose )
  {
    // read vertices
    int nv = 0;
    in >> nv;
      
#ifdef ALUGRIDDEBUG
    if( verbose ) 
      std::cerr << "    Number of Vertices:             " << nv << std::endl;
#endif

    std::vector< vertex_t * > v( nv );
    for(int i = 0; i < nv; ++i)
    {
      vertex_t *n = new fullvertex_t();
      n->read(in);
      v[i] = n;
      vl.insert(n);
    }

    // read elements
    int ne = 0;
    in >> ne;

#ifdef ALUGRIDDEBUG
    if( verbose ) 
      std::cerr << "    Number of MacroElements:        " << ne << std::endl;
#endif

    for(int i = 0; i < ne; ++i)
    {
      triang_t *tr = new triang_t();
      tr->read(in, &(v[0]), nv);
      mel.insert(new macroelement_t(*tr));
    }

    // read boundaries
    {
      std::string line;
      // read line and skip empty lines 
      while( in && line.empty() )
      {
        std::getline(in, line);
        line.erase(0, line.find_first_not_of( ' ' ));
      }
      std::istringstream linein( line );

      // read number of boundary segments
      int nb = 0;
      linein >> nb;

#ifdef ALUGRIDDEBUG 
      if( verbose )
        std::cerr << "    Number of BoundarySegments:     " << nb << std::endl;
#endif

      // some variables for periodic boundary treatment
      int npb = 0;

      std::vector< std::pair< bndel_triang_t *, int > > bnd_list( nb );
      int perbnd_invalid = 0;

      struct axis_struct { double p0, p1; bndel_t *b; };
      axis_struct *x_axis = new axis_struct[nb];
      axis_struct *y_axis = new axis_struct[nb];
      int y_card = 0, x_card = 0, x_ok = 0, y_ok = 0;

      for( int i = 0; i < nb; ++i ) 
      {
        getline( in, line );
        std::istringstream linein( line );

        // peek boundary element type
        int lt;
        linein >> lt;
        linein.seekg( 0 );
        typename bndel_t::bnd_t t = (typename bndel_t::bnd_t)lt;

        // create boundary element, depending on its type
        switch( t )
        {
        case bndel_t::periodic:
          if( ncoord != 2 )
          {
            std::cerr << "Error in Hmesh :: asciireadtriang: "
                      << "Boundary type " << bndel_t::periodic
                      << " is only supported for flat grids." << std::endl;
            abort();
          }
          bnd_list[i].first = new bndel_periodic_t(i);
          ++npb;
          break;

       case bndel_t::general_periodic:
          bnd_list[i].first = new bndel_periodic_t(i);
          ++npb;
          break;

        default:
          bnd_list[i].first = new bndel_triang_t(i, t);
          break;
        }
        alugrid_assert ( bnd_list[i].first );
        bnd_list[i].second = -1;

        bnd_list[i].first->read(linein, &(v[0]), nv);
        mbl.insert(new macrobndel_t(*bnd_list[i].first));

        if( t == bndel_t::general_periodic )
        {
          linein >> bnd_list[i].second;
          if( bnd_list[i].second < 0 )
          {
            std::cerr << "Error in Hmesh :: asciireadtriang: "
                      << "Periodic neighbor boundary has negative index."
                      << std::endl;
            abort();
          }

          if( bnd_list[i].second < i )
          {
            const int j = bnd_list[i].second;
            if( bnd_list[j].second < 0 )
            {
              std::cerr << "Error in Hmesh :: asciireadtriang: "
                        << "Neighbor of periodic boundary is non-periodic. "
                        << "(" << i << " -> " << j << ")"
                        << std::endl;
              abort();
            }
            if( bnd_list[j].second != i )
            {
              std::cerr << "Error in Hmesh :: asciireadtriang: "
                        << "Periodic boundaries not linked symmetrically "
                        << "(" << i << " -> " << j << ", but "
                        << j << " -> " << bnd_list[j].second << ")."
                        << std::endl;
              abort();
            }
            ((bndel_periodic_t *)bnd_list[i].first)->set_pnb(bnd_list[j].first);
            ((bndel_periodic_t *)bnd_list[j].first)->set_pnb(bnd_list[i].first);
            --perbnd_invalid;
          }
          else
            ++perbnd_invalid;
        }
        else if( t == bndel_t::periodic )
        {
          bndel_t *b = bnd_list[i].first;
          if (fabs(b->vertex(0)->coord()[0]-b->vertex(1)->coord()[0])<EPS)
          {
            double y0,y1;
            if (b->vertex(0)->coord()[1]<b->vertex(1)->coord()[1])
        { y0=b->vertex(0)->coord()[1];y1=b->vertex(1)->coord()[1]; }
            else
        { y0=b->vertex(1)->coord()[1];y1=b->vertex(0)->coord()[1]; }
            int y;
            for (y=0;y<y_card;y++)
              if (fabs(y_axis[y].p0-y0)+fabs(y_axis[y].p1-y1)<EPS) {
                ((bndel_periodic_t *)b)->set_pnb(y_axis[y].b);
                ((bndel_periodic_t *)y_axis[y].b)->set_pnb(b);
                y_ok++;
                break;
              }
            if (y==y_card) {
              y_axis[y_card].p0=y0;
              y_axis[y_card].p1=y1;
              y_axis[y_card].b=b;
              y_card++;
            }
          }
          else 
          {
            alugrid_assert (fabs(b->vertex(0)->coord()[1]-b->vertex(1)->coord()[1])<EPS);

            double x0,x1;
            if (b->vertex(0)->coord()[0]<b->vertex(1)->coord()[0])
        { x0=b->vertex(0)->coord()[0];x1=b->vertex(1)->coord()[0]; }
            else
        { x0=b->vertex(1)->coord()[0];x1=b->vertex(0)->coord()[0]; }
            int x;
            for (x=0;x<x_card;x++)
              if (fabs(x_axis[x].p0-x0)+fabs(x_axis[x].p1-x1)<EPS) {
                ((bndel_periodic_t *)b)->set_pnb(x_axis[x].b);
                ((bndel_periodic_t *)x_axis[x].b)->set_pnb(b);
                x_ok++;
                break;
              }
            if (x==x_card) {
              x_axis[x_card].p0=x0;
              x_axis[x_card].p1=x1;
              x_axis[x_card].b=b;
              x_card++;
            }
          }
        }
      }

      if( perbnd_invalid != 0 )
      {
        std::cerr << "Error in Hmesh :: asciireadtriang: "
                  << "Periodic boundaries don't match."
                  << std::endl;
        abort();
      }

      delete[]( y_axis );
      delete[]( x_axis );

      alugrid_assert (y_ok == y_card);
      alugrid_assert (x_ok == x_card);

#ifdef ALUGRIDDEBUG
      if( verbose )
        std::cerr << "    Number of periodic boundaries:  " << npb << std::endl;
#endif
    
    }       

#ifdef ALUGRIDDEBUG
    if( verbose )
      std::cerr << "\n  -------------------------- closed.\n" << std::endl;
#endif

    vl.renumber();

    makeneighbours();

    {
      Listwalk_impl < macroelement_t > walk(mel);
      for (walk.first(); !walk.done(); walk.next() )
      {
        triang_t &tr=( (triang_t &)(*walk.getitem()) );
        for (int l=0;l<tr.numfaces();l++) {
          alugrid_assert ( tr.neighbour(l) );
          if (!tr.normaldir(l)) 
          {
            tr.setnormdir(l,1);
            if (tr.neighbour(l)->thinis(thinelement_t::element_like))
              tr.nbel(l)->setnormdir(tr.opposite(l),-1); 
          }
          if (tr.neighbour(l)->edge(tr.opposite(l))) {
            tr.edgeconnect(l,tr.neighbour(l)->edge(tr.opposite(l)));
          } else {
            Edge *e=new Edge(this);
            tr.edgeconnect(l,e);
          }
        }
      }
    }
    setorientation();
  }

  template <int N,int NV>
  void Hmesh_basic<N,NV> :: setorientation() 
  {
    Listwalk_impl < macroelement_t > walkel(mel);
    if (N == 2)
    {
      for (walkel.first(); !walkel.done(); walkel.next() )
      {
        walkel.getitem()->setorientation();
        if (walkel.getitem()->numvertices() == 3)
          walkel.getitem()->setrefine();
      }
    }
    else
    {
      std::vector< OrientStr > orientStack;
      std::vector< bool > visited(walkel.size(),false);
      for (walkel.first(); !walkel.done(); walkel.next() )
      {
        if ( visited[ walkel.getitem()->getIndex() ] ) continue;
        walkel.getitem()->setorientation(orientStack);
        visited[ walkel.getitem()->getIndex() ] = true;
        while ( !(orientStack.empty()) )
        {
          OrientStr &str = orientStack.back();
          if ( str.nextNb < str.el->numfaces() )
          {
            element_t* nb = str.el->nbel(str.nextNb);
            ++(str.nextNb);
            if ( nb )
              if ( !visited[ nb->getIndex() ] )
              {
                nb->setorientation(orientStack);
                visited[ nb->getIndex() ] = true;
              }
          }
          else
            orientStack.pop_back();
        }
      }
    }
    Listwalk_impl < macrobndel_t > walkbnd(mbl);
    for (walkbnd.first(); !walkbnd.done(); walkbnd.next() ) {
      walkbnd.getitem()->setorientation();
      walkbnd.getitem()->edgeconnect(0,walkbnd.getitem()->neighbour(0)->edge(walkbnd.getitem()->opposite(0)));
    }
  }

  template <int N,int NV>
  void
  Hmesh<N,NV> :: asciiwritetriang(const std::string &filename,
                                  double time, unsigned long int nbr)
  {
#ifdef ALUGRIDDEBUG
    std::cerr << "\n  Hmesh_basic::asciiwritetriang(?) opens: ";
    std::cerr << filename << "\n" << std::endl;
#endif

    // create stream 
    std::ofstream out( filename.c_str(), std::ios::out | std::ios::trunc );

    // call write triang with stream 
    hmesh_basic_t::asciiwritetriang(out, time, nbr, _nconfDeg, refinement_rule);
  }
   
  template <int N,int NV>
  void
  Hmesh_basic<N,NV> :: asciiwritetriang(std::ostream &out,
                                        double time, unsigned long int nbr,
                                        int nconfDeg, Refco::tag_t refinement_rule)
  {
    vl.renumber();

    out.setf( std::ios::fixed, std::ios::floatfield );
    
    out << std::scientific;
    out.precision(16);

    out << "!Backup ";
    out << time << " " << nbr << " ";
    out << nconfDeg << " " << refinement_rule << std::endl;

    {
   
      Listwalk_impl < vertex_t > walk(vl);
    
#ifdef ALUGRIDDEBUG
      std::cerr << "    Number of Vertices:       " << walk.size() << std::endl;
#endif
      
      int nr = 0;
      
      for( walk.first(); ! walk.done(); walk.next() ) {
      
        vertex_t & v = walk.getitem();

        if (v.isMacro()) ++nr;
                
      }

      out << nr << std::endl;

      for( walk.first(); ! walk.done(); walk.next() ) {
      
        vertex_t & v = walk.getitem();

        if (v.isMacro())
          v.write(out);
      
      }
      
    }
    
    {

      Listwalk_impl < macroelement_t > walk(mel);

      int count = 0;

      const int numMacroElements = walk.size();

#ifdef ALUGRIDDEBUG
      std::cerr << "    Number of macro Elements:  " << numMacroElements << std::endl;
#endif
      
      out << numMacroElements << std::endl;
      
      for( walk.first(); ! walk.done(); walk.next() ) {
      
        walk.getitem()->write(out);

        count += walk.getitem()->count();

      }

#ifdef ALUGRIDDEBUG
      std::cerr << "    Number of Elements:       " << count << std::endl;
#endif
    }
    
    {
      Listwalk_impl < macrobndel_t > walk(mbl);
      const int numMacroBoundaryElements = walk.size();

#ifdef ALUGRIDDEBUG
      std::cerr << "    Number of macro boundary Elements:   " << numMacroBoundaryElements << std::endl;
#endif

      out << numMacroBoundaryElements << std::endl;

      int index = 0, count = 0;
      for( walk.first(); ! walk.done(); walk.next() )
      {
        // make sure we can use the segment index to write out periodic neighbors
        if( index != walk.getitem()->segmentIndex() )
        {
          std::cerr << "Error: Index in macro boundary element list does not coincide with segment index." << std::endl;
          abort();
        }

        walk.getitem()->write(out);

        count += walk.getitem()->count();
        ++index;
      }

#ifdef ALUGRIDDEBUG
      std::cerr << "    Number of boundary Elements:       " << count << std::endl;
#endif
    }
    
#ifdef ALUGRIDDEBUG
    std::cerr << "\n  -------------------------- closed.\n" << std::endl;
#endif
   
  }

  template< int N, int NV >
  void Hmesh< N, NV >::storeGrid ( const std::string &filename, double time, unsigned long int nbr )
  {
    // create outstream 
    std::ofstream out ( filename.c_str(), std::ios::out | std::ios::trunc );

    if( !out ) 
    {
      std::cerr << "ERROR: could not open file " << filename << std::endl << std::endl;
      return;
    }

    // call stream version of store grid 
    storeGrid( out, time, nbr );
  }

  template <int N,int NV>
  void Hmesh<N,NV>::
  storeGrid(std::ostream &out, double time, unsigned long int nbr)
  {
    // write macro triangulation 
    hmesh_basic_t::asciiwritetriang(out, time, nbr, _nconfDeg, refinement_rule);

    // Status des Gitters sichern
    for( int level = 0;; level++ ) 
    {
      Levelwalk < element_t > walk(mel, level);
      if( ! walk.size() ) 
      {
        break;
      } 
      else 
      {
        for( walk.first(); !walk.done(); walk.next() )
          out.put(walk.getitem().splitrule());
      }
    }

    // write indices 
    storeIndicies(out);
  }

  template <int N,int NV>
  void
  Hmesh<N,NV>::storeIndicies(std::ostream &out) 
  {
    // backup index managers 
    for (int i=0;i<numOfIndexManager2d; ++i) 
    {
      indexmanager[i].backupIndexSet(out);
    }

    // backup vertex indices 
    {
      Listwalk_impl < vertex_t > walk(vl);
      for( walk.first(); ! walk.done(); walk.next() ) 
      {
        int idx=walk.getitem().getIndex();
        out.write( ((const char *) &idx ), sizeof(int) );
      }
    }
    
    // backup element and edge indices 
    {
      Levelwalk < element_t > walk(mel, 0);
      for( walk.first(); !walk.done(); walk.next() ) 
      {
        SubtreeIterator < element_t > hier(&(walk.getitem()));
        for (hier.first(); !hier.done(); hier.next() ) 
        {
          // element 
          {
                  int idx=hier.getitem().getIndex();
                  out.write( ((const char *) &idx ), sizeof(int) );
          }

          // edges 
                for (int e=0;e<hier.getitem().numfaces(); ++e) 
          {
                  int idx=hier.getitem().edge(e)->getIndex();
                  out.write( ((const char *) &idx ), sizeof(int) );
                }
        }
      }
    }
  }

  template <int N,int NV>
  bool
  Hmesh<N,NV>::recoverGrid(std::istream &in)
  {
    int compwarn = 0;

    // Gitter wiederherstellen
    for( int level = 0;; level++ ) 
    {
      {
        Levelwalk < element_t > walk(mel, level);
        if( !walk.size() )
          break;
        for( walk.first(); !walk.done(); walk.next() ) 
        {
          char flag;
          in.get(flag);
          switch (flag) 
          {
            case thinelement_t::unsplit:
              break;
            case thinelement_t::triang_bnd:
              std::cerr << "ERROR (Hmesh::recoverGrid()): "
                        << "splitrule \"triang_bnd\" is not allowed for elements!"
                        << std::endl;
              abort();
              break;
            case thinelement_t::triang_conf2:
              walk.getitem().mark(Refco::ref_1);
              break;
            case thinelement_t::triang_quarter:          
              walk.getitem().mark(Refco::quart);
              break;
            case thinelement_t::compatibility:
              if (!compwarn)
              {
                std::cerr << "WARNING (Hmesh::recoverGrid()): "
                          << "using compatibility mode for obsolete file format!"
                          << std::endl;
                compwarn = 1;
              }
              walk.getitem().mark(Refco::ref_1);
              break;
            default:
              std::cerr << "ERROR (Hmesh::recoverGrid()): "
                        << "unknown splitrule!"
                        << std::endl;
              abort();
           }
          }
        }
      refine();
    }

    // read indices 
    recoverIndicies(in);
   
    return true;
  }

  template <int N,int NV>
  void
  Hmesh<N,NV>::recoverIndicies(std::istream &in) 
  {
    // use the systems byte order (otherwise store byte order in storeIndices)
    RestoreInfo restoreInfo ( RestoreInfo :: systemByteOrder () );

    // reads maxIndex of Index Manager 
    for (int i=0;i<numOfIndexManager2d; ++i) 
    {
      indexmanager[i].restoreIndexSet(in, restoreInfo);
    }
    
    //////////////////////////////////////////
    //  read vertices 
    //////////////////////////////////////////
    {
      IndexManager2dType& vertexManager = indexmanager[IndexProvider::IM_Vertices];
      const int idxSize = vertexManager.getMaxIndex();

      // create vector, all entries are marked true 
      std::vector< bool > isHole( idxSize, true );

      Listwalk_impl < vertex_t > walk(vl);
      for( walk.first(); ! walk.done(); walk.next() ) 
      {
        vertex_t& vx = walk.getitem();
        in.read ( ((char *) &(vx.setIndex())), sizeof(int) );
        alugrid_assert ( vx.getIndex() < idxSize );
        isHole[vx.getIndex()] = false;  
      }

      // all remaining indices are reinserted as holes 
      vertexManager.generateHoles( isHole );
    }
    
    //////////////////////////////////////////
    //  read elements and edges 
    //////////////////////////////////////////
    {
      IndexManager2dType& elementManager = indexmanager[IndexProvider::IM_Elements];
      const int elSize = elementManager.getMaxIndex();
      
      IndexManager2dType& edgeManager = indexmanager[IndexProvider::IM_Edges];
      const int edgeSize = edgeManager.getMaxIndex();
      // create vector, all entries are marked true
      std::vector< bool > elementIsHole ( elSize, true );
      std::vector< bool > edgeIsHole ( edgeSize, true );
      
      Levelwalk < element_t > walk(mel, 0);
      for( walk.first(); !walk.done(); walk.next() ) 
      {
        SubtreeIterator < element_t > hier(&(walk.getitem()));
        for (hier.first(); !hier.done(); hier.next() ) 
        {
          element_t &elem = hier.getitem();

          // read element index
          int &index = elem.setIndex();
          in.read ( ((char *)&index), sizeof(int) );
          alugrid_assert ( elem.getIndex() < elSize );
          elementIsHole[elem.getIndex()] = false;
          
          // read edges 
          for (int e=0; e<elem.numfaces(); ++e) 
          {
            int edgeNum = -1; 
                  in.read ( ((char *) &(edgeNum)), sizeof(int) );
            alugrid_assert ( edgeNum < edgeSize );
            edgeIsHole[edgeNum] = false;
            // set edge index 
            elem.edge(e)->setIndex() = edgeNum;
                }
        }
      } 

      // reinsert remaining indices as holes 
      elementManager.generateHoles( elementIsHole );
      edgeManager.generateHoles( edgeIsHole );
    }
  }

} // namespace ALU2DGrid
