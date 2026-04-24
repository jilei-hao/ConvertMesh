#ifndef CONVERTMESH_MESH_DIFF_H
#define CONVERTMESH_MESH_DIFF_H

#include "adapters/AdapterBase.h"

#include <string>

/**
 * Compute pointwise mesh-to-mesh distance. Pops the top-of-stack mesh (A),
 * reads the reference mesh from `filename` (B), and pushes A back with a
 * new "Distance" point data array containing the per-vertex distance from
 * A's points to B's surface. Also prints summary statistics (mean, RMS,
 * Hausdorff) to the driver's output stream.
 *
 * Matches the distance-array behavior of cmrep's meshdiff `-d` option; the
 * volumetric Dice / overlap portion of meshdiff will arrive separately as
 * its own adapter when the rasterization path is in place.
 */
template <class TPixel, unsigned int VDim>
class MeshDiff : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  struct Parameters
  {
    std::string reference;
    std::string array_name = "Distance";
  };

  explicit MeshDiff(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const Parameters &p);
};

#endif
