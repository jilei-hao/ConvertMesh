#include "TestHarness.h"

#include "adapters/ExtractIsoSurface.h"
#include "adapters/MergeArrays.h"
#include "adapters/RasterizeMesh.h"
#include "adapters/SampleImageAtMesh.h"
#include "adapters/WarpMesh.h"
#include "driver/ConvertMeshDriver.h"
#include "io/ImageIO.h"
#include "io/MeshIO.h"

#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkVector.h>

#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

#include <cmath>
#include <cstdio>
#include <string>

typedef ConvertMeshDriver<float, 3>          Driver;
typedef itk::Image<float, 3>                 ImageType;
typedef itk::Image<itk::Vector<float, 3>, 3> VectorImageType;

static ImageType::Pointer MakeSphereImage()
{
  constexpr int N = 32;
  ImageType::Pointer img = ImageType::New();
  ImageType::SizeType sz  = { { N, N, N } };
  ImageType::IndexType ix = { { 0, 0, 0 } };
  ImageType::RegionType region(ix, sz);
  img->SetRegions(region);
  ImageType::SpacingType s; s.Fill(1.0);
  img->SetSpacing(s);
  img->Allocate();
  const double c = N / 2.0, r = N / 4.0;
  itk::ImageRegionIteratorWithIndex<ImageType> it(img, region);
  for(; !it.IsAtEnd(); ++it)
  {
    auto i = it.GetIndex();
    double dx = i[0] - c, dy = i[1] - c, dz = i[2] - c;
    it.Set(std::sqrt(dx * dx + dy * dy + dz * dz) < r ? 1.0f : 0.0f);
  }
  return img;
}

// Extract a mesh from the sphere image at iso = 0.5 — reused across tests.
static vtkSmartPointer<vtkPolyData> SphereMesh(Driver &d)
{
  d.m_Stack.PushImage(MakeSphereImage());
  ExtractIsoSurface<float, 3>::Parameters p;
  p.threshold = 0.5;
  p.clean = true;
  ExtractIsoSurface<float, 3> op(&d);
  op(p);
  return d.m_Stack.PopMesh();
}


