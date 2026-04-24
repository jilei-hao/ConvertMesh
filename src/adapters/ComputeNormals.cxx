#include "adapters/ComputeNormals.h"

#include <vtkNew.h>
#include <vtkPolyDataNormals.h>

template <class TPixel, unsigned int VDim>
void ComputeNormals<TPixel, VDim>::operator()(const Parameters &p)
{
  MeshPointer mesh = c->m_Stack.PopMesh();

  vtkNew<vtkPolyDataNormals> normals;
  normals->SetInputData(mesh);
  normals->SetFeatureAngle(p.feature_angle);
  normals->SetSplitting(p.splitting);
  normals->SetConsistency(p.consistency);
  normals->SetAutoOrientNormals(p.auto_orient);
  normals->Update();

  c->Verbose() << "ComputeNormals: feature_angle=" << p.feature_angle
               << (p.auto_orient ? ", auto-oriented" : "") << std::endl;
  c->m_Stack.PushMesh(normals->GetOutput());
}

template class ComputeNormals<float, 3>;
