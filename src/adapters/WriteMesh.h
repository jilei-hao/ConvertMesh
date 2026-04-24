#ifndef CONVERTMESH_WRITE_MESH_H
#define CONVERTMESH_WRITE_MESH_H

#include "adapters/AdapterBase.h"

#include <string>

template <class TPixel, unsigned int VDim>
class WriteMesh : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  explicit WriteMesh(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const std::string &filename);
};

#endif
