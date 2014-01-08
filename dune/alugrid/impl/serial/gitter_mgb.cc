// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 
#include <config.h>

#include <sstream>

#include "gitter_sti.h"
#include "gitter_mgb.h"

namespace ALUGrid
{

  std::pair< Gitter::Geometric::VertexGeo *, bool > MacroGridBuilder::
  InsertUniqueVertex (double x, double y, double z, int i) {
    vertexMap_t::const_iterator hit = _vertexMap.find (i);
    if (hit == _vertexMap.end ()) {
      VertexGeo * v = myBuilder ().insert_vertex (x,y,z,i);
      _vertexMap [i] = v;
      return std::pair< VertexGeo *, bool > (v,true);
    } else {
      return std::pair< VertexGeo *, bool > ((*hit).second, false);
    }
  }
   
  std::pair< Gitter::Geometric::hedge1_GEO *, bool > MacroGridBuilder::
  InsertUniqueHedge (int l, int r) {
    if (l > r) { 
      int i = l; l = r; r = i;
    }
    edgeKey_t key (l,r);
    edgeMap_t::const_iterator hit = _edgeMap.find (key);
    if (hit == _edgeMap.end ()) 
    {
      vertexMap_t::const_iterator a = _vertexMap.find (l), b = _vertexMap.find (r);

      alugrid_assert ( a != _vertexMap.end() );
      alugrid_assert ( b != _vertexMap.end() );
      
      hedge1_GEO * h = myBuilder ().insert_hedge1 ((*a).second,(*b).second);
      _edgeMap [key] = h;
      return std::pair< hedge1_GEO *, bool > (h,true);
    } else {
      return std::pair< hedge1_GEO *, bool > ((*hit).second,false);
    }
  }

  std::pair< Gitter::Geometric::hface3_GEO *, bool > MacroGridBuilder::
  InsertUniqueHface (int (&v)[3]) {
    cyclicReorder (v,v+3);
    faceKey_t key (v[0],v[1],v[2]);
    faceMap_t::const_iterator hit = _face3Map.find (key);
    if (hit == _face3Map.end ()) {
      hedge1_GEO * edge [3];
      int dire [3] = { 0, 0, 1 };
      edge [0] = InsertUniqueHedge (v[0],v[1]).first;
      edge [1] = InsertUniqueHedge (v[1],v[2]).first;
      edge [2] = InsertUniqueHedge (v[2],v[0]).first;
      hface3_GEO * f3 = myBuilder ().insert_hface3 (edge,dire);
      _face3Map [key] = f3;
      return std::pair< hface3_GEO *, bool > (f3,true);
    } else {
      return std::pair< hface3_GEO *, bool > ((hface3_GEO *)(*hit).second,false);
    }
  }

  std::pair< Gitter::Geometric::hface4_GEO *, bool > MacroGridBuilder::InsertUniqueHface (int (&v)[4]) {
    cyclicReorder (v,v+4);
    faceKey_t key (v[0],v[1],v[2]);
    faceMap_t::const_iterator hit = _face4Map.find (key);
    if (hit == _face4Map.end ()) {
      hedge1_GEO * edge [4];
      int dire [4]; 
      edge [0] = InsertUniqueHedge (v[0],v[1]).first;
      edge [1] = InsertUniqueHedge (v[1],v[2]).first;
      edge [2] = InsertUniqueHedge (v[2],v[3]).first;
      edge [3] = InsertUniqueHedge (v[3],v[0]).first;  
      dire [0] = v[0] < v[1] ? 0 : 1;
      dire [1] = v[1] < v[2] ? 0 : 1;
      dire [2] = v[2] < v[3] ? 0 : 1;
      dire [3] = v[3] < v[0] ? 0 : 1;
      hface4_GEO * f4 = myBuilder ().insert_hface4 (edge,dire);
      _face4Map [key] = f4;
      return std::pair< hface4_GEO *, bool > (f4,true);
    } else {
      return std::pair< hface4_GEO *, bool > ((hface4_GEO *)(*hit).second,false);
    }
  }

  std::pair< Gitter::Geometric::tetra_GEO *, bool > MacroGridBuilder::
  InsertUniqueTetra (int (&v)[4], int orientation) 
  {
    elementKey_t key (v [0], v [1], v [2], v [3]);
    elementMap_t::const_iterator hit = _tetraMap.find (key);
    if (hit == _tetraMap.end ()) {
      hface3_GEO * face [4];
      int twst [4];
      for (int fce = 0; fce < 4; ++fce ) 
      {
        int x [3];
        x [0] = v [Tetra::prototype [fce][0]];
        x [1] = v [Tetra::prototype [fce][1]];
        x [2] = v [Tetra::prototype [fce][2]];
        twst [fce] = cyclicReorder (x,x+3);
        face [fce] =  InsertUniqueHface (x).first;
      }
      tetra_GEO * t = myBuilder ().insert_tetra (face,twst,orientation);
      alugrid_assert (t);
      _tetraMap [key] = t;
      return std::pair< tetra_GEO *, bool > (t,true);
    } 
    else 
    {
      return std::pair< tetra_GEO *, bool > ((tetra_GEO *)(*hit).second,false);
    }
  }

  std::pair< Gitter::Geometric::hexa_GEO *, bool > MacroGridBuilder::InsertUniqueHexa (int (&v)[8]) 
  {
    elementKey_t key (v [0], v [1], v [3], v[4]);
    elementMap_t::const_iterator hit = _hexaMap.find (key);
    if (hit == _hexaMap.end ()) {
      hface4_GEO * face [6];
      int twst [6];
      for (int fce = 0; fce < 6; ++fce) 
      {
        int x [4];
        x [0] = v [Hexa::prototype [fce][0]];
        x [1] = v [Hexa::prototype [fce][1]];
        x [2] = v [Hexa::prototype [fce][2]];
        x [3] = v [Hexa::prototype [fce][3]];
        twst [fce] = cyclicReorder (x,x+4);
        face [fce] =  InsertUniqueHface (x).first;
      }
      hexa_GEO * hx = myBuilder ().insert_hexa (face,twst);
      _hexaMap [key] = hx;
      return std::pair< hexa_GEO *, bool > (hx,true);
    } else {
      return std::pair< hexa_GEO *, bool > ((hexa_GEO *)(*hit).second,false);
    }
  }

