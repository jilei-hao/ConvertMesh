#include "adapters/MergeArrays.h"

#include "io/MeshIO.h"

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

template <class TPixel, unsigned int VDim>
void MergeArrays<TPixel, VDim>::operator()(const Parameters &p)
{
  MeshPointer dest = c->m_Stack.back().mesh;
  if(!c->m_Stack.back().IsMesh())
    throw TypeMismatchException("mesh", c->m_Stack.back().KindName());

  auto source = MeshIO::ReadPolyData(p.source_mesh);

  vtkDataSetAttributes *src_attr = p.cell_data
      ? static_cast<vtkDataSetAttributes *>(source->GetCellData())
      : static_cast<vtkDataSetAttributes *>(source->GetPointData());
  vtkDataSetAttributes *dst_attr = p.cell_data
      ? static_cast<vtkDataSetAttributes *>(dest->GetCellData())
      : static_cast<vtkDataSetAttributes *>(dest->GetPointData());

  vtkDataArray *src_arr = src_attr->GetArray(p.array_name.c_str());
  if(!src_arr)
    throw ConvertMeshException("MergeArrays: '" + p.array_name
                               + "' not found in source mesh");

  vtkIdType expected = p.cell_data ? dest->GetNumberOfCells()
                                   : dest->GetNumberOfPoints();
  if(src_arr->GetNumberOfTuples() != expected)
  {
    std::ostringstream os;
    os << "MergeArrays: '" << p.array_name << "' has "
       << src_arr->GetNumberOfTuples()
       << " tuples but destination has " << expected
       << (p.cell_data ? " cells" : " points");
    throw ConvertMeshException(os.str());
  }

  // Shallow-copy semantics are fine: we want the destination mesh to share
  // the raw buffer with the source mesh we just read.
  auto copy = vtkSmartPointer<vtkDataArray>::Take(src_arr->NewInstance());
  copy->DeepCopy(src_arr);
  if(!p.rename_to.empty())
    copy->SetName(p.rename_to.c_str());

  dst_attr->AddArray(copy);
  c->Verbose() << "MergeArrays: '" << p.array_name << "' ("
               << copy->GetNumberOfComponents() << " components, "
               << copy->GetNumberOfTuples() << " tuples) copied from "
               << p.source_mesh << std::endl;
}

template class MergeArrays<float, 3>;
