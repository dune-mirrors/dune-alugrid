// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//
/**

Copyright 2019 Marcel Koch

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef MESH_CONSISTENCY_INCLUDE_IMPL_SORT_NODES_HH
#define MESH_CONSISTENCY_INCLUDE_IMPL_SORT_NODES_HH

#include "elements.hh"

namespace MeshConsistency {
  namespace impl {
    //! sorts the 4 global indices of the nodes of the tetrahedron in ascending order
    template<typename NodeList>
    void sort_nodes(NodeList &l, const impl::Tetrahedron &) {
      if (l[0] > l[1]) std::swap(l[0], l[1]);
      if (l[2] > l[3]) std::swap(l[2], l[3]);
      if (l[0] > l[2]) std::swap(l[0], l[2]);
      if (l[1] > l[3]) std::swap(l[1], l[3]);
      if (l[1] > l[2]) std::swap(l[1], l[2]);
    }

    //! sorts the 3 global indices of the nodes of the triangle in ascending order
    template<typename NodeList>
    void sort_nodes(NodeList &l, const impl::Triangle &) {
      if (l[0] > l[1]) std::swap(l[0], l[1]);
      if (l[0] > l[2]) std::swap(l[0], l[2]);
      if (l[1] > l[2]) std::swap(l[1], l[2]);
    }
  }
}

#endif //MESH_CONSISTENCY_INCLUDE_IMPL_SORT_NODES_HH
