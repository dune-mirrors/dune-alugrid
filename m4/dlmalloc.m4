# searches for DLMALLOC header and lib 
AC_DEFUN([ALUGRID_PATH_DLMALLOC],[
  AC_REQUIRE([AC_PROG_CC])

  AC_ARG_WITH(dlmalloc,
    AC_HELP_STRING([--with-dlmalloc=PATH],[directory with Doug Lea's malloc (version >= 2.8.6) inside]))

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
LIBS=""

## do nothing if no --with-dlmalloc was supplied
if test x$with_dlmalloc != x && test x$with_dlmalloc != xno ; then

  if test x$with_dlmalloc == xyes ; then
    AC_MSG_ERROR([You have to provide a directory --with-dlmalloc=PATH])
  fi

  if test -d $with_dlmalloc; then
    # expand tilde / other stuff
    DLMALLOCROOT=`cd $with_dlmalloc && pwd`
  else
    AC_MSG_ERROR([Path $with_dlmalloc supplied for --with-dlmalloc does not exist!])
  fi
fi  

DLMALLOC_INCLUDE_PATH="$DLMALLOCROOT"

DLSOURCE=$DLMALLOC_INCLUDE_PATH/malloc.c

# if not DLMALLOC is used, then check for old DLMALLOC Version
if ! test -f "$DLSOURCE" ; then
  AC_MSG_WARN([Could not find malloc.c source file!])
  if ! test $DLMALLOC_INCLUDE_PATH/malloc*.c ; then 
    AC_MSG_ERROR([Could not find malloc*.c source file!])
  else 
    DLSOURCE=`ls $DLMALLOC_INCLUDE_PATH/malloc*.c`
  fi
fi

# check for header
AC_LANG_PUSH([C])
AC_CHECK_HEADER([$DLSOURCE], 
 [HAVE_DLMALLOC="1"],
  AC_MSG_WARN([$DLSOURCE not found in $DLMALLOC_INCLUDE_PATH]))

# pop default language 
AC_LANG_POP([C])

# survived all tests?
if test x$HAVE_DLMALLOC = x1 ; then
  AC_DEFINE(HAVE_DLMALLOC, 1, [Define to 1 if dlmalloc package is found])
  AC_DEFINE_UNQUOTED(DLMALLOC_SOURCE_INCLUDE, ["$DLSOURCE"], [Include source file for dlmalloc])

  # set variable for summary
  with_dlmalloc="yes" 
else
  # set variable for summary
  with_dlmalloc="no"
fi
  
# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

DUNE_ADD_SUMMARY_ENTRY([dlmalloc],[$with_dlmalloc ($DLSOURCE)])

])
