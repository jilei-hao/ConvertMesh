#include "adapters/ReadMesh.h"

#include "io/MeshIO.h"

template <class TPixel, unsigned int VDim>
void ReadMesh<TPixel, VDim>::operator()(const std::string &filename)
{
  vtkSmartPointer<vtkPolyData> mesh = MeshIO::ReadPolyData(filename);
  c->Verbose() << "Reading mesh #" << (c->m_Stack.size() + 1) << " from "
               << filename << " (" << mesh->GetNumberOfPoints() << " points, "
               << mesh->GetNumberOfCells() << " cells)" << std::endl;
  c->m_Stack.PushMesh(mesh);
}

template class ReadMesh<float, 3>;
