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
  src/adapters/StackOps.cxx)

set(CMESH_API_SOURCES
  src/api/ConvertMeshAPI.cxx)

add_library(cmesh_driver ${CMESH_DRIVER_SOURCES})
foreach(_dir IN LISTS CONVERTMESH_INCLUDE_DIRS)
  target_include_directories(cmesh_driver PUBLIC $<BUILD_INTERFACE:${_dir}>)
endforeach()
target_include_directories(cmesh_driver INTERFACE $<INSTALL_INTERFACE:include>)
target_link_libraries(cmesh_driver PUBLIC ${ITK_LIBRARIES} ${VTK_LIBRARIES})

add_library(cmesh_api ${CMESH_API_SOURCES})
foreach(_dir IN LISTS CONVERTMESH_INCLUDE_DIRS)
  target_include_directories(cmesh_api PUBLIC $<BUILD_INTERFACE:${_dir}>)
endforeach()
target_include_directories(cmesh_api INTERFACE $<INSTALL_INTERFACE:include>)
target_link_libraries(cmesh_api PUBLIC cmesh_driver)

# Convenience variable for downstream (like c3d's C3D_LINK_LIBRARIES)
set(CMESH_LINK_LIBRARIES cmesh_api cmesh_driver ${ITK_LIBRARIES} ${VTK_LIBRARIES})
