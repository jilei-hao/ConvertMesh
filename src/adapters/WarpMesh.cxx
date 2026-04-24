#include "adapters/WarpMesh.h"

#include "io/VTKToITKBridge.h"

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkPoint.h>
#include <itkVectorLinearInterpolateImageFunction.h>
#include <itkVectorImage.h>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

template <class TPixel, unsigned int VDim>
void WarpMesh<TPixel, VDim>::operator()(const Parameters &p)
{
  MeshPointer mesh = c->m_Stack.PopMesh();

  typedef itk::Vector<float, 3>              VectorType;
  typedef itk::Image<VectorType, 3>          WarpImageType;
  typedef itk::VectorLinearInterpolateImageFunction<WarpImageType> InterpType;

  // Read the warp field (must be 3-component vector image).
  typename itk::ImageFileReader<WarpImageType>::Pointer reader
    = itk::ImageFileReader<WarpImageType>::New();
  reader->SetFileName(p.warp_field);
  try
  {
    reader->Update();
  }
  catch(itk::ExceptionObject &e)
  {
    throw ConvertMeshException("failed to read warp field '" + p.warp_field
                               + "': " + e.GetDescription());
  }

  typename WarpImageType::Pointer warp = reader->GetOutput();
  typename InterpType::Pointer interp = InterpType::New();
  interp->SetInputImage(warp);

  // Rewrite the mesh's point coordinates in place. vtkSmartPointer on a
  // freshly allocated copy keeps things safe if the source is shared.
  auto pts = vtkSmartPointer<vtkPoints>::New();
  pts->DeepCopy(mesh->GetPoints());

  vtkIdType outside = 0;
  for(vtkIdType i = 0; i < pts->GetNumberOfPoints(); ++i)
  {
    double p_ras[3];
    pts->GetPoint(i, p_ras);
    // Mesh coords are RAS; the warp field is an ITK image in LPS.
    typename WarpImageType::PointType P_lps;
    VTKToITKBridge::RasToLpsPoint<typename WarpImageType::PointType>(p_ras, P_lps);

    if(interp->IsInsideBuffer(P_lps))
    {
      VectorType d_lps = interp->Evaluate(P_lps);
      double d_ras[3];
      VTKToITKBridge::LpsToRasVector(d_lps, d_ras);
      double p_out[3] = { p_ras[0] + d_ras[0],
                          p_ras[1] + d_ras[1],
                          p_ras[2] + d_ras[2] };
      pts->SetPoint(i, p_out);
    }
    else
    {
      ++outside;
    }
  }

  auto warped = vtkSmartPointer<vtkPolyData>::New();
  warped->DeepCopy(mesh);
  warped->SetPoints(pts);

  c->Verbose() << "WarpMesh: " << p.warp_field
               << " (" << pts->GetNumberOfPoints() << " points, "
               << outside << " outside field)" << std::endl;

  if(outside > 0)
    c->Err() << "warning: " << outside
             << " mesh vertices fell outside the warp field extent "
             << "and were left unchanged." << std::endl;

  c->m_Stack.PushMesh(warped);
}

template class WarpMesh<float, 3>;
