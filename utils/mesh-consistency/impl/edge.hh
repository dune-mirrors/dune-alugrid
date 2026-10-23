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

#ifndef MESH_CONSISTENCY_INCLUDE_IMPL_EDGE_HH
#define MESH_CONSISTENCY_INCLUDE_IMPL_EDGE_HH

#include <array>
#include <vector>
#include <algorithm>
namespace MeshConsistency {
  namespace impl {
    //! An edge is defined by the global indices of its nodes.
    using Edge = std::array<std::size_t, 2>;

    template<typename T>
    struct hash {};

    /*
     * @brief A hash funtion for edges -> k * v_1 + v_2, where k is large enough s.t. this is unique if v_1 < v_2.
     */
    template<>
    struct hash<Edge> {
      //! Maximal possible global index of an edge.
      const std::size_t max_value;

      explicit hash(const std::size_t max_value_) : max_value(max_value_) {}

      std::size_t operator()(const Edge &a) const noexcept {
        return a[0] * max_value + a[1];
      }
    };

    /*
     * @brief An edge within one element.
     * @tparam E TensorElement<dim>
     */
    template<typename E>
    struct LocalEdge : public Edge {
      //! Index within the element, uses DUNE numbering
      std::size_t localIdx = 0;
      //! Global index of the edge.
      std::size_t globalIdx = 0;
      //! Is the local orientation aligned with the global orientation?
      bool aligned_orientation = true;

      LocalEdge() : Edge{} {};

      LocalEdge(const Edge &nodes, const std::size_t localIdx_, const std::size_t globalIdx_,
                const bool aligned_orientation_, E &element_)
              : Edge(nodes), localIdx(localIdx_), globalIdx(globalIdx_), aligned_orientation(aligned_orientation_),
                element_ptr(&element_) {}

      const E &element() const { return *element_ptr; }

    private:
      //! Pointer to element containing this edge. Local edges are constructed before elements,
      //! thus this needs to be a pointer.
      E *element_ptr = nullptr;
    };

    /*
     * @brief A global representation of an edge, which knows all of its corresponding local edges.
     * @tparam TensorElemend<dim>
     */
    template<typename E>
    struct GlobalEdge : public Edge {
      //! Global index of the edge.
      std::size_t globalIdx = 0;
      //! All local edges, which corresepond to this global edge. In 2D there are at most two local edges,
      //! whereas in 3D the number can be arbitrary. To achive linear complexity it is assumed, that
      //! this number is also bound in 3D.
      std::vector<LocalEdge<E>> localEdges = {};

      GlobalEdge() : Edge{} {};
    };
  }
}

#endif //MESH_CONSISTENCY_INCLUDE_IMPL_EDGE_HH
