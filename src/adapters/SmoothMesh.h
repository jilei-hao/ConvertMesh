#ifndef CONVERTMESH_SMOOTH_MESH_H
#define CONVERTMESH_SMOOTH_MESH_H

#include "adapters/AdapterBase.h"

/**
 * Laplacian smoothing (vtkSmoothPolyDataFilter). Reads backend selection
 * from the driver's sticky state — VCG backend to be added in Phase 3.
 */
template <class TPixel, unsigned int VDim>
class SmoothMesh : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  struct Parameters
  {
    int    iterations       = 20;
    double relaxation_factor = 0.1;
    double feature_angle    = 45.0;
    bool   boundary_smoothing = true;
    bool   feature_edge_smoothing = false;
  };

  explicit SmoothMesh(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const Parameters &p);
};

#endif
