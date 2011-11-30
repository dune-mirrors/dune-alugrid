// (c) bernhard schupp 1997 - 1998
// modifications for Dune Interface 
// (c) Robert Kloefkorn 2004 - 2005 

#include "gitter_sti.h"
#include "gitter_mgb.h"
  
pair < Gitter :: Geometric :: VertexGeo *, bool > MacroGridBuilder :: 
InsertUniqueVertex (double x, double y, double z, int i) {
  vertexMap_t :: const_iterator hit = _vertexMap.find (i) ;
  if (hit == _vertexMap.end ()) {
    VertexGeo * v = myBuilder ().insert_vertex (x,y,z,i) ;
    _vertexMap [i] = v ;
    return pair < VertexGeo *, bool > (v,true) ;
  } else {
    return pair < VertexGeo *, bool > ((*hit).second, false) ;
  }
}
 
pair < Gitter :: Geometric :: hedge1_GEO *, bool > MacroGridBuilder :: InsertUniqueHedge1 (int l, int r) {
  if (l > r) { 
    int i = l ; l = r ; r = i ;
  }
  edgeKey_t key (l,r) ;
  edgeMap_t :: const_iterator hit = _edgeMap.find (key) ;
  if (hit == _edgeMap.end ()) 
  {
    vertexMap_t :: const_iterator a = _vertexMap.find (l), b = _vertexMap.find (r);

    assert( a != _vertexMap.end() );
    assert( b != _vertexMap.end() );
    
    hedge1_GEO * h = myBuilder ().insert_hedge1 ((*a).second,(*b).second) ;
    _edgeMap [key] = h ;
    return pair < hedge1_GEO *, bool > (h,true) ;
  } else {
    return pair < hedge1_GEO *, bool > ((*hit).second,false) ;
  }
}

pair < Gitter :: Geometric :: hface3_GEO *, bool > MacroGridBuilder :: InsertUniqueHface3 (int (&v)[3]) {
  cyclicReorder (v,v+3) ;
  faceKey_t key (v[0],v[1],v[2]) ;
  faceMap_t :: const_iterator hit = _face3Map.find (key) ;
  if (hit == _face3Map.end ()) {
    hedge1_GEO * edge [3] ;
    int dire [3] = { 0, 0, 1 } ;
    edge [0] = InsertUniqueHedge1 (v[0],v[1]).first ;
    edge [1] = InsertUniqueHedge1 (v[1],v[2]).first ;
    edge [2] = InsertUniqueHedge1 (v[2],v[0]).first ;
    hface3_GEO * f3 = myBuilder ().insert_hface3 (edge,dire) ;
    _face3Map [key] = f3 ;
    return pair < hface3_GEO *, bool > (f3,true) ;
  } else {
    return pair < hface3_GEO *, bool > ((hface3_GEO *)(*hit).second,false) ;
  }
}

pair < Gitter :: Geometric :: hface4_GEO *, bool > MacroGridBuilder :: InsertUniqueHface4 (int (&v)[4]) {
  cyclicReorder (v,v+4) ;
  faceKey_t key (v[0],v[1],v[2]) ;
  faceMap_t :: const_iterator hit = _face4Map.find (key) ;
  if (hit == _face4Map.end ()) {
    hedge1_GEO * edge [4] ;
    int dire [4]; 
    edge [0] = InsertUniqueHedge1 (v[0],v[1]).first ;
    edge [1] = InsertUniqueHedge1 (v[1],v[2]).first ;
    edge [2] = InsertUniqueHedge1 (v[2],v[3]).first ;
    edge [3] = InsertUniqueHedge1 (v[3],v[0]).first ;  
    dire [0] = v[0] < v[1] ? 0 : 1 ;
    dire [1] = v[1] < v[2] ? 0 : 1 ;
    dire [2] = v[2] < v[3] ? 0 : 1 ;
    dire [3] = v[3] < v[0] ? 0 : 1 ;
    hface4_GEO * f4 = myBuilder ().insert_hface4 (edge,dire) ;
    _face4Map [key] = f4 ;
    return pair < hface4_GEO *, bool > (f4,true) ;
  } else {
    return pair < hface4_GEO *, bool > ((hface4_GEO *)(*hit).second,false) ;
  }
}

pair < Gitter :: Geometric :: tetra_GEO *, bool > MacroGridBuilder :: 
InsertUniqueTetra (int (&v)[4]) 
{
  elementKey_t key (v [0], v [1], v [2], v [3]) ;
  elementMap_t :: const_iterator hit = _tetraMap.find (key) ;
  if (hit == _tetraMap.end ()) {
    hface3_GEO * face [4] ;
    int twst [4] ;
    for (int fce = 0 ; fce < 4 ; fce ++ ) {
      int x [3] ;
      x [0] = v [Tetra :: prototype [fce][0]] ;
      x [1] = v [Tetra :: prototype [fce][1]] ;
      x [2] = v [Tetra :: prototype [fce][2]] ;
      twst [fce] = cyclicReorder (x,x+3) ;
      face [fce] =  InsertUniqueHface3 (x).first ;
    }
    tetra_GEO * t = myBuilder ().insert_tetra (face,twst) ;
    assert (t) ;
    _tetraMap [key] = t ;
    return pair < tetra_GEO *, bool > (t,true) ;
  } else {
    return pair < tetra_GEO *, bool > ((tetra_GEO *)(*hit).second,false) ;
  }
}

