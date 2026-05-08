cmake_minimum_required(VERSION 3.16)
project(meshviewer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable AUTOUIC, AUTOMOC, and AUTORCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets OpenGL Xml Concurrent)

# Find Assimp and GLM
find_package(assimp REQUIRED)
find_package(glm REQUIRED)

# Find QGLViewer
# We search for QGLViewer. Users might need to provide -DCMAKE_PREFIX_PATH if it's in a non-standard location.
find_path(QGLVIEWER_INCLUDE_DIR QGLViewer/qglviewer.h)
find_library(QGLVIEWER_LIBRARY NAMES QGLViewer QGLViewer-qt6 QGLViewer-qt5)

if (QGLVIEWER_INCLUDE_DIR AND QGLVIEWER_LIBRARY)
    message(STATUS "Found QGLViewer: ${QGLVIEWER_LIBRARY}")
else()
    message(FATAL_ERROR "QGLViewer not found. Please set QGLVIEWER_INCLUDE_DIR and QGLVIEWER_LIBRARY.")
endif()

add_executable(meshviewer
    main.cpp
    MainWindow.cpp
    MainWindow.h
    Viewer.cpp
    Viewer.h
    Mesh.cpp
    Mesh.h
    resources.qrc
)

target_include_directories(meshviewer PRIVATE 
    ${QGLVIEWER_INCLUDE_DIR}
    ${ASSIMP_INCLUDE_DIRS}
)

target_link_libraries(meshviewer PRIVATE
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
    target_link_libraries(meshviewer PRIVATE "-framework OpenGL")
endif()
