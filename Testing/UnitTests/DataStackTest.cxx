#include "TestHarness.h"

#include "driver/ConvertMeshException.h"
#include "driver/DataStack.h"

#include <vtkNew.h>
#include <vtkPolyData.h>

typedef DataStack<float, 3>     Stack;
typedef DataItem<float, 3>      Item;
typedef Item::ImageType         ImageType;

int main()
{
  Stack s;
  CM_CHECK(s.empty());
  CM_CHECK_EQ(s.size(), 0u);

  // Push two distinct polydata items.
  vtkNew<vtkPolyData> pd1;
  vtkNew<vtkPolyData> pd2;
  s.PushMesh(pd1);
  s.PushMesh(pd2);
  CM_CHECK_EQ(s.size(), 2u);
  CM_CHECK(s.back().IsMesh());
  CM_CHECK_EQ(s.back().mesh.GetPointer(), pd2.GetPointer());

  // Pop as mesh (typed).
  auto popped = s.PopMesh();
  CM_CHECK_EQ(popped.GetPointer(), pd2.GetPointer());
  CM_CHECK_EQ(s.size(), 1u);

  // Push an image and verify type-mismatch catches wrong pop.
  ImageType::Pointer img = ImageType::New();
  s.PushImage(img);
  CM_CHECK_EQ(s.size(), 2u);
  CM_CHECK(s.back().IsImage());
  CM_CHECK_THROWS(s.PopMesh(), TypeMismatchException);

  // Typed pop succeeds.
  auto popped_img = s.PopImage();
  CM_CHECK(popped_img.IsNotNull());
  CM_CHECK_EQ(s.size(), 1u);

  // Underflow after clearing.
  s.clear();
  CM_CHECK(s.empty());
  CM_CHECK_THROWS(s.pop(), StackAccessException);
  CM_CHECK_THROWS(s.PopMesh(), StackAccessException);
  CM_CHECK_THROWS(s.back(), StackAccessException);

  return 0;
}
