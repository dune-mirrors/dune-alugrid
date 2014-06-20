#/bin/bash

make clean
make EXTRAFLAGS="-DNO_OUTPUT" main_transport
mv main_transport transport_persistent
make EXTRAFLAGS="-DUSE_VECTOR_FOR_PWF -DNO_OUTPUT" main_transport
mv main_transport transport_vector

cd loadbalance
make EXTRAFLAGS="-DNO_OUTPUT" PROBLEM="-DTRANSPORT" main_internal
mv main_internal transport_persistent
make EXTRAFLAGS="-DUSE_VECTOR_FOR_PWF -DNO_OUTPUT" PROBLEM="-DTRANSPORT" main_internal
mv main_internal transport_vector

cd ..

./transport_persistent 2 0 3 >& transport_persistent.out
mv speedup.1 transport_persistent.speedup.1
./transport_vector 2 0 3 >& transport_vector.out
mv speedup.1 transport_vector.speedup.1

cd loadbalance
./transport_persistent 2 0 3 >& transport_persistent.out
mv speedup.1 transport_persistent.speedup.1
./transport_vector 2 0 3 >& transport_vector.out
mv speedup.1 transport_vector.speedup.1

cd ..

grep "finished" transport_persistent.out \
                transport_vector.out \
                loadbalance/transport_persistent.out \
                loadbalance/transport_vector.out
