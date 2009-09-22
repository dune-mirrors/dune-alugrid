#!/bin/sh 

extractVersion()
{
V1=`grep "$1" $4 | awk '{ printf "%1d" , $3}'`
V2=`grep "$2" $4 | awk '{ printf "%1d" , $3}'`
V3=`grep "$3" $4 | awk '{ printf "%1d" , $3}'`

echo "$V1.$V2.$V3" 
}

# use with parmetis header 
PARMETISHEADER=$1 
(extractVersion "_MAJOR_VERSION" "_MINOR_VERSION" "_SUBMINOR_VERSION" $PARMETISHEADER)
exit 0
