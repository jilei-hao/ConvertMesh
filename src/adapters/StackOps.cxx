#include "adapters/StackOps.h"

template <class TPixel, unsigned int VDim>
void StackOps<TPixel, VDim>::Pop()
{
  c->m_Stack.pop();
}

template <class TPixel, unsigned int VDim>
void StackOps<TPixel, VDim>::Dup()
{
  if(c->m_Stack.empty()) throw StackAccessException();
  // Value-copy the top DataItem; the held vtk/itk smart pointers are shared
  // (shallow copy), matching c3d's -dup semantics.
  Item copy = c->m_Stack.back();
  c->m_Stack.push(copy);
}

template <class TPixel, unsigned int VDim>
void StackOps<TPixel, VDim>::Swap()
{
  if(c->m_Stack.size() < 2)
    throw StackAccessException("-swap requires at least two items on the stack");
  Item a = c->m_Stack.back(); c->m_Stack.pop();
  Item b = c->m_Stack.back(); c->m_Stack.pop();
  c->m_Stack.push(a);
  c->m_Stack.push(b);
}

template <class TPixel, unsigned int VDim>
void StackOps<TPixel, VDim>::Clear()
{
  c->m_Stack.clear();
}

template <class TPixel, unsigned int VDim>
void StackOps<TPixel, VDim>::As(const std::string &name)
{
  if(c->m_Stack.empty()) throw StackAccessException();
  c->SetVariable(name, c->m_Stack.back());
}

template <class TPixel, unsigned int VDim>
void StackOps<TPixel, VDim>::PopAs(const std::string &name)
{
  if(c->m_Stack.empty()) throw StackAccessException();
  c->SetVariable(name, c->m_Stack.back());
  c->m_Stack.pop();
}

template <class TPixel, unsigned int VDim>
void StackOps<TPixel, VDim>::Push(const std::string &name)
{
  c->m_Stack.push(c->GetVariable(name));
}

template class StackOps<float, 3>;
