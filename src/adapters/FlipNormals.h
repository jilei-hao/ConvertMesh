#ifndef CONVERTMESH_FLIP_NORMALS_H
#define CONVERTMESH_FLIP_NORMALS_H

#include "adapters/AdapterBase.h"

/**
 * Reverse triangle winding while preserving point/cell data arrays. Uses the
 * lightweight vtkFlipPolyFaces filter lifted from ITK-SNAP — much faster
 * than vtkPolyDataNormals and does not recompute normals. If a "Normals"
 * point array exists on the input, it is passed through unchanged; run
 * ComputeNormals afterward if you need flipped normal vectors.
 */
template <class TPixel, unsigned int VDim>
class FlipNormals : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  explicit FlipNormals(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()();
};

#endif
