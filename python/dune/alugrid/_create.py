from ._grids import *

registry = dict()
registry["grid"] = {
        "ALU"        : aluGrid,
        "ALUConform" : aluConformGrid,
        "ALUCube" :    aluCubeGrid,
        "ALUSimplex" : aluSimplexGrid,
    }
