#ifndef CONVERTMESH_RASTERIZE_MESH_H
#define CONVERTMESH_RASTERIZE_MESH_H

#include "adapters/AdapterBase.h"

#include <string>

/**
 * Rasterize the top-of-stack polydata into a 3D binary image (mesh2img
 * equivalent). Produces an itk::Image<TPixel, VDim> populated with `inside`
 * inside the closed surface and `outside` elsewhere.
 *
 * Geometry is determined by one of:
 *   - Reference image on disk (inherits origin / spacing / direction).
 *   - Explicit origin + size + spacing parameters.
 *   - Auto-bounding-box from the mesh itself with a uniform margin.
 *
 * The output replaces the mesh on the stack.
 */
template <class TPixel, unsigned int VDim>
class RasterizeMesh : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  struct Parameters
  {
    std::string reference;      // path to a reference image (optional)
    double      spacing[3]     = { 1.0, 1.0, 1.0 };
    double      margin         = 2.0;   // auto-bbox margin in world units
    TPixel      inside_value   = static_cast<TPixel>(1);
    TPixel      outside_value  = static_cast<TPixel>(0);
  };

  explicit RasterizeMesh(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const Parameters &p);
};

#endif
