import sys
import numpy as np
from dune.grid import reader
import numpy as np

def meshDim(mesh):
    cell_names = {c.type.lower() for c in mesh.cells}
    three_d = {"tetra", "hexahedron", "pyramid", "wedge", "tetra10", "hexa20"}
    two_d = {"triangle", "quad", "triangle6", "quad9"}
    elementType="general"
    if cell_names & three_d:
        if len(cell_names & three_d) == 1:
            if cell_names & {"tetra"}: elementType="simplex"
            elif cell_names & {"hexahedron"}: elementType="cube"
        return 3, elementType
    if cell_names and (cell_names <= two_d or (cell_names & two_d and not (cell_names & three_d))):
        if len(cell_names & two_d) == 1:
            if cell_names & {"triangle"}: elementType="simplex"
            elif cell_names & {"quad"}: elementType="cube"
        return 2, elementType
    return 1, "simplex"

# remark: currently does not support surface grids
# Nice to have: Also setting other properties, i.e., longest edge, type of refinement etc.
def importMesh(msh, ignoreInternalId=0, defaultBndId=None):
    try:
        import meshio
    except ImportError:
        raise ImportError("Function `importMesh` uses the `meshio` package - run `pip install meshio`")
    mesh = meshio.read(msh)
    dim, elementType = meshDim(mesh)

    if dim == 2:
        zCoord = mesh.points[:,2]
        assert np.isclose(min(zCoord),max(zCoord)) # check it's not a surface grid
        points = np.delete( mesh.points, 2, 1) # remove the z component from the points
        bndCells = ["line"]
    else:
        points = mesh.points
        bndCells = {"triangle", "quad", "triangle6", "quad9"}

    cells = mesh.cells_dict
    points = points.astype("float")

    segments = {}
    segs = []
    for cell in bndCells:
        # we prefer 'physical' tagging
        try:
            bnd = list(cells[cell])
        except KeyError:
            continue
        try:
            for i,(line,id) in enumerate( zip(bnd,mesh.cell_data_dict["gmsh:physical"][cell]) ):
                if id == ignoreInternalId: continue # inside skeleton tag
                if id <= 0: continue
                if id in segments:
                    segments[id] += [line]
                else:
                    segments[id] = [line]
                segs += [[id,*line]]
                bnd[i] = None
        except KeyError:
           pass
        # we prefer 'physical' but will try 'geometrical' tagging as well
        try:
            for line,id in zip(bnd,mesh.cell_data_dict["gmsh:geometrical"][cell]):
                if line is None: continue
                if id == ignoreInternalId: continue # inside skeleton tag
                if id <= 0: continue
                if id in segments:
                    segments[id] += [line]
                else:
                    segments[id] = [line]
                segs += [[id,*line]]
        except KeyError:
           pass
    segs = np.array(segs)

    if elementType == "simplex":
        cells = cells["triangle"] if dim==2 else cells["tetra"]
        minIndex = np.inf
        for c in cells:
            minIndex = min(minIndex,min(c))
        for c in cells:
            c -= minIndex
        for s in segs:
            s -= minIndex
        domain = {"vertices":points, "simplices":cells, "boundaries":segs}
        if defaultBndId:
            domain["defaultBndId"] = defaultBndId
    elif elementType == "cube":
        cells = cells["quad"] if dim==2 else cells["hexahedron"]
        if dim==2:
            for c in cells: # gmsh reader has different cube ordering (this could be done in alugrid's gridfactory)
                c[2],c[3] = c[3],c[2]
        elif dim==3:
            for c in cells: # gmsh reader has different cube ordering (this could be done in alugrid's gridfactory)
                c[2],c[3] = c[3],c[2]
                c[6],c[7] = c[7],c[6]
        minIndex = np.inf
        for c in cells:
            minIndex = min(minIndex,min(c))
        for c in cells:
            c -= minIndex
        for s in segs:
            s -= minIndex
        domain = {"vertices":points, "cubes":cells, "boundaries":segs}
        if defaultBndId:
            domain["defaultBndId"] = defaultBndId
    return {"constructor":domain, "dimgrid":dim, "elementType":elementType}
