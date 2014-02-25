#!/bin/bash

FILES=`ls speedup.*`

RESULT=scaling.dat
OPTIMAL=optimal.dat

MINPROC=1000000000
MINTIME=0
MAXPROC=0

echo "#procs    time" > $RESULT
echo "#procs    time" > $OPTIMAL

echo $FILES 

for DAT in $FILES; do 
  LINE=`tail -n 3 $DAT | head -1`
  PROC=`head -1 $DAT | cut -d " " -f 4`

  TIME=`echo $LINE | cut -d " " -f 5`
  MEM=`echo $LINE | cut -d " " -f 6`
  if test $PROC -le $MINPROC ; then 
    MINPROC=$PROC
    MINTIME=$TIME
  fi  
  if test $PROC -ge $MAXPROC ; then 
    MAXPROC=$PROC
  fi  
  echo "$PROC $TIME" >> $RESULT
done  

cp $RESULT /tmp/$RESULT
sort -g /tmp/$RESULT > $RESULT

MAXTIME=`echo "$MINTIME / $MAXPROC * $MINPROC" | bc -l`

echo "$MINPROC $MINTIME" >> $OPTIMAL 
echo "$MAXPROC $MAXTIME" >> $OPTIMAL 
