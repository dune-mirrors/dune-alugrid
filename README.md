DUNE-ALUGrid
============

[DUNE-ALUGrid][0] is a [Distributed and Unified Numerics Environment][1]
module which implements the DUNE grid interface
providing unstructured simplicial and cube grids.

A detailed description of all the newer features and some more
details concerning the inner workings of DUNE-ALUGrid can be found
in the paper

[Alkämper, Dedner, Klöfkorn, Nolte. The DUNE-ALUGrid Module, Archive of Numerical Software 4(1), 2016][3] [(bibtex)][6].

**This is the paper we would ask everyone to cite when using DUNE-ALUGrid.**

Download via git:

```
git clone https://gitlab.dune-project.org/extensions/dune-alugrid.git
```

Features of DUNE-ALUGrid include

  *  Cube and simplex grids in 2D and 3D with nonconforming refinement
  *  Simplex grids with conforming refinement (newest vertex bisection) for 2D and 3D
  *  Parallelization and dynamic load balancing for all grids
  *  Internal load balancing based on space filling curves
     making DUNE-ALUGrid self contained also in parallel
  *  Bindings for fully parallel partitioning using [Zoltan][4]
  *  Complete user control of the load balancing
  *  Improved memory footprint

The old ALUGrid version is deprecated and not supported anymore.
We have removed the special grid types e.g. ALUConformGrid, ALUSimplexGrid, and ALUCubeGrid.
Instead the type of the grid is always of the form
Dune::ALUGrid< dimgrid, dimworld, eltype, refinetype, communicator > (where communicator has a default value). The values for eltype are cube,simplex and for refinetype the values are conforming, nonconforming defined in the DUNE namespace.
The GRIDTYPE defines can still be used as before.

The define HAVE_ALUGRID will not work correctly anymore. Since DUNE-ALUGrid is now
a dune module the correct name for the define is HAVE_DUNE_ALUGRID.

License
-------

The DUNE-ALUGrid library, headers and test programs are free open-source software,
licensed under version 2 or later of the GNU General Public License.

See the file [COPYING][5] for full copying permissions.

Installation
------------

For general installation instructions please see the [DUNE website][2].

For installation of Zoltan we recommend to download the package from the
[Zoltan][4] page, unpack and configure with the following parameters:

```
configure CXXFLAGS="-O3 -DNDEBUG -fPIC" CFLAGS="-O3 -DNDEBUG -fPIC" --prefix=PATH_TO_INSTALL_ZOLTAN --with-mpi-compilers=yes --enable-shared

make
make install
```

[0]: https://gitlab.dune-project.org/extensions/dune-alugrid
[1]: http://www.dune-project.org
[2]: http://www.dune-project.org/doc/installation
[3]: http://journals.ub.uni-heidelberg.de/index.php/ans/article/view/23252
[4]: https://sandialabs.github.io/Zoltan/
[5]: https://gitlab.dune-project.org/extensions/dune-alugrid/blob/master/COPYING
[6]: https://gitlab.dune-project.org/extensions/dune-alugrid/blob/master/doc/dunealugrid.bib
