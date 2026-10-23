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

#ifndef MESH_CONSISTENCY_INCLUDE_IMPL_ELEMENTS_HH
#define MESH_CONSISTENCY_INCLUDE_IMPL_ELEMENTS_HH

#include <vector>
#include "element_traits.hh"
#include "edge.hh"

namespace MeshConsistency {
  namespace impl {

    /*
     * @brief Represents a quadrilateral or hexahedron element.
     * @tparam dim dimension of element.
     */
    template<std::size_t dim>
    struct TensorElement {
      //! Geometry information of the element.
      using Traits = impl::TensorElementTraits<dim>;

      std::array<std::size_t, Traits::nodes_per_element> nodes = {};
      std::array<impl::LocalEdge<TensorElement<dim>>, Traits::edges_per_element> localEdges = {};

      TensorElement() = default;
    };

    /*
     * @brief Represents a triangle or tetrahedron element.
     * @tparam dim dimension of element.
     */
    template<std::size_t dim>
    struct SimplexElement {};

    using Quadrilateral = impl::TensorElement<2>;
    using Hexahedron = impl::TensorElement<3>;

    using Triangle = impl::SimplexElement<2>;
    using Tetrahedron = impl::SimplexElement<3>;
  }
}

#endif //MESH_CONSISTENCY_INCLUDE_IMPL_ELEMENTS_HH
