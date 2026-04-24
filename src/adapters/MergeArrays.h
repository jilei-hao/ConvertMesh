#ifndef CONVERTMESH_MERGE_ARRAYS_H
#define CONVERTMESH_MERGE_ARRAYS_H

#include "adapters/AdapterBase.h"

#include <string>

/**
 * Copy a named point-data (or cell-data) array from a source mesh on disk
 * onto the top-of-stack mesh. Useful to re-attach arrays after a VCG or
 * TetGen round-trip that strips them.
 *
 * The source mesh must have the same number of points (or cells) as the
 * destination; the array is copied value-for-value in vertex/cell-index
 * order — there is no nearest-neighbor projection here (that will live in
 * a future `-resample-arrays` adapter).
 */
template <class TPixel, unsigned int VDim>
class MergeArrays : public AdapterBase<TPixel, VDim>
{
public:
  CMESH_STANDARD_TYPEDEFS
  using AdapterBase<TPixel, VDim>::c;

  struct Parameters
  {
    std::string source_mesh;
    std::string array_name;
    bool        cell_data = false;   // true for cell data; default is point
    std::string rename_to = "";      // optional rename of the copied array
  };

  explicit MergeArrays(Driver *d) : AdapterBase<TPixel, VDim>(d) {}

  void operator()(const Parameters &p);
};

#endif
