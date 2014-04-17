#ifndef DUNE_ALUGRID_IMPL_BINARYIO_HH
#define DUNE_ALUGRID_IMPL_BINARYIO_HH

#include <cstddef>
#include <iostream>

namespace ALUGrid
{

  enum BinaryFormat { rawBinary, zlibCompressed };

  void readBinary ( std::istream &stream, void *data, std::size_t size, BinaryFormat format );
  void writeBinary ( std::ostream &stream, const void *data, std::size_t size, BinaryFormat format );

} // namespace ALUGrid

#endif // #ifndef DUNE_ALUGRID_IMPL_BINARYIO_HH
