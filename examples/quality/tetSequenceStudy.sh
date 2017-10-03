#!/bin/bash

BUILDDIR=${HOME}/software/dune/dune-alugrid/build-optim/examples/quality

cd ${BUILDDIR}

make -j4 main_quality

mkdir meshquality

for MESH in 00 0 1 2 3 4 5 6 ; do
  mkdir meshquality/tet.${MESH}.msh
  for ANNOUNCED in 0 1 ; do
    ./main_quality ../dgf/tet.${MESH}.msh.dgf 0 0 ${ANNOUNCED} &> ./meshquality/tet.${MESH}.msh/quality-0-0-${ANNOUNCED}.out
    for VARIANT in 1 2 ; do
      for THRESHOLD in `seq 0 35` ; do
          ./main_quality ../dgf/tet.${MESH}.msh.dgf ${VARIANT} ${THRESHOLD} ${ANNOUNCED} &> ./meshquality/tet.${MESH}.msh/quality-${VARIANT}-${THRESHOLD}-${ANNOUNCED}.out
      done
    done
  done
done
