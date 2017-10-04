#!/bin/bash

BUILDDIR=${HOME}/software/dune/dune-alugrid/build-optim/examples/quality/
PAPERDIR=${HOME}/NMH/git/WeakCompCond/plotdata/
cd ${BUILDDIR}

for MESH in 00 0 1 2 3 4 5 6 ; do
  mkdir ${PAPERDIR}/tet.${MESH}.msh
  for ANNOUNCED in 0 1 ; do
    for VARIANT in 1 2 ; do
      cp ./meshquality/tet.${MESH}.msh/${VARIANT}-0-${ANNOUNCED}-meshquality.gnu ./meshquality/tet.${MESH}.msh/threshold-${VARIANT}-${ANNOUNCED}.plot
      grep --after-context=1 "#V0" ./meshquality/tet.${MESH}.msh/quality-${VARIANT}-0-${ANNOUNCED}.out > ./meshquality/tet.${MESH}.msh/V0V1-${VARIANT}-${ANNOUNCED}.plot
      grep --after-context=1 "MacroFaces" ./meshquality/tet.${MESH}.msh/quality-${VARIANT}-0-${ANNOUNCED}.out > ./meshquality/tet.${MESH}.msh/NonCompat-${VARIANT}-${ANNOUNCED}.plot
      for THRESHOLD in `seq 1 35` ; do
        tail -n 1 ./meshquality/tet.${MESH}.msh/${VARIANT}-${THRESHOLD}-${ANNOUNCED}-meshquality.gnu >> ./meshquality/tet.${MESH}.msh/threshold-${VARIANT}-${ANNOUNCED}.plot
        grep --after-context=1 "#V0" ./meshquality/tet.${MESH}.msh/quality-${VARIANT}-${THRESHOLD}-${ANNOUNCED}.out | tail -n 1 >> ./meshquality/tet.${MESH}.msh/V0V1-${VARIANT}-${ANNOUNCED}.plot
        grep --after-context=1 "MacroFaces" ./meshquality/tet.${MESH}.msh/quality-${VARIANT}-${THRESHOLD}-${ANNOUNCED}.out | tail -n 1 >> ./meshquality/tet.${MESH}.msh/NonCompat-${VARIANT}-${ANNOUNCED}.plot
      done
      cp ./meshquality/tet.${MESH}.msh/V0V1-${VARIANT}-${ANNOUNCED}.plot ${PAPERDIR}/tet.${MESH}.msh/
      cp ./meshquality/tet.${MESH}.msh/threshold-${VARIANT}-${ANNOUNCED}.plot ${PAPERDIR}/tet.${MESH}.msh/
      cp ./meshquality/tet.${MESH}.msh/NonCompat-${VARIANT}-${ANNOUNCED}.plot ${PAPERDIR}/tet.${MESH}.msh/
    done
  done
done