int main(int argc, char *argv[])
{
  std::string dir = (argc > 1) ? argv[1] : ".";

  // ------------------------------------------------------------------
  // RasterizeMesh: mesh -> binary image, voxel count must be > 0.
  // ------------------------------------------------------------------
  {
    Driver d;
    auto mesh = SphereMesh(d);
    d.m_Stack.PushMesh(mesh);
    RasterizeMesh<float, 3>::Parameters p;
    p.spacing[0] = p.spacing[1] = p.spacing[2] = 0.5;
    p.margin = 1.0;
    RasterizeMesh<float, 3> op(&d);
    op(p);

    CM_CHECK(d.m_Stack.back().IsImage());
    auto img = d.m_Stack.back().image;
    size_t n = img->GetBufferedRegion().GetNumberOfPixels();
    size_t inside = 0;
    const float *buf = img->GetBufferPointer();
    for(size_t i = 0; i < n; ++i) if(buf[i] > 0.5f) ++inside;
    CM_CHECK(inside > 0);
    CM_CHECK(inside < n);   // sanity: sphere doesn't fill the whole bbox
  }

  // ------------------------------------------------------------------
  // SampleImageAtMesh: sample sphere image at its own mesh — interior
  // points should sample ~1, surface points get < 1.
  // ------------------------------------------------------------------
  {
    Driver d;
    auto mesh = SphereMesh(d);
    d.m_Stack.PushMesh(mesh);
    d.m_Stack.PushImage(MakeSphereImage());
    SampleImageAtMesh<float, 3>::Parameters p;
    p.array_name = "Sample";
    SampleImageAtMesh<float, 3> op(&d);
    op(p);

    CM_CHECK(d.m_Stack.back().IsMesh());
    auto arr = d.m_Stack.back().mesh->GetPointData()->GetArray("Sample");
    CM_CHECK(arr != nullptr);
    // All surface-point samples should be in [0, 1] (image range).
    for(vtkIdType i = 0; i < arr->GetNumberOfTuples(); ++i)
    {
      double v = arr->GetTuple1(i);
      CM_CHECK(v >= -0.01 && v <= 1.01);
    }
  }

  // ------------------------------------------------------------------
  // WarpMesh: build a constant translation field (dx=2, 0, 0) and verify
  // that every mesh point shifts by +2 in X.
  // ------------------------------------------------------------------
  {
    Driver d;
    auto mesh = SphereMesh(d);

    // Constant warp field covering the same space as the sphere image.
    VectorImageType::Pointer warp = VectorImageType::New();
    VectorImageType::SizeType sz = { { 32, 32, 32 } };
    VectorImageType::IndexType ix = { { 0, 0, 0 } };
    warp->SetRegions(VectorImageType::RegionType(ix, sz));
    VectorImageType::SpacingType s; s.Fill(1.0);
    warp->SetSpacing(s);
    warp->Allocate();
    itk::Vector<float, 3> v; v[0] = 2.0; v[1] = 0.0; v[2] = 0.0;
    warp->FillBuffer(v);

    std::string warp_path = dir + "/warp-tx.nii.gz";
    itk::ImageFileWriter<VectorImageType>::Pointer w = itk::ImageFileWriter<VectorImageType>::New();
    w->SetFileName(warp_path);
    w->SetInput(warp);
    w->SetUseCompression(true);
    w->Update();

    // Record original first-point.
    double before[3]; mesh->GetPoint(0, before);

    d.m_Stack.PushMesh(mesh);
    WarpMesh<float, 3>::Parameters wp;
    wp.warp_field = warp_path;
    WarpMesh<float, 3> op(&d);
    op(wp);

    double after[3];
    d.m_Stack.back().mesh->GetPoint(0, after);
    // Warp field vector is (+2, 0, 0) in ITK LPS. The ConvertMesh stack
    // stores meshes in NIFTI RAS, so a +X LPS displacement maps to a -X
    // RAS displacement (LPS→RAS negates X and Y). Z is preserved.
    CM_CHECK(std::fabs((after[0] - before[0]) - (-2.0)) < 1e-4);
    CM_CHECK(std::fabs(after[1] - before[1]) < 1e-4);
    CM_CHECK(std::fabs(after[2] - before[2]) < 1e-4);
  }

  // ------------------------------------------------------------------
  // MergeArrays: copy an array between two identical meshes.
  // ------------------------------------------------------------------
  {
    Driver d;
    auto mesh = SphereMesh(d);

    // Attach an array 'Colors' = vertex index.
    vtkNew<vtkFloatArray> arr;
    arr->SetName("Colors");
    arr->SetNumberOfTuples(mesh->GetNumberOfPoints());
    for(vtkIdType i = 0; i < mesh->GetNumberOfPoints(); ++i)
      arr->SetTuple1(i, static_cast<float>(i));
    mesh->GetPointData()->AddArray(arr);

    std::string src_path = dir + "/sphere-with-colors.vtp";
    MeshIO::WritePolyData(mesh, src_path);

    // Push a fresh copy that lacks the array, then merge.
    auto bare = SphereMesh(d);
    CM_CHECK(bare->GetPointData()->GetArray("Colors") == nullptr);
    d.m_Stack.PushMesh(bare);

    MergeArrays<float, 3>::Parameters mp;
    mp.source_mesh = src_path;
    mp.array_name  = "Colors";
    MergeArrays<float, 3> op(&d);
    op(mp);

    auto merged = d.m_Stack.back().mesh->GetPointData()->GetArray("Colors");
    CM_CHECK(merged != nullptr);
    CM_CHECK_EQ(merged->GetNumberOfTuples(), mesh->GetNumberOfPoints());
  }

  std::printf("InteropTest passed\n");
  return 0;
}