pair < Gitter :: Geometric :: hexa_GEO *, bool > MacroGridBuilder :: InsertUniqueHexa (int (&v)[8]) 
{
  elementKey_t key (v [0], v [1], v [3], v[4]) ;
  elementMap_t :: const_iterator hit = _hexaMap.find (key) ;
  if (hit == _hexaMap.end ()) {
    hface4_GEO * face [6] ;
    int twst [6] ;
    for (int fce = 0 ; fce < 6 ; fce ++) {
      int x [4] ;
      x [0] = v [Hexa :: prototype [fce][0]] ;
      x [1] = v [Hexa :: prototype [fce][1]] ;
      x [2] = v [Hexa :: prototype [fce][2]] ;
      x [3] = v [Hexa :: prototype [fce][3]] ;
      twst [fce] = cyclicReorder (x,x+4) ;
      face [fce] =  InsertUniqueHface4 (x).first ;
    }
    hexa_GEO * hx = myBuilder ().insert_hexa (face,twst) ;
    _hexaMap [key] = hx ;
    return pair < hexa_GEO *, bool > (hx,true) ;
  } else {
    return pair < hexa_GEO *, bool > ((hexa_GEO *)(*hit).second,false) ;
  }
}

bool MacroGridBuilder :: InsertUniqueHbnd3 (int (&v)[3],Gitter :: hbndseg_STI ::bnd_t bt) 
{
  int twst = cyclicReorder (v,v+3) ;
  faceKey_t key (v [0], v [1], v [2]) ;
  if (bt == Gitter :: hbndseg_STI :: closure) 
  {
    if (_hbnd3Int.find (key) == _hbnd3Int.end ()) {
      hface3_GEO * face =  InsertUniqueHface3 (v).first ;
      _hbnd3Int [key] = new Hbnd3IntStorage (face,twst) ;
      return true ;
    }
  } 
  else 
  {
    if (_hbnd3Map.find (key) == _hbnd3Map.end ()) {
      hface3_GEO * face  = InsertUniqueHface3 (v).first ;
      hbndseg3_GEO * hb3 = myBuilder ().insert_hbnd3 (face,twst,bt) ;
      _hbnd3Map [key] = hb3 ;
      return true ;
    }
  }
  return false ;
}

bool MacroGridBuilder :: InsertUniqueHbnd4 (int (&v)[4], Gitter :: hbndseg_STI ::bnd_t bt) {
  int twst = cyclicReorder (v,v+4) ;
  faceKey_t key (v [0], v [1], v [2]) ;
  if (bt == Gitter :: hbndseg_STI :: closure) 
  {
    if (_hbnd4Int.find (key) == _hbnd4Int.end ()) {
      hface4_GEO * face =  InsertUniqueHface4 (v).first ;
      _hbnd4Int [key] = new Hbnd4IntStorage (face,twst) ;
      return true ;
    }
  } 
  else 
  {
    if (_hbnd4Map.find (key) == _hbnd4Map.end ()) 
    {
      hface4_GEO * face =  InsertUniqueHface4 (v).first ;
      hbndseg4_GEO * hb4 = myBuilder ().insert_hbnd4 (face,twst,bt) ;
      _hbnd4Map [key] = hb4 ;
      return true ;
    }
  }
  return false ;
}

pair < Gitter :: Geometric :: periodic3_GEO *, bool > MacroGridBuilder :: 
InsertUniquePeriodic3 (int (&v)[6], const Gitter :: hbndseg_STI ::bnd_t (&bt)[2] )  
{

  // Vorsicht: Der Schl"ussel f"ur das periodische Randelement wird
  // dummerweise mit dem eines Hexaeders verwechselt, falls nicht
  // der letzte Knoten negativ (mit umgekehrtem Vorzeichen) in die
  // Schl"ussel eingef"ugt wird.

  elementKey_t key (v [0], v [1], v [2], -(v [3])-1) ;
  elementMap_t :: const_iterator hit = _periodic3Map.find (key) ;
  if (hit == _periodic3Map.end ()) {
    hface3_GEO * face [2] ;
    int twst [2] ;
    for (int fce = 0 ; fce < 2 ; fce ++ ) {
      int x [3] ;
      x [0] = v [Periodic3 :: prototype [fce][0]] ;
      x [1] = v [Periodic3 :: prototype [fce][1]] ;
      x [2] = v [Periodic3 :: prototype [fce][2]] ;
      twst [fce] = cyclicReorder (x,x+3) ;
      face [fce] = InsertUniqueHface3 (x).first ;
    }
    periodic3_GEO * t = myBuilder ().insert_periodic3 (face,twst,bt) ;
    assert (t) ;
    _periodic3Map [key] = t ;
    return pair < periodic3_GEO *, bool > (t,true) ;
  } else {
    return pair < periodic3_GEO *, bool > ((periodic3_GEO *)(*hit).second,false) ;
  }
}

pair < Gitter :: Geometric :: periodic4_GEO *, bool > MacroGridBuilder :: 
InsertUniquePeriodic4 (int (&v)[8], const Gitter :: hbndseg_STI ::bnd_t (&bt)[2] ) 
{

  // Vorsicht: Der Schl"ussel f"ur das periodische Randelement wird
  // dummerweise mit dem eines Hexaeders verwechselt, falls nicht
  // der letzte Knoten negativ (mit umgekehrtem Vorzeichen) in die
  // Schl"ussel eingef"ugt wird.

  elementKey_t key (v [0], v [1], v [3], -(v [4])-1) ;
  elementMap_t :: const_iterator hit = _periodic4Map.find (key) ;
  if (hit == _periodic4Map.end ()) {
    hface4_GEO * face [2] ;
    int twst [2] ;
    for (int fce = 0 ; fce < 2 ; fce ++ ) 
    {
      int x [4] ;
      x [0] = v [Periodic4 :: prototype [fce][0]] ;
      x [1] = v [Periodic4 :: prototype [fce][1]] ;
      x [2] = v [Periodic4 :: prototype [fce][2]] ;
      x [3] = v [Periodic4 :: prototype [fce][3]] ;
      twst [fce] = cyclicReorder (x,x+4) ;
      face [fce] = InsertUniqueHface4 (x).first ;
    }
    periodic4_GEO * t = myBuilder ().insert_periodic4 (face,twst,bt) ;
    assert (t) ;
    _periodic4Map [key] = t ;
    return pair < periodic4_GEO *, bool > (t,true) ;
  } 
  else 
  {
    return pair < periodic4_GEO *, bool > ((periodic4_GEO *)(*hit).second,false) ;
  }
}
// Ende - Neu am 23.5.02 (BS)

