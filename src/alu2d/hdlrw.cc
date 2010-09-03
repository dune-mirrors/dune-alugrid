// typdef these stream because this code uses a lot strstream
typedef basic_stringbuf<char>  strstreambuf_t ;

#include <vector>

#include "grid.h"
#include "triang.h"
#include "handle.h"

template <int N,int NV>
bool
Hmesh<N,NV> :: ascireadtriang(istream &in, double &time, unsigned long int &nbr)
{
  bool isbackup = false; // Wiederaufsetzen?
  int c = in.peek();
  assert(in.good());
  // Beginnt die erste Zeile mit einem Kommentarzeichen?
  if( c == int('!') )
  {
    in.get(); // Kommentarzeichen entfernen

    // Erste Zeile in einen strstreambuf lesen und auf 'Backup' untersuchen.
    strstreambuf_t buf;
    in.get(buf);
    istream is(& buf);
    std::string str;
    is >> str;
    if( str == std::string( "Backup" ) )
    {
      isbackup = true;
      cerr << "Backup-file found" << endl;
      is >> time >> nbr;
      int rrule;
      is >> _nconfDeg >> rrule;
      refinement_rule = (Refco::tag_t)rrule;
      if( _nconfDeg < 0 )
      {
        cerr << "Error in Hmesh :: ascireadtriang: "
             << "Negative degree of nonconformity encountered." << endl;
        abort();
      }
      if( (_nconfDeg > 0) && (refinement_rule != Refco::quart) )
      {
        cerr << "Error in Hmesh :: asciireadtriang: "
             << "Nonconform grids must use quartering as refinement rule." << endl;
        abort();
      }
    } 
    else if( str != std::string( "Triangles" ) )
    {
      cerr << "Error in Hmesh :: ascireadtriang: "
           << "Wrong macrogrid format: " << endl;
      cerr << "file with !-line but command not recognized" << endl;
      abort();
    }
  }
  hmesh_basic_t::ascireadtriang(in, isbackup);
  return isbackup;
}
 
