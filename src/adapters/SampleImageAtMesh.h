#ifndef CONVERTMESH_SAMPLE_IMAGE_AT_MESH_H
#define CONVERTMESH_SAMPLE_IMAGE_AT_MESH_H

#include "adapters/AdapterBase.h"

#include <string>

/**
 * Sample an ITK scalar image at the vertex positions of a vtkPolyData, and
 * store the result as a named point-data array on the mesh.
 *
 * Stack contract: the top-of-stack item must be the *image*, and the next
 * item below must be the *mesh* to be annotated. Pops the image, peeks the
 * mesh below and modifies it in place. Post-condition: mesh is on top of
 * the stack with a new array named `array_name`.
 *
 * Interpolation mode is driven by the driver's sticky `-int` flag:
 *   linear (default) | nn | bspline
 */
template <class TPixel, unsigned int VDim>
class SampleImageAtMesh : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  struct Parameters
  {
    std::string array_name       = "Sample";
    TPixel      background_value = static_cast<TPixel>(0);
  };

  explicit SampleImageAtMesh(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const Parameters &p);
};

#endif