void MacroGridBuilder :: removeElement (const elementKey_t & k, const bool realElement ) 
{
  // Der Schl"ussel sollte nur in genau einer Map vorliegen.

  assert ((_hexaMap.find (k) == _hexaMap.end () ? 0 : 1)
        + (_tetraMap.find(k) == _tetraMap.end () ? 0 : 1)
        + (_periodic3Map.find (k) == _periodic3Map.end () ? 0 : 1)
        + (_periodic4Map.find (k) == _periodic4Map.end () ? 0 : 1) == 1) ;

  if( realElement ) 
  {
    elementMap_t :: iterator hit = _tetraMap.find (k) ;
    if (hit != _tetraMap.end ()) 
    {
      tetra_GEO * tr = (tetra_GEO *)(*hit).second ;
      for (int i = 0 ; i < 4 ; ++i) 
      {
        hface3_GEO* face = tr->myhface3 (i) ;
        _hbnd3Int [faceKey_t (face->myvertex (0)->ident (), 
                              face->myvertex (1)->ident (), 
                              face->myvertex (2)->ident ())] 
          = new Hbnd3IntStorage (face, tr->twist (i), tr , i ) ;
      }

      delete tr ;
      _tetraMap.erase (hit) ;

      return ;
    }

    hit = _hexaMap.find (k) ;
    if (hit != _hexaMap.end ()) 
    {
      hexa_GEO * hx = (hexa_GEO *)(*hit).second ;
      for (int i = 0 ; i < 6 ; ++i) 
      {
        hface4_GEO* face = hx->myhface4 (i);
        _hbnd4Int [faceKey_t (face->myvertex (0)->ident (), 
                              face->myvertex (1)->ident (), 
                              face->myvertex (2)->ident ())
                  ] = new Hbnd4IntStorage ( face, hx->twist (i), hx, i );
      }

      delete hx ;
      _hexaMap.erase (hit) ;

      return ;
    }
  }
  else 
  {
    elementMap_t :: iterator hit = _periodic3Map.find (k) ;
    if (hit != _periodic3Map.end ()) 
    {
      periodic3_GEO * p3 = (periodic3_GEO *)(*hit).second ;
      for (int i = 0 ; i < 2 ; ++i) 
      {
        hface3_GEO* face = p3->myhface3 (i);
        _hbnd3Int [faceKey_t ( face->myvertex (0)->ident (), 
                               face->myvertex (1)->ident (), 
                               face->myvertex (2)->ident ())
                  ] = new Hbnd3IntStorage ( face, p3->twist (i)) ;
      }

      delete p3 ;
      _periodic3Map.erase (hit) ;
      return ;
    }

    hit = _periodic4Map.find (k) ;
    if (hit != _periodic4Map.end ()) 
    {
      periodic4_GEO * p4 = (periodic4_GEO *)(*hit).second ;
      for (int i = 0 ; i < 2 ; ++i) 
      {
        hface4_GEO* face = p4->myhface4 (i);
        _hbnd4Int [faceKey_t ( face->myvertex (0)->ident (), 
                               face->myvertex (1)->ident (), 
                               face->myvertex (2)->ident ())
                  ] = new Hbnd4IntStorage ( face, p4->twist (i));
      }

      delete p4 ;
      _periodic4Map.erase (hit) ;

      return ;
    }
  }

  abort () ;
  return ;
}

void MacroGridBuilder :: cubeHexaGrid (int n, ostream & out) {

  // cubeHexaGrid () ist eine statische Methode, die einen ASCII Strom
  // mit einem gleichm"assigen Hexaedernetz auf dem Einheitsw"urfel
  // [0,1]^3 beschreibt, wobei <n> die Aufl"osung der Raumrichtungen
  // vorgibt: Es entstehen n^3 Hexaederelemente, und (n+1)^3 Knoten.
  // Es ist als Servicemethode f"ur die 'ball' und 'ball_pll' Test-
  // programme n"otig und deshalb in der MacrogridBuilder Klasse 
  // beheimatet.

  const int bndtype = -1 ;
  out.setf(ios::fixed, ios::floatfield) ;
  out.precision (14) ;
  n = n < 0 ? 0 : n ;
  int npe = n + 1 ;
  out << (npe * npe * npe) << endl ;
  double delta = 1.0 / (double)(n) ;
  {
    for(int i = 0 ; i < npe ; i ++) {
      for(int j = 0 ; j < npe ; j ++) {
        for(int k = 0 ; k < npe ; k ++) {
          out << double(k * delta) << "  "  << double(j * delta) << "  " << double(i * delta) << "\n" ;
        }
      }
    }
  }
  out << n * n * n << "\n" ;
  {
    for(int i = 0 ; i < n ; i ++) {
      int ipea = (i + 1) * npe * npe, ia = i * npe * npe ;
      for(int j = 0 ; j < n ; j ++) {
        int jpea = (j + 1) * npe, ja = j * npe ;  
        for(int k = 0 ; k < n ; k ++) {
          int kpe = k + 1 ;  
          out << k + ia + ja << "  " << kpe + ia + ja << "  "  << kpe + ia + jpea << "  " << k + ia + jpea << "  "         
              << k + ja + ipea << "  " << kpe + ja + ipea << "  " << kpe + ipea + jpea << "  " << k + ipea + jpea << "\n" ; 
        }  
      }    
    }
    out << endl ;  
  }
  out << 6 * n * n << endl ;
  { // unten und oben
    int l = n * npe * npe ;
    for(int j = 0 ; j < n ; j ++) {
      int jpea = (j + 1) * npe, ja = j * npe ;
      for(int k = 0 ; k < n ; k ++) {
        int kpe = k + 1 ;
        out << bndtype << "  " << 4 << "  " << (kpe + ja) << "  " << (kpe + jpea) << "  "
            << (k + jpea) << "  " << (k + ja) << "\n" << bndtype << "  " << 4 << "  "
            << (k + jpea + l) << "  " << (kpe + jpea + l) << "  " << (kpe + ja + l) << "  " << (k + ja + l) << "\n" ;
      }
    }
    out << endl ;
  }
  { // links und rechts
    int l = n * npe ;
    for(int j = 0 ; j < n ; j ++) {
      int jpea = (j + 1) * npe * npe, ja = j * npe * npe ;
      for(int ka = 0 ; ka < n ; ka ++) {
        int kpea = (ka + 1) ;
        out << bndtype << "  " << 4 << "  " << ka + jpea << "  " << kpea + jpea << "  " 
            << kpea + ja << "  " << ka + ja << "\n" << bndtype << "  " << 4 << "  " 
      << kpea + ja + l << "  " << kpea + jpea + l << "  " << ka + jpea + l << "  " << ka + ja + l << "\n" ;
      }
    }
    out << endl ;
  }
  { // hinten und vorne
    int l = n ;
    for(int j = 0 ; j < n ; j ++) {
      int jpea = (j + 1) * npe * npe, ja = j * npe * npe ;
      for(int k = 0 ; k < n ; k ++) {
        int kpea = (k + 1) * npe, ka = k * npe ;
        out << bndtype << "  " << 4 << "  " << kpea + ja << "  " << kpea + jpea << "  " 
            << ka + jpea << "  " << ka + ja << "\n" << bndtype << "  " << 4 << "  " 
            << ka + jpea + l << "  " << kpea + jpea + l << "  " << kpea + ja + l << "  " << ka + ja + l << "\n" ; 
      }
    }
    out << endl ;
  }
  return ;
}

