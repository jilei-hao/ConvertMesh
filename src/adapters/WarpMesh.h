#ifndef CONVERTMESH_WARP_MESH_H
#define CONVERTMESH_WARP_MESH_H

#include "adapters/AdapterBase.h"

#include <string>

/**
 * Deform the top-of-stack mesh by an ITK displacement field. Each mesh
 * vertex X is moved to X + D(X), where D is a 3-component vector image
 * read from disk. The mesh is assumed to live in the same physical space
 * as the warp field (ITK-standard direction cosines / origin / spacing).
 *
 * Full coordinate-system-mode support (ijk / ijkos / lps / ras / ants —
 * cmrep's warpmesh) is deferred; for now we operate in the warp field's
 * physical space, which matches the output of ExtractIsoSurface and is
 * sufficient for meshes + warps produced by the same pipeline (e.g. greedy).
 */
template <class TPixel, unsigned int VDim>
class WarpMesh : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  struct Parameters
  {
    std::string warp_field;
  };

  explicit WarpMesh(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const Parameters &p);
};

#endif
