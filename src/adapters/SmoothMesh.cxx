#include "adapters/SmoothMesh.h"

#include <vtkNew.h>
#include <vtkSmoothPolyDataFilter.h>

template <class TPixel, unsigned int VDim>
void SmoothMesh<TPixel, VDim>::operator()(const Parameters &p)
{
  if(c->m_Backend == "vcg")
  {
    c->Err() << "warning: VCG smoothing backend not yet available "
             << "(arrives in Phase 3); falling back to VTK." << std::endl;
  }
  else if(c->m_Backend == "gpu")
  {
    c->Err() << "warning: GPU smoothing backend not yet available; "
             << "falling back to VTK." << std::endl;
  }

  MeshPointer mesh = c->m_Stack.PopMesh();

  vtkNew<vtkSmoothPolyDataFilter> smooth;
  smooth->SetInputData(mesh);
  smooth->SetNumberOfIterations(p.iterations);
  smooth->SetRelaxationFactor(p.relaxation_factor);
  smooth->SetFeatureAngle(p.feature_angle);
  smooth->SetBoundarySmoothing(p.boundary_smoothing);
  smooth->SetFeatureEdgeSmoothing(p.feature_edge_smoothing);
  smooth->Update();

  c->Verbose() << "SmoothMesh: " << p.iterations << " iterations, "
               << "relax=" << p.relaxation_factor << std::endl;
  c->m_Stack.PushMesh(smooth->GetOutput());
}

template class SmoothMesh<float, 3>;
