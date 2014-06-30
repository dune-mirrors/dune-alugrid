#/bin/bash

PROBLEM="TRANSPORT"
MPICALL="mpirun -np $1"

echo "running on $1 procs"

make EXTRAFLAGS="-DNO_OUTPUT" PROBLEM="$PROBLEM" clean main
mv main main_nonblocking
cd communication
make EXTRAFLAGS="-DNO_OUTPUT -DNON_BLOCKING" PROBLEM="$PROBLEM" clean main
mv main main_blocking

$MPICALL ./main_nonblocking 2 0 3 >& nonblocking.$1.out
mv speedup.$1 main_nonblocking.speedup.$1
$MPICALL ./main_blocking 2 0 3 >& blocking.$1.out
mv speedup.$1 main_blocking.speedup.$1

grep "finished" nonblocking.$1.out \
                blocking.$1.out 
