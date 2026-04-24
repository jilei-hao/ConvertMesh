# ConvertMesh tests. Uses simple assert-based test executables plus CTest
# shell scripts against the cmesh binary — matches cmrep's minimal style
# without pulling in gtest. If a richer harness is desired later, swap in
# FetchContent(GoogleTest).

# -----------------------------------------------------------------------
# Unit tests
# -----------------------------------------------------------------------
add_executable(DataStackTest UnitTests/DataStackTest.cxx)
target_include_directories(DataStackTest PRIVATE ${CONVERTMESH_INCLUDE_DIRS})
target_link_libraries(DataStackTest PRIVATE cmesh_driver)
add_test(NAME DataStackTest COMMAND DataStackTest)

add_executable(MeshIOTest UnitTests/MeshIOTest.cxx)
target_include_directories(MeshIOTest PRIVATE ${CONVERTMESH_INCLUDE_DIRS})
target_link_libraries(MeshIOTest PRIVATE cmesh_driver)
add_test(NAME MeshIOTest COMMAND MeshIOTest ${CMAKE_CURRENT_BINARY_DIR})

add_executable(IsoSurfaceTest UnitTests/IsoSurfaceTest.cxx)
target_include_directories(IsoSurfaceTest PRIVATE ${CONVERTMESH_INCLUDE_DIRS})
target_link_libraries(IsoSurfaceTest PRIVATE cmesh_driver)
add_test(NAME IsoSurfaceTest COMMAND IsoSurfaceTest ${CMAKE_CURRENT_BINARY_DIR})

add_executable(MeshOpsTest UnitTests/MeshOpsTest.cxx)
target_include_directories(MeshOpsTest PRIVATE ${CONVERTMESH_INCLUDE_DIRS})
target_link_libraries(MeshOpsTest PRIVATE cmesh_driver)
add_test(NAME MeshOpsTest COMMAND MeshOpsTest ${CMAKE_CURRENT_BINARY_DIR})

# -----------------------------------------------------------------------
# CLI end-to-end tests (shell scripts driving the cmesh binary)
# -----------------------------------------------------------------------
if(TARGET cmesh)
  add_test(NAME CLIHelp
           COMMAND $<TARGET_FILE:cmesh> --help)

  add_test(NAME CLIVersion
           COMMAND $<TARGET_FILE:cmesh> --version)
  set_tests_properties(CLIVersion PROPERTIES
    PASS_REGULAR_EXPRESSION "cmesh.*version")

  # Mesh round-trip: create -> write VTP -> read it back -> write STL.
  # Driven by MeshIOTest which creates a small .vtp fixture in its build dir.
  add_test(NAME CLIRoundTrip
           COMMAND ${CMAKE_COMMAND} -E env
             $<TARGET_FILE:cmesh>
             ${CMAKE_CURRENT_BINARY_DIR}/cube.vtp
             -o ${CMAKE_CURRENT_BINARY_DIR}/cube-roundtrip.stl)
  set_tests_properties(CLIRoundTrip PROPERTIES
    DEPENDS MeshIOTest
    REQUIRED_FILES ${CMAKE_CURRENT_BINARY_DIR}/cube.vtp)

  # IsoSurface pipeline end-to-end: read image, extract, smooth, decimate, write.
  add_test(NAME CLIIsoSurfacePipeline
           COMMAND ${CMAKE_COMMAND} -E env
             $<TARGET_FILE:cmesh>
             -verbose
             ${CMAKE_CURRENT_BINARY_DIR}/sphere.nii.gz
             -extract-isosurface 0.5 -clean
             -smooth-mesh 3 0.2
             -decimate 0.3
             -compute-normals
             -o ${CMAKE_CURRENT_BINARY_DIR}/sphere-pipeline.vtp)
  set_tests_properties(CLIIsoSurfacePipeline PROPERTIES
    DEPENDS IsoSurfaceTest
    REQUIRED_FILES ${CMAKE_CURRENT_BINARY_DIR}/sphere.nii.gz)

  # MeshDiff against the unmodified reference — mean distance should be 0.
  add_test(NAME CLIMeshDiffZero
           COMMAND ${CMAKE_COMMAND} -E env
             $<TARGET_FILE:cmesh>
             ${CMAKE_CURRENT_BINARY_DIR}/cube.vtp
             -meshdiff ${CMAKE_CURRENT_BINARY_DIR}/cube.vtp)
  set_tests_properties(CLIMeshDiffZero PROPERTIES
    DEPENDS MeshIOTest
    PASS_REGULAR_EXPRESSION "mean=0")
endif()
