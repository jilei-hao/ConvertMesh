#ifndef CONVERTMESH_MESH_IO_H
#define CONVERTMESH_MESH_IO_H

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

#include <string>

/**
 * Format-agnostic mesh I/O for ConvertMesh. Supports VTK (.vtk), VTP (.vtp),
 * STL (.stl), OBJ (.obj), PLY (.ply), and BYU (.byu/.y). Lifts and unifies
 * the patterns found in cmrep's ReadWriteVTK and ITK-SNAP's GuidedMeshIO.
 */
namespace MeshIO
{

// Extract lowercase extension (no leading dot). Handles .nii.gz-style double
// extensions by returning the outer suffix only.
std::string GetExtension(const std::string &filename);

// Read a polygonal mesh. Throws ConvertMeshException on failure.
vtkSmartPointer<vtkPolyData> ReadPolyData(const std::string &filename);

// Read an unstructured grid (tetrahedral meshes etc). Throws on failure.
vtkSmartPointer<vtkUnstructuredGrid> ReadUnstructuredGrid(const std::string &filename);

// Write a polygonal mesh. Throws on failure. `binary` is a hint honored only
// by formats that support it (VTK legacy, VTP). For STL/BYU ASCII vs binary
// is controlled by the format's own writer default.
void WritePolyData(vtkPolyData *mesh,
                   const std::string &filename,
                   bool binary = true);

// Write an unstructured grid. Throws on failure.
void WriteUnstructuredGrid(vtkUnstructuredGrid *mesh,
                           const std::string &filename,
                           bool binary = true);

} // namespace MeshIO

#endif
