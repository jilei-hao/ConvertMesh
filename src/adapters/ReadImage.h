#ifndef CONVERTMESH_READ_IMAGE_H
#define CONVERTMESH_READ_IMAGE_H

#include "adapters/AdapterBase.h"

#include <string>

template <class TPixel, unsigned int VDim>
class ReadImage : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  explicit ReadImage(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const std::string &filename);
};

#endif