  bool MacroGridBuilder::
  InsertUniqueHbnd3 (int (&v)[3],Gitter::hbndseg_STI ::bnd_t bt, int ldbVertexIndex, int master) 
  {
    int twst = cyclicReorder (v,v+3);
    faceKey_t key (v [0], v [1], v [2]);
    if (bt == Gitter::hbndseg_STI::closure) 
    {
      if (_hbnd3Int.find (key) == _hbnd3Int.end ()) {
        hface3_GEO * face =  InsertUniqueHface (v).first;
        _hbnd3Int [key] = new Hbnd3IntStorage (face, twst, ldbVertexIndex, master);
        return true;
      }
    } 
    else 
    {
      if (_hbnd3Map.find (key) == _hbnd3Map.end ()) 
      {
        hface3_GEO * face  = InsertUniqueHface (v).first;
        hbndseg3_GEO * hb3 = myBuilder ().insert_hbnd3 (face,twst,bt);
        hb3->setLoadBalanceVertexIndex( ldbVertexIndex );
        hb3->setMaster( master );
        _hbnd3Map [key] = hb3;
        return true;
      }
    }
    return false;
  }

  bool MacroGridBuilder::
  InsertUniqueHbnd4 (int (&v)[4], Gitter::hbndseg_STI ::bnd_t bt, int ldbVertexIndex, int master ) 
  {
    int twst = cyclicReorder (v,v+4);
    faceKey_t key (v [0], v [1], v [2]);
    if (bt == Gitter::hbndseg_STI::closure) 
    {
      if (_hbnd4Int.find (key) == _hbnd4Int.end ()) {
        hface4_GEO * face =  InsertUniqueHface (v).first;
        _hbnd4Int [key] = new Hbnd4IntStorage (face, twst, ldbVertexIndex, master );
        return true;
      }
    } 
    else 
    {
      if (_hbnd4Map.find (key) == _hbnd4Map.end ()) 
      {
        hface4_GEO * face =  InsertUniqueHface (v).first;
        hbndseg4_GEO * hb4 = myBuilder ().insert_hbnd4 (face,twst,bt);
        hb4->setLoadBalanceVertexIndex( ldbVertexIndex );
        hb4->setMaster( master );
        _hbnd4Map [key] = hb4;
        return true;
      }
    }
    return false;
  }

  std::pair< Gitter::Geometric::periodic3_GEO *, bool > MacroGridBuilder::
  InsertUniquePeriodic (int (&v)[6], const Gitter::hbndseg_STI ::bnd_t (&bt)[2] )  
  {

    // Vorsicht: Der Schl"ussel f"ur das periodische Randelement wird
    // dummerweise mit dem eines Hexaeders verwechselt, falls nicht
    // der letzte Knoten negativ (mit umgekehrtem Vorzeichen) in die
    // Schl"ussel eingef"ugt wird.

    elementKey_t key (v [0], v [1], v [2], -(v [3])-1);
    elementMap_t::const_iterator hit = _periodic3Map.find (key);
    if (hit == _periodic3Map.end ()) {
      hface3_GEO * face [2];
      int twst [2];
      for (int fce = 0; fce < 2; ++fce ) 
      {
        int x [3];
        x [0] = v [Periodic3::prototype [fce][0]];
        x [1] = v [Periodic3::prototype [fce][1]];
        x [2] = v [Periodic3::prototype [fce][2]];
        twst [fce] = cyclicReorder (x,x+3);
        face [fce] = InsertUniqueHface (x).first;
      }
      periodic3_GEO * t = myBuilder ().insert_periodic3 (face,twst,bt);
      alugrid_assert (t);
      _periodic3Map [key] = t;
      return std::pair< periodic3_GEO *, bool > (t,true);
    } else {
      return std::pair< periodic3_GEO *, bool > ((periodic3_GEO *)(*hit).second,false);
    }
  }

  std::pair< Gitter::Geometric::periodic4_GEO *, bool > MacroGridBuilder::
  InsertUniquePeriodic (int (&v)[8], const Gitter::hbndseg_STI ::bnd_t (&bt)[2] ) 
  {

    // Vorsicht: Der Schl"ussel f"ur das periodische Randelement wird
    // dummerweise mit dem eines Hexaeders verwechselt, falls nicht
    // der letzte Knoten negativ (mit umgekehrtem Vorzeichen) in die
    // Schl"ussel eingef"ugt wird.

    elementKey_t key (v [0], v [1], v [3], -(v [4])-1);
    elementMap_t::const_iterator hit = _periodic4Map.find (key);
    if (hit == _periodic4Map.end ()) {
      hface4_GEO * face [2];
      int twst [2];
      for (int fce = 0; fce < 2; ++fce ) 
      {
        int x [4];
        x [0] = v [Periodic4::prototype [fce][0]];
        x [1] = v [Periodic4::prototype [fce][1]];
        x [2] = v [Periodic4::prototype [fce][2]];
        x [3] = v [Periodic4::prototype [fce][3]];
        twst [fce] = cyclicReorder (x,x+4);
        face [fce] = InsertUniqueHface (x).first;
      }
      periodic4_GEO * t = myBuilder ().insert_periodic4 (face,twst,bt);
      alugrid_assert (t);
      _periodic4Map [key] = t;
      return std::pair< periodic4_GEO *, bool > (t,true);
    } 
    else 
    {
      return std::pair< periodic4_GEO *, bool > ((periodic4_GEO *)(*hit).second,false);
    }
  }
  // Ende - Neu am 23.5.02 (BS)

