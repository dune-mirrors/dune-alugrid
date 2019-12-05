/* begin dune-alugrid
   put the definitions for config.h specific to
   your project here. Everything above will be
   overwritten
*/
/* begin private */
/* Name of package */
#define PACKAGE "@DUNE_MOD_NAME@"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "@DUNE_MAINTAINER@"

/* Define to the full name of this package. */
#define PACKAGE_NAME "@DUNE_MOD_NAME@"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "@DUNE_MOD_NAME@ @DUNE_MOD_VERSION@"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "@DUNE_MOD_NAME@"

/* Define to the home page for this package. */
#define PACKAGE_URL "@DUNE_MOD_URL@"

/* Define to the version of this package. */
#define PACKAGE_VERSION "@DUNE_MOD_VERSION@"

/* end private */


#define DUNE_ALUGRID_VERSION "${DUNE_ALUGRID_VERSION}"

/* Define to the major version of dune-alugrid */
#define DUNE_ALUGRID_VERSION_MAJOR ${DUNE_ALUGRID_VERSION_MAJOR}

/* Define to the minor version of dune-alugrid */
#define DUNE_ALUGRID_VERSION_MINOR ${DUNE_ALUGRID_VERSION_MINOR}

/* Define to the revision of dune-alugrid*/
#define DUNE_ALUGRID_VERSION_REVISION ${DUNE_ALUGRID_VERSION_REVISION}

/* Define to build more .cc into library */
#cmakedefine DUNE_ALUGRID_COMPILE_BINDINGS_IN_LIB 1

/* Define if we have dlmalloc */
#cmakedefine HAVE_DLMALLOC 1

/* Define if we have zoltan */
#cmakedefine HAVE_ZOLTAN 1

/* Define if we have ZLIB */
#cmakedefine HAVE_ZLIB 1

/* Include source file for dlmalloc */
#cmakedefine DLMALLOC_SOURCE_INCLUDE ${DLMALLOC_SOURCE_INCLUDE}

/* Define if we have thread local storage */
#cmakedefine HAVE_PTHREAD_TLS 1

/* Define if we have pthreads */
#cmakedefine HAVE_PTHREAD 1

/* Define if testgrids.hh from dune-grid have been found in docs/grids/gridfactory */
#cmakedefine HAVE_DUNE_GRID_TESTGRIDS 1

/* Grid type magic for DGF parser */
@ALUGRID_CONFIG_H_BOTTOM@
/* end dune-alugrid */
