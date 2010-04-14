# searches for STARMPI header and lib 
AC_DEFUN([ALUGRID_PATH_STARMPI],[
  AC_REQUIRE([AC_PROG_CC])

  AC_ARG_WITH(starmpi,
    AC_HELP_STRING([--with-starmpi=PATH],[directory with STAR MPI (version >= 1.0.1) inside]))

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
LIBS=""

## do nothing if no --with-starmpi was supplied
if test x$with_starmpi != x && test x$with_starmpi != xno ; then

  if test x$with_starmpi == xyes ; then
    AC_MSG_ERROR([You have to provide a directory --with-starmpi=PATH])
  fi

  if test -d $with_starmpi; then
    # expand tilde / other stuff
    STARMPIROOT=`cd $with_starmpi && pwd`
  else
    AC_MSG_ERROR([Path $with_starmpi supplied for --with-starmpi does not exist!])
  fi
fi  

STARMPI_INCLUDE_PATH="$STARMPIROOT"

# if not ParSTARMPI is used, then check for old STARMPI Version
if  test ! -f "$STARMPI_INCLUDE_PATH/star-mpi.h" ; then
  AC_MSG_WARN([Could not find STAR-MPI header file!])
fi

# set variables so that tests can use them
REM_CPPFLAGS=$CPPFLAGS

STARMPICPPSUGG="-I$STARMPI_INCLUDE_PATH -I $STARMPI_INCLUDE_PATH/algorithms"
CPPFLAGS="$CPPFLAGS $STARMPICPPSUGG"

# check for header
  AC_LANG_PUSH([C])
  AC_CHECK_HEADER([star-mpi.h], 
   [STARMPI_CPPFLAGS="$STARMPICPPSUGG"
	  HAVE_STARMPI="1"],
   AC_MSG_WARN([star-mpi.h not found in $STARMPI_INCLUDE_PATH]))

CPPFLAGS="$REM_CPPFLAGS $STARMPICPPSUGG"
REM_CPPFLAGS=

# pop default language 
AC_LANG_POP([C])

# survived all tests?
if test x$HAVE_STARMPI = x1 ; then
  AC_SUBST(STARMPI_LIBS, $STARMPI_LIBS)
  AC_SUBST(STARMPI_LDFLAGS, $STARMPI_LDFLAGS)
  AC_SUBST(STARMPI_CPPFLAGS, $STARMPI_CPPFLAGS)
  AC_DEFINE(HAVE_STARMPI, 1, [Define to 1 if STAR-MPI package is found])

  # add to global list
  ALUGRID_PKG_LDFLAGS="$ALUGRID_PKG_LDFLAGS $STARMPI_LDFLAGS"
  ALUGRID_PKG_LIBS="$ALUGRID_PKG_LIBS $STARMPI_LIBS"
  ALUGRID_PKG_CPPFLAGS="$ALUGRID_PKG_CPPFLAGS $STARMPI_CPPFLAGS"

  # set variable for summary
  with_starmpi="yes" 
else
  AC_SUBST(STARMPI_LIBS, "")
  AC_SUBST(STARMPI_LDFLAGS, "")
  AC_SUBST(STARMPI_CPPFLAGS, "")

  # set variable for summary
  with_starmpi="no"
fi
  
# also tell automake
AM_CONDITIONAL(STARMPI, test x$HAVE_STARMPI = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

])
