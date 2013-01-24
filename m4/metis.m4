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
#if test x$with_parmetis != x ; then
#  with_metis=$with_parmetis
#  AC_MSG_WARN([Using METIS version of ParMETIS!])
#  ALU_METIS_VERSION="(ParMETIS)"
#fi 

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

METIS_LIB_PATH="$METISROOT/lib"
if ! test -d $METIS_LIB_PATH ; then 
  METIS_LIB_PATH=$METISROOT
fi  

METIS_INCLUDE_PATH="$METISROOT/include/"
if ! test -d $METIS_INCLUDE_PATH ; then 
  METIS_INCLUDE_PATH="$METISROOT/METISLib"
fi

# if not ParMETIS is used, then check for old METIS Version
if  test ! -f "$METIS_INCLUDE_PATH/metis.h" ; then
  if test -f "$METISROOT/Lib/metis.h" ; then
    # METIS 4.0 
    METIS_INCLUDE_PATH="$METISROOT/Lib"
    ALU_METIS_VERSION="(Version 4.0)"
  elif test -f $METISROOT/include/metis.h ; then 
    # METIS 5.0
    METIS_INCLUDE_PATH="$METISROOT/include"
    ARCH="`uname -s`-`uname -m`"
    METIS_LIB_PATH="$METISROOT/build/$ARCH"
    ALU_METIS_VERSION="(Version 5.x)"
  elif test -f $METISROOT/include/metis/metis.h ; then
    # check for Debian's libparmetis-dev 
    METIS_LIB_PATH="$METISROOT/lib"
    METIS_INCLUDE_PATH="$METISROOT/include/metis"
  else 
    AC_MSG_ERROR([Could not find METIS header file!])
  fi  
fi

# set variables so that tests can use them
REM_CPPFLAGS=$CPPFLAGS

LDFLAGS="$LDFLAGS -L$METIS_LIB_PATH -lm"
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
REM_LIBS=$LIBS
# add math library 
LIBS="$LIBS -lm"

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
LIBS=$REM_LIBS

# pop default language 
AC_LANG_POP([C])

## end of metis check (--without wasn't set)
fi

# survived all tests?
if test x$HAVE_METIS = x1 ; then
  AC_SUBST(METIS_LIBS, $METIS_LIBS)
  AC_SUBST(METIS_LDFLAGS, $METIS_LDFLAGS)
  AC_SUBST(METIS_CPPFLAGS, $METIS_CPPFLAGS)
  AC_DEFINE(HAVE_METIS, 1, [Define to 1 if metis-library is found])

  # add to global list
  DUNE_ADD_ALL_PKG([METIS], [\${METIS_CPPFLAGS}],
                   [\${METIS_LDFLAGS}], [\${METIS_LIBS}])

  # set variable for summary
  with_metis="yes $ALU_METIS_VERSION"
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
