#ifndef CONVERTMESH_API_H
#define CONVERTMESH_API_H

#include <itkImage.h>
#include <itkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

#include <ostream>
#include <string>

template <class TPixel, unsigned int VDim> class ConvertMeshDriver;

/**
 * Public C++ API for embedding ConvertMesh in other projects (cmrep,
 * itksnap, a future pybind11 wrapper). Mirrors the role of c3d's
 * ConvertAPI — holds a private driver instance and exposes push/pop plus
 * a string-based Execute() entry point.
 */
template <class TPixel, unsigned int VDim>
class ConvertMeshAPI
{
public:
  typedef itk::Image<TPixel, VDim> ImageType;
  typedef itk::SmartPointer<ImageType> ImagePointer;
  typedef vtkSmartPointer<vtkPolyData> MeshPointer;
  typedef vtkSmartPointer<vtkUnstructuredGrid> UGridPointer;

  ConvertMeshAPI();
  ~ConvertMeshAPI();

  ConvertMeshAPI(const ConvertMeshAPI &) = delete;
  ConvertMeshAPI &operator=(const ConvertMeshAPI &) = delete;

  // Push items onto the stack by reference (kept alive by the smart pointer).
  void PushMesh(vtkPolyData *mesh);
  void PushImage(ImageType *image);
  void PushUGrid(vtkUnstructuredGrid *ugrid);

  // Pop from the top of the stack. Throws TypeMismatchException if the top
  // is not the requested kind.
  MeshPointer  PopMesh();
  ImagePointer PopImage();
  UGridPointer PopUGrid();

  // Named-variable support (analogue of -as / -popas on the CLI).
  void        AddMesh(const std::string &name, vtkPolyData *mesh);
  MeshPointer GetMesh(const std::string &name);

  // Execute one or more whitespace-separated commands against this driver.
  // Returns true on success, false on any caught exception (use GetError()).
  bool Execute(const std::string &command);

  // Redirect driver stdout/stderr.
  void RedirectOutput(std::ostream &sout, std::ostream &serr);

  unsigned    GetStackSize() const;
  std::string GetError() const { return m_Error; }

private:
  ConvertMeshDriver<TPixel, VDim> *m_Driver;
  std::string                      m_Error;
};

#endif
