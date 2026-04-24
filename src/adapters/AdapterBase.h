#ifndef CONVERTMESH_ADAPTER_BASE_H
#define CONVERTMESH_ADAPTER_BASE_H

#include "driver/ConvertMeshDriver.h"

// Common typedefs for adapter classes. Parallels c3d's CONVERTER_STANDARD_TYPEDEFS.
#define CMESH_STANDARD_TYPEDEFS                                   \
  typedef ConvertMeshDriver<TPixel, VDim>      Driver;            \
  typedef typename Driver::Item                Item;              \
  typedef typename Driver::ImageType           ImageType;         \
  typedef typename Driver::ImagePointer        ImagePointer;      \
  typedef typename Driver::MeshPointer         MeshPointer;       \
  typedef typename Driver::UGridPointer        UGridPointer;

/**
 * Base class for ConvertMesh adapters. Each adapter owns a back-pointer to
 * the driver and reads/writes the stack via operator().
 */
template <class TPixel, unsigned int VDim>
class AdapterBase
{
public:
  CMESH_STANDARD_TYPEDEFS

  explicit AdapterBase(Driver *d) : c(d) {}

  // Optional metadata an adapter can override. Used by the driver to drive
  // data-loss warnings for topology-changing or non-VTK-native ops.
  virtual bool ChangesTopology() const { return false; }
  virtual bool MayDropArrays()   const { return false; }

  virtual ~AdapterBase() = default;

protected:
  Driver *c;
};

#endif