void MacroGridBuilder :: generateRawHexaImage (istream & in, ostream & os) {

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
  
  const int start = clock () ;
  int nv = 0, ne = 0, nb = 0, nper = 0 ;
  int (* vnum)[8] = 0, (* bvec)[5] = 0, (* pervec)[9] = 0, * pident = 0 ;
  double (* coord)[3] = 0 ;
  {
    in >> nv ;
    coord = new double [nv][3] ;
    assert (coord) ;
    for (int i = 0 ; i < nv ; i ++) in >> coord [i][0] >> coord [i][1] >> coord [i][2] ;
  }
  {
    in >> ne ;
    vnum = new int [ne][8] ;
    assert (vnum) ;
    for (int i = 0 ; i < ne ; i ++ )
      in >> vnum [i][0] >> vnum [i][1] >> vnum [i][2] >> vnum [i][3] >> vnum [i][4] >> vnum [i][5] >> vnum [i][6] >> vnum [i][7] ;
  }
  
  {
    int temp_nb;
    in >> temp_nb ;
    bvec = new int [temp_nb][5] ;
    pervec = new int [temp_nb][9] ;
    assert (bvec);
    assert (pervec);
    for (int i = 0 ; i < temp_nb ; i ++) 
    {
      int n ;
      int identification ;
      in >> identification >> n;
      if (n == 4) 
      {
        in >> bvec [nb][0] >> bvec [nb][1] >> bvec [nb][2] >> bvec [nb][3] ;
        bvec [nb][4] = identification ;
        nb++; 
      } 
      else if (n == 8) 
      {
        //if( std::abs(identification) != Gitter :: hbndseg_STI :: periodic ) 
        //  cerr << "WARNING: ignoring boundary id " << identification << " for periodic boundaries!" << endl;

        in >> pervec [nper][0] >> pervec [nper][1] >> pervec [nper][2] >> pervec [nper][3]
           >> pervec [nper][4] >> pervec [nper][5] >> pervec [nper][6] >> pervec [nper][7] ;

        // keep boundary information 
        pervec [nper][8] = identification ;
        nper++;
      }
      else {
        cerr << "**FEHLER (FATAL):  "__FILE__ << " " << __LINE__ << " ... Exiting." << endl ;
        abort();
      }
    }
  }
  
  if (! in.good ()) {
    cerr << "**FEHLER (FATAL): Dateiende zu fr\"uh erreicht (inkonsistente Datei). " ;
    cerr << __FILE__ << " " << __LINE__ << " ... Exiting." << endl ;
    exit (1) ;
  }
  pident = new int [nv] ;
  {
    int dummy ;
    for (int i = 0 ; i < nv ; i ++ ) in >> pident [i] >> dummy ; 
  }
  if (!in.good()) {
    cerr << "**WARNUNG (IGNORIERT) MacroGridBuilder :: generateRawHexaImage () " ;
    cerr << "Identifierliste unvollst\"andig oder nicht vorhanden. Daher keine parallele " ;
    cerr << "Identifikation falls aus mehreren Gittern geladen wurde." << endl ;
    for (int i = 0 ; i < nv ; i ++ ) pident [i] = i ;
  }
  {
    os << nv << endl ;
    for (int i = 0 ; i < nv ; i ++ )
      os << pident [i] << " " << coord [i][0] << " " << coord [i][1] << " " << coord [i][2] << endl ;
  }
  {
    os << (ne + nper) << endl ;
    for (int i = 0 ; i < ne ; i ++)
      os << HEXA_RAW << " " << pident [vnum [i][0]] << " " << pident [vnum [i][1]] << " "
         << pident [vnum [i][2]] << " " << pident [vnum [i][3]] << " " << pident [vnum [i][4]] << " " 
         << pident [vnum [i][5]] << " " << pident [vnum [i][6]] << " " << pident [vnum [i][7]] << endl ;
  }
  {
    for (int i = 0 ; i < nper ; i ++)
    {
      os << PERIODIC4_RAW << " " << pident [pervec [i][0]] << " " << pident [pervec [i][1]] << " "
         << pident [pervec [i][2]] << " " << pident [pervec [i][3]] << " "
         << pident [pervec [i][4]] << " " << pident [pervec [i][5]] << " " 
         << pident [pervec [i][6]] << " " << pident [pervec [i][7]] << " " << pervec [i][8] << endl ;
    }
  }
  {
    os << nb << endl ;
    for (int i = 0 ; i < nb ; i ++)
      os << 4 << " " << pident [bvec [i][0]] << " " << pident [bvec [i][1]] << " "
         << pident [bvec [i][2]] << " " << pident [bvec [i][3]] << " " << bvec [i][4] << endl ;
  }
  delete [] vnum ;
  delete [] coord ;
  delete [] pervec ;
  delete [] bvec ;
  delete [] pident ;
  if (debugOption (4))
    cout << "**INFO MacroGridBuilder :: generateRawHexaImage () used: " 
         << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec." << endl ;
  return ;
}

