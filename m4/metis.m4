# searches for METIS header and lib 

AC_DEFUN([ALUGRID_PATH_METIS],[
  AC_REQUIRE([AC_PROG_CC])

  AC_ARG_WITH(metis,
    AC_HELP_STRING([--with-metis=PATH],[directory with METIS (version >= 4.0)  inside]))

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
LIBS=""

## do nothing if no --with-metis was supplied
if test x$with_metis != x && test x$with_metis != xno ; then

  if test x$with_metis == xyes ; then
    AC_MSG_ERROR([You have to provide a directory --with-metis=PATH])
  fi

  if test -d $with_metis; then
    # expand tilde / other stuff
    METISROOT=`cd $with_metis && pwd`
  else
    AC_MSG_ERROR([Path $with_metis supplied for --with-metis does not exist!])
  fi

METIS_LIB_PATH="$METISROOT"
METIS_INCLUDE_PATH="$METISROOT/Lib"

# set variables so that tests can use them
REM_CPPFLAGS=$CPPFLAGS

LDFLAGS="$LDFLAGS -L$METIS_LIB_PATH"
CPPFLAGS="$CPPFLAGS $METISDEF -I$METIS_INCLUDE_PATH"

# check for header
  AC_LANG_PUSH([C])
  AC_CHECK_HEADER([metis.h], 
   [METIS_CPPFLAGS="$METISDEF -I$METIS_INCLUDE_PATH"
	HAVE_METIS="1"],
  AC_MSG_WARN([metis.h not found in $METIS_INCLUDE_PATH]))

CPPFLAGS="$REM_CPPFLAGS $METISDEF -I$METIS_INCLUDE_PATH"
REM_CPPFLAGS=

REM_LDFLAGS=$LDFLAGS
# add math library 
LDFLAGS="$LDFLAGS -lm"

# if header is found...
if test x$HAVE_METIS = x1 ; then
  AC_CHECK_LIB(metis,[METIS_PartGraphKway],
	[METIS_LIBS="-lmetis"
         METIS_LDFLAGS="-L$METIS_LIB_PATH"
         LIBS="$LIBS $METIS_LIBS"],
	[HAVE_METIS="0"
	AC_MSG_WARN(libmetis.a not found in $METIS_LIB_PATH)])
fi

LDFLAGS=$REM_LDFLAGS

# pop default language 
AC_LANG_POP 

## end of metis check (--without wasn't set)
fi

# survived all tests?
if test x$HAVE_METIS = x1 ; then
  AC_SUBST(METIS_LIBS, $METIS_LIBS)
  AC_SUBST(METIS_LDFLAGS, $METIS_LDFLAGS)
  AC_SUBST(METIS_CPPFLAGS, $METIS_CPPFLAGS)
  AC_DEFINE(HAVE_METIS, 1, [Define to 1 if metis-library is found])

  # add to global list
  ALUGRID_PKG_LDFLAGS="$ALUGRID_PKG_LDFLAGS $METIS_LDFLAGS"
  ALUGRID_PKG_LIBS="$ALUGRID_PKG_LIBS $METIS_LIBS"
  ALUGRID_PKG_CPPFLAGS="$ALUGRID_PKG_CPPFLAGS $METIS_CPPFLAGS"

  # set variable for summary
  with_metis="yes"
else
  AC_SUBST(METIS_LIBS, "")
  AC_SUBST(METIS_LDFLAGS, "")
  AC_SUBST(METIS_CPPFLAGS, "")

  # set variable for summary
  with_metis="no"
fi
  
# also tell automake
AM_CONDITIONAL(METIS, test x$HAVE_METIS = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

])
