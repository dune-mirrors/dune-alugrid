#/bin/bash

PROBLEM="TRANSPORT"
PARAM="21 0 4 none"

make clean
make EXTRAFLAGS="-DNO_OUTPUT" PROBLEM="$PROBLEM" clean main
mv main main_persistent
make EXTRAFLAGS="-DUSE_VECTOR_FOR_PWF -DNO_OUTPUT" PROBLEM="$PROBLEM" clean main
mv main main_vector

make EXTRAFLAGS="-DNO_OUTPUT -DCALLBACK_ADAPTATION" PROBLEM="$PROBLEM" clean main
mv main main_persistent_callback
make EXTRAFLAGS="-DUSE_VECTOR_FOR_PWF -DNO_OUTPUT -DCALLBACK_ADAPTATION" PROBLEM="$PROBLEM" clean main
mv main main_vector_callback

./main_persistent $PARAM >& persistent.out
mv speedup.1 persistent.speedup.1
./main_vector $PARAM >& vector.out
mv speedup.1 vector.speedup.1

./main_persistent_callback $PARAM >& persistent_callback.out
mv speedup.1 persistent_callback.speedup.1
./main_vector_callback $PARAM >& vector_callback.out
mv speedup.1 vector_callback.speedup.1

grep "finished" persistent.out \
                vector.out \
                persistent_callback.out \
                vector_callback.out

