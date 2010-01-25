#!/bin/bash 

REM_PKG_CONFIG_PATH=$PKG_CONFIG_PATH
PKG_CONFIG_PATH=`pwd`:$PKG_CONFIG_PATH

ALUGRID_VERSION=`pkg-config --modversion alugrid`
echo "Testing Version $ALUGRID_VERSION"
if pkg-config --atleast-version=1.22 alugrid ; then 
  echo "Version ok" 
else 
  echo "Version not ok"
fi 

PKG_CONFIG_PATH=$REM_PKG_CONFIG_PATH
#echo $PKG_CONFIG_PATH
