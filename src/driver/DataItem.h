#ifndef CONVERTMESH_DATA_ITEM_H
#define CONVERTMESH_DATA_ITEM_H

#include <itkImage.h>
#include <itkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

#include <string>

/**
 * Discriminated union representing one item on the ConvertMesh stack.
 *
 * The image member is templated on pixel type and dimension at the driver
 * level, but all stack items share a common runtime type tag so the driver
 * can dispatch generically.
 *
 * This header is templated so an itk::Image<TPixel, VDim> can live alongside
 * vtkPolyData / vtkUnstructuredGrid on the stack.
 */
template <class TPixel, unsigned int VDim>
class DataItem
{
public:
  typedef itk::Image<TPixel, VDim> ImageType;
  typedef itk::SmartPointer<ImageType> ImagePointer;
  typedef vtkSmartPointer<vtkPolyData> MeshPointer;
  typedef vtkSmartPointer<vtkUnstructuredGrid> UGridPointer;

  enum Kind { NONE = 0, MESH, UGRID, IMAGE };

  DataItem() : kind(NONE) {}

  static DataItem FromMesh(vtkPolyData *m)
  {
    DataItem d; d.kind = MESH; d.mesh = m; return d;
  }
  static DataItem FromUGrid(vtkUnstructuredGrid *g)
  {
    DataItem d; d.kind = UGRID; d.ugrid = g; return d;
  }
  static DataItem FromImage(ImageType *img)
  {
    DataItem d; d.kind = IMAGE; d.image = img; return d;
  }

  bool IsMesh()  const { return kind == MESH; }
  bool IsUGrid() const { return kind == UGRID; }
  bool IsImage() const { return kind == IMAGE; }
  bool IsEmpty() const { return kind == NONE; }

  const char *KindName() const
  {
    switch(kind) {
    case MESH:  return "mesh";
    case UGRID: return "ugrid";
    case IMAGE: return "image";
    default:    return "none";
    }
  }

  Kind          kind;
  MeshPointer   mesh;
  UGridPointer  ugrid;
  ImagePointer  image;
};

#endif
