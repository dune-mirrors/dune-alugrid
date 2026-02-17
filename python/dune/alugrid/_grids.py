from __future__ import absolute_import, division, print_function, unicode_literals

import os
import logging
logger = logging.getLogger(__name__)

# class holding an env variable name and whether to delete it again
class ALUGridEnvVar:
    def __init__(self, varname, value):
        self._deleteEnvVar = False
        self._varname = varname
        if varname not in os.environ:
            os.environ[self._varname] = str(value)
            self._deleteEnvVar = True
    def __del__(self):
        if self._deleteEnvVar:
            del os.environ[self._varname]
#-------------------------------------------------------------------
# grid module loading
def checkModule(includes, typeName, typeTag):
    from importlib import import_module
    from dune.grid.grid_generator import module

    # check if pre-compiled module exists and if so load it
    try:
        gridModule = import_module("dune.alugrid._alugrid._alugrid_" + typeTag)
        return gridModule
    except ImportError:
        # otherwise proceed with generate, compile, load
        gridModule = module(includes, typeName)
        return gridModule

def aluGrid(constructor, dimgrid=None, dimworld=None, elementType=None, refinement=None, comm=None, serial=False, verbose=False,
            lbMethod=9, lbUnder=0.0, lbOver=1.2, **parameters):
    """
    Create an ALUGrid instance.

    Note: This functions has to be called on all cores and the parameters passed should be the same.
          Otherwise unexpected behavior will occur.

    Parameters:
    -----------

        constructor  means of constructing the grid, i.e. a grid reader or a
                     dictionary holding macro grid information
        dimgrid      dimension of grid, i.e. 2 or 3
        dimworld     dimension of world, i.e. 2 or 3 and >= dimension
        comm         MPI communication (not yet implemented)
        serial       creates a grid without MPI support (default False)
        verbose      adds some verbosity output (default False)
        lbMethod     load balancing algorithm. Possible choices are (default is 9):
                         0  None
                         1  Collect (to rank 0)
                         4  ALUGRID_SpaceFillingCurveLinkage (assuming the macro
                            elements are ordering along a space filling curve)
                         5  ALUGRID_SpaceFillingCurveSerialLinkage (serial version
                            of 4 which requires the entire graph to fit to one core)
                         9  ALUGRID_SpaceFillingCurve (like 4 without linkage
                            storage), this is the default option.
                         10 ALUGRID_SpaceFillingCurveSerial (serial version
                            of 10 which requires the entire graph to fit to one core)
                         11 METIS_PartGraphKway, METIS method PartGraphKway, see
                            http://glaros.dtc.umn.edu/gkhome/metis/metis/overview
                         12 METIS_PartGraphRecursive, METIS method
                            PartGraphRecursive, see
                            http://glaros.dtc.umn.edu/gkhome/metis/metis/overview
                         13 ZOLTAN_LB_HSFC, Zoltan's geometric load balancing based
                            on a Hilbert space filling curve, see https://sandialabs.github.io/Zoltan/
                         14 ZOLTAN_LB_GraphPartitioning, Zoltan's load balancing
                            method based on graph partitioning, see https://sandialabs.github.io/Zoltan/
                         15 ZOLTAN_LB_PARMETIS, using ParMETIS through Zoltan, see
                            https://sandialabs.github.io/Zoltan/
        lbUnder      value between 0.0 and 1.0 (default 0.0)
        lbOver       value between 1.0 and 2.0 (default 1.2)

    Returns:
    --------

    An ALUGrid instance with given refinement (conforming or nonconforming) and element type (simplex or cube).
    """
    from dune.grid.grid_generator import module, getDimgrid

    if not dimgrid:
        dimgrid = getDimgrid(constructor)

    if dimworld is None:
        dimworld = dimgrid
    if elementType is None:
        elementType = parameters.pop("type")

    verbosity = ALUGridEnvVar('ALUGRID_VERBOSITY_LEVEL', 2 if verbose else 0)

    if lbMethod < 0 or lbMethod > 15:
        raise ValueError("lbMethod should be between 0 and 15!")

    lbMth = ALUGridEnvVar('ALUGRID_LB_METHOD', lbMethod)
    lbUnd = ALUGridEnvVar('ALUGRID_LB_UNDER',  lbUnder)
    lbOve = ALUGridEnvVar('ALUGRID_LB_OVER',   lbOver)

    if not (2 <= dimgrid and dimgrid <= dimworld):
        raise KeyError("Parameter error in ALUGrid with dimgrid=" + str(dimgrid) + ": dimgrid has to be either 2 or 3")
    if not (2 <= dimworld and dimworld <= 3):
        raise KeyError("Parameter error in ALUGrid with dimworld=" + str(dimworld) + ": dimworld has to be either 2 or 3")
    if refinement=="Dune::conforming" and elementType=="Dune::cube":
        raise KeyError("Parameter error in ALUGrid with refinement=" + refinement + " and type=" + elementType + ": conforming refinement is only available with simplex element type")

    typeTag = str(dimgrid) + str(dimworld) + "_" + elementType
    typeName = "Dune::ALUGrid< " + str(dimgrid) + ", " + str(dimworld) + ", Dune::" + elementType
    if refinement is not None:
        assert refinement == 'conforming' or refinement == 'nonconforming', "Refinement should be 'conforming' or 'nonconforming' if selected."
        typeName += ", Dune::" + refinement

    # if serial flag is true serial version is forced.
    if serial:
        typeName += ", Dune::ALUGridNoComm"

    typeName += " >"
    includes = ["dune/alugrid/grid.hh", "dune/alugrid/dgf.hh"]
    gridModule = checkModule(includes, typeName, typeTag)

    if comm is not None:
        raise Exception("Passing communicator to grid construction is not yet implemented in Python bindings of dune-grid")
        # return gridModule.LeafGrid(gridModule.reader(constructor, comm))

    gridView = gridModule.LeafGrid(gridModule.reader(constructor))

    # in case of a carteisan domain store if old or new boundary ids was used
    # this can be removed in later version - it is only used in dune-fem
    # to give a warning that the boundary ids for the cartesian domains have changed
    try:
        gridView.hierarchicalGrid._cartesianConstructionWithIds = constructor.boundaryWasSet
    except AttributeError:
        pass
    return gridView

def aluConformGrid(*args, **kwargs):
    # enable conforming refinement for duration of grid creation
    refVar = ALUGridEnvVar('ALUGRID_CONFORMING_REFINEMENT', 1)
    return aluGrid(*args, **kwargs, elementType="simplex")
aluConformGrid.__doc__ = aluGrid.__doc__

def aluCubeGrid(*args, **kwargs):
    return aluGrid(*args, **kwargs, elementType="cube")
aluCubeGrid.__doc__ = aluGrid.__doc__

def aluSimplexGrid(*args, **kwargs):
    return aluGrid(*args, **kwargs, elementType="simplex")
aluSimplexGrid.__doc__ = aluGrid.__doc__

grid_registry = {
        "ALU"        : aluGrid,
        "ALUConform" : aluConformGrid,
        "ALUCube" :    aluCubeGrid,
        "ALUSimplex" : aluSimplexGrid,
    }

if __name__ == "__main__":
    import doctest
    doctest.testmod(optionflags=doctest.ELLIPSIS)
