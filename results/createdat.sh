#!/bin/bash

FILES=`ls speedup.*`

RESULT=scaling.dat
OPTIMAL=optimal.dat

MINPROC=1000000000
MINVAL=0
MAXPROC=0

echo "#procs    time     adapt     lb      mem " > $RESULT
echo "#procs    time" > $OPTIMAL

echo $FILES 

for DAT in $FILES; do 
  LINE=`tail -n 3 $DAT | head -1`
  PROC=`head -1 $DAT | cut -d " " -f 4`

  TIME=`echo $LINE | cut -d " " -f 5`
  LBTIME=`echo $LINE | cut -d " " -f 4`
  ADTIME=`echo $LINE | cut -d " " -f 3`
  MEM=`echo $LINE | cut -d " " -f 6`
  if test $PROC -le $MINPROC ; then 
    MINPROC=$PROC
    MINVAL[0]=$TIME
    MINVAL[1]=$ADTIME
    MINVAL[2]=$LBTIME
    MINVAL[3]=$MEM
  fi  
  if test $PROC -ge $MAXPROC ; then 
    MAXPROC=$PROC
  fi  
  echo "$PROC $TIME $ADTIME $LBTIME $MEM" >> $RESULT
done  

cp $RESULT /tmp/$RESULT
sort -g /tmp/$RESULT > $RESULT

MAXVAL[0]=`echo "${MINVAL[0]} / $MAXPROC * $MINPROC" | bc -l` 
MAXVAL[1]=`echo "${MINVAL[1]} / $MAXPROC * $MINPROC" | bc -l` 
MAXVAL[2]=`echo "${MINVAL[2]} / $MAXPROC * $MINPROC" | bc -l` 
MAXVAL[3]=`echo "${MINVAL[3]} / $MAXPROC * $MINPROC" | bc -l` 

echo "$MINPROC ${MINVAL[0]} ${MINVAL[1]} ${MINVAL[2]} ${MINVAL[3]}" >> $OPTIMAL 
echo "$MAXPROC ${MAXVAL[0]} ${MAXVAL[1]} ${MAXVAL[2]} ${MAXVAL[3]}" >> $OPTIMAL 
