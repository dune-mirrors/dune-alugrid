# searches for PARTY header and lib 

AC_DEFUN([ALUGRID_PATH_PARTY],[
  AC_REQUIRE([AC_PROG_CC])

  AC_ARG_WITH(party,
    AC_HELP_STRING([--with-party=PATH],[directory with PARTY (version >= 1.1)  inside]))

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
LIBS=""

## do nothing if no --with-party was supplied
if test x$with_party != x && test x$with_party != xno ; then

  if test x$with_party == xyes ; then
    AC_MSG_ERROR([You have to provide a directory --with-party=PATH])
  fi

  if test -d $with_party; then
    # expand tilde / other stuff
    PARTYROOT=`cd $with_party && pwd`
  else
    AC_MSG_ERROR([Path $with_party supplied for --with-party does not exist!])
  fi

PARTY_LIB_PATH="$PARTYROOT"
PARTY_INCLUDE_PATH="$PARTYROOT"

# Alberta needs special defined symbols

# set variables so that tests can use them
REM_CPPFLAGS=$CPPFLAGS

LDFLAGS="$LDFLAGS -L$PARTY_LIB_PATH"
CPPFLAGS="$CPPFLAGS -I$PARTY_INCLUDE_PATH"

# check for header
  AC_CHECK_HEADER([party_lib.h], 
   [PARTY_CPPFLAGS="$PARTYDEF -I$PARTY_INCLUDE_PATH"
	HAVE_PARTY="1"],
  AC_MSG_WARN([party.h not found in $PARTY_INCLUDE_PATH]))

CPPFLAGS="$REM_CPPFLAGS -I$PARTY_INCLUDE_PATH"
REM_CPPFLAGS=

REM_LDFLAGS=$LDFLAGS

# TODO: check if static flag exists 
# link_static_flag defines the flag for the linker to link only static
# didnt work, with $link_static_flag, so quick hack here
LDFLAGS="$LDFLAGS -lm"

# if header is found...
if test x$HAVE_PARTY = x1 ; then
  AC_CHECK_LIB(party,[global_lin],
	[PARTY_LIBS="-lparty"
   PARTY_LDFLAGS="-L$PARTY_LIB_PATH"
   LIBS="$LIBS $PARTY_LIBS"],
	[HAVE_PARTY="0"
	AC_MSG_WARN(libparty.a found!)])
fi

if test x$HAVE_PARTY = x1 ; then
AC_LANG_PUSH([C++]) 
AC_TRY_COMPILE([#include <party_lib.h> 
                #ifndef VW 
                #error "VW is not defined!" 
                #endif
               template <class T> class Foo; 
               template <> class Foo<float> {};
      
               /* check that VW is defined as float */
               Foo< VW > foo; 
               ],
               [],
               [HAVE_PARTY="1"],
               [HAVE_PARTY="0"
               AC_MSG_WARN("PARTY lib was build with VW != float -- check PARTY lib header and recompile PARTY lib!")
              ])
AC_LANG_POP([C])
fi

LDFLAGS=$REM_LDFLAGS

## end of party check (--without wasn't set)
fi

# survived all tests?
if test x$HAVE_PARTY = x1 ; then
  AC_SUBST(PARTY_LIBS, $PARTY_LIBS)
  AC_SUBST(PARTY_LDFLAGS, $PARTY_LDFLAGS)
  AC_SUBST(PARTY_CPPFLAGS, $PARTY_CPPFLAGS)
  AC_DEFINE(HAVE_PARTY, 1, [Define to 1 if party-library is found])

  # add to global list
  ALUGRID_PKG_LDFLAGS="$ALUGRID_PKG_LDFLAGS $PARTY_LDFLAGS"
  ALUGRID_PKG_LIBS="$ALUGRID_PKG_LIBS $PARTY_LIBS"
  ALUGRID_PKG_CPPFLAGS="$ALUGRID_PKG_CPPFLAGS $PARTY_CPPFLAGS"

  # set variable for summary
  with_party="yes (Version 1.99)"
else
  AC_SUBST(PARTY_LIBS, "")
  AC_SUBST(PARTY_LDFLAGS, "")
  AC_SUBST(PARTY_CPPFLAGS, "")

  # set variable for summary
  with_party="no"
fi
  
# also tell automake
AM_CONDITIONAL(PARTY, test x$HAVE_PARTY = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

])
