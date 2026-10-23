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

#ifndef MESH_CONSISTENCY_INCLUDE_ELEMENT_TYPES_HH
#define MESH_CONSISTENCY_INCLUDE_ELEMENT_TYPES_HH

namespace MeshConsistency {
  /*
   * @brief Run time element type.
   * Used to choose algorithm at run time.
   */
  enum class ElementType { triangle, quadrilateral, tetrahedron, hexahedron };

  constexpr auto quadrilateralType = ElementType::quadrilateral;
  constexpr auto hexahedronType = ElementType::hexahedron;

  constexpr auto triangleType = ElementType::triangle;
  constexpr auto tetrahedronType = ElementType::tetrahedron;

  /*
   * @brief Compile time element type.
   * Used to choose alorithm at compile time.
   */
  template<std::size_t dim>
  struct TensorElementTag {};

  template<std::size_t dim>
  struct SimplexElementTag {};

  constexpr auto quadrilateralTag = TensorElementTag<2>();
  constexpr auto hexahedronTag = TensorElementTag<3>();

  constexpr auto triangleTag = SimplexElementTag<2>();
  constexpr auto tetrahedronTag = SimplexElementTag<3>();
}

#endif //MESH_CONSISTENCY_INCLUDE_ELEMENT_TYPES_HH
