# searches for zoltan-headers and libs

# DUNE_PATH_ZOLTAN()
#
# shell variables:
#   with_zoltan
#     no or yes
#   ZOLTANROOT
#   ZOLTAN_VERSIONNO
#   ZOLTAN_LIB_PATH
#   ZOLTAN_INCLUDE_PATH
#   ZOLTAN_CPPFLAGS
#   ZOLTAN_LDFLAGS
#   ZOLTAN_LIBS
#   HAVE_ZOLTAN
#     undef or 1 or 0
#
# substitutions:
#   ZOLTAN_CPPFLAGS
#   ZOLTAN_LDFLAGS
#   ZOLTAN_LIBS
#
# defines:
#   HAVE_ZOLTAN
#     ENABLE_ZOLTAN or undefined
#   ZOLTAN_PARALLEL_H
#   ZOLTAN_SERIAL_H
#
# conditionals:
#   ZOLTAN
AC_DEFUN([DUNE_PATH_ZOLTAN],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([DUNE_MPI])
  AC_REQUIRE([DUNE_PATH_PARMETIS])

  AC_ARG_WITH(zoltan,
    AC_HELP_STRING([--with-zoltan=PATH],[directory where ZOLTAN is installed]))

# do not use zoltan debug lib 

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"

# initilize to sane value
HAVE_ZOLTAN=0

## do nothing if no --with-zoltan was supplied
if test x$with_zoltan != xno ; then

  # is --with-zoltan=PATH used?
  AS_IF([test "x$with_zoltan" != "x"],[
    AS_IF([test -d $with_zoltan],[
      AC_MSG_NOTICE([searching for ZOLTAN in $with_zoltan...])
      ZOLTANROOT=`cd $with_zoltan && pwd`
    ],[
      AC_MSG_WARN([ZOLTAN directory '$with_zoltan' does not exist or is inaccessible])
    ])
  ],[
    # educated guess for zoltan root
    for d in /usr /usr/local /usr/local/zoltan /opt/zoltan; do
      AC_MSG_NOTICE([searching for ZOLTAN in $d...])
      AS_IF([test -f $d/lib/pkgconfig/zoltan.pc -o -x $d/bin/zoltanversion],[
        ZOLTANROOT="$d"
        break
      ])
    done
  ])

  REM_PKG_CONFIG_PATH=$PKG_CONFIG_PATH
  PKG_CONFIG_PATH="$ZOLTANROOT:$ZOLTANROOT/lib/pkgconfig:$ZOLTANROOT/lib64/pkgconfig:$PKG_CONFIG_PATH"

  # lib dir and include path 
  ZOLTAN_INCLUDE_PATH="$ZOLTANROOT/include"

  ZOLTAN_LIB_PATH="$ZOLTANROOT/lib"

  # restore PKG_CONFIG_PATH 
  PKG_CONFIG_PATH=$REM_PKG_CONFIG_PATH

  AC_LANG_PUSH([C++])

  # set variables so that tests can use them
  ZOLTAN_INC_FLAG="-I$ZOLTAN_INCLUDE_PATH -DENABLE_ZOLTAN=1 $DUNEMPICPPFLAGS"
  CPPFLAGS="$ac_save_CPPFLAGS $ZOLTAN_INC_FLAG $DUNEMPILDFLAGS"
  # check for header
  AC_CHECK_HEADERS([zoltan_cpp.h], 
     [ZOLTAN_CPPFLAGS="$ZOLTAN_INC_FLAG"
      ZOLTAN_LDFLAGS=""
      ZOLTAN_LIBS="-L$ZOLTAN_LIB_PATH -lzoltan"
      HAVE_ZOLTAN="1"],
      AC_MSG_WARN([zoltan_cpp.h not found in $ZOLTAN_INCLUDE_PATH]))
   
  # We check only whether linking with the library works, not for an actual
  # function from that library.  So we won't need any special stuff in the
  # CPPFLAGS
  CPPFLAGS="$ac_save_CPPFLAGS"
  ac_save_LDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS -L$ZOLTAN_LIB_PATH $DUNEMPILDFLAGS"
  ac_save_LIBS="$LIBS"
  LIBS="$LIBS $PARMETIS_LIBS $DUNEMPILIBS"

  # if header is found...
  if test x$HAVE_ZOLTAN = x1 ; then
  AC_CHECK_LIB([zoltan],[Zoltan_LB_Partition],
      [: #dumy argument to avoid default action
      ],
	  [HAVE_ZOLTAN="0"
	  AC_MSG_WARN(libzoltan not found!)])
  fi

  LDFLAGS="$ac_save_LDFLAGS"
  LIBS="$ac_save_LIBS"

  AC_LANG_POP([C++])

## end of zoltan check (--without wasn't set)
fi

# survived all tests?
if test x$HAVE_ZOLTAN = x1 ; then
  AC_SUBST(ZOLTAN_LIBS, $ZOLTAN_LIBS)
  AC_SUBST(ZOLTAN_LDFLAGS, $ZOLTAN_LDFLAGS)
  AC_SUBST(ZOLTAN_CPPFLAGS, $ZOLTAN_CPPFLAGS)
  AC_DEFINE(HAVE_ZOLTAN, ENABLE_ZOLTAN,
    [This is only true if zoltan-library was found by configure 
     _and_ if the application uses the ZOLTAN_CPPFLAGS])

  # add to global list
  DUNE_ADD_ALL_PKG([ZOLTAN], [\${ZOLTAN_CPPFLAGS}],
                   [\${ZOLTAN_LDFLAGS}], [\${ZOLTAN_LIBS}])

  with_zoltan="version $ZOLTAN_VERSION $with_zoltan_parallel"
  with_zoltan_long="$ZOLTANROOT"
else
  AC_SUBST(ZOLTAN_LIBS, "")
  AC_SUBST(ZOLTAN_LDFLAGS, "")
  AC_SUBST(ZOLTAN_CPPFLAGS, "")

  # set variable for summary
  with_zoltan="no"
  with_zoltan_long=""
fi
  
# also tell automake
AM_CONDITIONAL(ZOLTAN, test x$HAVE_ZOLTAN = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

DUNE_ADD_SUMMARY_ENTRY([ZOLTAN],[$with_zoltan],[$with_zoltan_long])

])
