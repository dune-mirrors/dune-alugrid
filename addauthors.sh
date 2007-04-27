#!/bin/bash 
AUTHORS=$1
FILE=$2
OUT=tmp.cc.$LOGNAME 
cat $AUTHORS $FILE > $OUT
cp $OUT $FILE 
rm -f $OUT
