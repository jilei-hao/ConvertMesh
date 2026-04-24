#include "adapters/DecimateMesh.h"

#include <vtkDecimatePro.h>
#include <vtkNew.h>
#include <vtkTriangleFilter.h>

template <class TPixel, unsigned int VDim>
void DecimateMesh<TPixel, VDim>::operator()(const Parameters &p)
{
  if(c->m_Backend == "vcg")
  {
    c->Err() << "warning: VCG decimation backend not yet available "
             << "(arrives in Phase 3); falling back to VTK." << std::endl;
  }

  MeshPointer mesh = c->m_Stack.PopMesh();

  // vtkDecimatePro requires triangles only.
  vtkNew<vtkTriangleFilter> tri;
  tri->SetInputData(mesh);
  tri->PassLinesOff();
  tri->PassVertsOff();
  tri->Update();

  vtkNew<vtkDecimatePro> deci;
  deci->SetInputConnection(tri->GetOutputPort());
  deci->SetTargetReduction(p.reduction);
  deci->SetFeatureAngle(p.feature_angle);
  if(p.preserve_topology) deci->PreserveTopologyOn();
  else                    deci->PreserveTopologyOff();
  deci->SetBoundaryVertexDeletion(p.boundary_deletion);
  deci->Update();

  auto out = deci->GetOutput();
  c->Verbose() << "DecimateMesh: target_reduction=" << p.reduction
               << ", out points=" << out->GetNumberOfPoints()
               << ", out cells=" << out->GetNumberOfCells() << std::endl;
  c->m_Stack.PushMesh(out);
}

template class DecimateMesh<float, 3>;
