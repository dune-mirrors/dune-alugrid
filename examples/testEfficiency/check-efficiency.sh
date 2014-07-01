#/bin/bash

NODES=3
PROCSPERNODE=4

# include a machine file here and other flags
MPICALL="mpirun"
# use the Euler test case
PROBLEM="-DEULER"
# parameters: shock interaction problem with coarse macro grid (21)
PARAM="21 0 1"
EXTRAFLAGS="-DNO_OUTPUT -DCALLBACK_ADAPTATION"

make EXTRAFLAGS="$EXTRAFLAGS" PROBLEM="$PROBLEM" clean main

P=1
while [  $P -le $PROCSPERNODE ]; do
  echo "running ./main $PARAM with $P processes on one machine"
  $MPICALL -np $P ./main "$PARAM" >& main.$P.out
  echo "# running ./main $PARAM with $P processes on one machine" >> main.$P.out
  mv speedup.$P transport_persistent.speedup.$P
  let "P=2*P"
done

N=2
while [  $N -le $NODES ]; do
  let "P=N*PROCSPERNODE"
  echo "running ./main $PARAM with $P processes on $N machine"
  $MPICALL -np $P ./main "$PARAM" >& main.$P.out
  echo "# running ./main $PARAM with $P processes on $N machine" >> main.$P.out
  mv speedup.$P transport_persistent.speedup.$P
  let "N=N+1"
done

grep "finished" main.*.out 

