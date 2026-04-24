#ifndef CONVERTMESH_STACK_OPS_H
#define CONVERTMESH_STACK_OPS_H

#include "adapters/AdapterBase.h"

#include <string>

/**
 * Collection of simple stack-manipulation commands (-pop, -dup, -swap,
 * -as, -popas, -push, -clear). Grouped into one adapter since they share
 * no per-call state.
 */
template <class TPixel, unsigned int VDim>
class StackOps : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  explicit StackOps(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void Pop();
  void Dup();
  void Swap();
  void Clear();
  void As(const std::string &name);
  void PopAs(const std::string &name);
  void Push(const std::string &name);
};

#endif