  void MacroGridBuilder::removeElement (const elementKey_t & k, const bool realElement ) 
  {
    // Der Schl"ussel sollte nur in genau einer Map vorliegen.

    alugrid_assert ((_hexaMap.find (k) == _hexaMap.end () ? 0 : 1)
          + (_tetraMap.find(k) == _tetraMap.end () ? 0 : 1)
          + (_periodic3Map.find (k) == _periodic3Map.end () ? 0 : 1)
          + (_periodic4Map.find (k) == _periodic4Map.end () ? 0 : 1) == 1);

    if( realElement ) 
    {
      elementMap_t::iterator hit = _tetraMap.find (k);
      if (hit != _tetraMap.end ()) 
      {
        tetra_GEO * tr = (tetra_GEO *)(*hit).second;
        int ldbVertexIndex = tr->ldbVertexIndex();
        int master = tr->master();

        typedef hbnd3intMap_t::iterator iterator;
        const iterator end = _hbnd3Int.end();
        for (int i = 0; i < 4; ++i) 
        {
          // for periodic neighbours we do not create internal storages 
          if( tr->myneighbour( i ).first->isperiodic() ) 
            continue;

          hface3_GEO* face = tr->myhface3 (i);
          faceKey_t key (face->myvertex (0)->ident (), 
                         face->myvertex (1)->ident (), 
                         face->myvertex (2)->ident ());

          // if the face does not exist in the map of internal boundaries 
          // we need to insert this 
          iterator hbndit = _hbnd3Int.find( key );
          if( hbndit == end ) 
          {
            Hbnd3IntStorage* hbnd = 
              new Hbnd3IntStorage (face, tr->twist (i), ldbVertexIndex, master, tr , i );
            _hbnd3Int.insert( std::make_pair( key, hbnd ) );
          }
          // if the face already exists this means we can delete it, 
          // since both adjacent element will disappear 
          else 
          {
            Hbnd3IntStorage* hbnd = (*hbndit).second;
            _hbnd3Int.erase( hbndit );
            delete hbnd;
          }
        }

        delete tr;
        _tetraMap.erase (hit);

        return;
      }

      hit = _hexaMap.find (k);
      if (hit != _hexaMap.end ()) 
      {
        hexa_GEO * hx = (hexa_GEO *)(*hit).second;
        int ldbVertexIndex = hx->ldbVertexIndex();
        int master = hx->master();

        typedef hbnd4intMap_t::iterator iterator;
        const iterator end = _hbnd4Int.end();
        for (int i = 0; i < 6; ++i) 
        {
          // for periodic neighbours we do not create internal storages 
          if( hx->myneighbour( i ).first->isperiodic() ) 
            continue;

          hface4_GEO* face = hx->myhface4 (i);
          faceKey_t key (face->myvertex (0)->ident (), 
                         face->myvertex (1)->ident (), 
                         face->myvertex (2)->ident ());

          iterator hbndit = _hbnd4Int.find( key );
          // if the face does not exist in the map of internal boundaries 
          // we need to insert this 
          if( hbndit == end ) 
          {
            Hbnd4IntStorage* hbnd = 
              new Hbnd4IntStorage ( face, hx->twist (i), ldbVertexIndex, master, hx, i );

            _hbnd4Int.insert( std::make_pair( key, hbnd ) );
          }
          // if the face already exists this means we can delete it, 
          // since both adjacent element will disappear 
          else 
          {
            Hbnd4IntStorage* hbnd = (*hbndit).second;
            _hbnd4Int.erase( hbndit );
            delete hbnd;
          }
        }

        delete hx;
        _hexaMap.erase (hit);

        return;
      }
    }
    else 
    {
      elementMap_t::iterator hit = _periodic3Map.find (k);
      if (hit != _periodic3Map.end ()) 
      {
        periodic3_GEO * p3 = (periodic3_GEO *)(*hit).second;

        delete p3;
        _periodic3Map.erase (hit);

        return;
      }

      hit = _periodic4Map.find (k);
      if (hit != _periodic4Map.end ()) 
      {
        periodic4_GEO * p4 = (periodic4_GEO *)(*hit).second;

        delete p4;
        _periodic4Map.erase (hit);

        return;
      }
    }

    abort ();
    return;
  }

  void MacroGridBuilder::cubeHexaGrid ( int n, std::ostream &out )
  {

    // cubeHexaGrid () ist eine statische Methode, die einen ASCII Strom
    // mit einem gleichm"assigen Hexaedernetz auf dem Einheitsw"urfel
    // [0,1]^3 beschreibt, wobei <n> die Aufl"osung der Raumrichtungen
    // vorgibt: Es entstehen n^3 Hexaederelemente, und (n+1)^3 Knoten.
    // Es ist als Servicemethode f"ur die 'ball' und 'ball_pll' Test-
    // programme n"otig und deshalb in der MacrogridBuilder Klasse 
    // beheimatet.

    const int bndtype = -1;
    out.setf( std::ios::fixed, std::ios::floatfield );
    out.precision ( ALUGridExternalParameters::precision() );
    n = n < 0 ? 0 : n;
    int npe = n + 1;
    out << (npe * npe * npe) << std::endl;
    double delta = 1.0 / (double)(n);
    {
      for(int i = 0; i < npe; i ++) {
        for(int j = 0; j < npe; j ++) {
          for(int k = 0; k < npe; k ++) {
            out << double(k * delta) << "  "  << double(j * delta) << "  " << double(i * delta) << "\n";
          }
        }
      }
    }
    out << n * n * n << "\n";
    {
      for(int i = 0; i < n; i ++) {
        int ipea = (i + 1) * npe * npe, ia = i * npe * npe;
        for(int j = 0; j < n; j ++) {
          int jpea = (j + 1) * npe, ja = j * npe;  
          for(int k = 0; k < n; k ++) {
            int kpe = k + 1;  
            out << k + ia + ja << "  " << kpe + ia + ja << "  "  << kpe + ia + jpea << "  " << k + ia + jpea << "  "         
                << k + ja + ipea << "  " << kpe + ja + ipea << "  " << kpe + ipea + jpea << "  " << k + ipea + jpea << "\n"; 
          }  
        }    
      }
      out << std::endl;  
    }
    out << 6 * n * n << std::endl;
    { // unten und oben
      int l = n * npe * npe;
      for(int j = 0; j < n; j ++) {
        int jpea = (j + 1) * npe, ja = j * npe;
        for(int k = 0; k < n; k ++) {
          int kpe = k + 1;
          out << bndtype << "  " << 4 << "  " << (kpe + ja) << "  " << (kpe + jpea) << "  "
              << (k + jpea) << "  " << (k + ja) << "\n" << bndtype << "  " << 4 << "  "
              << (k + jpea + l) << "  " << (kpe + jpea + l) << "  " << (kpe + ja + l) << "  " << (k + ja + l) << "\n";
        }
      }
      out << std::endl;
    }
    { // links und rechts
      int l = n * npe;
      for(int j = 0; j < n; j ++) {
        int jpea = (j + 1) * npe * npe, ja = j * npe * npe;
        for(int ka = 0; ka < n; ka ++) {
          int kpea = (ka + 1);
          out << bndtype << "  " << 4 << "  " << ka + jpea << "  " << kpea + jpea << "  " 
              << kpea + ja << "  " << ka + ja << "\n" << bndtype << "  " << 4 << "  " 
        << kpea + ja + l << "  " << kpea + jpea + l << "  " << ka + jpea + l << "  " << ka + ja + l << "\n";
        }
      }
      out << std::endl;
    }
    { // hinten und vorne
      int l = n;
      for(int j = 0; j < n; j ++) {
        int jpea = (j + 1) * npe * npe, ja = j * npe * npe;
        for(int k = 0; k < n; k ++) {
          int kpea = (k + 1) * npe, ka = k * npe;
          out << bndtype << "  " << 4 << "  " << kpea + ja << "  " << kpea + jpea << "  " 
              << ka + jpea << "  " << ka + ja << "\n" << bndtype << "  " << 4 << "  " 
              << ka + jpea + l << "  " << kpea + jpea + l << "  " << kpea + ja + l << "  " << ka + ja + l << "\n"; 
        }
      }
      out << std::endl;
    }
    return;
  }

