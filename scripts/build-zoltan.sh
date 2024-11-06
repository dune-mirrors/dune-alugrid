#!/bin/bash
ZOLTAN_VERSION=3.901

mkdir ZoltanDist
cd ZoltanDist
if ! test -f v$ZOLTAN_VERSION.tar.gz; then
  wget https://github.com/sandialabs/Zoltan/archive/refs/tags/v$ZOLTAN_VERSION.tar.gz
fi
tar zxvf v$ZOLTAN_VERSION.tar.gz

cd ../
if test -d zoltan; then
  rm -rf zoltan
  mkdir zoltan
fi
cd zoltan

../ZoltanDist/Zoltan-$ZOLTAN_VERSION/configure CXXFLAGS="-O3 -DNDEBUG -fPIC" CFLAGS="-O3 -DNDEBUG -fPIC" --prefix=`pwd` --with-mpi-compilers=yes --enable-shared
make -j6
make install

echo "#################################################################################################"
echo "##"
echo "##  echo Use -DZOLTAN_ROOT=`pwd` in DUNE's config.opts"
echo "##"
echo "#################################################################################################"

cd ../
