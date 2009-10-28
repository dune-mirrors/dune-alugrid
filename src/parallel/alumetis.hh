#ifndef METIS_H_INCLUDED
#define METIS_H_INCLUDED

#if HAVE_METIS 
// cover metis-4.0 bug   
#define __log2 __METIS__log2
extern "C" {
  #include <metis.h>
}
#else 
static const char metmess [] =  "**INFO Due to license reasons the library METIS is\n"
        "       not part of the ALUGrid library distribution. \n"
        "       To use this feature get a copy of the METIS library \n"
        "       (see http://www-users.cs.umn.edu/~karypis/metis/metis/ )\n"
        "       and re-configure the ALUGrid library with the \n"
        "       --with-metis=PATH_TO_METIS option, \n"
        "       or choose another Graph partitioning method. \n"
        "       Exiting program, bye! \n";
#endif

typedef int idxtype ;

inline void CALL_METIS_PartGraphKway(int *n, idxtype *edge_p, idxtype *edge,
                idxtype * vertex_wInt, idxtype *edge_w,
                int *wgtflag, int *numflag, int *npart,
                int *options, int *edgecut, idxtype *neu)
{
#if HAVE_METIS  
  // call metis function 
  :: METIS_PartGraphKway (n, edge_p, edge, 
                          vertex_wInt, edge_w,
                          wgtflag,  numflag, npart, 
                          options, edgecut, neu) ;
#else 
  std::cerr << "**ERROR The use of METIS_PartGraphKway is not supported, when the METIS library is missing!  in: " << __FILE__ << " line: " << __LINE__ << "\n";
  std::cerr << metmess << std::endl ;
  exit(1);
#endif
  return ;
}

inline void CALL_METIS_PartGraphRecursive(int *n, idxtype *edge_p, idxtype *edge,
                idxtype * vertex_wInt, idxtype *edge_w,
                int *wgtflag, int *numflag, int *npart,
                int *options, int *edgecut, idxtype *neu)
{
#if HAVE_METIS  
  // call metis function 
  :: METIS_PartGraphRecursive(n, edge_p, edge, 
                          vertex_wInt, edge_w,
                          wgtflag,  numflag, npart, 
                          options, edgecut, neu) ;
#else 
  std::cerr << "**ERROR The use of METIS_PartGraphRecursive is not supported, when the METIS library is missing!  in: " << __FILE__ << " line: " << __LINE__ << "\n";
  std::cerr << metmess << std::endl ;
  exit(1);
#endif
  return ;
}
#endif
