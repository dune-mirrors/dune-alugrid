DUNE-ALUGrid
============

[DUNE-ALUGrid][0] is a [Distributed and Unified Numerics Environment][1]
module which implements the DUNE grid interface
providing unstructured simplicial and cube grids.

A detailed description of all the new features and some more
details concerning the inner workings of DUNE-ALUGrid can be found
in the paper The DUNE-ALUGrid Module. http://arxiv.org/abs/1407.6954.

This is the paper we would ask everyone to cite when using DUNE-ALUGrid.

Download via git:
git clone https://gitlab.dune-project.org/extensions/dune-alugrid.git

New features and improvements include

  *  Conforming refinement for the 3D simplex grid
  *  Parallel 2D grids
  *  Internal load balancing based on space filling curves
     making DUNE-ALUGrid self contained also in parallel
  *  Bindings for fully parallel partitioners using [Zoltan][3]
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

See the file [LICENSE][4] for full copying permissions.

Installation
------------

For installation instructions please see the [DUNE website][2].

Links
-----

 [0]: http://www.dune-project.org/extensions/dune-alugrid
 [1]: http://www.dune-project.org
 [2]: http://www.dune-project.org/doc/installation-notes.html
 [3]: http://www.cs.sandia.gov/Zoltan/
 [4]: LICENSE