  void MacroGridBuilder::generateRawHexaImage ( std::istream& in, std::ostream &os )
  {
    generateRawImage( in, os, HEXA_RAW, PERIODIC4_RAW );
  }

  void MacroGridBuilder::generateRawHexaImage ( ObjectStream& in, std::ostream &os )
  {
    generateRawImage( in, os, HEXA_RAW, PERIODIC4_RAW );
  }

  void MacroGridBuilder::generateRawTetraImage ( std::istream &in, std::ostream &os )
  {
    generateRawImage( in, os, TETRA_RAW, PERIODIC3_RAW );
  }

  void MacroGridBuilder::generateRawTetraImage ( ObjectStream &in, std::ostream &os )
  {
    generateRawImage( in, os, TETRA_RAW, PERIODIC3_RAW );
  }

  template< class istream_t >
  void MacroGridBuilder
    ::generateRawImage ( istream_t &in, std::ostream &os,
                         const ElementRawID elementId, const ElementRawID periodicId )
  {
    // generateRawHexaImage () ist im nur ein Adapter, der aus den 
    // bisherigen Hexaederdateiformaten ein entsprechendes 'rohes'
    // Dateiformat f"ur den Macrogridinflator erzeugt. Damit bleibt
    // die Option erhalten das Format der rohen Dateien auf weitere
    // Elemente auszudehnen und zu modifizieren, ohne die 
    // Kompatibilit"at zu den alten Hexaederdateien zu verlieren.
    // Das alte Format sieht im wesentlichen so aus:
    //
    // <Anzahl der Knoten : int >     /* 1.Zeile der Datei
    // <x-Koordinate : float>  <y-Koo. : float>  <z-Koo. : float>
    // ...            /* f"ur den letzten Knoten
    // <Anzahl der Elemente : int>
    // <KnotenNr. 0: int> ... <KnotenNr. 7: int>  /* f"ur das erste Hexaederelement
    // ...            /* f"ur das letzte Hexaederelement
    // <Anzahl der Randfl"achen : int>
    // <Randtyp>  4  <KnotenNr. 0> ... <KnotenNr. 3>/* erste Randfl"ache
    // ...            /* letzte Randfl"ache
    // <Identifier f"ur den 0. Knoten : int>  /* Identifierliste ist im seriellen
    // ...            /* Verfahren oder beim Aufsetzen aus
    // <Identifier f"ur den letzten Knoten : int> /* einem Gitter optional, sonst muss
    //            /* jeder Vertex eine eigene Nummer haben
    
    const int start = clock ();
    int nv = 0, ne = 0, nb = 0, nper = 0;
    int (* vnum)[8] = 0, (* bvec)[5] = 0, (* pervec)[9] = 0, * pident = 0;
    double (* coord)[3] = 0;

    const int elementVertices = elementId;
    const int faceVertices = (elementId == TETRA_RAW) ? 3 : 4;
    const int periodicVertices = (elementId == TETRA_RAW) ? 6 : 8;
    alugrid_assert ( faceVertices == 4 ? elementId == HEXA_RAW : true );
    alugrid_assert ( periodicVertices == 8 ? elementId == HEXA_RAW : true );
    {
      in >> nv;
      coord = new double [nv][3];
      alugrid_assert (coord);
      for (int i = 0; i < nv; ++i) in >> coord [i][0] >> coord [i][1] >> coord [i][2];
    }

    {
      in >> ne;
      vnum = new int [ne][8];
      alugrid_assert (vnum);
      for (int i = 0; i < ne; ++i)
      {
        for( int vx = 0; vx < elementVertices; ++ vx ) 
        {
          in >> vnum [i][vx];
        }
      }
    }
    
    {
      int temp_nb;
      in >> temp_nb;
      bvec = new int [temp_nb][5];
      pervec = new int [temp_nb][9];
      alugrid_assert (bvec);
      alugrid_assert (pervec);

      for (int i = 0; i < temp_nb; ++i) 
      {
        int n;
        int identification;
        in >> identification >> n;
        // hexa or tetra element boundary
        if ( n == faceVertices ) 
        {
          for( int vx=0; vx<n; ++ vx) 
          {
            in >> bvec [nb][vx];
          }
          // use last component for storage of identification 
          bvec [nb][n] = identification;
          nb++; 
        } 
        // periodic boundary 
        else if ( n == periodicVertices ) 
        {
          for( int vx=0; vx<n; ++ vx) 
          {
            in >> pervec [nper][vx];
          }

          // keep boundary information 
          // use last component for storage of identification 
          pervec [nper][n] = identification;
          nper++;
        }
        else
        {
          std::cerr << "ERROR (fatal):  " << __FILE__ << " " << __LINE__ << " ... Exiting." << std::endl;
          abort();
        }
      }
    }
    
    if ( !in.good() )
    {
      std::cerr << "ERROR (fatal): Unexpected end of file." << std::endl;
      abort();
    }
    pident = new int [nv];
    {
      int dummy;
      for (int i = 0; i < nv; ++ i ) in >> pident [i] >> dummy; 
    }

    if( !in.good() )
    {
      std::cerr << "WARNING (ignored) No parallel identification applied due to incomplete (or non-existent) identifier list." << std::endl;
      for( int i = 0; i < nv; ++ i )
        pident[ i ] = i;
    }

    // get last std::endl character (from backup to make stream consistent)
    if( !in.eof() )
      in.get();

    // write vertices 
    os << nv << std::endl;
    for (int i = 0; i < nv; ++i )
      os << pident [i] << " " << coord [i][0] << " " << coord [i][1] << " " << coord [i][2] << std::endl;

    // write elements 
    os << (ne + nper) << std::endl;
    for (int i = 0; i < ne; ++i)
    {
      os << elementId << " ";
      for( int vx = 0; vx < elementVertices; ++ vx ) 
      {
        os << pident[ vnum [i][vx] ] << " ";
      }
      os << std::endl;
    }

    // write periodic elements 
    for (int i = 0; i < nper; ++i)
    {
      os << periodicId << " ";
      for( int vx = 0; vx < periodicVertices; ++vx ) 
      {
        os << pident[ pervec[i][vx] ] << " ";
      }
      // write the identification 
      os << pervec [i][ periodicVertices ] << std::endl;
    }

    // write boundaries 
    os << nb << std::endl;
    for (int i = 0; i < nb; ++i)
    {
      os << faceVertices << " ";
      for( int vx = 0; vx < faceVertices; ++vx ) 
      {
        os << pident[ bvec[i][vx] ] << " ";
      }
      // write the identification 
      os << bvec[i][ faceVertices ] << std::endl;
    }

    // delete temporary memory 
    delete [] vnum;
    delete [] coord;
    delete [] pervec;
    delete [] bvec;
    delete [] pident;
    if( debugOption( 4 ) )
      std::cout << "INFO: MacroGridBuilder::generateRawHexaImage() used: " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " s." << std::endl;
  }

