add_executable(tests
    test_mesh.cpp
    test_layout.cpp
)

target_include_directories(tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(tests PRIVATE
    meshviewer_core
    Catch2::Catch2WithMain
)

# Standard CTest integration
include(Catch)
catch_discover_tests(tests)
