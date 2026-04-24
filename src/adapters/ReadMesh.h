#ifndef CONVERTMESH_READ_MESH_H
#define CONVERTMESH_READ_MESH_H

#include "adapters/AdapterBase.h"

#include <string>

template <class TPixel, unsigned int VDim>
class ReadMesh : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  explicit ReadMesh(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const std::string &filename);
};

#endif
