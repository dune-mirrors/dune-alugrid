// (c) --
#ifndef __parmetis_h__

#warning  -- Compiling parallel version without ParMETIS !!!

#ifndef PARMETIS_C_INCLUDED
#define PARMETIS_C_INCLUDED

using namespace std; 

static const char parmetmess [] = 
        "**INFO Due to license reasons the library ParMETIS is\n"
				"       not part of the ALUGrid library distribution. \n"
				"       To use this feature get a copy of the ParMETIS library \n"
        "       (see http://www-users.cs.umn.edu/~karypis/metis/metis/ )\n"
        "       and re-configure the ALUGrid library with the \n"
        "       --with-parmetis=PATH_TO_ParMETIS option, \n"
        "       or choose another Graph partitioning method. \n"
        "       Exiting program, bye! \n";

// dummy ParMETIS_PartGraphKway, when ParMETIS library is missing 
void ParMETIS_V3_PartKway(int *,idxtype *,idxtype *,idxtype *,idxtype *,int *,int *,int *,int *,int *,idxtype *, MPI_Comm *) 
{
  cerr << "**ERROR The use of ParMETIS_V3_PartKway is not supported, when the METIS library is missing! in: " << __FILE__ << " line: " << __LINE__ << "\n";
  cerr << parmetmess << endl ;
  exit(1); 
  return ;
}

#endif
#endif
