#ifndef CONVERTMESH_WRITE_IMAGE_H
#define CONVERTMESH_WRITE_IMAGE_H

#include "adapters/AdapterBase.h"

#include <string>

template <class TPixel, unsigned int VDim>
class WriteImage : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  explicit WriteImage(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const std::string &filename);
};

#endif
