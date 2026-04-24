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
 * Bridge helpers between itk::Image and VTK's image pipeline.
 *
 * Coordinate-system convention (matches cmrep's vtklevelset and 3D Slicer):
 *   - Meshes on the ConvertMesh stack live in **NIFTI RAS** space.
 *   - itk::Image physical points are **LPS** (ITK's native convention).
 *
 * The flip between the two is simply `diag(-1, -1, 1)`. Adapters that hand a
 * mesh point to ITK (SampleImageAtMesh, WarpMesh, RasterizeMesh, …) must
 * negate X and Y at the boundary; use `RasToLpsPoint` / `LpsToRasVector`
 * below to avoid getting the convention wrong.
 */
namespace VTKToITKBridge
{

// Wire an itk::VTKImageExport into a vtkImageImport so the VTK pipeline can
// consume an itk::Image without copying. Note: ITK's VTKImageExport exposes
// origin + spacing via callbacks, but NOT the direction matrix — VTK's
// vtkImageData has no native direction-cosines slot, so direction is lost
// in the export. Adapters that care about direction must re-apply it, which
// is exactly what `VtkToRasMatrix` below does.
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
 * Build the 4×4 matrix that converts a point emitted by a VTK filter
 * downstream of `itk::VTKImageExport` (coordinates = `diag(spacing) * voxel
 * + origin`, with the ITK direction matrix dropped) into NIFTI RAS space.
 *
 * Derivation:
 *   voxel          v
 *   vtk output     x_vtk = diag(spacing) * v + origin
 *     → solve for v: v = diag(1/spacing) * (x_vtk - origin)
 *   ITK LPS point  x_lps = direction * diag(spacing) * v + origin
 *                        = direction * (x_vtk - origin) + origin
 *                        = direction * x_vtk + (I - direction) * origin
 *   NIFTI RAS      x_ras = diag(-1, -1, 1) * x_lps
 *
 * So the final matrix is `diag(-1,-1,1) * direction` in the 3×3 block and
 * `diag(-1,-1,1) * (I - direction) * origin` in the translation column.
 */
template <class TImage>
vtkSmartPointer<vtkMatrix4x4> VtkToRasMatrix(TImage *image)
{
  auto mat = vtkSmartPointer<vtkMatrix4x4>::New();
  mat->Identity();

  const auto &direction = image->GetDirection();
  const auto &origin    = image->GetOrigin();

  double lps_to_ras[3] = { -1.0, -1.0, 1.0 };

  for(unsigned int r = 0; r < 3; ++r)
  {
    // 3×3 block: diag(lps_to_ras) * direction
    for(unsigned int c = 0; c < 3; ++c)
      mat->SetElement(r, c, lps_to_ras[r] * direction(r, c));

    // Translation: diag(lps_to_ras) * (I - direction) * origin
    double sum = 0.0;
    for(unsigned int k = 0; k < 3; ++k)
    {
      double i_minus_d = (r == k ? 1.0 : 0.0) - direction(r, k);
      sum += i_minus_d * origin[k];
    }
    mat->SetElement(r, 3, lps_to_ras[r] * sum);
  }
  return mat;
}

/** Flip a RAS-space mesh point to ITK LPS for passing to ITK APIs. */
template <class PointType>
inline void RasToLpsPoint(double ras_in[3], PointType &lps_out)
{
  lps_out[0] = -ras_in[0];
  lps_out[1] = -ras_in[1];
  lps_out[2] =  ras_in[2];
}

/** Flip an LPS-space vector (e.g. a displacement) to RAS. */
template <class VectorType>
inline void LpsToRasVector(const VectorType &lps_in, double ras_out[3])
{
  ras_out[0] = -lps_in[0];
  ras_out[1] = -lps_in[1];
  ras_out[2] =  lps_in[2];
}

} // namespace VTKToITKBridge

#endif
