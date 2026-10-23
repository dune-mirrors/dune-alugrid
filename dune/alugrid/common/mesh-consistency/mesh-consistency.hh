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


#ifndef MESH_CONSISTENCY_INCLUDE_MESH_CONSISTENCY_HH
#define MESH_CONSISTENCY_INCLUDE_MESH_CONSISTENCY_HH

#include <iostream>
#include "element_types.hh"
#include "impl/initialization.hh"
#include "impl/elements.hh"
#include "impl/orientation.hh"
#include "impl/reorder_nodes.hh"
#include "impl/sort_nodes.hh"

namespace MeshConsistency{
  /*
   * @brief Reorder the node indices for each element to achive consistent orientation.
   *
   * @tparam Vertex describes a point in 2D or 3D, like std::array or std::vector.
   * @tparam NodeList describes a list of global indices.
   * @tparam dim dimension of element, 2 for triangles, 3 for tetrahedron.
   *
   * @param vertices vertices in the mesh.
   * @param[in,out] node_lists list of global node indices for all elements.
   * @param tag triangleTag or tetrahedraTag.
   *
   * @exception NotOrientable if the mesh is not orientable.
   */
  template<typename Vertex, typename NodeList, std::size_t dim>
  bool orient_consistently(const std::vector<Vertex> &vertices,
                           std::vector<NodeList> &node_lists,
                           const SimplexElementTag<dim> tag) {
    for (auto &node_list : node_lists)
      impl::sort_nodes(node_list, impl::SimplexElement<dim>());
    return true;
  }

  /*
   * @brief Reorder the node indices for each element to achive consistent orientation.
   *
   * @tparam Vertex describes a point in 2D or 3D, like std::array or std::vector.
   * @tparam NodeList describes a list of global indices.
   * @tparam dim dimension of elements, 2 for quadrilaterals, 3 for hexahedron.
   *
   * @param vertices vertices in the mesh.
   * @param[in,out] node_lists list of global node indices for all elements.
   * @param tag quadrilateralTag or hexahedraTag.
   *
   * @exception NotOrientable if the mesh is not orientable.
   */
  template<typename Vertex, typename NodeList, std::size_t dim>
  bool orient_consistently(const std::vector<Vertex> &vertices,
                           std::vector<NodeList> &node_lists,
                           const TensorElementTag<dim> tag) {
    using namespace impl;

    auto elements = init_elements(node_lists, TensorElement<dim>());

    auto edges = init_global_edges(elements);

    if (not(is_consistently_oriented(edges))) {
      try {
        const auto orientations = compute_edge_orientations(edges);

        reorder_nodes(vertices, elements, orientations, node_lists);
        return true;
      } catch (NotOrientable &e) {
        std::cout << "The mesh has no consistent edge orientation.\n"
                  << "Please refine the mesh once to make a consitent edge orientation possible." << std::endl;
        throw;
      }
    }
    return false;
  }

  /*
   * @brief Reorder the node indices for each element to achive consistent orientation.
   *
   * @tparam Vertex describes a point in 2D or 3D, like std::array or std::vector.
   * @tparam NodeList describes a list of global indices.
   *
   * @param vertices vertices in the mesh.
   * @param[in,out] node_lists list of global node indices for all elements.
   * @param type the element type.
   *
   * @exception NotOrientable if the mesh is not orientable.
   */
  template<typename Vertex, typename NodeList>
  bool orient_consistently(const std::vector<Vertex> &vertices,
                           std::vector<NodeList> &node_lists,
                           const ElementType type) {
    if (!node_lists.empty()) {
      if (type == ElementType::quadrilateral)
        return  orient_consistently(vertices, node_lists, quadrilateralTag);
      else if (type == ElementType::hexahedron)
        return orient_consistently(vertices, node_lists, hexahedronTag);
      else if (type == ElementType::triangle)
        return orient_consistently(vertices, node_lists, triangleTag);
      else if (type == ElementType::tetrahedron)
        return orient_consistently(vertices, node_lists, tetrahedronTag);
      else
        throw std::runtime_error("Element type not recognized");
    }
    return false;
  }
}

#endif //MESH_CONSISTENCY_INCLUDE_MESH_CONSISTENCY_HH
