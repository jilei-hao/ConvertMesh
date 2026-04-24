#ifndef CONVERTMESH_VTK_TO_ITK_BRIDGE_H
#define CONVERTMESH_VTK_TO_ITK_BRIDGE_H

#include <itkImage.h>
#include <itkSmartPointer.h>
#include <itkVTKImageExport.h>
#include <vtkImageImport.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

/**
 * Bridge helpers between itk::Image and VTK's image pipeline. Also provides
 * the VTK-voxel-coordinate -> RAS/LPS transform that ITK-based image I/O
 * implies (isosurface extraction needs this to produce meshes in physical
 * space).
 */
namespace VTKToITKBridge
{

// Wire an itk::VTKImageExport into a vtkImageImport so the VTK pipeline can
// consume an itk::Image without copying.
template <class TImage>
inline void Connect(itk::VTKImageExport<TImage> *src, vtkImageImport *dst)
{
  dst->SetUpdateInformationCallback(src->GetUpdateInformationCallback());
  dst->SetPipelineModifiedCallback(src->GetPipelineModifiedCallback());
  dst->SetWholeExtentCallback(src->GetWholeExtentCallback());
  dst->SetSpacingCallback(src->GetSpacingCallback());
  dst->SetOriginCallback(src->GetOriginCallback());
  dst->SetScalarTypeCallback(src->GetScalarTypeCallback());
  dst->SetNumberOfComponentsCallback(src->GetNumberOfComponentsCallback());
  dst->SetPropagateUpdateExtentCallback(src->GetPropagateUpdateExtentCallback());
  dst->SetUpdateDataCallback(src->GetUpdateDataCallback());
  dst->SetDataExtentCallback(src->GetDataExtentCallback());
  dst->SetBufferPointerCallback(src->GetBufferPointerCallback());
  dst->SetCallbackUserData(src->GetCallbackUserData());
}

/**
 * Compute the 4x4 transform that takes VTK voxel-space coordinates (what
 * vtkMarchingCubes produces for an itk::Image with identity direction and
 * zero origin) into the image's physical (ITK RAS/LPS) space.
 *
 * Matches cmrep's vtklevelset behavior (ConstructVTKtoNiftiTransform),
 * but we operate directly on the itk::Image's direction/origin/spacing
 * without dragging in the NIfTI-specific transform math. The returned
 * matrix is exactly `D * diag(spacing) | origin`.
 */
template <class TImage>
vtkSmartPointer<vtkMatrix4x4> VoxelToPhysicalMatrix(TImage *image)
{
  auto mat = vtkSmartPointer<vtkMatrix4x4>::New();
  mat->Identity();

  const auto &direction = image->GetDirection();
  const auto &spacing   = image->GetSpacing();
  const auto &origin    = image->GetOrigin();

  for(unsigned int r = 0; r < 3; ++r)
  {
    for(unsigned int c = 0; c < 3; ++c)
      mat->SetElement(r, c, direction(r, c) * spacing[c]);
    mat->SetElement(r, 3, origin[r]);
  }
  return mat;
}

} // namespace VTKToITKBridge

#endif
