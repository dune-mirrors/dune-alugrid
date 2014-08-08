#/bin/bash

# use the Transport test case
PROBLEM="TRANSPORT"
# parameters: use discontinuous advection problem
PARAM="2 0 3"
MPICALL="mpirun -np $1"

echo "running on $1 procs"

make EXTRAFLAGS="-DNO_OUTPUT" PROBLEM="$PROBLEM" clean main
mv main main_blocking
make EXTRAFLAGS="-DNO_OUTPUT -DNON_BLOCKING" PROBLEM="$PROBLEM" clean main
mv main main_nonblocking

$MPICALL ./main_blocking $PARAM >& blocking.$1.out
mv speedup.$1 main_blocking.speedup.$1
$MPICALL ./main_nonblocking $PARAM >& nonblocking.$1.out
mv speedup.$1 main_nonblocking.speedup.$1

grep "finished" nonblocking.$1.out \
                blocking.$1.out 

