# Mesh Viewer

A high-performance, interactive 3D mesh viewer built with C++, Qt6, and OpenGL. This application leverages Assimp for robust file importing and libQGLViewer for intuitive camera manipulation.

## Features

### 🚀 Performance & Optimization
- **Asynchronous Loading:** Large mesh files are loaded in background threads to keep the UI responsive.
- **VBO Rendering:** Utilizes Vertex Buffer Objects (VBOs) for high-performance rendering of complex geometries.
- **Memory Efficient:** Optimized data structures with pre-allocation to handle millions of vertices and faces.

### 🛠 Visualization Tools
- **Multiple Formats:** Supports OBJ, PLY, STL, OFF, and more via Assimp.
- **Shading & Wireframe:** Toggle between smooth shaded mode and wireframe overlay.
- **Normal Visualization:** View vertex normals to inspect surface orientation.
- **Metadata Labels:** Toggle index labels for both vertices and faces (optimized for visibility).
- **Bounding Box:** Visual representation of the mesh's spatial extents.

### 🖱 Interactive Controls
- **Intuitive Camera:** Standard trackball rotation, zooming, and panning.
- **Responsive UI:** Real-time feedback with a status bar and progress dialogs for long-running operations.
- **Keyboard Shortcuts:** Quick toggles for all visualization modes (e.g., 'W' for Wireframe, 'N' for Normals).

## Dependencies

- **Qt 6.x** (Core, Gui, Widgets, OpenGLWidgets)
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
./meshviewer path/to/your/model.obj
```

## Shortcuts
| Key | Action |
|-----|--------|
| **O** | Open Mesh |
| **W** | Toggle Wireframe |
| **N** | Toggle Normals |
| **V** | Toggle Vertex Labels |
| **F** | Toggle Face Labels |
| **B** | Toggle Bounding Box |
| **H** | Help (Viewer Controls) |
