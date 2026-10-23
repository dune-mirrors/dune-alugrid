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

#ifndef MESH_CONSISTENCY_INCLUDE_IMPL_INITIALIZATION_HH
#define MESH_CONSISTENCY_INCLUDE_IMPL_INITIALIZATION_HH

#include <map>
#include <unordered_map>
#include <cassert>
#include "edge.hh"

namespace MeshConsistency {
  namespace impl {
    /*
     * @brief Initialized the nodes of all elements.
     * @tparam NodeList Container for indices.
     * @tparam E TensorElement<dim>
     * @param node_lists container of global node indices
     * @param elements default initialized elements
     */
    template<typename NodeList, typename E>
    void init_element_nodes(const std::vector<NodeList> &node_lists, std::vector<E> &elements) {
      for (std::size_t i = 0; i < node_lists.size(); ++i) {
        assert(node_lists[i].size() == E::Traits::nodes_per_element);
        std::copy_n(node_lists[i].begin(), E::Traits::nodes_per_element, elements[i].nodes.begin());
      }
    }

    /*
     * @brief Initializes the local edges member of all elements. Nodes need to be initialized before.
     * @tparam E TensorElement<dim>
     * @param[in,out] elements elements with initialized nodes
     */
    template<typename E>
    void init_element_local_edges(std::vector<E> &elements) {
      const std::size_t edges_size_upper_limit = E::Traits::edges_per_element * elements.size();
      std::unordered_map<Edge, std::size_t, hash<Edge>> edgeToIdx{10, hash<Edge>(edges_size_upper_limit)};

      auto try_emplace_globalId = [&](const auto &edgeNodes) {
        const auto it = edgeToIdx.find(edgeNodes);
        if (it == edgeToIdx.end()) {
          edgeToIdx.emplace(edgeNodes, edgeToIdx.size());
          return edgeToIdx.size() - 1;
        } else {
          return it->second;
        }
      };

      for (auto &element : elements) {
        constexpr auto edges_per_element = E::Traits::edges_per_element;

        std::array<LocalEdge<E>, edges_per_element> localEdges = {};

        for (std::size_t e = 0; e < localEdges.size(); ++e) {
          const auto &localNodes = E::Traits::localEdgeNodes[e];
          const auto globalNodes = Edge{element.nodes[localNodes[0]], element.nodes[localNodes[1]]};
          const auto sorted_nodes =
                  globalNodes[0] < globalNodes[1] ? globalNodes : Edge{globalNodes[1], globalNodes[0]};
          const bool aligned_orientation = globalNodes == sorted_nodes;

          const auto globalId = try_emplace_globalId(sorted_nodes);

          localEdges[e] = LocalEdge<E>(globalNodes, e, globalId, aligned_orientation, element);
        }

        element.localEdges = localEdges;
      };
    }


    /*
     * @brief Compute elements of the mesh.
     * @tparam NodeList Container for indices.
     * @tparam E TensorElement<dim>.
     * @param node_lists container of list of global node indices.
     * @return vector of elements, nodes and local edges initialized.
     */
    template<typename NodeList, typename E>
    std::vector<E> init_elements(const std::vector<NodeList> &node_lists, const E &) {
      std::vector<E> elements(node_lists.size());

      init_element_nodes(node_lists, elements);

      init_element_local_edges(elements);

      return elements;
    }

    /*
     * @brief Compute the global edges of the mesh.
     * @tparam E TensorElement<dim>.
     * @param elements fully initialized elements, especially the local edges need to be initialized.
     * @return vector of global edges.
     */
    template<typename E>
    std::vector<GlobalEdge<E>> init_global_edges(const std::vector<E> &elements) {
      std::size_t maxGlobalIdx = 0;
      for (const auto &element : elements)
        for (const auto &localEdge : element.localEdges)
          maxGlobalIdx = std::max(maxGlobalIdx, localEdge.globalIdx);

      std::vector<GlobalEdge<E>> edges(maxGlobalIdx + 1);

      for (std::size_t i = 0; i < edges.size(); ++i)
        edges[i].globalIdx = i;

      for (const auto &element : elements) {
        for (const auto &localEdge: element.localEdges) {
          const std::size_t globalIdx = localEdge.globalIdx;

          edges[globalIdx].localEdges.emplace_back(localEdge);
        }
      }

      return edges;
    }
  }
}

#endif //MESH_CONSISTENCY_INCLUDE_IMPL_INITIALIZATION_HH
