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
endif()
