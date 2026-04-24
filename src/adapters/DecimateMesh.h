#ifndef CONVERTMESH_DECIMATE_MESH_H
#define CONVERTMESH_DECIMATE_MESH_H

#include "adapters/AdapterBase.h"

/**
 * Reduce polygon count using vtkDecimatePro (VTK backend). Topology is
 * preserved by default. May drop point/cell arrays when the user opts into
 * aggressive decimation — flagged via MayDropArrays so the driver can warn.
 */
template <class TPixel, unsigned int VDim>
class DecimateMesh : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  struct Parameters
  {
    double reduction         = 0.5;   // target fraction to remove (0..1)
    double feature_angle     = 15.0;
    bool   preserve_topology = true;
    bool   boundary_deletion = false;
  };

  explicit DecimateMesh(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  bool ChangesTopology() const override { return true; }
  bool MayDropArrays()   const override { return false; }

  void operator()(const Parameters &p);
};

#endif