void MacroGridBuilder :: generateRawTetraImage (istream & in, ostream & os) {
  const int start = clock () ;
  int nv = 0, ne = 0, nb = 0, nper = 0;
  int (* vnum)[4] = 0, (* bvec)[4] = 0, (* pervec)[7] = 0 ,
      * pident = 0 ;
  double (* coord)[3] = 0 ;
  {
    in >> nv ;
    coord = new double [nv][3] ;
    assert (coord) ;
    for (int i = 0 ; i < nv ; i ++) in >> coord [i][0] >> coord [i][1] >> coord [i][2] ;
  }
  {
    in >> ne ;
    vnum = new int [ne][4] ;
    assert (vnum) ;
    for (int i = 0 ; i < ne ; i ++ )
      in >> vnum [i][0] >> vnum [i][1] >> vnum [i][2] >> vnum [i][3] ;
  }
  {
    int temp_nb;
    in >> temp_nb ;
    bvec = new int [temp_nb][4] ;
    pervec = new int [temp_nb][7] ;
    assert (bvec);
    assert (pervec);
    for (int i = 0 ; i < temp_nb ; i ++) 
    {
      int n ;
      int identification ;
      in >> identification >> n;
      if (n==3) {
        in >> bvec [nb][0] >> bvec [nb][1] >> bvec [nb][2] ;
        bvec [nb][3] = identification ;
        nb++; 
      } 
      else if (n == 6) 
      {
        //if( std::abs(identification) != Gitter :: hbndseg_STI :: periodic ) 
        //  cerr << "WARNING: ignoring boundary id " << identification << " for periodic boundaries!" << endl;

        in >> pervec [nper][0] >> pervec [nper][1] >> pervec [nper][2] >>
              pervec [nper][3] >> pervec [nper][4] >> pervec [nper][5];

        pervec [nper][6] = identification ;
        nper++;
      }
      else 
        abort();
    }
  }
  if (! in.good ()) {
    cerr << "**FEHLER (FATAL): Dateiende zu fr\"uh erreicht (inkonsistente Datei). " 
         << __FILE__ << " " << __LINE__ << " ... Exiting." << endl ;
    exit (1) ;
  }
  pident = new int [nv] ;
  assert(pident);
  {
    int dummy ;
    for (int i = 0 ; i < nv ; i ++ ) in >> pident [i] >> dummy ; 
  }
  if (!in.good()) {
    cerr << "**WARNING (ignored) MacroGridBuilder :: generateRawTetraImage () -- " ;
    cerr << "identifier list incomplete or not available. Therefore no parallel " ;
    cerr << "identification possible if macro grid restored from more than one Grid." << endl ;
    for (int i = 0 ; i < nv ; i ++ ) pident [i] = i ;
  }
  {
    os << nv << endl ;
    for (int i = 0 ; i < nv ; i ++ )
      os << pident [i] << " " << coord [i][0] << " " << coord [i][1] << " " << coord [i][2] << endl ;
  }
  {
    int i;
    os << ne+nper << endl ;
    for (i = 0 ; i < ne ; i ++)
      os << TETRA_RAW << " " << pident [vnum [i][0]] << " " << pident [vnum [i][1]] << " "
         << pident [vnum [i][2]] << " " << pident [vnum [i][3]] << endl ;
    for (i = 0 ; i < nper ; i ++)
      os << PERIODIC3_RAW << " " << pident [pervec [i][0]] << " " << pident [pervec [i][1]] << " "
         << pident [pervec [i][2]] << " " << pident [pervec [i][3]] << " "
         << pident [pervec [i][4]] << " " << pident [pervec [i][5]] << " " << pervec [i][6] << endl ;
  }
  {
    os << nb << endl ;
    for (int i = 0 ; i < nb ; i ++)
      os << 3 << " " << pident [bvec [i][0]] << " " << pident [bvec [i][1]] << " "
         << pident [bvec [i][2]] << " " << " " << bvec [i][3] << endl ;
  }
  delete [] vnum ;
  delete [] coord ;
  delete [] bvec ;
  delete [] pervec ;
  delete [] pident ;
  if (debugOption (4))
    cout << "**INFO MacroGridBuilder :: generateRawTetraImage () used: " << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec." << endl ;
  return ;
}

// default of init == true
MacroGridBuilder :: MacroGridBuilder (BuilderIF & b, const bool init) 
 : _initialized(false) 
 , _finalized(false) 
 , _mgb (b) 
{
  if(init) initialize();
}

// deprecated constructor, project vertex has been removed 
MacroGridBuilder :: MacroGridBuilder (BuilderIF & b, ProjectVertex* ) 
 : _initialized(false) 
 , _finalized(false) 
 , _mgb (b) 
{
  initialize();
}

