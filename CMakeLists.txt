cmake_minimum_required(VERSION 3.16)
project(meshviewer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable AUTOUIC, AUTOMOC, and AUTORCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Testing
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)
enable_testing()

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets OpenGL Xml Concurrent)

# Find Assimp and GLM
find_package(assimp REQUIRED)
find_package(glm REQUIRED)

# Find QGLViewer
# Users might need to provide -DCMAKE_PREFIX_PATH if it's in a non-standard location.
# Common locations: /usr/local (Homebrew), /opt/homebrew (Apple Silicon Homebrew)
find_path(QGLVIEWER_INCLUDE_DIR QGLViewer/qglviewer.h
    PATHS
        /usr/local/include
        /opt/homebrew/include
        ${CMAKE_PREFIX_PATH}/include
    DOC "QGLViewer include directory"
)

find_library(QGLVIEWER_LIBRARY NAMES QGLViewer QGLViewer-qt6 QGLViewer-qt5
    PATHS
        /usr/local/lib
        /usr/local/lib/cmake/QGLViewer
        /opt/homebrew/lib
        /opt/homebrew/lib/cmake/QGLViewer
        ${CMAKE_PREFIX_PATH}/lib
    DOC "QGLViewer library"
)

if (QGLVIEWER_INCLUDE_DIR AND QGLVIEWER_LIBRARY)
    message(STATUS "Found QGLViewer: ${QGLVIEWER_LIBRARY}")
    message(STATUS "QGLViewer include: ${QGLVIEWER_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "QGLViewer not found. Please install via 'brew install qglviewer' or set -DCMAKE_PREFIX_PATH")
endif()

# Convert to absolute paths
get_filename_component(QGLVIEWER_INCLUDE_DIR "${QGLVIEWER_INCLUDE_DIR}" ABSOLUTE)
get_filename_component(QGLVIEWER_LIBRARY "${QGLVIEWER_LIBRARY}" ABSOLUTE)

add_library(meshviewer_core STATIC
    src/MainWindow.cpp
    src/MainWindow.h
    src/MeshViewer.cpp
    src/MeshViewer.h
    src/Mesh.cpp
    src/Mesh.h
)

target_include_directories(meshviewer_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QGLVIEWER_INCLUDE_DIR}
    ${ASSIMP_INCLUDE_DIRS}
    /var/folders/kb/dq9v0bpn67gb64fkjtrvs38c0000gn/T/opencode/libQGLViewer
)

target_link_libraries(meshviewer_core PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::OpenGL
    Qt6::Xml
    Qt6::Concurrent
    assimp::assimp
    glm::glm
    ${QGLVIEWER_LIBRARY}
)

if(APPLE)
    target_link_libraries(meshviewer_core PUBLIC "-framework OpenGL")
endif()

add_executable(meshviewer
    src/main.cpp
    src/resources.qrc
)

target_link_libraries(meshviewer PRIVATE
    meshviewer_core
)

add_subdirectory(tests)
