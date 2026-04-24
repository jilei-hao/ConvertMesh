#ifndef CONVERTMESH_COMPUTE_NORMALS_H
#define CONVERTMESH_COMPUTE_NORMALS_H

#include "adapters/AdapterBase.h"

template <class TPixel, unsigned int VDim>
class ComputeNormals : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  struct Parameters
  {
    double feature_angle = 30.0;
    bool   splitting     = false;
    bool   consistency   = true;
    bool   auto_orient   = false;  // vtkPolyDataNormals::AutoOrientNormals
  };

  explicit ComputeNormals(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const Parameters &p);
};

#endif
