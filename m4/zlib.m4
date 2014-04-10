AC_DEFUN([DUNE_PATH_ZLIB],[
  AC_PREREQ([2.5.0])
  AC_REQUIRE([PKG_PROG_PKG_CONFIG])

  AC_LANG_PUSH([C++])

  AC_ARG_WITH([zlib], AC_HELP_STRING([--with-zlib=PATH], [directory containing zlib]))

  HAVE_ZLIB="no"
  ZLIB_SUMMARY="no"

  AC_MSG_CHECKING([for zlib])
  AS_IF([test -z "$PKG_CONFIG"],[
    AC_MSG_RESULT([failed])
    AC_MSG_NOTICE([pkg-config is required to find zlib])
  ],[
    ac_save_PKG_CONFIG_PATH="$PKG_CONFIG_PATH"
    AS_IF([test "x$with_zlib" != "x"],
          [PKG_CONFIG_PATH="$with_zlib:$with_zlib/lib/pkgconfig:$PKG_CONFIG_PATH"])
    AS_IF([PKG_CONFIG_PATH=$PKG_CONFIG_PATH $PKG_CONFIG --exists zlib],[
      HAVE_ZLIB="yes"
      ZLIB_VERSION="`PKG_CONFIG_PATH=$PKG_CONFIG_PATH $PKG_CONFIG --modversion zlib`"
      ZLIB_CPPFLAGS="`PKG_CONFIG_PATH=$PKG_CONFIG_PATH $PKG_CONFIG --cflags zlib` -DENABLE_ZLIB=1"
      ZLIB_LIBS="`PKG_CONFIG_PATH=$PKG_CONFIG_PATH $PKG_CONFIG --libs zlib`"
    ])
    AC_MSG_RESULT([$HAVE_ZLIB])
    PKG_CONFIG_PATH="$ac_save_PKG_CONFIG_PATH"
  ])

  AS_IF([test "$HAVE_ZLIB" != "no"],[
    AC_DEFINE([HAVE_ZLIB], [ENABLE_ZLIB], [Was zlib found and ZLIB_CPPFLAGS used?])
    AC_SUBST([ZLIB_CPPFLAGS])
    AC_SUBST([ZLIB_LIBS])
    DUNE_ADD_ALL_PKG([ZLIB], [\${ZLIB_CPPFLAGS}], [], [\${ZLIB_LIBS}])
    ZLIB_SUMMARY="version $ZLIB_VERSION"
  ])
  AM_CONDITIONAL([ZLIB], [test "$HAVE_ZLIB" != "no"])

  DUNE_ADD_SUMMARY_ENTRY([zlib], [$ZLIB_SUMMARY])

  AC_LANG_POP()
])
