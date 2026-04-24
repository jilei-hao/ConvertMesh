#include "api/ConvertMeshAPI.h"

#include "driver/ConvertMeshDriver.h"

#include <sstream>
#include <vector>

namespace
{

// Naive whitespace tokenizer; quoting/escaping will arrive with
// a real Execute() implementation later.
std::vector<std::string> TokenizeCommand(const std::string &cmd)
{
  std::vector<std::string> tokens;
  std::istringstream iss(cmd);
  std::string tok;
  while(iss >> tok)
    tokens.push_back(tok);
  return tokens;
}

} // namespace


template <class TPixel, unsigned int VDim>
ConvertMeshAPI<TPixel, VDim>::ConvertMeshAPI()
    : m_Driver(new ConvertMeshDriver<TPixel, VDim>())
{
}

template <class TPixel, unsigned int VDim>
ConvertMeshAPI<TPixel, VDim>::~ConvertMeshAPI()
{
  delete m_Driver;
}

template <class TPixel, unsigned int VDim>
void ConvertMeshAPI<TPixel, VDim>::PushMesh(vtkPolyData *mesh)
{
  m_Driver->m_Stack.PushMesh(mesh);
}

template <class TPixel, unsigned int VDim>
void ConvertMeshAPI<TPixel, VDim>::PushImage(ImageType *image)
{
  m_Driver->m_Stack.PushImage(image);
}

template <class TPixel, unsigned int VDim>
void ConvertMeshAPI<TPixel, VDim>::PushUGrid(vtkUnstructuredGrid *ugrid)
{
  m_Driver->m_Stack.PushUGrid(ugrid);
}

template <class TPixel, unsigned int VDim>
typename ConvertMeshAPI<TPixel, VDim>::MeshPointer
ConvertMeshAPI<TPixel, VDim>::PopMesh()
{
  return m_Driver->m_Stack.PopMesh();
}

template <class TPixel, unsigned int VDim>
typename ConvertMeshAPI<TPixel, VDim>::ImagePointer
ConvertMeshAPI<TPixel, VDim>::PopImage()
{
  return m_Driver->m_Stack.PopImage();
}

template <class TPixel, unsigned int VDim>
typename ConvertMeshAPI<TPixel, VDim>::UGridPointer
ConvertMeshAPI<TPixel, VDim>::PopUGrid()
{
  return m_Driver->m_Stack.PopUGrid();
}

template <class TPixel, unsigned int VDim>
void ConvertMeshAPI<TPixel, VDim>::AddMesh(const std::string &name,
                                           vtkPolyData *mesh)
{
  typename ConvertMeshDriver<TPixel, VDim>::Item item
      = ConvertMeshDriver<TPixel, VDim>::Item::FromMesh(mesh);
  m_Driver->SetVariable(name, item);
}

template <class TPixel, unsigned int VDim>
typename ConvertMeshAPI<TPixel, VDim>::MeshPointer
ConvertMeshAPI<TPixel, VDim>::GetMesh(const std::string &name)
{
  const auto &item = m_Driver->GetVariable(name);
  if(!item.IsMesh())
    throw TypeMismatchException("mesh", item.KindName());
  return item.mesh;
}

template <class TPixel, unsigned int VDim>
bool ConvertMeshAPI<TPixel, VDim>::Execute(const std::string &command)
{
  try
  {
    auto tokens = TokenizeCommand(command);
    std::vector<char *> argv;
    for(auto &t : tokens)
      argv.push_back(const_cast<char *>(t.c_str()));
    m_Driver->ProcessCommandList(static_cast<int>(argv.size()), argv.data());
    m_Error.clear();
    return true;
  }
  catch(std::exception &e)
  {
    m_Error = e.what();
    return false;
  }
}

template <class TPixel, unsigned int VDim>
void ConvertMeshAPI<TPixel, VDim>::RedirectOutput(std::ostream &sout,
                                                  std::ostream &serr)
{
  m_Driver->RedirectOutput(sout, serr);
}

template <class TPixel, unsigned int VDim>
unsigned ConvertMeshAPI<TPixel, VDim>::GetStackSize() const
{
  return static_cast<unsigned>(m_Driver->m_Stack.size());
}

template class ConvertMeshAPI<float, 3>;
