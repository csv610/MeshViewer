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

# QGLViewer as submodule
add_subdirectory(external/libQGLViewer)

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
    ${ASSIMP_INCLUDE_DIRS}
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
    QGLViewer
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
