# run with mpirun -np 2 python test_parallelctor.py
# does nothing with other process counts:


import dune.common
from dune.grid import reader, cartesianDomain
from dune.alugrid import aluConformGrid as leafGridView

def test(version):
    rank = dune.common.comm.rank

    if version == 0:
        if rank == 0:
            domain = (reader.meshio, "wave_tank.msh")
        else:
            domain = {}
    else:
        domain = (reader.meshio, "wave_tank.msh")
    gridView  = leafGridView( domain, dimgrid=2 )
    assert (rank==0 and gridView.size(0)==41764) or (rank==1 and gridView.size(0)==0)
    # print("rank:", rank, gridView.size(0),flush=True)
    gridView.hierarchicalGrid.loadBalance()
    assert gridView.size(0)==20882
    # print("rank:", rank, gridView.size(0),flush=True)

    # Note that using 'version==0' approach from above would fail
    domain = cartesianDomain([4, 4], [8, 8], [3, 3])
    gridView  = leafGridView( domain, dimgrid=2 )
    assert gridView.size(0)==9
    # print("rank:", rank, gridView.size(0),flush=True)

if dune.common.comm.size == 2:
    test(version=0)
    # this does not work
    # test(version=1)
