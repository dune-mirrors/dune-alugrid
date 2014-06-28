#/bin/bash

MPICALL="mpirun -np $1"

echo "running on $1 procs"

make clean

make EXTRAFLAGS="-DNO_OUTPUT" main_transport
mv main_transport transport_persistent

cd communication
make clean
make EXTRAFLAGS="-DNO_OUTPUT" main_transport
mv main_transport transport_overlapcom

cd ..

echo "non overlapping"
$MPICALL ./transport_persistent 2 0 3 >& transport_persistent.$1.out
mv speedup.$1 transport_persistent.speedup.$1

cd communication
echo "overlapping"
$MPICALL ./transport_overlapcom 2 0 3 >& transport_overlapcom.$1.out
mv speedup.$1 transport_overlapcom.speedup.$1

cd ..

grep "finished" transport_persistent.$1.out \
                communication/transport_overlapcom.$1.out 

