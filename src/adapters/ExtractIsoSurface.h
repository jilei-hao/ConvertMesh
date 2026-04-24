#ifndef CONVERTMESH_EXTRACT_ISOSURFACE_H
#define CONVERTMESH_EXTRACT_ISOSURFACE_H

#include "adapters/AdapterBase.h"

/**
 * Pop an image from the stack and extract an iso-surface as a vtkPolyData.
 * Analogue of cmrep's vtklevelset / ITK-SNAP's VTKMeshPipeline.
 */
template <class TPixel, unsigned int VDim>
class ExtractIsoSurface : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  struct Parameters
  {
    double threshold    = 0.5;
    // Multi-label mode: loop over discrete integer labels from `threshold`
    // upwards and produce one mesh per label, with a "Label" point-data
    // scalar.
    bool   multi_label  = false;
    // Optional pre-smoothing of the image (Gaussian std-dev in voxels).
    double smooth_pre   = 0.0;
    // Optional decimation reduction (0..1). 0 disables.
    double decimate     = 0.0;
    // Clean + triangulate after marching cubes.
    bool   clean        = false;
    // Compute normals (and auto-flip them if the voxel->physical transform
    // has negative determinant).
    bool   compute_normals = true;
  };

  explicit ExtractIsoSurface(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const Parameters &p);
};

#endif