void MacroGridBuilder :: initialize () 
{
  {
    for ( BuilderIF :: vertexlist_t :: iterator i = myBuilder ()._vertexList.begin () ;
      i != myBuilder ()._vertexList.end () ; myBuilder ()._vertexList.erase (i ++)) 
        _vertexMap [(*i)->ident ()] = (*i) ;
  }
  {
    for ( BuilderIF :: hedge1list_t :: iterator i = myBuilder ()._hedge1List.begin () ;
      i != myBuilder ()._hedge1List.end () ; myBuilder ()._hedge1List.erase (i ++)) {
      long k = (*i)->myvertex (0)->ident (), l = (*i)->myvertex (1)->ident () ;
      _edgeMap [edgeKey_t (k < l ? k : l, k < l ? l : k)] = (*i) ;
    }
  }
  {for ( BuilderIF :: hface3list_t :: iterator i = myBuilder ()._hface3List.begin () ; i != myBuilder ()._hface3List.end () ;
     myBuilder ()._hface3List.erase (i ++)) {
      _face3Map [faceKey_t ((*i)->myvertex (0)->ident (),(*i)->myvertex (1)->ident (), (*i)->myvertex (2)->ident ())] = (*i) ;
  }}
  {
    for ( BuilderIF :: hface4list_t :: iterator i = myBuilder ()._hface4List.begin () ; i != myBuilder ()._hface4List.end () ; 
      myBuilder ()._hface4List.erase (i ++)) _face4Map [faceKey_t ((*i)->myvertex (0)->ident (),(*i)->myvertex (1)->ident (),
        (*i)->myvertex (2)->ident ())] = (*i) ;
  }
  {for ( BuilderIF :: hbndseg4list_t :: iterator i = myBuilder ()._hbndseg4List.begin () ; i != myBuilder ()._hbndseg4List.end () ; myBuilder ()._hbndseg4List.erase (i++)) {
    faceKey_t key ((*i)->myhface4 (0)->myvertex (0)->ident (), (*i)->myhface4 (0)->myvertex (1)->ident (), (*i)->myhface4 (0)->myvertex (2)->ident ()) ;
    if ((*i)->bndtype () == Gitter :: hbndseg_STI :: closure) {
      _hbnd4Int [key] = new Hbnd4IntStorage ((*i)->myhface4 (0),(*i)->twist (0)) ;
      delete (*i) ;
    } else {
      _hbnd4Map [key] = (*i) ;
    }
  }}
  {for ( BuilderIF :: hbndseg3list_t :: iterator i = myBuilder ()._hbndseg3List.begin () ; i != myBuilder ()._hbndseg3List.end () ;
    myBuilder ()._hbndseg3List.erase (i++)) {
    faceKey_t key ((*i)->myhface3 (0)->myvertex (0)->ident (), (*i)->myhface3 (0)->myvertex (1)->ident (), (*i)->myhface3 (0)->myvertex (2)->ident ()) ;
    if ((*i)->bndtype () == Gitter :: hbndseg_STI :: closure) 
    {
      _hbnd3Int [key] = new Hbnd3IntStorage ((*i)->myhface3 (0), (*i)->twist (0)) ;
      delete (*i) ;
    } else {
      _hbnd3Map [key] = (*i) ;
    }
  }}
  {for ( BuilderIF :: tetralist_t :: iterator i = myBuilder ()._tetraList.begin () ; i != myBuilder ()._tetraList.end () ; 
      myBuilder ()._tetraList.erase (i++)) {
      _tetraMap [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
           (*i)->myvertex (2)->ident (), (*i)->myvertex (3)->ident ())] = (*i) ;
  }}
  {for (BuilderIF :: periodic3list_t :: iterator i = myBuilder ()._periodic3List.begin () ; i != myBuilder ()._periodic3List.end () ; 
      myBuilder ()._periodic3List.erase (i++)) {
      _periodic3Map [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
           (*i)->myvertex (2)->ident (), -((*i)->myvertex (3)->ident ())-1)] = (*i) ;
  }}
  {for (BuilderIF :: periodic4list_t :: iterator i = myBuilder ()._periodic4List.begin () ; i != myBuilder ()._periodic4List.end () ; 
      myBuilder ()._periodic4List.erase (i++)) {
      _periodic4Map [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
           (*i)->myvertex (3)->ident (), -((*i)->myvertex (4)->ident ())-1)] = (*i) ;
  }}
  {
    for (BuilderIF :: hexalist_t :: iterator i = myBuilder ()._hexaList.begin () ; i != myBuilder ()._hexaList.end () ; 
      myBuilder ()._hexaList.erase (i++)) _hexaMap [elementKey_t ((*i)->myvertex (0)->ident (), (*i)->myvertex (1)->ident (), 
                  (*i)->myvertex (3)->ident (), (*i)->myvertex (4)->ident ())] = (*i) ;
  }

  _initialized = true;
  return ; 
}

MacroGridBuilder :: ~MacroGridBuilder () 
{
  // _finalized is true if the method was called in inherited classes 
  if(!_finalized) finalize();
}

