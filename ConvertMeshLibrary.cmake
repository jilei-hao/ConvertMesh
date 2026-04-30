# Sources + library targets for ConvertMesh.
# Intended to be included from the top-level CMakeLists.txt or from a parent
# project that has already found ITK + VTK.

set(CONVERTMESH_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/driver
  ${CMAKE_CURRENT_SOURCE_DIR}/src/adapters
  ${CMAKE_CURRENT_SOURCE_DIR}/src/io
  ${CMAKE_CURRENT_SOURCE_DIR}/src/impl
  ${CMAKE_CURRENT_SOURCE_DIR}/src/api
  ${CMAKE_CURRENT_BINARY_DIR})

set(CMESH_DRIVER_SOURCES
  src/driver/ConvertMeshDriver.cxx
  src/io/MeshIO.cxx
  src/io/ImageIO.cxx
  src/adapters/ReadMesh.cxx
  src/adapters/WriteMesh.cxx
  src/adapters/ReadImage.cxx
  src/adapters/WriteImage.cxx
  src/adapters/StackOps.cxx
  src/adapters/ExtractIsoSurface.cxx
  src/adapters/SmoothMesh.cxx
  src/adapters/DecimateMesh.cxx
  src/adapters/ComputeNormals.cxx
  src/adapters/FlipNormals.cxx
  src/adapters/MeshDiff.cxx
  src/adapters/RasterizeMesh.cxx
  src/adapters/WarpMesh.cxx
  src/adapters/SampleImageAtMesh.cxx
  src/adapters/MergeArrays.cxx
  src/impl/vtk/FlipPolyFacesFilter.cxx)

set(CMESH_API_SOURCES
  src/api/ConvertMeshAPI.cxx)

# ---------------------------------------------------------------------------
# Optional VCG-backed adapters
# ---------------------------------------------------------------------------
# When CONVERTMESH_BUILD_VCG=ON we expect VCGLIB_DIR to point at a vcglib
# checkout (the same one cmrep uses for mesh_decimate_vcg / mesh_smooth_curv).
# Header-only — we just add include paths and pull a couple of impl/vcg/*.cxx
# files into cmesh_driver.
if(CONVERTMESH_BUILD_VCG)
  if(NOT VCGLIB_DIR)
    find_path(VCGLIB_DIR
      NAMES vcg/simplex/vertex/base.h
      DOC "Path to a vcglib checkout (root that contains vcg/, wrap/, eigenlib/)")
  endif()
  if(NOT VCGLIB_DIR OR NOT EXISTS "${VCGLIB_DIR}/vcg/simplex/vertex/base.h")
    message(FATAL_ERROR
      "CONVERTMESH_BUILD_VCG=ON but VCGLIB_DIR is not set to a valid vcglib "
      "checkout. Pass -DVCGLIB_DIR=/path/to/vcglib (the same path cmrep uses; "
      "scripts/build-cmrep.sh clones it to lib/vcglib).")
  endif()
  message(STATUS "ConvertMesh: VCG support enabled, VCGLIB_DIR=${VCGLIB_DIR}")
  list(APPEND CMESH_DRIVER_SOURCES src/impl/vcg/VCGTriMesh.cxx)
endif()

add_library(cmesh_driver ${CMESH_DRIVER_SOURCES})
foreach(_dir IN LISTS CONVERTMESH_INCLUDE_DIRS)
  target_include_directories(cmesh_driver PUBLIC $<BUILD_INTERFACE:${_dir}>)
endforeach()
target_include_directories(cmesh_driver INTERFACE $<INSTALL_INTERFACE:include>)
target_link_libraries(cmesh_driver PUBLIC ${ITK_LIBRARIES} ${VTK_LIBRARIES})

if(CONVERTMESH_BUILD_VCG)
  # vcglib pulls in its own bundled Eigen via <Eigen/...> includes, located
  # at ${VCGLIB_DIR}/eigenlib. We keep this PRIVATE so downstream consumers
  # of cmesh_driver are unaffected.
  target_include_directories(cmesh_driver PRIVATE
    ${VCGLIB_DIR}
    ${VCGLIB_DIR}/eigenlib)
  target_compile_definitions(cmesh_driver PUBLIC CONVERTMESH_HAVE_VCG)
endif()

add_library(cmesh_api ${CMESH_API_SOURCES})
foreach(_dir IN LISTS CONVERTMESH_INCLUDE_DIRS)
  target_include_directories(cmesh_api PUBLIC $<BUILD_INTERFACE:${_dir}>)
endforeach()
target_include_directories(cmesh_api INTERFACE $<INSTALL_INTERFACE:include>)
target_link_libraries(cmesh_api PUBLIC cmesh_driver)

# Convenience variable for downstream (like c3d's C3D_LINK_LIBRARIES)
set(CMESH_LINK_LIBRARIES cmesh_api cmesh_driver ${ITK_LIBRARIES} ${VTK_LIBRARIES})
