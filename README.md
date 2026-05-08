# Mesh Viewer

A high-performance, interactive 3D mesh viewer built with **C++20**, **Qt6**, and **Modern OpenGL**. This application leverages Assimp for robust file importing and libQGLViewer for intuitive camera manipulation.

## Features

### 🚀 Performance & Optimization
- **Modern Shader Pipeline:** Uses GLSL shaders and **Vertex Array Objects (VAOs)** for highly efficient GPU-based rendering.
- **Binary Mesh Caching:** Automatically saves processed meshes to the system's temporary directory (`/tmp`) for nearly instantaneous re-loading.
- **Asynchronous Loading:** Large mesh files are loaded in background threads, keeping the UI responsive with real-time progress feedback.
- **Real-time Benchmarking:** Built-in performance monitoring displaying average draw time (ms) and estimated FPS in the status bar.
- **Memory Efficient:** Optimized data structures with pre-allocation to handle millions of vertices and faces smoothly.

### 🛠 Visualization Tools
- **Multiple Formats:** Supports OBJ, PLY, STL, OFF, and more via Assimp.
- **Advanced Shading:** Toggle between modern shader-based shading and a legacy fallback path.
- **Wireframe Overlay:** High-visibility wireframe with Z-fighting prevention.
- **Normal Visualization:** View vertex normals to inspect surface orientation.
- **Metadata Labels:** Toggle index labels for both vertices and faces.
- **Bounding Box:** Visual representation of the mesh's spatial extents.

### 🖱 Interactive Controls & UI
- **Intuitive Camera:** Standard trackball rotation, zooming, and panning via libQGLViewer.
- **Comprehensive UI:** Features a toolbar for quick toggles and a menu bar for file management and advanced tools.
- **Cache Management:** Tools to clear specific mesh caches directly from the application menu.

## Dependencies

- **Qt 6.x** (Core, Gui, Widgets, OpenGLWidgets, Concurrent)
- **Assimp** (Open Asset Import Library)
- **GLM** (OpenGL Mathematics)
- **libQGLViewer** (Qt-based OpenGL viewer library)

## Build Instructions

### Prerequisites
Ensure all dependencies are installed. On macOS using Homebrew:
```bash
brew install qt assimp glm
```
*Note: libQGLViewer may need to be built from source or provided via a custom path.*

### Compiling
```bash
mkdir build && cd build
cmake .. -DQGLVIEWER_INCLUDE_DIR=/path/to/libQGLViewer -DQGLVIEWER_LIBRARY=/path/to/libQGLViewer.dylib
make
```

## Usage

Run the executable and open a mesh file through the UI, or pass it as a command-line argument:
```bash
./build/meshviewer path/to/your/model.obj
```

## Shortcuts
| Key | Action |
|-----|--------|
| **O** | Open Mesh |
| **S** | Toggle Modern Shaders (Benchmarking) |
| **W** | Toggle Wireframe |
| **N** | Toggle Normals |
| **V** | Toggle Vertex Labels |
| **F** | Toggle Face Labels |
| **B** | Toggle Bounding Box |
| **H** | Help (Viewer Controls) |
