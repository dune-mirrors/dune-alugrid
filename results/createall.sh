#!/bin/bash

FILES=`ls -d mb*`
PWD=`pwd`

echo $FILES

for DIR in $FILES ; do 
  cd $DIR 
  ../createdat.sh 
  cd ../
done  
