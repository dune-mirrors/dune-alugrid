// typdef these stream because this code uses a lot strstream
typedef basic_stringbuf<char>  strstreambuf_t ;

#include "grid.h"
#include "triang.h"
#include "handle.h"

template <int N,int NV>
bool Hmesh<N,NV> :: 
ascireadtriang(istream &in,
               double& time, 
               unsigned long int& nbr)
{
  bool isbackup=false;
  // Wiederaufsetzen?
  int c = in.peek();
  //int c = in.get () ;
  //assert (!in.eof ()) ;
  //in.putback (c) ;
  assert (in.good ()) ;
  if (c == int ('!')) {
    // Kommentar gefunden: Die erste Zeile in den strstreambuf buf lesen
    // und auf 'Backup' untersuchen.
    strstreambuf_t buf ;
    in.get () ;         // Das Kommentarzeichen wird entfernt.
    in.get (buf) ;
    int len = in.gcount () ;
    // in.get () ;         // Der folgende Zeilenumbruch wird auch entfernt.
    istream is (& buf) ;
    char * str = new char [len + 1] ;
    assert (str) ;
    is >> str ;         // Das erste Wort nach dem Kommentar steht jetzt in str.
    // Alle weiteren k"onnen noch aus is gelesen werden, das
    // array str ist so lang, wie die gesamte Zeile in 'buf'.
    
    if (0 == strcmp (str, "Backup")) {
      cerr << "Backup-file found" << endl;
      // cerr << "STIRNG: " << buf << endl;
      int rrule;
      is >> time >> nbr;
      is >> _nconfDeg >> rrule; 
      refinement_rule=(Refco::tag_t)rrule;
      assert(_nconfDeg>=0);
      assert(_nconfDeg==0 || refinement_rule==Refco::quart);
      isbackup=true;
    } 
    else if (0 != strcmp (str, "Triangles")) 
    {
      cerr << "Error in Hmesh :: ascireadtriang: "
           << "Wrong macrogrid format: " << endl;
      cerr << "file with !-line but command not recognized" << endl;
      abort();
    }
    delete [] str;
  }
  hmesh_basic_t::ascireadtriang(in, isbackup);
  return isbackup;
}
 
