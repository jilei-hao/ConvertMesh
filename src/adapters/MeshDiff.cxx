#include "adapters/MeshDiff.h"

#include "io/MeshIO.h"

#include <vtkCellLocator.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkTriangleFilter.h>

#include <cmath>

template <class TPixel, unsigned int VDim>
void MeshDiff<TPixel, VDim>::operator()(const Parameters &p)
{
  MeshPointer source = c->m_Stack.PopMesh();
  auto reference = MeshIO::ReadPolyData(p.reference);

  // vtkCellLocator requires triangles to produce reliable distances.
  vtkNew<vtkTriangleFilter> tri_ref;
  tri_ref->SetInputData(reference);
  tri_ref->PassLinesOff();
  tri_ref->PassVertsOff();
  tri_ref->Update();

  vtkNew<vtkCellLocator> locator;
  locator->SetDataSet(tri_ref->GetOutput());
  locator->BuildLocator();

  vtkIdType n = source->GetNumberOfPoints();
  vtkNew<vtkFloatArray> dist;
  dist->SetName(p.array_name.c_str());
  dist->SetNumberOfComponents(1);
  dist->SetNumberOfTuples(n);

  double sum = 0.0, sum_sq = 0.0, dmax = 0.0;
  for(vtkIdType i = 0; i < n; ++i)
  {
    double src_pt[3];
    source->GetPoint(i, src_pt);
    double closest[3];
    vtkIdType cell_id;
    int sub_id;
    double d2 = 0.0;
    locator->FindClosestPoint(src_pt, closest, cell_id, sub_id, d2);
    double d = std::sqrt(d2);
    dist->SetTuple1(i, static_cast<float>(d));
    sum    += d;
    sum_sq += d * d;
    if(d > dmax) dmax = d;
  }

  source->GetPointData()->AddArray(dist);

  double mean = (n > 0) ? sum / n : 0.0;
  double rms  = (n > 0) ? std::sqrt(sum_sq / n) : 0.0;

  c->Out() << "MeshDiff: " << p.reference << std::endl
           << "  N=" << n
           << "  mean=" << mean
           << "  rms="  << rms
           << "  hausdorff(source->ref)=" << dmax << std::endl;

  c->m_Stack.PushMesh(source);
}

template class MeshDiff<float, 3>;
