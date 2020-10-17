# Meta data
name="dune-alugrid"
version="2.8.201014"
author="Robert Kloefkorn, Martin Alkaemper, Andreas Dedner and Martin Nolte"
author_email="dune@lists.dune-project.org"
description="module providing the DUNE grid interface for unstructured simplicial and cube grids in 2 and 3 space dimensions."
url="https://gitlab.dune-project.org/extensions/dune-alugrid"

# Package dependencies
install_requires=['dune-grid']

# Files to include in the source package
manifest='''\
graft cmake
graft doc
graft dune
graft lib
graft python
graft utils
include CMakeLists.txt
include config.h.cmake
include dune-alugrid.pc.in
include dune.module
include pyproject.toml
include README.md
'''
