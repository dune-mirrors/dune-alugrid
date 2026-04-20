#!/bin/bash

# using patched Zoltan version from Robert's github
if ! test -d Zoltan; then
  git clone -b bugfix/openmpi5x https://github.com/dr-robertk/Zoltan
fi

if test -d zoltan; then
  rm -rf zoltan
fi
mkdir zoltan
cd zoltan

FLAGS="-O3 -DNDEBUG -fPIC -Wall"
ZOLTANDIST=../Zoltan

$ZOLTANDIST/configure CXXFLAGS="$FLAGS" CFLAGS="$FLAGS" --prefix=`pwd` --with-mpi-compilers=yes
make -j6
make install

echo "#################################################################################################"
echo "##"
echo "##  echo Use -DZOLTAN_ROOT=`pwd` in DUNE's config.opts"
echo "##"
echo "#################################################################################################"

cd ../
