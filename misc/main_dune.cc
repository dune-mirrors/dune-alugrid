//
//   main_dune.cc
//
//

#include <iostream>
using namespace std;
#include "gitter_impl.hh"
#include "walk.hh"

void globaladapt(GitterBasisImpl* grid, int Anzahl) {
   for ( ; Anzahl > 0; Anzahl--) {
   cout << "globaladapt " << Anzahl << endl;
       {
          LeafIterator < Gitter::helement_STI > w (*grid) ;
          for (w->first () ; ! w->done () ; w->next ())
             w->item ().tagForGlobalRefinement ();
       }
       grid->adapt ();
       grid->printsize () ;
   }
}

void levelwalk(GitterBasisImpl* grid, int level) {
   typedef Insert <AccessIterator <
     Gitter::helement_STI>::Handle, 
     TreeIterator <Gitter :: helement_STI, any_has_level <Gitter::helement_STI> > > MacIt;

   MacIt mac (grid->container(), level);
   int i = 0;
   for (mac.first(); !mac.done(); mac.next()) {
      cout << "<" << i++ << ">";
   }
   cout << endl;
}

//vergroebern

int main (int argc, const char ** argv, const char ** envp) {
   bool restart;
   if (!(argc > 1)) {
      cout << "Kein Aufruf MIT Parametern - Abbruch!" << endl;
      abort();
   }
   char *macroname, *DATAPATH = "./", *DATABASE = "ft3d";
   char *RECOVERPATH = "checkpoint", *RECOVERBASE = "checkpoint";

   if (argc > 2) {
      macroname = new char [ strlen(RECOVERPATH) + strlen(RECOVERBASE)
                             + strlen("macro.") + strlen(argv[2]) + 10 ];
      sprintf(macroname,"%smacro.%s%s", RECOVERPATH,RECOVERBASE,argv[2]);
   }
   else {
      //macroname = new char [strlen(MhdSolver::MACROGRID)+1];
      //sprintf(macroname,"%s",MhdSolver::MACROGRID);
       macroname = "param.git";
   }

   cout << "\n  -----------------------------------------------\n\n";
   cout << "   Macrogitter aus < " << macroname << " > lesen" << endl;
   cout << "   Daten/Gitter nach < "<< DATAPATH << DATABASE << " > schreiben\n";
   cout << "   -----------------------------------------------\n";

   GitterBasisImpl grid(macroname);
   delete [] macroname;

   if (argc > 2) {
      char *bakname = new char [strlen(RECOVERBASE) + 
                                strlen(argv[2]) + 10];
        sprintf(bakname,"%s%s",RECOVERBASE,argv[2]);

        cout << "   Neustart aus < " << RECOVERPATH
             << bakname << " > versuchen\n" << endl;
        //grd.restore(RECOVERPATH,bakname);
        delete [] bakname;
   }
   else {
      globaladapt(&grid, 1);
//      levelwalk(&grid, 0);
   }

   {
      char *bakname = new char[strlen(RECOVERBASE)+10];
      if (!restart)
        sprintf(bakname,"%s.final",RECOVERBASE);
      else
        sprintf(bakname,"%s.restart",RECOVERBASE);

      //grd.backup(RECOVERPATH,bakname);

      delete [] bakname;
   } 

   return 0;
}