// clean the map tables 
void MacroGridBuilder :: finalize () 
{
  assert(_initialized);
  
  {
    //myBuilder ()._hexaList.reserve(_hexaMap.size());
    for (elementMap_t :: iterator i = _hexaMap.begin () ; i != _hexaMap.end () ; _hexaMap.erase (i++))
      myBuilder ()._hexaList.push_back ((hexa_GEO *)(*i).second) ;
  }
  {
    //myBuilder ()._tetraList.reserve(_tetraMap.size());
    for (elementMap_t :: iterator i = _tetraMap.begin () ; i != _tetraMap.end () ; _tetraMap.erase (i++))
      myBuilder ()._tetraList.push_back ((tetra_GEO *)(*i).second) ;
  }
  {
    //myBuilder ()._periodic3List.reserve(_periodic3Map.size());
    for (elementMap_t :: iterator i = _periodic3Map.begin () ; i != _periodic3Map.end () ; _periodic3Map.erase (i++))
      myBuilder ()._periodic3List.push_back ((periodic3_GEO *)(*i).second) ;
  }
  
  {
    //myBuilder ()._periodic4List.reserve(_periodic4Map.size());
    for (elementMap_t :: iterator i = _periodic4Map.begin () ; i != _periodic4Map.end () ; _periodic4Map.erase (i++))
      myBuilder ()._periodic4List.push_back ((periodic4_GEO *)(*i).second) ;
  }

  {
    //myBuilder ()._hbndseg4List.reserve(_hbnd4Map.size() + _hbnd4Int.size());
    for (faceMap_t :: iterator i = _hbnd4Map.begin () ; i != _hbnd4Map.end () ; )
    {
      if (((hbndseg4_GEO *)(*i).second)->myhface4 (0)->ref == 1) 
      {
        delete (hbndseg4_GEO *)(*i).second ;
        _hbnd4Map.erase (i++) ;
      } 
      else 
      {
        myBuilder ()._hbndseg4List.push_back ((hbndseg4_GEO *)(*i ++).second) ;
      }
    }
  }
  {
    //myBuilder ()._hbndseg3List.reserve(_hbnd3Map.size() + _hbnd3Int.size());
    for (faceMap_t :: iterator i = _hbnd3Map.begin () ; i != _hbnd3Map.end () ; )
    {
      if (((hbndseg3_GEO *)(*i).second)->myhface3 (0)->ref == 1) {
        delete (hbndseg3_GEO *)(*i).second ;
        _hbnd3Map.erase (i++) ;
      } 
      else 
      {
        myBuilder ()._hbndseg3List.push_back ((hbndseg3_GEO *)(*i ++).second) ;
      }
    }
  }
  {
    for (hbnd4intMap_t :: iterator i = _hbnd4Int.begin () ; i != _hbnd4Int.end () ; i ++) {
    const Hbnd4IntStorage & p = * ((*i).second);
    if (p.first()->ref == 1) {
      hbndseg4_GEO * hb4 = 
        myBuilder ().insert_hbnd4 (p.first(), p.second(), 
                                   Gitter :: hbndseg_STI :: closure) ;
      myBuilder ()._hbndseg4List.push_back (hb4) ;
    }
    delete (*i).second;
  }}

  // here the internal boundary elements are created 
  {for (hbnd3intMap_t :: iterator i = _hbnd3Int.begin () ; i != _hbnd3Int.end () ; i ++) {
    const Hbnd3IntStorage & p = * ((*i).second);
    if (p.first()->ref == 1) {
      hbndseg3_GEO * hb3 = 
        myBuilder ().insert_hbnd3 (p.first(),p.second(), Gitter :: hbndseg_STI :: closure) ;    
      myBuilder ()._hbndseg3List.push_back (hb3) ;
    }
    delete (*i).second;
  }}
  {
    //myBuilder ()._hface4List.reserve( _face4Map.size() );
    for (faceMap_t :: iterator i = _face4Map.begin () ; i != _face4Map.end () ; )
    if (!((hface4_GEO *)(*i).second)->ref) {
      delete (hface4_GEO *)(*i).second ;
      _face4Map.erase (i++) ;
    } else {
      assert (((hface4_GEO *)(*i).second)->ref == 2) ;
      myBuilder ()._hface4List.push_back ((hface4_GEO *)(*i ++).second ) ;
    }
  }
  {
    //myBuilder ()._hface3List.reserve( _face3Map.size() );
    for (faceMap_t :: iterator i = _face3Map.begin () ; i != _face3Map.end () ; ) {
    if (!((hface3_GEO *)(*i).second)->ref) {
      delete (hface3_GEO *)(*i).second ;
      _face3Map.erase (i++) ;
    } else 
    {
      assert (((hface3_GEO *)(*i).second)->ref == 2) ;
      myBuilder ()._hface3List.push_back ((hface3_GEO *)(*i ++).second ) ;
    }
  }}
  {
    //myBuilder ()._hedge1List.reserve(_edgeMap.size());
    for (edgeMap_t :: iterator i = _edgeMap.begin () ; i != _edgeMap.end () ; )
    if (!(*i).second->ref) {
      delete (*i).second ;
      _edgeMap.erase (i++) ;
    } else {
      assert ((*i).second->ref >= 1) ;
      myBuilder ()._hedge1List.push_back ((*i ++).second) ;
    }
  }
  {
    //myBuilder ()._vertexList.reserve(_vertexMap.size());
    for (vertexMap_t :: iterator i = _vertexMap.begin () ; i != _vertexMap.end () ; )
    if (!(*i).second->ref) {
      delete (*i).second ;
      _vertexMap.erase (i++) ;
    } else {
      assert ((*i).second->ref >= 2) ;
      myBuilder ()._vertexList.push_back ((*i ++).second) ;
    }
  }
  myBuilder ()._modified = true ; // wichtig !
  _finalized = true;
  return ;
}

