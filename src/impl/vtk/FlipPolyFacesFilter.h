#ifndef CONVERTMESH_FLIP_POLY_FACES_FILTER_H
#define CONVERTMESH_FLIP_POLY_FACES_FILTER_H

#include <vtkPolyDataAlgorithm.h>

/**
 * Small VTK filter that reverses triangle winding while preserving point and
 * cell data arrays. Lifted from ITK-SNAP's VTKMeshPipeline.cxx so ConvertMesh
 * can share the same implementation.
 *
 * Compared to vtkReverseSense, this avoids computing new normals and keeps
 * attribute pass-through explicit. Compared to vtkPolyDataNormals with
 * FlipNormals=On, this is significantly faster because it touches no normals.
 */
class vtkFlipPolyFaces : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkFlipPolyFaces, vtkPolyDataAlgorithm);
  void PrintSelf(std::ostream &os, vtkIndent indent) override;

  static vtkFlipPolyFaces *New();

  vtkSetMacro(FlipFaces, vtkTypeBool);
  vtkGetMacro(FlipFaces, vtkTypeBool);
  vtkBooleanMacro(FlipFaces, vtkTypeBool);

protected:
  vtkFlipPolyFaces();
  ~vtkFlipPolyFaces() override = default;

  vtkTypeBool FlipFaces;

  int RequestData(vtkInformation        *request,
                  vtkInformationVector **inputVector,
                  vtkInformationVector  *outputVector) override;

private:
  vtkFlipPolyFaces(const vtkFlipPolyFaces &) = delete;
  void operator=(const vtkFlipPolyFaces &) = delete;
};

#endif
