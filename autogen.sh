#!/bin/sh
#### barf on errors
set -e

# everybody who checks out the CVS wants the maintainer-mode to be enabled
# (should be off for source distributions, this should happen automatically)
#
# we suppose that (right now) most of the persons checking out from
# CVS will have a local installation of dune -> enable-localdune
DEFAULTCONFOPT=""

# default values
DEBUG=1
OPTIM=0

usage () {
    echo "Usage: ./autogen.sh [options]"
    echo "  -i, --intel        use intel compiler"
    echo "  -g, --gnu          use gnu compiler (default)"
    echo "  -m, --mpi          use mpiCC"
    echo "  --opts=FILE        use compiler-options from FILE"
    echo "  -d, --debug        switch debug-opts on"
    echo "  -n, --nodebug      switch debug-opts off"
    echo "  -o, --optim        switch optimization on"
    echo "  --with-metis=PATH  directory with METIS inside"
    echo "  --with-party=PATH  directory with PARTY Partitioning-Library inside"
    echo "  --enable-shared    enable option for building also shared libraries (default == disabled)" 
    echo "  -h, --help         you already found this :)"
}

# no compiler set yet
COMPSET=0
for OPT in $* ; do

    set +e
    # stolen from configure...
    # when no option is set, this returns an error code
    arg=`expr "x$OPT" : 'x[^=]*=\(.*\)'`
    set -e

    case "$OPT" in
	-i|--intel)   . ./icc.opts ; COMPSET=1 ;;
	-g|--gnu)     . ./gcc.opts ; COMPSET=1 ;;
	-m|--mpi)     . ./mpi.opts ; COMPSET=1 ;;
	--opts=*)
	    if [ -r $arg ] ; then
	      echo "reading options from $arg..."
	      . ./$arg ;
	      COMPSET=1;
	    else
	      echo "Cannot open compiler options file $arg!" ;
	      exit 1;
	    fi ;;
	-d|--debug)   DEBUG=1 ;;
	-n|--nodebug) DEBUG=0 ;;
	-o|--optim)   OPTIM=1 ;;
	-h|--help) usage ; exit 0 ;;
	# special hack: use the with-dune-dir for aclocal-includes
	# pass unknown opts to ./configure
	*) CONFOPT="$CONFOPT $OPT" ;;
    esac
done

# set special m4-path if --with-dune is set
    # aclocal from automake 1.8 seems to need an absolute path for inclusion
# use the nonfree compiler as default :-(
if [ "$COMPSET" != "1" ] ; then
    echo "No compiler set, using Intel compiler as default"
    . ./icc.opts
fi

# create flags
COMPFLAGS="$FLAGS"

# maybe add debug flag
if [ "$DEBUG" = "1" ] ; then	
    COMPFLAGS="$COMPFLAGS $DEBUGFLAGS"
fi

# maybe add optimization flag
if [ "$OPTIM" = "1" ] ; then	
    COMPFLAGS="$COMPFLAGS $OPTIMFLAGS"
fi

#### create all autotools-files

echo "--> libtoolize..."
# force to write new versions of files, otherwise upgrading libtools
# doesn't do anything...
libtoolize --force

echo "--> aclocal..."
aclocal -I m4 

# sanity check to catch missing --with-dune

echo "--> autoheader..."
autoheader

echo "--> automake..."
automake --add-missing

echo "--> autoconf..."
autoconf

#### start configure with special environment

export CC=$COMP
export CXX=$CXXCOMP
export CPP="$COMP -E"

export CFLAGS="$COMPFLAGS"
export CXXFLAGS="$COMPFLAGS"

./configure $DEFAULTCONFOPT $CONFOPT

