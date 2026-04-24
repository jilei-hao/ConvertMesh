#include "adapters/SampleImageAtMesh.h"

#include "io/VTKToITKBridge.h"

#include <itkBSplineInterpolateImageFunction.h>
#include <itkInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>

#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

namespace
{

template <class ImageType>
typename itk::InterpolateImageFunction<ImageType, double>::Pointer
MakeInterpolator(const std::string &mode, ImageType *image)
{
  typedef itk::InterpolateImageFunction<ImageType, double> Base;
  typename Base::Pointer interp;
  if(mode == "nn" || mode == "nearest" || mode == "nearestneighbor")
  {
    auto nn = itk::NearestNeighborInterpolateImageFunction<ImageType, double>::New();
    interp = nn.GetPointer();
  }
  else if(mode == "bspline")
  {
    auto b = itk::BSplineInterpolateImageFunction<ImageType, double>::New();
    b->SetSplineOrder(3);
    interp = b.GetPointer();
  }
  else
  {
    // default: linear
    auto lin = itk::LinearInterpolateImageFunction<ImageType, double>::New();
    interp = lin.GetPointer();
  }
  interp->SetInputImage(image);
  return interp;
}

} // namespace


template <class TPixel, unsigned int VDim>
void SampleImageAtMesh<TPixel, VDim>::operator()(const Parameters &p)
{
  // Stack: [ ..., mesh, image (top) ]
  ImagePointer image = c->m_Stack.PopImage();
  if(c->m_Stack.empty() || !c->m_Stack.back().IsMesh())
    throw TypeMismatchException("mesh",
                                c->m_Stack.empty() ? "(empty)"
                                                   : c->m_Stack.back().KindName());
  MeshPointer mesh = c->m_Stack.back().mesh;

  auto interp = MakeInterpolator<ImageType>(c->m_Interpolation, image);

  vtkIdType n = mesh->GetNumberOfPoints();
  vtkNew<vtkFloatArray> arr;
  arr->SetName(p.array_name.c_str());
  arr->SetNumberOfComponents(1);
  arr->SetNumberOfTuples(n);

  vtkIdType n_outside = 0;
  for(vtkIdType i = 0; i < n; ++i)
  {
    double pt_ras[3];
    mesh->GetPoint(i, pt_ras);
    // Mesh coords are NIFTI RAS (our convention); ITK physical points are LPS.
    typename ImageType::PointType P;
    VTKToITKBridge::RasToLpsPoint<typename ImageType::PointType>(pt_ras, P);

    double v;
    if(interp->IsInsideBuffer(P))
      v = interp->Evaluate(P);
    else
    {
      v = static_cast<double>(p.background_value);
      ++n_outside;
    }
    arr->SetTuple1(i, static_cast<float>(v));
  }

  mesh->GetPointData()->AddArray(arr);

  c->Verbose() << "SampleImageAtMesh: array='" << p.array_name
               << "' mode=" << c->m_Interpolation
               << " (" << n << " pts, " << n_outside
               << " outside image)" << std::endl;
}

template class SampleImageAtMesh<float, 3>;
