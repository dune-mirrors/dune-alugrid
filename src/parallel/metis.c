// (c) --
#if defined METISTITLE || defined METIS_H 
#define METIS_C_INCLUDED 
#endif

#ifndef METIS_C_INCLUDED
#define METIS_C_INCLUDED

#warning  -- Compiling parallel version without METIS !!!

using namespace std;

static const char metmess [] = 	"**INFO Due to license reasons the library METIS is\n"
				"       not part of the ALUGrid library distribution. \n"
				"       To use this feature get a copy of the METIS library \n"
        "       (see http://www-users.cs.umn.edu/~karypis/metis/metis/ )\n"
        "       and re-configure the ALUGrid library with the \n"
        "       --with-metis=PATH_TO_METIS option, \n"
        "       or choose another Graph partitioning method. \n"
        "       Exiting program, bye! \n";

// dummy METIS_PartGraphKway, when METIS library is missing 
void METIS_PartGraphKway(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *) {
  cerr << "**ERROR The use of METIS_PartGraphKway is not supported, when the METIS library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << metmess << endl ;
  exit(1); 
  return ;
}

// dummy METIS_PartGraphRecursive, when METIS library is missing 
void METIS_PartGraphRecursive(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *) {
  cerr << "**ERROR The use of METIS_PartGraphRecursive is not supported, when the METIS library is missing! " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << metmess << endl ;
  exit(1);
  return ;
}

#endif
