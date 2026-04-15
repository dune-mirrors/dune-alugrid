from __future__ import absolute_import, division, print_function, unicode_literals

import os
import logging
from dune.grid import reader
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

from dune.fem.importmesh import importMesh

def aluGrid(constructor, dimgrid=None, dimworld=None, elementType=None, comm=None, serial=False, verbose=False,
            lbMethod=9, lbUnder=0.0, lbOver=1.2, defaultBndId=1,
            refinement=None, conforming=False,
            **parameters):
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
        conforming   bool (default False): set to True for simplex grid to get conforming refinement
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
        defaultBndId value used for the id when adding missing boundaries segments
                     default set to '1' - but using the 'not_used' bnd_t enum would be better

    Returns:
    --------

    An ALUGrid instance with given refinement (conforming or nonconforming) and element type (simplex or cube).
    """
    if type(constructor) == dict and "constructor" in constructor.keys():
        return aluGrid(**constructor)
    try:
        useMeshio = constructor[0] == reader.meshio
        fileName = constructor[1]
    except:
        useMeshio = False
    if useMeshio:
        return aluGrid(**importMesh(fileName, defaultBndId=defaultBndId),
                       refinement=refinement, comm=comm, serial=serial, verbose=verbose,
                       lbMethod=lbMethod, lbUnder=lbUnder, lbOver=lbOver,
                       parameters=parameters)

    from dune.grid.grid_generator import module, getDimgrid

    if not dimgrid:
        dimgrid = getDimgrid(constructor)
    if dimworld is None:
        dimworld = dimgrid
    if elementType is None:
        elementType = parameters.pop("type")

    # enable conforming refinement for duration of grid creation
    if conforming or refinement=="Dune::conforming":
        if not elementType == "simplex":
            raise ValueError("conforming grids are only implemented for simplex meshes")
        refVar = ALUGridEnvVar('ALUGRID_CONFORMING_REFINEMENT', 1)
    else:
        refVar = ALUGridEnvVar('ALUGRID_CONFORMING_REFINEMENT', 0)
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
    if refinement=="Dune::conforming" and elementType=="cube":
        raise KeyError("Parameter error in ALUGrid with refinement=" + refinement + " and type=" + elementType + ": conforming refinement is only available with simplex element type")

    typeTag = str(dimgrid) + str(dimworld) + "_" + elementType
    typeName = "Dune::ALUGrid< " + str(dimgrid) + ", " + str(dimworld) + ", Dune::" + elementType

    # if serial flag is true serial version is forced.
    if serial:
        typeName += ", Dune::ALUGridNoComm"

    typeName += " >"
    includes = ["dune/alugrid/grid.hh",
                "dune/alugrid/dgf.hh",
                "dune/python/alugrid/hierarchical.hh"]
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
    return aluGrid(*args, **kwargs, elementType="simplex", conforming=True)
aluConformGrid.__doc__ = aluGrid.__doc__

def aluCubeGrid(*args, **kwargs):
    return aluGrid(*args, **kwargs, elementType="cube")
aluCubeGrid.__doc__ = aluGrid.__doc__

def aluSimplexGrid(*args, **kwargs):
    return aluGrid(*args, **kwargs, elementType="simplex", conforming=False)
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
