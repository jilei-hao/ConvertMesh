#include "adapters/DecimateMesh.h"

#include <vtkDecimatePro.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>

#ifdef CONVERTMESH_HAVE_VCG
#  include "impl/vcg/VCGTriMesh.h"
#endif

namespace
{

// VTK backend: vtkDecimatePro. Topology-preserving by default.
vtkSmartPointer<vtkPolyData>
DecimateVTK(vtkPolyData *mesh, double reduction, double feature_angle,
            bool preserve_topology, bool boundary_deletion)
{
  vtkNew<vtkTriangleFilter> tri;
  tri->SetInputData(mesh);
  tri->PassLinesOff();
  tri->PassVertsOff();
  tri->Update();

  vtkNew<vtkDecimatePro> deci;
  deci->SetInputConnection(tri->GetOutputPort());
  deci->SetTargetReduction(reduction);
  deci->SetFeatureAngle(feature_angle);
  if(preserve_topology) deci->PreserveTopologyOn();
  else                  deci->PreserveTopologyOff();
  deci->SetBoundaryVertexDeletion(boundary_deletion);
  deci->Update();

  vtkSmartPointer<vtkPolyData> out = vtkSmartPointer<vtkPolyData>::New();
  out->DeepCopy(deci->GetOutput());
  return out;
}

#ifdef CONVERTMESH_HAVE_VCG
// VCG backend: quadric edge-collapse remeshing. Mirrors cmrep's
// mesh_decimate_vcg pipeline so cmesh and cmrep produce matching outputs
// for the same input + reduction factor.
//
// `reduction` is the VTK convention (fraction *to remove*); we convert to
// VCG's "keep fraction" by passing 1 - reduction.
vtkSmartPointer<vtkPolyData>
DecimateVCG(vtkPolyData *mesh, double reduction)
{
  cmesh_vcg::VCGTriMesh tri_mesh;
  tri_mesh.ImportFromVTK(mesh);
  tri_mesh.CleanMesh();

  // Same defaults cmrep's mesh_decimate_vcg uses, sourced from VCGTriMesh's
  // GetDefaultQuadricEdgeCollapseRemeshingParameters.
  vcg::tri::TriEdgeCollapseQuadricParameter qparams =
      cmesh_vcg::VCGTriMesh::GetDefaultQuadricEdgeCollapseRemeshingParameters();

  double keep_fraction = 1.0 - reduction;
  if(keep_fraction < 0.0) keep_fraction = 0.0;
  if(keep_fraction > 1.0) keep_fraction = 1.0;

  tri_mesh.QuadricEdgeCollapseRemeshing(keep_fraction, qparams);
  tri_mesh.CleanMesh();
  tri_mesh.RecomputeNormals();

  vtkSmartPointer<vtkPolyData> out = vtkSmartPointer<vtkPolyData>::New();
  tri_mesh.ExportToVTK(out);
  return out;
}
#endif

} // anonymous namespace


template <class TPixel, unsigned int VDim>
void DecimateMesh<TPixel, VDim>::operator()(const Parameters &p)
{
  MeshPointer mesh = c->m_Stack.PopMesh();

  vtkSmartPointer<vtkPolyData> out;
  if(c->m_Backend == "vcg")
  {
#ifdef CONVERTMESH_HAVE_VCG
    out = DecimateVCG(mesh, p.reduction);
    c->Verbose() << "DecimateMesh[vcg]: target_reduction=" << p.reduction
                 << ", out points=" << out->GetNumberOfPoints()
                 << ", out cells=" << out->GetNumberOfCells() << std::endl;
#else
    c->Err() << "warning: ConvertMesh built without VCG support; "
             << "falling back to VTK decimation." << std::endl;
    out = DecimateVTK(mesh, p.reduction, p.feature_angle,
                      p.preserve_topology, p.boundary_deletion);
    c->Verbose() << "DecimateMesh[vtk]: target_reduction=" << p.reduction
                 << ", out points=" << out->GetNumberOfPoints()
                 << ", out cells=" << out->GetNumberOfCells() << std::endl;
#endif
  }
  else
  {
    out = DecimateVTK(mesh, p.reduction, p.feature_angle,
                      p.preserve_topology, p.boundary_deletion);
    c->Verbose() << "DecimateMesh[vtk]: target_reduction=" << p.reduction
                 << ", out points=" << out->GetNumberOfPoints()
                 << ", out cells=" << out->GetNumberOfCells() << std::endl;
  }

  c->m_Stack.PushMesh(out);
}

template class DecimateMesh<float, 3>;
