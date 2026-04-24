#include "adapters/WriteMesh.h"

#include "io/MeshIO.h"

template <class TPixel, unsigned int VDim>
void WriteMesh<TPixel, VDim>::operator()(const std::string &filename)
{
  if(c->m_Stack.empty()) throw StackAccessException();
  const Item &top = c->m_Stack.back();

  if(top.IsMesh())
  {
    c->Verbose() << "Writing mesh #" << c->m_Stack.size() << " to "
                 << filename << std::endl;
    MeshIO::WritePolyData(top.mesh, filename);
  }
  else if(top.IsUGrid())
  {
    c->Verbose() << "Writing ugrid #" << c->m_Stack.size() << " to "
                 << filename << std::endl;
    MeshIO::WriteUnstructuredGrid(top.ugrid, filename);
  }
  else
  {
    throw TypeMismatchException("mesh or ugrid", top.KindName());
  }
}

template class WriteMesh<float, 3>;