void MacroGridBuilder :: inflateMacroGrid (istream & rawInput) {
  const int start = clock () ;
  {
    int nv = 0 ;
    rawInput >> nv ;
    for (int i = 0 ; i < nv ; i ++ ) {
      int id ;
      double x, y, z ;
      rawInput >> id >> x >> y >> z ;
      InsertUniqueVertex (x,y,z,id) ;
    }
  }
  {
    int ne = 0 ;
    rawInput >> ne ;
    for (int i = 0 ; i < ne ; i ++ ) 
    {
      int elementType ;
      rawInput >> elementType ;
      switch (elementType) 
      {
      case HEXA_RAW :
        {
          int v [8] ;
          rawInput >> v [0] >> v [1] >> v [2] >> v [3] >> v [4] >> v [5] >> v [6] >> v [7] ;
          InsertUniqueHexa (v) ;
        }
        break ;
      case TETRA_RAW :
        {
          int v [4] ;
          rawInput >> v [0] >> v [1] >> v [2] >> v [3] ;
          InsertUniqueTetra (v) ;
        }
        break ;
      case PERIODIC3_RAW :
        {
          int v [6] ;
          int bt ;
          rawInput >> v [0] >> v [1] >> v [2] >> v [3] >> v [4] >> v [5] >> bt ;
          if( ! ( Gitter :: hbndseg_STI :: bndRangeCheck(bt) ) )
          {
            cerr << "**ERROR (FATAL): boundary id = " << bt << "  out of range! Valid are: " << Gitter :: hbndseg_STI :: validRanges() << endl ;
            exit(1);
          }
          Gitter :: hbndseg :: bnd_t btAbs = (Gitter :: hbndseg :: bnd_t)(std::abs(bt));
          Gitter :: hbndseg :: bnd_t bndId[ 2 ] = { btAbs, btAbs };
          InsertUniquePeriodic3 (v, bndId ) ;
        }
        break ;
      case PERIODIC4_RAW :
        {
          int v [8] ;
          int bt ;
          rawInput >> v [0] >> v [1] >> v [2] >> v [3] >> v [4] >> v [5] >> v [6] >> v [7] >> bt ;
          if( ! ( Gitter :: hbndseg_STI :: bndRangeCheck(bt) ) )
          {
            cerr << "**ERROR (FATAL): boundary id = " << bt << "  out of range! Valid are: " << Gitter :: hbndseg_STI :: validRanges() << endl ;
            exit(1);
          }
          Gitter :: hbndseg :: bnd_t btAbs = (Gitter :: hbndseg :: bnd_t)(std::abs(bt));
          Gitter :: hbndseg :: bnd_t bndId[ 2 ] = { btAbs, btAbs };
          InsertUniquePeriodic4 (v, bndId ) ;
        }
        break ;
      default :
        cerr << "**ERROR (FATAL): unknown ElementID in Rawformat File [" 
             << elementType << "] in "__FILE__ << " " << __LINE__ << " ... Exiting. " << endl ;
             exit (1) ;
        break ;
      }
    }
  }
  {
    int nb = 0 ;
    rawInput >> nb ;
    for (int i = 0 ; i < nb ; i ++) 
    {
      int polygonLen ;
      rawInput >> polygonLen ;
      if (polygonLen == 4) 
      {
        int bt, v [4] ;
        rawInput >> v [0] >> v [1] >> v [2] >> v [3] >> bt ;
        if( ! ( Gitter :: hbndseg_STI :: bndRangeCheck(bt) ) )
        {
          cerr << "**ERROR (FATAL): boundary id = " << bt << "  out of range! Valid are: " << Gitter :: hbndseg_STI :: validRanges() << endl ;
          exit(1);
        }
        InsertUniqueHbnd4 (v,(Gitter :: hbndseg :: bnd_t)(std::abs(bt))) ;
      } 
      else if (polygonLen == 3) 
      {
        int bt, v [3] ;
        rawInput >> v [0] >> v [1] >> v [2] >> bt ;
        if( ! ( Gitter :: hbndseg_STI :: bndRangeCheck(bt) ) )
        {
          cerr << "**ERROR (FATAL): boundary id = " << bt << "  out of range! Valid are: " << Gitter :: hbndseg_STI :: validRanges() << endl ;
          exit(1);
        }
        InsertUniqueHbnd3 (v,(Gitter :: hbndseg :: bnd_t)(std::abs(bt))) ;
      } 
      else 
      {
        cerr << " MacroGridBuilder :: inflateMacroGrid (istream &) ERROR (fatal): cannot" ;
        cerr << " create BoundarySegments with polygon length " << polygonLen << "!" << endl ;
        abort () ;
      }
    }
  }
  if (debugOption (3)) {
    cout << "**INFO MacroGridBuilder :: inflateMacroGrid () used: " ;
    cout << (float)(clock () - start)/(float)(CLOCKS_PER_SEC) << " sec." << endl ;
  }
  return ;
}

void Gitter :: Geometric :: BuilderIF :: macrogridBuilder (istream & in) 
{
  strstream_t raw ;
  
  // set scientific mode and high precision 
  raw << scientific ;
  raw.precision( 16 );

  MacroGridBuilder mm (*this) ;
  int c = in.get () ;
  assert (!in.eof ()) ;
  in.putback (c) ;
  assert (in.good ()) ;
  if (c == int ('!')) 
  {
    // Kommentar gefunden: Die erste Zeile in den strstreambuf buf lesen
    // und auf 'Tetraeder' oder 'Hexaeder' untersuchen.

    strstreambuf_t buf ;
    in.get () ;   // Das Kommentarzeichen wird entfernt.
    in.get (buf) ;
    int len = in.gcount () ;
    in.get () ;   // Der folgende Zeilenumbruchwird auch entfernt.
    istream is (& buf) ;
    char * str = new char [len + 1] ;
    assert (str) ;
    is >> str ;   // Das erste Wort nach dem Kommentar steht jetzt in str.
          // Alle weiteren k"onnen noch aus is gelesen werden, das
      // array str ist so lang, wie die gesamte Zeile in 'buf'.

    if ((0 == strcmp (str, "Tetraeder")) ||
        (0 == strcmp (str, "Tetrahedra")))
    {
      // Versuchen wir's mal mit Tetraedern
      MacroGridBuilder :: generateRawTetraImage (in,raw) ;
    } 
    else if 
      ((0 == strcmp (str, "Hexaeder")) || 
       (0 == strcmp (str, "Hexahedra")))
    {
      // oder andernfalls mit Hexaedern.
      MacroGridBuilder :: generateRawHexaImage (in,raw) ;
    } 
    else 
    {
      cerr << "**WARNING (IGNORED) Unknown comment to file format: " << str ;
      cerr << " In : " << __FILE__ << " " << __LINE__ << endl ;
      delete [] str ;
      return ;
    }
    delete [] str ;
  } 
  else 
  {
    cerr << "**WARNING (IGNORED) No identifier for file format found!\n" ;
    cerr << "  -> Try to read as hexa mesh." << endl ;
    MacroGridBuilder :: generateRawHexaImage (in,raw) ;
  }
  mm.inflateMacroGrid (raw) ;
  return ;
}



