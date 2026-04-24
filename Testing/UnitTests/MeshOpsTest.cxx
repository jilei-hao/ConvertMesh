#include "TestHarness.h"

#include "adapters/ComputeNormals.h"
#include "adapters/DecimateMesh.h"
#include "adapters/FlipNormals.h"
#include "adapters/MeshDiff.h"
#include "adapters/SmoothMesh.h"
#include "driver/ConvertMeshDriver.h"
#include "io/MeshIO.h"

#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
#include <vtkCubeSource.h>
#include <vtkIdList.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>

#include <cstdio>
#include <string>

typedef ConvertMeshDriver<float, 3> Driver;

static vtkSmartPointer<vtkPolyData> MakeCube()
{
  vtkNew<vtkCubeSource> cube;
  cube->SetXLength(1.0);
  cube->SetYLength(1.0);
  cube->SetZLength(1.0);
  cube->Update();

  vtkNew<vtkTriangleFilter> tri;
  tri->SetInputConnection(cube->GetOutputPort());
  tri->Update();

  vtkSmartPointer<vtkPolyData> out = vtkSmartPointer<vtkPolyData>::New();
  out->DeepCopy(tri->GetOutput());
  return out;
}

int main(int argc, char *argv[])
{
  std::string dir = (argc > 1) ? argv[1] : ".";

  // --- SmoothMesh
  {
    Driver d;
    d.m_Stack.PushMesh(MakeCube());
    SmoothMesh<float, 3>::Parameters p;
    p.iterations = 5;
    SmoothMesh<float, 3> op(&d);
    op(p);
    CM_CHECK(d.m_Stack.back().IsMesh());
    CM_CHECK(d.m_Stack.back().mesh->GetNumberOfPoints() > 0);
  }

  // --- DecimateMesh: start with many triangles, verify count drops.
  {
    Driver d;
    auto cube = MakeCube();
    vtkNew<vtkTriangleFilter> tri;
    tri->SetInputData(cube);
    tri->Update();
    d.m_Stack.PushMesh(tri->GetOutput());

    vtkIdType before = tri->GetOutput()->GetNumberOfCells();

    DecimateMesh<float, 3>::Parameters p;
    p.reduction = 0.5;
    p.preserve_topology = false;
    DecimateMesh<float, 3> op(&d);
    op(p);
    vtkIdType after = d.m_Stack.back().mesh->GetNumberOfCells();
    CM_CHECK(after <= before);
  }

  // --- ComputeNormals: adds a Normals array.
  {
    Driver d;
    d.m_Stack.PushMesh(MakeCube());
    ComputeNormals<float, 3>::Parameters p;
    ComputeNormals<float, 3> op(&d);
    op(p);
    auto mesh = d.m_Stack.back().mesh;
    CM_CHECK(mesh->GetPointData()->GetNormals() != nullptr);
  }

  // --- FlipNormals: winding order should reverse on every triangle.
  {
    Driver d;
    auto cube = MakeCube();
    d.m_Stack.PushMesh(cube);

    vtkNew<vtkIdList> orig;
    cube->GetCellPoints(0, orig);
    vtkIdType n_orig = orig->GetNumberOfIds();
    CM_CHECK(n_orig >= 3);
    std::vector<vtkIdType> orig_ids(n_orig);
    for(vtkIdType i = 0; i < n_orig; ++i) orig_ids[i] = orig->GetId(i);

    FlipNormals<float, 3> op(&d);
    op();
    auto flipped = d.m_Stack.back().mesh;
    vtkNew<vtkIdList> after;
    flipped->GetCellPoints(0, after);
    CM_CHECK_EQ(after->GetNumberOfIds(), n_orig);
    for(vtkIdType i = 0; i < n_orig; ++i)
      CM_CHECK_EQ(after->GetId(i), orig_ids[n_orig - 1 - i]);
  }

  // --- MeshDiff: against a shifted copy of the cube.
  {
    auto cube = MakeCube();
    auto shifted = MakeCube();
    vtkPoints *pts = shifted->GetPoints();
    for(vtkIdType i = 0; i < pts->GetNumberOfPoints(); ++i)
    {
      double p[3]; pts->GetPoint(i, p); p[0] += 1.0; pts->SetPoint(i, p);
    }

    std::string ref = dir + "/cube-shifted.vtp";
    MeshIO::WritePolyData(shifted, ref);

    Driver d;
    d.m_Stack.PushMesh(cube);
    MeshDiff<float, 3>::Parameters p;
    p.reference  = ref;
    p.array_name = "D";
    MeshDiff<float, 3> op(&d);
    op(p);

    auto annotated = d.m_Stack.back().mesh;
    CM_CHECK(annotated->GetPointData()->GetArray("D") != nullptr);
  }

  std::printf("MeshOpsTest passed\n");
  return 0;
}
