#include "adapters/ExtractIsoSurface.h"

#include "impl/vtk/FlipPolyFacesFilter.h"
#include "io/VTKToITKBridge.h"

#include <itkVTKImageExport.h>

#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkDecimatePro.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageImport.h>
#include <vtkMarchingCubes.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkUnsignedShortArray.h>

#include <algorithm>
#include <cmath>

template <class TPixel, unsigned int VDim>
void ExtractIsoSurface<TPixel, VDim>::operator()(const Parameters &p)
{
  ImagePointer img = c->m_Stack.PopImage();

  // --- ITK -> VTK bridge ---
  typedef itk::VTKImageExport<ImageType> ExporterType;
  typename ExporterType::Pointer exporter = ExporterType::New();
  exporter->SetInput(img);
  vtkNew<vtkImageImport> importer;
  VTKToITKBridge::Connect(exporter.GetPointer(), importer);

  // Optional Gaussian pre-smoothing in voxel units.
  vtkAlgorithmOutput *mc_input = importer->GetOutputPort();
  vtkNew<vtkImageGaussianSmooth> smoother;
  if(p.smooth_pre > 0.0)
  {
    smoother->SetInputConnection(importer->GetOutputPort());
    smoother->SetStandardDeviations(p.smooth_pre, p.smooth_pre, p.smooth_pre);
    smoother->Update();
    mc_input = smoother->GetOutputPort();
  }

  // --- Marching cubes (single contour or multi-label) ---
  vtkSmartPointer<vtkPolyData> mesh;

  if(p.multi_label)
  {
    // Compute the label range of the image so we know when to stop.
    TPixel imin = std::numeric_limits<TPixel>::max();
    TPixel imax = std::numeric_limits<TPixel>::lowest();
    const TPixel *buf = img->GetBufferPointer();
    size_t n = img->GetBufferedRegion().GetNumberOfPixels();
    for(size_t i = 0; i < n; ++i)
    {
      if(buf[i] < imin) imin = buf[i];
      if(buf[i] > imax) imax = buf[i];
    }

    c->Verbose() << "ExtractIsoSurface (multi-label): range ["
                 << imin << ", " << imax << "]" << std::endl;

    vtkNew<vtkAppendPolyData> append;
    double start = std::max<double>(p.threshold, static_cast<double>(imin));
    for(double lbl = std::floor(start); lbl <= static_cast<double>(imax); lbl += 1.0)
    {
      vtkNew<vtkDiscreteMarchingCubes> dmc;
      dmc->SetInputConnection(mc_input);
      dmc->ComputeGradientsOff();
      dmc->ComputeScalarsOff();
      dmc->ComputeNormalsOn();
      dmc->SetNumberOfContours(1);
      dmc->SetValue(0, lbl);
      dmc->Update();

      vtkPolyData *labelMesh = dmc->GetOutput();
      if(labelMesh->GetNumberOfPoints() == 0) continue;

      vtkNew<vtkUnsignedShortArray> scalar;
      scalar->SetName("Label");
      scalar->SetNumberOfComponents(1);
      scalar->SetNumberOfTuples(labelMesh->GetNumberOfPoints());
      for(vtkIdType i = 0; i < labelMesh->GetNumberOfPoints(); ++i)
        scalar->SetTuple1(i, static_cast<unsigned short>(lbl));
      labelMesh->GetPointData()->SetScalars(scalar);
      append->AddInputData(labelMesh);
    }
    append->Update();
    mesh = append->GetOutput();
  }
  else
  {
    vtkNew<vtkMarchingCubes> mc;
    mc->SetInputConnection(mc_input);
    mc->ComputeScalarsOff();
    mc->ComputeGradientsOff();
    mc->ComputeNormalsOn();
    mc->SetNumberOfContours(1);
    mc->SetValue(0, p.threshold);
    mc->Update();
    mesh = mc->GetOutput();
  }

  if(!mesh || mesh->GetNumberOfPoints() == 0)
    throw ConvertMeshException("ExtractIsoSurface: no surface produced "
                               "(threshold/range may not intersect the image)");

  // Optional clean + re-triangulate.
  if(p.clean)
  {
    vtkNew<vtkTriangleFilter> tri1;
    tri1->SetInputData(mesh);
    tri1->PassLinesOff();
    tri1->PassVertsOff();
    tri1->Update();

    vtkNew<vtkCleanPolyData> clean;
    clean->SetInputConnection(tri1->GetOutputPort());
    clean->PointMergingOn();
    clean->SetTolerance(0.0);
    clean->Update();

    vtkNew<vtkTriangleFilter> tri2;
    tri2->SetInputConnection(clean->GetOutputPort());
    tri2->PassLinesOff();
    tri2->PassVertsOff();
    tri2->Update();

    mesh = tri2->GetOutput();
  }

  // --- Transform VTK-output space -> NIFTI RAS ---
  // Matches cmrep vtklevelset output (ConvertMesh's standard convention).
  auto mat = VTKToITKBridge::VtkToRasMatrix<ImageType>(img);
  vtkNew<vtkTransform> xform;
  xform->SetMatrix(mat);

  vtkNew<vtkTransformPolyDataFilter> txFilter;
  txFilter->SetInputData(mesh);
  txFilter->SetTransform(xform);
  txFilter->Update();
  mesh = txFilter->GetOutput();

  // --- Optional normal computation (and auto-flip if transform flips) ---
  bool flip = (xform->GetMatrix()->Determinant() < 0);
  if(p.compute_normals || flip)
  {
    vtkNew<vtkPolyDataNormals> normals;
    normals->SetInputData(mesh);
    if(flip) normals->FlipNormalsOn();
    normals->SplittingOff();
    normals->ConsistencyOn();
    normals->Update();
    mesh = normals->GetOutput();
  }

  // --- Optional decimation ---
  if(p.decimate > 0.0)
  {
    vtkNew<vtkDecimatePro> deci;
    deci->SetInputData(mesh);
    deci->SetTargetReduction(p.decimate);
    deci->PreserveTopologyOn();
    deci->Update();
    mesh = deci->GetOutput();
  }

  c->Verbose() << "ExtractIsoSurface: produced " << mesh->GetNumberOfPoints()
               << " points, " << mesh->GetNumberOfCells() << " cells"
               << std::endl;

  c->m_Stack.PushMesh(mesh);
}

template class ExtractIsoSurface<float, 3>;
