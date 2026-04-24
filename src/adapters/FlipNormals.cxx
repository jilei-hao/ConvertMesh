#include "adapters/FlipNormals.h"

#include "impl/vtk/FlipPolyFacesFilter.h"

#include <vtkSmartPointer.h>

template <class TPixel, unsigned int VDim>
void FlipNormals<TPixel, VDim>::operator()()
{
  MeshPointer mesh = c->m_Stack.PopMesh();

  auto flip = vtkSmartPointer<vtkFlipPolyFaces>::New();
  flip->SetInputData(mesh);
  flip->FlipFacesOn();
  flip->Update();

  c->Verbose() << "FlipNormals: reversed winding for "
               << mesh->GetNumberOfCells() << " cells" << std::endl;
  c->m_Stack.PushMesh(flip->GetOutput());
}

template class FlipNormals<float, 3>;
