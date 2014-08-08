#/bin/bash

NODES=3
PROCSPERNODE=2

# include a machine file here and other flags
MPICALL="mpirun"
# use the Euler test case
PROBLEM="EULER"
# parameters: shock interaction problem with coarse macro grid (21)
PARAM="21 0 2"
EXTRAFLAGS="-DNO_OUTPUT -DCALLBACK_ADAPTATION -DNON_BLOCKING"

make EXTRAFLAGS="$EXTRAFLAGS" PROBLEM="$PROBLEM" clean main

P=1
while [  $P -le $PROCSPERNODE ]; do
  echo "running ./main $PARAM with $P processes on one machine"
  name=$(printf main.%04d.out $P)
  $MPICALL -np $P ./main $PARAM >& $name
  echo "# running ./main $PARAM with $P processes on one machine" >> $name
  let "P=2*P"
done

N=2
while [  $N -le $NODES ]; do
  let "P=N*PROCSPERNODE"
  echo "running ./main $PARAM with $P processes on $N machine"
  name=$(printf main.%04d.out $P)
  $MPICALL -np $P ./main $PARAM >& $name
  echo "# running ./main $PARAM with $P processes on $N machine" >> $name
  let "N=N+1"
done

grep "finished" main.*.out 

