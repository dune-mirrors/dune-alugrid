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

#ifndef MESH_CONSISTENCY_INCLUDE_IMPL_REORDER_NODES_HH
#define MESH_CONSISTENCY_INCLUDE_IMPL_REORDER_NODES_HH

#include <cfloat>
#include "elements.hh"
#include "orientation.hh"

namespace MeshConsistency {
  namespace impl {
    /*
     * @brief Checks for quadrilaterals, if the volume of an element is negative.
     * @tparam Vertex Position in 2D.
     * @param all_vertices a vector with all vertices of the mesh.
     * @param element the element to check.
     * @return has the element negative volume.
     */
    template<typename Vertex>
    bool negative_volume(const std::vector<Vertex> &all_vertices, const Quadrilateral &element) {
      auto origin = all_vertices[element.nodes[0]];
      std::array<Vertex, 2> vertices = {};

      vertices[0] = all_vertices[element.nodes[1]];
      vertices[1] = all_vertices[element.nodes[2]];

      for (auto &v: vertices)
        for (std::size_t i = 0; i < origin.size(); ++i) {
          v[i] -= origin[i];
        }

      // use the same check as in UG to prevent UG from reordering the element corners
      return (vertices[0][0] * vertices[1][1] - vertices[1][0] * vertices[0][1]) < FLT_EPSILON * 10;
    }

    /*
     * @brief Corrects the sign of the volume of a quadrilateral, s.t. the orientation of its edges is untouched.
     * @tparam NodeList container of global indices.
     * @tparam Vertex Position in 2D.
     * @param[in,out] node_list global indices of the nodes of the element. After this call the element has positive
     *                          volume.
     * @param vertices all vertices of the mesh.
     * @param element the element corresponding to node_list.
     */
    template<typename NodeList, typename Vertex>
    void correct_volume(NodeList &node_list, const std::vector<Vertex> &vertices, const Quadrilateral &element) {
      if (negative_volume(vertices, element)) {
        std::swap(node_list[1], node_list[2]);
      }
    }

    /*
     * @brief Checks for hexahedra, if the volume of an element is negative.
     * @tparam Vertex Position in 3D.
     * @param all_vertices a vector with all vertices of the mesh.
     * @param element the element to check.
     * @return has the element negative volume.
     */
    template<typename Vertex>
    bool negative_volume(const std::vector<Vertex> &all_vertices, const Hexahedron &element) {
      auto origin = all_vertices[element.nodes[0]];
      std::array<Vertex, 3> vertices = {};

      vertices[0] = all_vertices[element.nodes[1]];
      vertices[1] = all_vertices[element.nodes[2]];
      vertices[2] = all_vertices[element.nodes[4]];

      for (auto &v: vertices)
        for (std::size_t i = 0; i < origin.size(); ++i) {
          v[i] -= origin[i];
        }

      Vertex n = {vertices[0][1] * vertices[1][2] - vertices[1][1] * vertices[0][2],
                  vertices[0][2] * vertices[1][0] - vertices[1][2] * vertices[0][0],
                  vertices[0][0] * vertices[1][1] - vertices[1][0] * vertices[0][1]};
      return n[0] * vertices[2][0] + n[1] * vertices[2][1] + n[2] * vertices[2][2] < FLT_EPSILON * 10;
    }

    /*
     * @brief Corrects the sign of the volume of a hexahedron, s.t. the orientation of its edges is untouched.
     * @tparam NodeList container of global indices.
     * @tparam Vertex Position in 3D.
     * @param[in,out] node_list global indices of the nodes of the element. After this call the element has positive
     *                          volume.
     * @param vertices all vertices of the mesh.
     * @param element the element corresponding to node_list.
     */
    template<typename NodeList, typename Vertex>
    void correct_volume(NodeList &node_list, const std::vector<Vertex> &vertices, const Hexahedron &element) {
      if (negative_volume(vertices, element)) {
        std::swap(node_list[1], node_list[2]);
        std::swap(node_list[5], node_list[6]);
      }
    }

    //! checks if the mesh is a 2D manifold.
    template<typename Vertex>
    bool is_manifold(const std::vector<Vertex> &vertices) {
      std::array<bool, 3> not_zeros = {false, false, false};

      if (not vertices.empty())
        if (vertices.begin()->size() == 3)
          for (const auto &vertex : vertices)
            for (std::size_t i = 0; i < not_zeros.size(); ++i)
              not_zeros[i] = std::abs(vertex[i]) > 1e-15;

      return std::count(not_zeros.begin(), not_zeros.end(), true) == 3;
    }

    /*
     * @brief Reorders the nodes of all elements, s.t all local edges have the prescribed orientation.
     * @tparam Vertex Position in 2D or 3D.
     * @tparam E TensorElement<dim>.
     * @tparam NodeList container of global node indices.
     * @param vertices the vertices of the mesh.
     * @param elements the elements of the mesh, needs to be initialized with init_elements(...).
     * @param orientations the global orientation of each edge, s.t. the mesh becomes consistently oriented.
     * @param[in,out] node_lists the global node indices of each element. After this call the corresponding mesh is
     *                           consistently oriented.
     */
    template<typename Vertex, typename E, typename NodeList>
    void reorder_nodes(const std::vector<Vertex> &vertices,
                       const std::vector<E> &elements,
                       const std::vector<Orientation> &orientations,
                       std::vector<NodeList> &node_lists) {
      // Does this local edge has the wrong global orientation?
      auto flipEdge = [&](const E &element, const std::size_t localIdx) {
        return edgeOrientation(element.localEdges[localIdx]) != orientations[element.localEdges[localIdx].globalIdx];
      };

      // do not need to correct volume if the mesh is a 2D manifold
      bool check_volume = E::Traits::dim == 2 ? not(is_manifold(vertices)) : true;

      for (std::size_t i = 0; i < node_lists.size(); ++i) {
        const auto &element = elements[i];

        auto &node_list = node_lists[i];

        // Check for each local edge if it needs to be flipped.
        // This only works, if the local edges are grouped by axis, i.e. in 3D the first 4 local edges
        // are parallel to the z-axis and so on.
        for (std::size_t e = 0; e < E::Traits::edges_per_element; ++e) {
          if (flipEdge(element, e)) {
            const auto &localEdgeIndices = E::Traits::localEdgeNodes[e];
            std::swap(node_list[localEdgeIndices[0]], node_list[localEdgeIndices[1]]);
          }
        }
        if (check_volume)
          correct_volume(node_list, vertices, element);
      }
    }
  }
}

#endif //MESH_CONSISTENCY_INCLUDE_IMPL_REORDER_NODES_HH
