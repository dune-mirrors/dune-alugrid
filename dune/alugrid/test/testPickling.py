import dune.alugrid as alu
from dune.grid import cartesianDomain
import dune.common.pickle
from matplotlib import pyplot
from dune.fem.plotting import plotPointData


def write(fileName, view, fig, axs):
    grid = view( cartesianDomain([-2,-2],[2,2],[2,2]) )
    print( grid.hierarchicalGrid.refineStepsForHalf )
    grid.hierarchicalGrid.globalRefine(2)
    plotPointData(grid,figure=(fig,axs[0][0]))
    with open(fileName,"wb") as f:
        dune.common.pickle.dump([grid],f)
    grid.hierarchicalGrid.globalRefine(2)
    plotPointData(grid,figure=(fig,axs[0][1]))

def read(fileName, fig, axs):
    with open(fileName,"rb") as f:
        grid, = dune.common.pickle.load(f)
    print( grid.hierarchicalGrid.refineStepsForHalf )
    plotPointData(grid,figure=(fig,axs[1][0]))
    grid.hierarchicalGrid.globalRefine(2)
    plotPointData(grid,figure=(fig,axs[1][1]))

fig,axs = pyplot.subplots(2,2)
write("testSimplex.dbf",alu.aluSimplexGrid, fig,axs)
read("testSimplex.dbf", fig,axs)
pyplot.show()

fig,axs = pyplot.subplots(2,2)
write("testConf.dbf",alu.aluConformGrid, fig,axs)
# this does not solve the problem (but gives the correct 'empty grid' output)
# refVar = alu.ALUGridEnvVar('ALUGRID_CONFORMING_REFINEMENT', 1)
read("testConf.dbf", fig,axs)
pyplot.show()