  // default of init == true
  MacroGridBuilder::MacroGridBuilder (BuilderIF & b, const bool init) 
   : _initialized(false) 
   , _finalized(false) 
   , _mgb (b) 
  {
    if(init) initialize();
  }

  // deprecated constructor, project vertex has been removed 
  MacroGridBuilder::MacroGridBuilder (BuilderIF & b, ProjectVertex* ) 
   : _initialized(false) 
   , _finalized(false) 
   , _mgb (b) 
  {
    initialize();
  }

  void MacroGridBuilder::initialize () 
  {
    {
      BuilderIF::vertexlist_t& _vertexList = myBuilder ()._vertexList;
      typedef BuilderIF::vertexlist_t::iterator  iterator;
      const iterator vertexListEnd = _vertexList.end ();
      // copy list entries to map
      for ( iterator i = _vertexList.begin (); i != vertexListEnd; ++i )
        _vertexMap [ (*i)->ident ()] = (*i);
      // clear list
      _vertexList.clear();
    }
    {
      BuilderIF::hedge1list_t& _hedge1List = myBuilder ()._hedge1List;
      typedef BuilderIF::hedge1list_t::iterator  iterator;
      const iterator hedge1ListEnd = _hedge1List.end ();
      // copy list entries to map
      for ( iterator i = _hedge1List.begin (); i != hedge1ListEnd; ++i )
      {
        long k = (*i)->myvertex (0)->ident (), l = (*i)->myvertex (1)->ident ();
        _edgeMap [edgeKey_t (k < l ? k : l, k < l ? l : k)] = (*i);
      }
      // clear list
      _hedge1List.clear();
    }
    {
      BuilderIF::hface3list_t& _hface3List = myBuilder ()._hface3List;
      typedef BuilderIF::hface3list_t::iterator  iterator;
      const iterator  hface3ListEnd = _hface3List.end ();
      // copy list entries to map
      for ( iterator i = _hface3List.begin (); i != hface3ListEnd; ++i )
      {
        _face3Map [faceKey_t ((*i)->myvertex (0)->ident (),(*i)->myvertex (1)->ident (), (*i)->myvertex (2)->ident ())] = (*i);
      }
      // clear list
      _hface3List.clear();
    }
    {
      BuilderIF::hface4list_t& _hface4List = myBuilder ()._hface4List;
      typedef BuilderIF::hface4list_t::iterator  iterator;
      const iterator hface4ListEnd = _hface4List.end ();
      // copy list entries to map
      for ( iterator i = _hface4List.begin (); i != hface4ListEnd; ++i )
      {
        _face4Map [faceKey_t ((*i)->myvertex (0)->ident (),(*i)->myvertex (1)->ident (), (*i)->myvertex (2)->ident ())] = (*i);
      }

      // clear list
      _hface4List.clear();
    }
    {
      BuilderIF::hbndseg4list_t& _hbndseg4List = myBuilder ()._hbndseg4List;
      typedef BuilderIF::hbndseg4list_t::iterator  iterator;
      const iterator hbndseg4ListEnd = _hbndseg4List.end ();
      // copy entries to map
      for ( iterator i = myBuilder ()._hbndseg4List.begin (); i != hbndseg4ListEnd; ++i )
      {
        faceKey_t key ((*i)->myhface4 (0)->myvertex (0)->ident (), (*i)->myhface4 (0)->myvertex (1)->ident (), (*i)->myhface4 (0)->myvertex (2)->ident ());
        if ((*i)->bndtype () == Gitter::hbndseg_STI::closure) {
          _hbnd4Int [key] = new Hbnd4IntStorage ((*i)->myhface4 (0),(*i)->twist (0),(*i)->ldbVertexIndex(),(*i)->master());
          delete (*i);
        } 
        else 
        {
          _hbnd4Map [key] = (*i);
        }
      } 
      // clear list
      _hbndseg4List.clear();
    }
    {
      BuilderIF::hbndseg3list_t& _hbndseg3List = myBuilder ()._hbndseg3List;
      typedef BuilderIF::hbndseg3list_t::iterator iterator;
      const iterator hbndseg3ListEnd = _hbndseg3List.end ();
      // copy entries to map
      for ( iterator i = _hbndseg3List.begin (); i != hbndseg3ListEnd; ++i )
      {
        faceKey_t key ((*i)->myhface3 (0)->myvertex (0)->ident (), (*i)->myhface3 (0)->myvertex (1)->ident (), (*i)->myhface3 (0)->myvertex (2)->ident ());
        if ((*i)->bndtype () == Gitter::hbndseg_STI::closure) 
        {
          _hbnd3Int [key] = new Hbnd3IntStorage ((*i)->myhface3 (0), (*i)->twist (0),(*i)->ldbVertexIndex(),(*i)->master());
          delete (*i);
        } 
        else 
        {
          _hbnd3Map [key] = (*i);
        }
      }
      // clear list
      _hbndseg3List.clear();
    }
    {
      BuilderIF::tetralist_t& _tetraList = myBuilder ()._tetraList;
      typedef BuilderIF::tetralist_t::iterator  iterator;
      const iterator tetraListEnd = _tetraList.end ();
      // copy entries to map
      for ( iterator i = _tetraList.begin (); i != tetraListEnd; ++i ) 
      {
        _tetraMap [elementKey_t ( (*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
                                  (*i)->myvertex (2)->ident (), (*i)->myvertex (3)->ident ())] = (*i);
      }
      // clear list
      _tetraList.clear();
    }
    {
      BuilderIF::periodic3list_t& _periodic3List = myBuilder ()._periodic3List;
      typedef BuilderIF::periodic3list_t::iterator iterator;
      const iterator periodic3ListEnd = _periodic3List.end ();
      // copy entries to map
      for ( iterator i = _periodic3List.begin (); i != periodic3ListEnd; ++i ) 
      {
        _periodic3Map [elementKey_t ( (*i)->myvertex (0)->ident (),  (*i)->myvertex (1)->ident (), 
                                      (*i)->myvertex (2)->ident (), -((*i)->myvertex (3)->ident ())-1)] = (*i);
      }
      // clear list
      _periodic3List.clear();
    }
    {
      BuilderIF::periodic4list_t& _periodic4List = myBuilder ()._periodic4List;
      typedef BuilderIF::periodic4list_t::iterator  iterator;
      const iterator periodic4ListEnd = _periodic4List.end ();
      // copy entries to map
      for ( iterator i = _periodic4List.begin (); i != periodic4ListEnd; ++i )
      {
        _periodic4Map [elementKey_t ( (*i)->myvertex (0)->ident (),  (*i)->myvertex (1)->ident (), 
                                      (*i)->myvertex (3)->ident (), -((*i)->myvertex (4)->ident ())-1)] = (*i);
      }
      // clear list
      _periodic4List.clear();
    }
    {
      BuilderIF::hexalist_t& _hexaList = myBuilder ()._hexaList;
      typedef BuilderIF::hexalist_t::iterator  iterator;
      const iterator  hexaListEnd = _hexaList.end ();
      // copy entries to map
      for ( iterator i = _hexaList.begin (); i != hexaListEnd; ++i )
        _hexaMap [elementKey_t ( (*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
                                 (*i)->myvertex (3)->ident (), (*i)->myvertex (4)->ident ())] = (*i);
      // clear list
      _hexaList.clear();
    }

    _initialized = true;
    return; 
  }

  MacroGridBuilder::~MacroGridBuilder () 
  {
    // _finalized is true if the method was called in inherited classes 
    if(!_finalized) finalize();
  }

  void MacroGridBuilder::
  hexaMapToList( elementMap_t& elementMap, hexalist_t& elemList, const bool setIndex  )
  {
    elementMapToList( elementMap, elemList, setIndex );
  }

  void MacroGridBuilder::
  tetraMapToList( elementMap_t& elementMap, tetralist_t& elemList, const bool setIndex  )
  {
    elementMapToList( elementMap, elemList, setIndex );
  }

  template< class elemlist_t > 
  void MacroGridBuilder::
  elementMapToList( elementMap_t& elementMap, elemlist_t& elemList, const bool setIndex  )
  {
    // elem_GEO_ptr is either hexa_GEO* or tetra_GEO* 
    typedef typename elemlist_t :: value_type elem_GEO_ptr; 
    {
      // sort by element numbering which is unique for macro elements 
      typedef std::map< int, elem_GEO_ptr > elemmap_t;
      elemmap_t elemMap;
      {
        typedef typename elementMap_t::iterator  iterator;
        const iterator elementMapEnd = elementMap.end();
        for (iterator i = elementMap.begin (); 
             i != elementMapEnd; elementMap.erase (i++) )
        {
          elem_GEO_ptr elem = (elem_GEO_ptr)(*i).second;
          // if ldbVertexIndex still needs to be set (in case of initial read)
          if( setIndex ) 
          {
            elem->setLoadBalanceVertexIndex( elem->getIndex() );
          }
          // ldbVertexIndex provides the unique index of the element across processes 
          elemMap[ elem->ldbVertexIndex() ] = elem;
        }
      }
      {
        int elemCount = 0;
        typedef typename elemmap_t::iterator  iterator;
        const iterator iend = elemMap.end();
        for ( iterator i = elemMap.begin (); i != iend; ++ i, ++elemCount )
        {
          elem_GEO_ptr elem = (elem_GEO_ptr)(*i).second;
          // make sure that the insertion order 
          // in the list is reflected by getIndex 
          alugrid_assert ( setIndex ? (elem->getIndex() == elemCount) : true );
          // insert into macro element list 
          elemList.push_back ( elem );
        }
      }
    }
  }

  // clean the map tables 
  void MacroGridBuilder::finalize () 
  {
    alugrid_assert (_initialized);
    
    // copy elements from hexa map to hexa list respecting the insertion order 
    hexaMapToList( _hexaMap, myBuilder()._hexaList, true );

    // copy elements from tetra map to tetra list respecting the insertion order 
    tetraMapToList( _tetraMap, myBuilder()._tetraList, true );

    {
      typedef elementMap_t::iterator  iterator;
      const iterator periodic3MapEnd = _periodic3Map.end ();
      for (elementMap_t::iterator i = _periodic3Map.begin (); i != periodic3MapEnd; _periodic3Map.erase (i++))
        myBuilder ()._periodic3List.push_back ((periodic3_GEO *)(*i).second);
    }
    
    {
      typedef elementMap_t::iterator  iterator;
      const iterator periodic4MapEnd = _periodic4Map.end ();
      for (elementMap_t::iterator i = _periodic4Map.begin (); i != periodic4MapEnd; _periodic4Map.erase (i++))
        myBuilder ()._periodic4List.push_back ((periodic4_GEO *)(*i).second);
    }

    {
      typedef faceMap_t::iterator iterator;
      const iterator hbnd4MapEnd =  _hbnd4Map.end ();
      for (faceMap_t::iterator i = _hbnd4Map.begin (); i != hbnd4MapEnd; )
      {
        if (((hbndseg4_GEO *)(*i).second)->myhface4 (0)->ref == 1) 
        {
          delete (hbndseg4_GEO *)(*i).second;
          _hbnd4Map.erase (i++);
        } 
        else 
        {
          myBuilder ()._hbndseg4List.push_back ((hbndseg4_GEO *)(*i ++).second);
        }
      }
    }
    {
      typedef faceMap_t::iterator iterator;
      const iterator hbnd3MapEnd = _hbnd3Map.end ();
      for (faceMap_t::iterator i = _hbnd3Map.begin (); i != hbnd3MapEnd; )
      {
        if (((hbndseg3_GEO *)(*i).second)->myhface3 (0)->ref == 1) {
          delete (hbndseg3_GEO *)(*i).second;
          _hbnd3Map.erase (i++);
        } 
        else 
        {
          myBuilder ()._hbndseg3List.push_back ((hbndseg3_GEO *)(*i ++).second);
        }
      }
    }
    {
      typedef hbnd4intMap_t::iterator iterator;
      const iterator hbnd4IntEnd = _hbnd4Int.end ();
      for (hbnd4intMap_t::iterator i = _hbnd4Int.begin (); i != hbnd4IntEnd; ++i) 
      {
        const Hbnd4IntStorage & p = * ((*i).second);
        if (p.first()->ref == 1) 
        {
          hbndseg4_GEO * hb4 = 
             myBuilder ().insert_hbnd4 (p.first(), p.second(), 
                                        Gitter::hbndseg_STI::closure);
          myBuilder ()._hbndseg4List.push_back (hb4);
        }
        delete (*i).second;
      } 
    }

    // here the internal boundary elements are created 
    {
      typedef hbnd3intMap_t::iterator  iterator;
      const iterator hbnd3IntEnd = _hbnd3Int.end ();
      for (hbnd3intMap_t::iterator i = _hbnd3Int.begin (); i != hbnd3IntEnd; ++i) 
      {
        const Hbnd3IntStorage & p = * ((*i).second);
        if (p.first()->ref == 1) 
        {
          hbndseg3_GEO * hb3 = 
            myBuilder ().insert_hbnd3 (p.first(),p.second(), Gitter::hbndseg_STI::closure);    
          myBuilder ()._hbndseg3List.push_back (hb3);
        }
        delete (*i).second;
      }
    }
    {
      typedef faceMap_t::iterator iterator;
      const iterator face4MapEnd = _face4Map.end ();
      for (faceMap_t::iterator i = _face4Map.begin (); i != face4MapEnd; )
      if (!((hface4_GEO *)(*i).second)->ref) 
      {
        delete (hface4_GEO *)(*i).second;
        _face4Map.erase (i++);
      } 
      else 
      {
        alugrid_assert (((hface4_GEO *)(*i).second)->ref == 2);
        myBuilder ()._hface4List.push_back ((hface4_GEO *)(*i ++).second );
      }
    }
    {
      typedef faceMap_t::iterator iterator;
      const iterator face3MapEnd = _face3Map.end ();
      for (faceMap_t::iterator i = _face3Map.begin (); i != face3MapEnd; ) 
      {
        if (!((hface3_GEO *)(*i).second)->ref) 
        {
          delete (hface3_GEO *)(*i).second;
          _face3Map.erase (i++);
        } 
        else 
        {
          alugrid_assert (((hface3_GEO *)(*i).second)->ref == 2);
          myBuilder ()._hface3List.push_back ((hface3_GEO *)(*i ++).second );
        }
      }
    }
    {
      typedef edgeMap_t::iterator iterator;
      const iterator edgeMapEnd = _edgeMap.end ();
      for (edgeMap_t::iterator i = _edgeMap.begin (); i != edgeMapEnd; )
      {
        if (!(*i).second->ref) 
        {
          delete (*i).second;
          _edgeMap.erase (i++);
        } 
        else 
        {
          alugrid_assert ((*i).second->ref >= 1);
          myBuilder ()._hedge1List.push_back ((*i ++).second);
        }
      }
    }
    {
      typedef vertexMap_t::iterator  iterator;
      const iterator vertexMapEnd = _vertexMap.end ();
      for (vertexMap_t::iterator i = _vertexMap.begin (); i != vertexMapEnd; )
      {
        if (!(*i).second->ref) 
        {
          delete (*i).second;
          _vertexMap.erase (i++);
        } 
        else {
          alugrid_assert ((*i).second->ref >= 2);
          myBuilder ()._vertexList.push_back ((*i ++).second);
        }
      }
    }
    _finalized = true;
    return;
  }

  void MacroGridBuilder::inflateMacroGrid ( std::istream &rawInput )
  {
    const int start = clock ();
    {
      int nv = 0;
      rawInput >> nv;
      for (int i = 0; i < nv; i ++ ) {
        int id;
        double x, y, z;
        rawInput >> id >> x >> y >> z;
        InsertUniqueVertex (x,y,z,id);
      }
    }
    {
      int ne = 0;
      rawInput >> ne;
      for (int i = 0; i < ne; i ++ ) 
      {
        int elementType;
        rawInput >> elementType;
        switch (elementType) 
        {
        case HEXA_RAW :
          {
            int v [8];
            rawInput >> v [0] >> v [1] >> v [2] >> v [3] >> v [4] >> v [5] >> v [6] >> v [7];
            InsertUniqueHexa (v);
          }
          break;
        case TETRA_RAW :
          {
            int v [4];
            rawInput >> v [0] >> v [1] >> v [2] >> v [3];
            int orientation = i%2;
            InsertUniqueTetra (v, orientation );
          }
          break;
        case PERIODIC3_RAW :
          {
            int v [6];
            int bt;
            rawInput >> v [0] >> v [1] >> v [2] >> v [3] >> v [4] >> v [5] >> bt;
            if( !Gitter::hbndseg_STI::bndRangeCheck( bt ) )
            {
              std::cerr << "ERROR (fatal): Boundary id = " << bt << " out of range (valid are " << Gitter::hbndseg_STI::validRanges() << ")." << std::endl;
              abort();
            }
            Gitter::hbndseg::bnd_t btAbs = (Gitter::hbndseg::bnd_t)(std::abs(bt));
            Gitter::hbndseg::bnd_t bndId[ 2 ] = { btAbs, btAbs };
            InsertUniquePeriodic (v, bndId );
          }
          break;

        case PERIODIC4_RAW:
          {
            int v [8];
            int bt;
            rawInput >> v [0] >> v [1] >> v [2] >> v [3] >> v [4] >> v [5] >> v [6] >> v [7] >> bt;
            if( !Gitter::hbndseg_STI::bndRangeCheck( bt ) )
            {
              std::cerr << "ERROR (fatal): Boundary id = " << bt << " out of range (valid are " << Gitter::hbndseg_STI::validRanges() << ")." << std::endl;
              abort();
            }
            Gitter::hbndseg::bnd_t btAbs = (Gitter::hbndseg::bnd_t)(std::abs(bt));
            Gitter::hbndseg::bnd_t bndId[ 2 ] = { btAbs, btAbs };
            InsertUniquePeriodic (v, bndId );
          }
          break;

        default:
          std::cerr << "ERROR (fatal): Unknown ElementID in Rawformat File [" << elementType << "]." << std::endl;
          abort();
          break;
        }
      }
    }
    {
      int nb = 0;
      rawInput >> nb;
      for (int i = 0; i < nb; i ++) 
      {
        int polygonLen;
        rawInput >> polygonLen;
        if (polygonLen == 4) 
        {
          int bt, v [4];
          rawInput >> v [0] >> v [1] >> v [2] >> v [3] >> bt;
          if( ! ( Gitter::hbndseg_STI::bndRangeCheck(bt) ) )
          {
            std::cerr << "ERROR (fatal): Boundary id = " << bt << " out of range (valid are " << Gitter::hbndseg_STI::validRanges() << ")." << std::endl;
            abort();
          }
          InsertUniqueHbnd4 (v,(Gitter::hbndseg::bnd_t)(std::abs(bt)));
        } 
        else if (polygonLen == 3) 
        {
          int bt, v [3];
          rawInput >> v [0] >> v [1] >> v [2] >> bt;
          if( ! ( Gitter::hbndseg_STI::bndRangeCheck(bt) ) )
          {
            std::cerr << "ERROR (fatal): Boundary id = " << bt << " out of range (valid are " << Gitter::hbndseg_STI::validRanges() << ")." << std::endl;
            abort();
          }
          InsertUniqueHbnd3 (v,(Gitter::hbndseg::bnd_t)(std::abs(bt)));
        } 
        else 
        {
          std::cerr << "ERROR (fatal): Cannot create boundary segments with polygon length " << polygonLen << "." << std::endl;
          abort();
        }
      }
    }
    if( debugOption( 3 ) )
      std::cout << "INFO: MacroGridBuilder::inflateMacroGrid() used " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " s." << std::endl;
  }

  void Gitter::Geometric::BuilderIF::macrogridBuilder ( std::istream &in )
  {
    macrogridBuilderImpl( in );
  }

  void Gitter::Geometric::BuilderIF::macrogridBuilder (ObjectStream & in) 
  {
    // macrogridBuilderImpl( in );
    std::cerr << "ERROR (fatal): BuilderIF::macrogridBuilder not implemented for ObjectStream." << std::endl;
    abort();
  }

  template<class istream_t> 
  void Gitter::Geometric::BuilderIF::macrogridBuilderImpl (istream_t & in) 
  {
    std::stringstream raw;
    
    // set scientific mode and high precision 
    raw << std::scientific;
    raw.precision( ALUGridExternalParameters::precision() );

    MacroGridBuilder mm (*this);

    std::string firstline;
    std::getline( in, firstline ); 

    // check first character 
    if ( firstline[ 0 ] == char('!')) 
    {
      // Das erste Wort nach dem Kommentar steht jetzt in str.
      // Alle weiteren k"onnen noch aus is gelesen werden, das
      // array str ist so lang, wie die gesamte Zeile in 'buf'.
      if( (firstline.find( "Tetrahedra" ) != std::string::npos) || (firstline.find( "Tetraeder"  ) != std::string::npos) )
      {
        // Versuchen wir's mal mit Tetraedern
        MacroGridBuilder::generateRawTetraImage (in,raw);
      } 
      else if( (firstline.find( "Hexahedra" ) != std::string::npos) || (firstline.find( "Hexaeder"  ) != std::string::npos) )
      {
        // oder andernfalls mit Hexaedern.
        MacroGridBuilder::generateRawHexaImage (in,raw);
      } 
      else 
      {
        std::cerr << "WARNING (ignored): Unknown comment to file format (" << firstline << ")." << std::endl;
        return;
      }
    }
    else
    {
      std::cerr << "WARNING (ignored) No identifier for file format found. Trying to read as hexahedral grid." << std::endl;
      MacroGridBuilder::generateRawHexaImage( in,raw );
    }
    mm.inflateMacroGrid( raw );
  }

} // namespace ALUGrid
