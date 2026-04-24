#include "adapters/RasterizeMesh.h"

#include "io/ImageIO.h"

#include <itkImage.h>
#include <itkImageRegionIterator.h>

#include <vtkImageData.h>
#include <vtkImageStencil.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>

#include <algorithm>
#include <cmath>

namespace
{

// Fill an ITK image by copying from a VTK unsigned-char image. The VTK image
// is assumed to live in the ITK image's physical coordinate system (i.e. we
// configured its origin/spacing to match, and we do not attempt to rotate).
template <class TImage>
void CopyVTKImageToITK(vtkImageData *src,
                       TImage *dst,
                       typename TImage::PixelType inside,
                       typename TImage::PixelType outside)
{
  typedef typename TImage::PixelType PixelType;
  const unsigned char *buf = static_cast<const unsigned char *>(src->GetScalarPointer());
  itk::ImageRegionIterator<TImage> it(dst, dst->GetBufferedRegion());
  size_t i = 0;
  for(; !it.IsAtEnd(); ++it, ++i)
    it.Set(buf[i] > 0 ? inside : outside);
}

} // namespace


template <class TPixel, unsigned int VDim>
void RasterizeMesh<TPixel, VDim>::operator()(const Parameters &p)
{
  MeshPointer mesh_ras = c->m_Stack.PopMesh();

  // --- Determine the output image geometry (ITK native: LPS) ---
  double spacing[3]   = { p.spacing[0], p.spacing[1], p.spacing[2] };
  double origin[3]    = { 0.0, 0.0, 0.0 };
  int    extent[6]    = { 0, 0, 0, 0, 0, 0 };
  typename ImageType::DirectionType direction;
  direction.SetIdentity();

  if(!p.reference.empty())
  {
    auto ref = ImageIO<TPixel, VDim>::Read(p.reference);
    auto refSpacing   = ref->GetSpacing();
    auto refOrigin    = ref->GetOrigin();
    auto refRegion    = ref->GetLargestPossibleRegion();
    auto refSize      = refRegion.GetSize();
    direction         = ref->GetDirection();
    for(unsigned int i = 0; i < 3; ++i)
    {
      spacing[i] = refSpacing[i];
      origin[i]  = refOrigin[i];
      extent[2*i]     = 0;
      extent[2*i + 1] = static_cast<int>(refSize[i]) - 1;
    }
    // Non-identity direction cosines would require rotating the mesh into
    // the voxel grid's frame before stenciling, which vtkImageStencil does
    // not support. Flag this as unsupported for now rather than producing
    // a silently-wrong output.
    for(unsigned int r = 0; r < 3; ++r)
      for(unsigned int col = 0; col < 3; ++col)
      {
        double expect = (r == col ? 1.0 : 0.0);
        if(std::fabs(direction(r, col) - expect) > 1e-6)
          throw ConvertMeshException(
            "RasterizeMesh: reference images with non-identity direction "
            "cosines are not supported yet (see ConvertMesh issue backlog)");
      }
  }
  else
  {
    // Compute the bbox in the *LPS* space we will write into, not RAS —
    // convert the mesh to LPS first so the bbox is in the stencil's frame.
    vtkNew<vtkMatrix4x4> ras_to_lps;
    ras_to_lps->Identity();
    ras_to_lps->SetElement(0, 0, -1.0);
    ras_to_lps->SetElement(1, 1, -1.0);
    vtkNew<vtkTransform> xf;
    xf->SetMatrix(ras_to_lps);
    vtkNew<vtkTransformPolyDataFilter> tmp;
    tmp->SetInputData(mesh_ras);
    tmp->SetTransform(xf);
    tmp->Update();

    double b[6];
    tmp->GetOutput()->GetBounds(b);
    for(unsigned int i = 0; i < 3; ++i)
    {
      double lo = b[2*i]   - p.margin;
      double hi = b[2*i+1] + p.margin;
      origin[i] = lo;
      int n = static_cast<int>(std::ceil((hi - lo) / spacing[i]));
      extent[2*i]     = 0;
      extent[2*i + 1] = std::max(1, n) - 1;
    }
  }

  // Convert the RAS mesh to LPS for the stencil. vtkImageStencil uses the
  // VTK image's origin/spacing directly and cannot apply a direction matrix,
  // so we need to hand it mesh coordinates in the same frame.
  vtkNew<vtkMatrix4x4> ras_to_lps;
  ras_to_lps->Identity();
  ras_to_lps->SetElement(0, 0, -1.0);
  ras_to_lps->SetElement(1, 1, -1.0);
  vtkNew<vtkTransform> ras2lps;
  ras2lps->SetMatrix(ras_to_lps);

  vtkNew<vtkTransformPolyDataFilter> toLps;
  toLps->SetInputData(mesh_ras);
  toLps->SetTransform(ras2lps);
  toLps->Update();

  // vtkPolyDataToImageStencil expects triangles only.
  vtkNew<vtkTriangleFilter> tri;
  tri->SetInputConnection(toLps->GetOutputPort());
  tri->PassLinesOff();
  tri->PassVertsOff();
  tri->Update();

  // --- VTK stencil pipeline ---
  vtkNew<vtkPolyDataToImageStencil> stencil;
  stencil->SetInputConnection(tri->GetOutputPort());
  stencil->SetOutputOrigin(origin);
  stencil->SetOutputSpacing(spacing);
  stencil->SetOutputWholeExtent(extent);
  stencil->Update();

  // Create a VTK image initialised to zero and apply the stencil to write 255
  // inside the mesh.
  vtkNew<vtkImageData> raster;
  raster->SetOrigin(origin);
  raster->SetSpacing(spacing);
  raster->SetExtent(extent);
  raster->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  std::memset(raster->GetScalarPointer(), 0,
              sizeof(unsigned char) * raster->GetNumberOfPoints());

  vtkNew<vtkImageStencil> apply;
  apply->SetInputData(raster);
  apply->SetStencilConnection(stencil->GetOutputPort());
  apply->ReverseStencilOn();    // write inside the stencil (the mesh interior)
  apply->SetBackgroundValue(255.0);
  apply->Update();

  // --- Build the ITK image with matching geometry ---
  ImagePointer out = ImageType::New();
  typename ImageType::SizeType sz;
  typename ImageType::IndexType ix;
  for(unsigned int i = 0; i < 3; ++i)
  {
    ix[i] = 0;
    sz[i] = extent[2*i + 1] - extent[2*i] + 1;
  }
  typename ImageType::RegionType region(ix, sz);
  out->SetRegions(region);

  typename ImageType::SpacingType itkSpacing;
  typename ImageType::PointType   itkOrigin;
  for(unsigned int i = 0; i < 3; ++i)
  {
    itkSpacing[i] = spacing[i];
    itkOrigin[i]  = origin[i];
  }
  out->SetSpacing(itkSpacing);
  out->SetOrigin(itkOrigin);
  out->SetDirection(direction);
  out->Allocate();
  out->FillBuffer(p.outside_value);

  CopyVTKImageToITK<ImageType>(apply->GetOutput(),
                               out,
                               p.inside_value,
                               p.outside_value);

  c->Verbose() << "RasterizeMesh: " << sz[0] << "x" << sz[1] << "x" << sz[2]
               << ", spacing=[" << spacing[0] << "," << spacing[1] << ","
               << spacing[2] << "]" << std::endl;

  c->m_Stack.PushImage(out);
}

template class RasterizeMesh<float, 3>;
