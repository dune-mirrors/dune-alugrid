from ._alugrid import *
from ._grids import *

registry = dict()
registry["grid"] = grid_registry

# references for ALUGrid
__cite_dune_module_as__ = """
@article{alugrid:16,
  author = {Alk{\\"a}mper, M. and Dedner, A. and Kl{\\"o}fkorn, R. and Nolte, M.},
  title = {{The DUNE-ALUGrid Module.}},
  journal = {Archive of Numerical Software},
  volume = {4},
  number = {1},
  year = {2016},
  pages = {1--28},
  doi = {10.11588/ans.2016.1.23252}
}
"""
__conforming_citations__ = None

# references for conforming ALUGrid
def _add_conforming_citation():
    global __cite_dune_module_as__
    global __conforming_citations__
    if __conforming_citations__ is None:
        __conforming_citations__ = True
        __cite_dune_module_as__ += """
@article{dnvb:16,
  author    = {Alk{\\"{a}}mper, M. and  Kl{\\"o}fkorn, R.},
  title = {{Distributed Newest Vertex Bisection}},
  journal = {Journal of Parallel and Distributed Computing},
  volume = {104},
  pages = {1 - 11},
  year = {2017},
  doi = {10.1016/j.jpdc.2016.12.003}
}
"""
