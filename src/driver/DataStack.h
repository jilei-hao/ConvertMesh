#ifndef CONVERTMESH_DATA_STACK_H
#define CONVERTMESH_DATA_STACK_H

#include "ConvertMeshException.h"
#include "DataItem.h"

#include <vector>

/**
 * Typed stack of DataItem<TPixel, VDim>. Behaves like c3d's ImageStack but
 * holds mixed meshes/ugrids/images and enforces kind checking on typed
 * accessors.
 */
template <class TPixel, unsigned int VDim>
class DataStack
{
public:
  typedef DataItem<TPixel, VDim>        Item;
  typedef typename Item::ImageType      ImageType;
  typedef typename Item::ImagePointer   ImagePointer;
  typedef typename Item::MeshPointer    MeshPointer;
  typedef typename Item::UGridPointer   UGridPointer;

  typedef std::vector<Item> StorageType;
  typedef typename StorageType::iterator iterator;
  typedef typename StorageType::const_iterator const_iterator;

  // Generic push/pop/peek
  void push(const Item &d) { m_Stack.push_back(d); }
  void pop()
  {
    if(m_Stack.empty()) throw StackAccessException();
    m_Stack.pop_back();
  }
  const Item &back() const
  {
    if(m_Stack.empty()) throw StackAccessException();
    return m_Stack.back();
  }
  Item &back()
  {
    if(m_Stack.empty()) throw StackAccessException();
    return m_Stack.back();
  }

  // Typed helpers — throw TypeMismatchException if the top isn't the
  // requested kind
  MeshPointer PopMesh()
  {
    if(m_Stack.empty()) throw StackAccessException();
    const Item &top = m_Stack.back();
    if(!top.IsMesh()) throw TypeMismatchException("mesh", top.KindName());
    MeshPointer out = top.mesh;
    m_Stack.pop_back();
    return out;
  }
  ImagePointer PopImage()
  {
    if(m_Stack.empty()) throw StackAccessException();
    const Item &top = m_Stack.back();
    if(!top.IsImage()) throw TypeMismatchException("image", top.KindName());
    ImagePointer out = top.image;
    m_Stack.pop_back();
    return out;
  }
  UGridPointer PopUGrid()
  {
    if(m_Stack.empty()) throw StackAccessException();
    const Item &top = m_Stack.back();
    if(!top.IsUGrid()) throw TypeMismatchException("ugrid", top.KindName());
    UGridPointer out = top.ugrid;
    m_Stack.pop_back();
    return out;
  }

  void PushMesh(vtkPolyData *m)          { m_Stack.push_back(Item::FromMesh(m)); }
  void PushUGrid(vtkUnstructuredGrid *g) { m_Stack.push_back(Item::FromUGrid(g)); }
  void PushImage(ImageType *img)         { m_Stack.push_back(Item::FromImage(img)); }

  size_t size() const { return m_Stack.size(); }
  bool   empty() const { return m_Stack.empty(); }
  void   clear() { m_Stack.clear(); }

  Item       &operator[](size_t i)
  {
    if(i >= m_Stack.size()) throw StackAccessException();
    return m_Stack[i];
  }
  const Item &operator[](size_t i) const
  {
    if(i >= m_Stack.size()) throw StackAccessException();
    return m_Stack[i];
  }

  iterator       begin()       { return m_Stack.begin(); }
  const_iterator begin() const { return m_Stack.begin(); }
  iterator       end()         { return m_Stack.end(); }
  const_iterator end()   const { return m_Stack.end(); }

private:
  StorageType m_Stack;
};

#endif
