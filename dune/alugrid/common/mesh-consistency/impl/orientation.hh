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

#ifndef MESH_CONSISTENCY_INCLUDE_IMPL_ORIENTATION_HH
#define MESH_CONSISTENCY_INCLUDE_IMPL_ORIENTATION_HH

#include "edge.hh"

namespace MeshConsistency {
  //! Exception if the mesh does not have a consistent orientation.
  class NotOrientable : public std::runtime_error {
  public:
    explicit NotOrientable() : std::runtime_error("Encountered non orientable surface") {}
  };

  namespace impl {
    //! Orientation of an edge e = (v_1, v_2). Positive if v_1 < v_2, negative otherwise.
    enum Orientation { negative = -1, positive = 1 };

    //! Computes the orientation of an edge.
    Orientation edgeOrientation(const Edge &edge) {
      if (edge[0] < edge[1])
        return positive;
      else
        return negative;
    }

    //! Flips the orientation.
    Orientation flipOrientation(const Orientation orientation) {
      if (orientation == Orientation::positive)
        return Orientation::negative;
      else
        return Orientation::positive;
    }

    /*
     * @brief Check if both edges have the same alignment
     * @tparam E TensorElement<dim>
     * Consider for example this element with the global node indices
     * 43 0-----1 7
     *    |     |
     * (2)|     |(3)
     *    |     |
     * 36 2-----3 8
     * then the local edge (2) is not aligned with its global orientation, but the local edge (3) is.
     * In this case the funcion returns true.
     */
    template<typename E>
    bool flippedRelativeOrientation(const LocalEdge <E> &a, const LocalEdge <E> &b) {
      return a.aligned_orientation != b.aligned_orientation;
    }

    template<typename E>
    bool is_consistently_oriented(const std::vector<GlobalEdge < E>>& edges) {
      for (const auto &edge : edges) {
        const auto alignedOrientations = (std::size_t)
          std::count_if(edge.localEdges.begin(), edge.localEdges.end(),
                        [&](const auto &localEdge) {
                          return localEdge.aligned_orientation == true;
                        });
        if (not(alignedOrientations == 0 or alignedOrientations == edge.localEdges.size()))
          return false;
      }
      return true;
    }

    /*
     * @brief Computes the global orientation for each edge s.t. the corresponding mesh becomes consistently oriented.
     * @tparam E TensorElement<dim>
     * @return A vector of global orientations.
     * @exception NotOrientable if the mesh does not have a consistent orientation, for example a Moebius strip.
     */
    template<typename E>
    std::vector<Orientation> compute_edge_orientations(const std::vector<GlobalEdge < E>>& edges){
      std::vector<bool> visited(edges.size(), false);
      std::vector<Orientation> globalOrientation(edges.size(), Orientation::positive);

      // iterate over all edges
      for (const auto &firstEdge : edges){
        if (!visited[firstEdge.globalIdx]) {
          visited[firstEdge.globalIdx] = true;

          globalOrientation[firstEdge.globalIdx] = edgeOrientation(firstEdge);

          std::vector<GlobalEdge < E>> stack{firstEdge};

          // iterate over the current edge set
          while (!stack.empty()) {
            const auto nextEdge = stack.back();
            stack.pop_back();

            for (const auto &localEdge : nextEdge.localEdges) {
              const auto &element = localEdge.element();
              const auto &parallelEdgesIndices = E::Traits::parallelIndices[localEdge.localIdx];

              for (const auto &parallelEdgeIndex : parallelEdgesIndices) {
                const auto &parallelEdge = element.localEdges[parallelEdgeIndex];
                const bool flipped = flippedRelativeOrientation(localEdge, parallelEdge);

                if (!visited[parallelEdge.globalIdx]) {
                  visited[parallelEdge.globalIdx] = true;

                  // if both edges have the same old orientation, they also have the same new orientation,
                  // otherwise the new orientation of the parallelEdge is the opposite of the currentEdge's orientation
                  if (flipped)
                    globalOrientation[parallelEdge.globalIdx] =
                      flipOrientation(globalOrientation[localEdge.globalIdx]);
                  else
                    globalOrientation[parallelEdge.globalIdx] = globalOrientation[localEdge.globalIdx];

                  stack.
                    emplace_back(edges[parallelEdge.globalIdx]);
                } else if (flipped xor (globalOrientation[parallelEdge.globalIdx]
                                        != globalOrientation[localEdge.globalIdx]))
                  throw  NotOrientable{};
              }
            }
          }
        }
      }

      return globalOrientation;
    }
  }
}

#endif //MESH_CONSISTENCY_INCLUDE_IMPL_ORIENTATION_HH