template <int N,int NV>
void Hmesh_basic<N,NV> :: ascireadtriang(istream &in, const bool verbose) 
{

  vertex_t ** v = 0 ;

  int nv = 0 ;

  {
    in >> nv ;
    
#ifndef NDEBUG
    if( verbose ) 
      cerr << "    Number of Vertices:             " << nv << endl ;
#endif
    
    v = new vertex_t *[nv] ;

    assert(v) ;
    
    for(int i = 0; i < nv ; i ++) {
         
      vertex_t * n = new fullvertex_t() ;
    
      n->read(in) ;

      v[i] = n ;

      vl.insert(n) ;
      
    }

  }

  {

    int ne = 0 ;
    
    in >> ne ;
  
#ifndef NDEBUG
    if( verbose ) 
      cerr << "    Number of MacroElements:        " << ne << endl ;
#endif
    
 
    for(int i = 0; i < ne ; i ++) {
    
      triang_t &tr = * new triang_t() ;

      tr.read(in, v, nv) ;

      mel.insert( new macroelement_t (tr) ) ;
    
    }

  }

  {

    string line;
    // read line and skip empty lines 
    while( in && line.empty() )
    {
      getline( in, line );
      line.erase( 0, line.find_first_not_of( ' ' ) );
    }
    istringstream linein( line );

    int nb = 0 ;
    linein >> nb ;

    int npb = 0;

#ifndef NDEBUG 
    if( verbose )
      cerr << "    Number of BoundarySegments:     " << nb << endl ;
#endif

    typedef struct
      {
        int pernb;
        bndel_t *b;
      } perbnd_struct;
    perbnd_struct *perbnd_list;
    int perbnd_card=0,perbnd_ok=0;
    //perbnd_list = (perbnd_struct *)malloc(sizeof(perbnd_struct)*nb);
    perbnd_list = new  perbnd_struct[nb];

    typedef struct
      {
        double p0,p1;
        bndel_t *b;
      } axis_struct;
    axis_struct *x_axis,*y_axis;
    int y_card=0,x_card=0,x_ok=0,y_ok=0;

    //x_axis = (axis_struct *)malloc(sizeof(axis_struct)*nb);
    //y_axis = (axis_struct *)malloc(sizeof(axis_struct)*nb);
    x_axis = new axis_struct[nb];
    y_axis = new axis_struct[nb];

    for( int i = 0 ; i <  nb ; ++i ) 
    {
      getline( in, line );
      istringstream linein( line );

      int lt;
      linein >> lt;
      linein.seekg( 0 );

      typename bndel_t::bnd_t t=(typename bndel_t::bnd_t)lt;

      bndel_triang_t * b;

      switch (t)
      {
        case bndel_t::periodic:
          assert(ncoord == 2);
          b=new bndel_periodic_t( i );
          ++npb;
          break;

        case bndel_t::general_periodic:
          b=new bndel_periodic_t( i );
          ++npb;
          break;

        default:
          b=new bndel_triang_t(i, t);
          break;
      }

      assert( b );
      b->read(linein, v, nv) ;

      mbl.insert( new macrobndel_t (*b) ) ;

      if (t==bndel_t::general_periodic) {
        int pernb;
        linein >> pernb;
        if (pernb<i) {
          assert(perbnd_list[pernb].pernb==i);
          ((bndel_periodic_t *)b)->set_pnb(perbnd_list[pernb].b);
          ((bndel_periodic_t *)perbnd_list[pernb].b)->set_pnb(b);
          perbnd_ok++;
        }
        else {
          perbnd_list[i].b=b;
          perbnd_list[i].pernb=pernb;
          perbnd_card++;
        }
      }
      else if (t==bndel_t::periodic)
      {
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

    delete [] y_axis;
    delete [] x_axis;
    delete [] perbnd_list;
    //free(y_axis);
    //free(x_axis);

    assert(y_ok==y_card);
    assert(x_ok==x_card);

#ifndef NDEBUG
    if( verbose )
      cerr << "    Number of periodic boundaries:  " << npb << endl ;
#endif
  
  }       

  delete[] v ;

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
void Hmesh<N,NV>::asciwritetriang(const char *filename,
                                  double time, unsigned long int nbr) {

#ifndef NDEBUG
  cerr << "\n  Hmesh_basic::asciwritetriang(?) opens: " ;
  cerr << filename << "\n" << endl ;
#endif

  vl.renumber() ;
  
  ofstream out(filename, ios::out|ios::trunc) ;
  
  out.setf(ios::fixed, ios::floatfield) ;
  
  out << scientific ;
  out.precision(16) ;

  out << "!Backup ";
  out << time << " " << nbr << " ";
  out << _nconfDeg << " " << refinement_rule << endl;
  hmesh_basic_t::asciwritetriang(out);
}
 
template <int N,int NV>
void Hmesh_basic<N,NV>::asciwritetriang(ostream &out) {
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
Hmesh<N,NV>::storeGrid(const char* fbase,
     double time, unsigned long int nbr)
{
  char *filename;

  filename=new char[strlen(fbase)+15];
  // filename=(char*)alloca(strlen(fbase)+15);
  sprintf(filename,"%s",fbase);

  asciwritetriang (filename,time,nbr);

  sprintf(filename,"%s.refine",fbase);

#ifndef NDEBUG
  cerr << "Hmesh::writeRecoverFile(): writing file \""
       << filename << "\" ...";
#endif

  ofstream out(filename, ios::out|ios::trunc);
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

  delete [] filename;
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
Hmesh<N,NV>::recoverGrid(const char* recoverFile,
                   double& time, unsigned long int &nbr)
{
  int compwarn = 0;

  cout << "Hmesh::recoverGrid(): trying to read file \""
       << recoverFile << "\" ...";

  char filename[1000];
  sprintf(filename,"%s.refine",recoverFile);

  ifstream in(filename);
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
      	in.read ( ((char *) &(elem.setIndex())), sizeof(int) );
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
