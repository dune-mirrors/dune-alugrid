#/bin/bash

NODES=2
PROCSPERNODE=4

# include a machine file here and other flags
MPICALL="mpirun"
# use the Euler test case
PROBLEM="-DEULER"
# parameters: shock interaction problem with coarse macro grid (21)
PARAM="21 0 1"
EXTRAFLAGS="-DNO_OUTPUT -DCALLBACK_ADAPTATION"

make clean
make EXTRAFLAGS="$EXTRAFLAGS" PROBLEM="$PROBLEM" main

P=1
while [  $P -le $PROCSPERNODE ]; do
  echo "running ./main $PARAM" with $P processes on one machine"
  $MPICALL -np $P ./main "$PARAM" >& main.$P.out
  echo "# running ./main $PARAM" with $P processes on one machine" >> main.$P.out
  mv speedup.$P transport_persistent.speedup.$P
  let P=2*P
done

M=2
while [  $M -le $NODES ]; do
  let P=M*$PROCSNODE
  echo "running ./main $PARAM" with $P processes on $M machine"
  $MPICALL -np $P ./main "$PARAM" >& main.$P.out
  echo "# running ./main $PARAM" with $P processes on $M machine" >> main.$P.out
  mv speedup.$P transport_persistent.speedup.$P
done

grep "finished" main.*.out 