template <int N,int NV>
void Hmesh_basic<N,NV> :: ascireadtriang(istream &in, const bool verbose) 
{
  // read vertices
  int nv = 0;
  in >> nv;
    
#ifndef NDEBUG
  if( verbose ) 
    cerr << "    Number of Vertices:             " << nv << endl;
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

#ifndef NDEBUG
  if( verbose ) 
    cerr << "    Number of MacroElements:        " << ne << endl ;
#endif

  for(int i = 0; i < ne; ++i)
  {
    triang_t *tr = new triang_t();
    tr->read(in, &(v[0]), nv);
    mel.insert(new macroelement_t(*tr));
  }

  // read boundaries
  {
    string line;
    // read line and skip empty lines 
    while( in && line.empty() )
    {
      getline(in, line);
      line.erase(0, line.find_first_not_of( ' ' ));
    }
    istringstream linein( line );

    // read number of boundary segments
    int nb = 0;
    linein >> nb;

#ifndef NDEBUG 
    if( verbose )
      cerr << "    Number of BoundarySegments:     " << nb << endl;
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
      istringstream linein( line );

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
          cerr << "Error in Hmesh :: ascireadtriang: "
               << "Boundary type " << bndel_t::periodic
               << " is only supported for flat grids." << endl;
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
      assert( bnd_list[i].first );
      bnd_list[i].second = -1;

      bnd_list[i].first->read(linein, &(v[0]), nv);
      mbl.insert(new macrobndel_t(*bnd_list[i].first));

      if( t == bndel_t::general_periodic )
      {
        linein >> bnd_list[i].second;
        if( bnd_list[i].second < 0 )
        {
          cerr << "Error in Hmesh :: ascireadtriang: "
               << "Periodic neighbor boundary has negative index." << endl;
          abort();
        }

        if( bnd_list[i].second < i )
        {
          const int j = bnd_list[i].second;
          if( bnd_list[j].second < 0 )
          {
            cerr << "Error in Hmesh :: ascireadtriang: "
                 << "Neighbor of periodic boundary is non-periodic. "
                 << "(" << i << " -> " << j << ")" << endl;
            abort();
          }
          if( bnd_list[j].second != i )
          {
            cerr << "Error in Hmesh :: ascireadtriang: "
                 << "Periodic boundaries not linked symmetrically "
                 << "(" << i << " -> " << j << ", but "
                 << j << " -> " << bnd_list[j].second << ")." << endl;
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
          assert(fabs(b->vertex(0)->coord()[1]-b->vertex(1)->coord()[1])<EPS);

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
      cerr << "Error in Hmesh :: ascireadtriang: "
           << "Periodic boundaries don't match." << endl;
      abort();
    }

    delete[]( y_axis );
    delete[]( x_axis );

    assert(y_ok == y_card);
    assert(x_ok == x_card);

#ifndef NDEBUG
    if( verbose )
      cerr << "    Number of periodic boundaries:  " << npb << endl ;
#endif
  
  }       

#ifndef NDEBUG
  if( verbose )
    cerr << "\n  -------------------------- closed.\n" <<endl ;
#endif

  vl.renumber() ;

  makeneighbours() ;

  {
    Listwalk_impl < macroelement_t > walk(mel) ;
    for (walk.first() ; !walk.done() ; walk.next() )
    {
      triang_t &tr=( (triang_t &)(*walk.getitem()) );
      for (int l=0;l<tr.numfaces();l++) {
        assert( tr.neighbour(l) );
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
  Listwalk_impl < macroelement_t > walkel(mel) ;
  if (N == 2)
  {
    for (walkel.first() ; !walkel.done() ; walkel.next() )
    {
      walkel.getitem()->setorientation();
      if (walkel.getitem()->numvertices() == 3)
        walkel.getitem()->setrefine();
    }
  }
  else
  {
    vector<OrientStr> orientStack;
    vector<bool> visited(walkel.size(),false);
    for (walkel.first() ; !walkel.done() ; walkel.next() )
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
  Listwalk_impl < macrobndel_t > walkbnd(mbl) ;
  for (walkbnd.first() ; !walkbnd.done() ; walkbnd.next() ) {
    walkbnd.getitem()->setorientation();
    walkbnd.getitem()->edgeconnect(0,walkbnd.getitem()->neighbour(0)->edge(walkbnd.getitem()->opposite(0)));
  }
}

template <int N,int NV>
void
Hmesh<N,NV>::asciwritetriang(const std::string &filename,
                             double time, unsigned long int nbr)
{
#ifndef NDEBUG
  cerr << "\n  Hmesh_basic::asciwritetriang(?) opens: " ;
  cerr << filename << "\n" << endl ;
#endif

  vl.renumber() ;
  
  ofstream out(filename.c_str(), ios::out|ios::trunc) ;
  
  out.setf(ios::fixed, ios::floatfield) ;
  
  out << scientific ;
  out.precision(16) ;

  out << "!Backup ";
  out << time << " " << nbr << " ";
  out << _nconfDeg << " " << refinement_rule << endl;
  hmesh_basic_t::asciwritetriang(out);
}
 
template <int N,int NV>
void
Hmesh_basic<N,NV>::asciwritetriang(ostream &out)
{
  {
 
    Listwalk_impl < vertex_t > walk(vl) ;
  
#ifndef NDEBUG
    cerr << "    Number of Vertices:       " << walk.size() << endl ;
#endif
    
    int nr = 0;
    
    for( walk.first() ; ! walk.done() ; walk.next() ) {
    
      vertex_t & v = walk.getitem() ;

      if (v.level()==-1) ++nr;
              
    }

    out << nr << endl;

    for( walk.first() ; ! walk.done() ; walk.next() ) {
    
      vertex_t & v = walk.getitem() ;

      if (v.level()==-1) 
        v.write(out) ;
    
    }
    
  }
  
  {

    Listwalk_impl < macroelement_t > walk(mel) ;

    int count = 0 ;

    const int numMacroElements = walk.size();

#ifndef NDEBUG
    cerr << "    Number of macro Elements:  " << numMacroElements << endl ;
#endif
    
    out << numMacroElements << endl;
    
    for( walk.first() ; ! walk.done() ; walk.next() ) {
    
      walk.getitem()->write(out) ;

      count += walk.getitem()->count() ;

    }

#ifndef NDEBUG
    cerr << "    Number of Elements:       " << count << endl ;
#endif

  }
  
  {
    Listwalk_impl < macrobndel_t > walk(mbl) ;
    const int numMacroBoundaryElements = walk.size();

#ifndef NDEBUG
    cerr << "    Number of macro boundary Elements:   " << numMacroBoundaryElements << endl ;
#endif

    out << numMacroBoundaryElements << endl;

    int index = 0, count = 0;
    for( walk.first() ; ! walk.done() ; walk.next() )
    {
      // make sure we can use the segment index to write out periodic neighbors
      if( index != walk.getitem()->segmentIndex() )
      {
        cerr << "Error: Index in macro boundary element list does not coincide with segment index." << endl;
        abort();
      }

      walk.getitem()->write(out) ;

      count += walk.getitem()->count() ;
      ++index;
    }

#ifndef NDEBUG
    cerr << "    Number of boundary Elements:       " << count << endl ;
#endif
  }
  
#ifndef NDEBUG
  cerr << "\n  -------------------------- closed.\n" <<endl ;
#endif
 
}

template <int N,int NV>
void
Hmesh<N,NV>::storeGrid(const std::string &fbase,
                       double time, unsigned long int nbr)
{
  asciwritetriang(fbase,time,nbr);

  const std::string refineFile = fbase + ".refine";
#ifndef NDEBUG
  cerr << "Hmesh::writeRecoverFile(): writing file \""
       << refineFile << "\" ...";
#endif

  ofstream out(refineFile.c_str(), ios::out|ios::trunc);
  assert(out);

  // out.setf(ios::scientific, ios::floatfield);
  // out.precision(16);

  // Zeit und Nummer des Zeitschritts schreiben
  /*
  out << time << " ";
  out << nbr << endl;
  */
  // Status des Gitters sichern
  for( int level = 0 ;; level++ ) {
    Levelwalk < element_t > walk(mel, level);
    if( !walk.size() ) 
    {
      break;
    } 
    else 
    {
      for( walk.first() ; !walk.done() ; walk.next() )
      	out.put(walk.getitem().splitrule());
      // out << walk.getitem().splitrule() << " ";
    }
    // out << endl;
  }
  // out << endl;

  /*
  // Daten sichern
  Leafwalk<Element> walk(mel);
  for( walk.first() ; !walk.done() ; walk.next() ) {
    out << walk.getitem();
  }
  */
  storeIndicies(out);

#ifndef NDEBUG
  cout << " done." << endl;
#endif
}
template <int N,int NV>
void
Hmesh<N,NV>::storeIndicies(ostream& out) 
{
  // backup index managers 
  for (int i=0;i<numOfIndexManager2d; ++i) 
  {
    indexmanager[i].backupIndexSet(out);
  }

  // backup vertex indices 
  {
    Listwalk_impl < vertex_t > walk(vl) ;
    for( walk.first() ; ! walk.done() ; walk.next() ) 
    {
      int idx=walk.getitem().getIndex();
      out.write( ((const char *) &idx ), sizeof(int) ) ;
    }
  }
  
  // backup element and edge indices 
  {
    Levelwalk < element_t > walk(mel, 0);
    for( walk.first() ; !walk.done() ; walk.next() ) 
    {
      SubtreeIterator < element_t > hier(&(walk.getitem()));
      for (hier.first() ; !hier.done() ; hier.next() ) 
      {
        // element 
        {
	        int idx=hier.getitem().getIndex();
	        out.write( ((const char *) &idx ), sizeof(int) ) ;
        }

        // edges 
	      for (int e=0;e<hier.getitem().numfaces(); ++e) 
        {
	        int idx=hier.getitem().edge(e)->getIndex();
	        out.write( ((const char *) &idx ), sizeof(int) ) ;
	      }
      }
    }
  }
}

template <int N,int NV>
bool
Hmesh<N,NV>::recoverGrid(const std::string &recoverFile,
                         double& time, unsigned long int &nbr)
{
  int compwarn = 0;

  cout << "Hmesh::recoverGrid(): trying to read file \""
       << recoverFile << "\" ...";

  const std::string refineFile = recoverFile + ".refine";
  ifstream in(refineFile.c_str());
  if( !in )
  {
    cout << " FAILED." << endl;
    return false;
  }

  // Zeit und Nummer des Zeitschritts lesen
  // in >> time;
  // in >> nbr;
  // Gitter wiederherstellen
  for( int level = 0 ;; level++ ) 
  {
    {
      Levelwalk < element_t > walk(mel, level);
      if( !walk.size() )
        break;
      for( walk.first() ; !walk.done() ; walk.next() ) 
      {
        char flag;
        in.get(flag);
        switch (flag) 
        {
          case thinelement_t::unsplit:
            break;
          case thinelement_t::triang_bnd:
            cerr << "ERROR (Hmesh::recoverGrid()): "
                 << "splitrule \"triang_bnd\" is not allowed for elements!"
                 << endl;
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
                    cerr << "WARNING (Hmesh::recoverGrid()): "
             << "using compatibility mode for obsolete file format!"
             << endl;
                    compwarn = 1;
            }
            walk.getitem().mark(Refco::ref_1);
            break;
          default:
            cerr << "ERROR (Hmesh::recoverGrid()): "
                 << "unknown splitrule!"
                 << endl;
            abort();
         }
        }
      }
    refine();
  }

  // read indices 
  recoverIndicies(in);
 
  cout << " done." << endl;
  return true;
}

template <int N,int NV>
void
Hmesh<N,NV>::recoverIndicies(istream& in) 
{
  // reads maxIndex of Index Manager 
  for (int i=0;i<numOfIndexManager2d; ++i) 
  {
    indexmanager[i].restoreIndexSet(in);
  }
  
  //////////////////////////////////////////
  //  read vertices 
  //////////////////////////////////////////
  {
    IndexManager2dType& vertexManager = indexmanager[IndexProvider::IM_Vertices];
    const int idxSize = vertexManager.getMaxIndex();
    // create vector, all entries are marked true 
    vector<bool> isHole (idxSize, true );

    Listwalk_impl < vertex_t > walk(vl) ;
    for( walk.first() ; ! walk.done() ; walk.next() ) 
    {
      vertex_t& vx = walk.getitem();
      in.read ( ((char *) &(vx.setIndex())), sizeof(int) );
      assert( vx.getIndex() < idxSize );
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
    vector<bool> elementIsHole (elSize, true );
    vector<bool> edgeIsHole  (edgeSize, true );
    
    Levelwalk < element_t > walk(mel, 0);
    for( walk.first() ; !walk.done() ; walk.next() ) 
    {
      SubtreeIterator < element_t > hier(&(walk.getitem()));
      for (hier.first() ; !hier.done() ; hier.next() ) 
      {
        element_t &elem = hier.getitem();

        // read element index
        int &index = elem.setIndex();
      	in.read ( ((char *)&index), sizeof(int) );
        assert( elem.getIndex() < elSize );
        elementIsHole[elem.getIndex()] = false;
        
        // read edges 
       	for (int e=0; e<elem.numfaces(); ++e) 
        {
          int edgeNum = -1; 
	        in.read ( ((char *) &(edgeNum)), sizeof(int) );
          assert( edgeNum < edgeSize );
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
