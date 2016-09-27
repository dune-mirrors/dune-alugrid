from __future__ import absolute_import, division, print_function, unicode_literals

def aluGrid(constructor, dimgrid, dimworld=None, elementType=None, **parameters):
    from dune.grid.grid_generator import module

    if dimworld is None:
        dimworld = dimgrid
    if elementType is None:
        elementType = parameters.pop("type")
    refinement = parameters["refinement"]

    if refinement == "conforming":
        refinement="Dune::conforming"
    elif refinement == "nonconforming":
        refinement="Dune::nonconforming"

    if not (2 <= dimworld and dimworld <= 3):
        raise KeyError("Parameter error in ALUGrid with dimworld=" + str(dimworld) + ": dimworld has to be either 2 or 3")
    if not (2 <= dimgrid and dimgrid <= dimworld):
        raise KeyError("Parameter error in ALUGrid with dimgrid=" + str(dimgrid) + ": dimgrid has to be either 2 or 3")
    if refinement=="Dune::conforming" and elementType=="Dune::cube":
        raise KeyError("Parameter error in ALUGrid with refinement=" + refinement + " and type=" + elementType + ": conforming refinement is only available with simplex element type")

    typeName = "Dune::ALUGrid< " + str(dimgrid) + ", " + str(dimworld) + ", " + elementType + ", " + refinement + " >"
    includes = ["dune/alugrid/grid.hh", "dune/alugrid/dgf.hh"]
    gridModule = module(includes, typeName)

    return gridModule.LeafGrid(gridModule.reader(constructor))


def aluConformGrid(constructor, dimgrid, dimworld=None, **parameters):
    return aluGrid(constructor, dimgrid, dimworld, elementType="Dune::simplex", refinement="Dune::conforming")


def aluCubeGrid(constructor, dimgrid, dimworld=None, **parameters):
    return aluGrid(constructor, dimgrid, dimworld, elementType="Dune::cube", refinement="Dune::nonconforming")


def aluSimplexGrid(constructor, dimgrid, dimworld=None):
    return aluGrid(constructor, dimgrid, dimworld, elementType="Dune::simplex", refinement="Dune::nonconforming")


if __name__ == "__main__":
    import doctest
    doctest.testmod(optionflags=doctest.ELLIPSIS)
