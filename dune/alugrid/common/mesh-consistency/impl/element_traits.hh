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

#ifndef MESH_CONSISTENCY_INCLUDE_IMPL_ELEMENT_TRAITS_HH
#define MESH_CONSISTENCY_INCLUDE_IMPL_ELEMENT_TRAITS_HH

#include <array>
#include "edge.hh"

namespace MeshConsistency {
  namespace impl {
    /*
     * @brief Local geometry information.
     * @tparam dim dimension of element.
     */
    template<std::size_t dim>
    class TensorElementTraits {};

    template<>
    class TensorElementTraits<2> {
    public:
      static constexpr std::size_t dim = 2;
      static constexpr std::size_t nodes_per_element = 4;
      static constexpr std::size_t edges_per_element = 4;
      //! local node indices for each edge
      static constexpr Edge localEdgeNodes[4] = {{0, 2}, {1, 3},
                                                 {0, 1}, {2, 3}};
      //! parallelIndices[i] is the local index of the edge locally parallel to the i-th edge
      static constexpr std::array<int, 1> parallelIndices[4] = {{1}, {0},
                                                                {3}, {2}};
    };
    constexpr std::size_t TensorElementTraits<2>::dim;
    constexpr std::size_t TensorElementTraits<2>::nodes_per_element;
    constexpr std::size_t TensorElementTraits<2>::edges_per_element;
    constexpr Edge TensorElementTraits<2>::localEdgeNodes[4];
    constexpr std::array<int, 1> TensorElementTraits<2>::parallelIndices[4];

    template<>
    class TensorElementTraits<3> {
    public:
      static constexpr std::size_t dim = 3;
      static constexpr std::size_t nodes_per_element = 8;
      static constexpr std::size_t edges_per_element = 12;
      //! local node indices for each edge
      static constexpr Edge localEdgeNodes[12] = {{0, 4}, {1, 5}, {2, 6}, {3, 7},
                                                  {0, 2}, {1, 3}, {4, 6}, {5, 7},
                                                  {0, 1}, {2, 3}, {4, 5}, {6, 7}};
      //! parallelIndices[i] is the local indices of the edges locally parallel to the i-th edge
      static constexpr std::array<int, 3> parallelIndices[12] = {{1, 2,  3}, {0, 2,  3}, {0, 1,  3}, {0, 1,  2},
                                                                 {5, 6,  7}, {4, 6,  7}, {4, 5,  7}, {4, 5,  6},
                                                                 {9, 10, 11}, {8, 10, 11}, {8, 9,  11}, {8, 9,  10}};

    };
    constexpr std::size_t TensorElementTraits<3>::dim;
    constexpr std::size_t TensorElementTraits<3>::nodes_per_element;
    constexpr std::size_t TensorElementTraits<3>::edges_per_element;
    constexpr Edge TensorElementTraits<3>::localEdgeNodes[12];
    constexpr std::array<int, 3> TensorElementTraits<3>::parallelIndices[12];
  }
}

#endif //MESH_CONSISTENCY_ELEMENT_INCLUDE_IMPL_TRAITS_HH
