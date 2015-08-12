#!/bin/bash

WORKDIR=`pwd`

echo "This script will download and build all DUNE modules"
echo "necessary to run the examples in dune-alugrid."
echo
echo "The installation directory is: $WORKDIR"
echo "Some third party libraries have to be downloaded manually."
echo "Please take a look at this script for parameters and options."
echo
read -p "Install DUNE modules to $WORKDIR? (Y/N) " YN
if [ "$YN" != "Y" ] ;then
  exit 1
fi

# this script downloads the necessary set of DUNE modules
# to build and run the examples in dune-alugrid
# NOTE: Zoltan has to be downloaded separately from

#change appropriately, i.e. 2.3 or empty (which refers to master)
DUNEVERSION=2.3

# use of cmake is not recommended, since it might not work
# and it won't make it easy to reproduce the paper results
USE_CMAKE=no

# your favorite compiler optimization flags
FLAGS="-O3 -DNDEBUG"
MAKE_FLAGS="-j4"

# download dlmalloc from ftp://g.oswego.edu/pub/misc/ via the following command
if ! test -d dlmalloc ; then
  echo "Downloading dlmalloc"
  mkdir dlmalloc ; cd dlmalloc
  wget ftp://g.oswego.edu/pub/misc/malloc.{h,c}
  cd ../
fi
# configure parameter for dlmalloc (v 2.8.6)
WITH_DLMALLOC="--with-dlmalloc=$WORKDIR/dlmalloc"

# most likely /usr if zlib is installed on the system
#WITH_ZLIB="--with-zlib=/usr"
WITH_ZLIB=

# Zoltan has to be downloaded from
# http://www.cs.sandia.gov/zoltan/
#WITH_ZOLTAN="--with-zoltan=$WORKDIR/zoltan"
WITH_ZOLTAN=

# SIONlib 1.5p1 has to be downloaded from
# http://www.fz-juelich.de/ias/jsc/EN/Expertise/Support/Software/SIONlib/sionlib-download_node
#WITH_SIONLIB="--with-sionlib=$WORKDIR/sionlib"
WITH_SIONLIB=

# dune modules needed to build dune-alugrid
DUNEMODULES="dune-common dune-geometry dune-grid dune-alugrid"

# build flags for all DUNE modules
# change according to your needs
CACHEFILE=$WORKDIR/cache.config
# if ! test -f config.opts ; then
echo "MAKE_FLAGS=\"$MAKE_FLAGS\"
USE_CMAKE=$USE_CMAKE
CONFIGURE_FLAGS=\"CXXFLAGS=\\\"$FLAGS\\\" \\
  --cache-file=$CACHEFILE \\
  --disable-documentation \\
  --enable-experimental-grid-extensions \\
  --enable-parallel \\
  --enable-fieldvector-size-is-method \\
  $WITH_DLMALLOC \\
  $WITH_ZLIB \\
  $WITH_ZOLTAN \\
  $WITH_SIONLIB\"" > config.opts
# fi

DUNEBRANCH=
if [ "$DUNEVERSION" != "" ] ; then
  DUNEBRANCH="-b releases/$DUNEVERSION"
fi

ALUGRIDBRANCH="-b papers/main"
# get all dune modules necessary
for MOD in $DUNEMODULES ; do
  if [ "$MOD" == "dune-alugrid" ] ; then
    # use the special branch papers/main for dune-alugrid
    git clone $ALUGRIDBRANCH http://users.dune-project.org/repositories/projects/dune-alugrid.git
  else
    git clone $DUNEBRANCH http://git.dune-project.org/repositories/$MOD
  fi
done

# delete old cache file
if test -f $CACHEFILE ; then
  rm -f $CACHEFILE
fi

# build all DUNE modules in the correct order
./dune-common/bin/dunecontrol --opts=config.opts all

cd dune-alugrid
TARGET=check
if [ "$USE_CMAKE" == "yes" ]; then
  cd build-cmake
  TARGET=test
fi
make $MAKE_FLAGS $TARGET
