# Mesh Viewer

A high-performance, interactive 3D mesh viewer built with **C++20**, **Qt6**, and **Modern OpenGL**. This application leverages Assimp for robust file importing and libQGLViewer for intuitive camera manipulation.

## Features

### 🚀 Performance & Optimization
- **Parallel Asynchronous Loading:** Multiple mesh files are loaded simultaneously in background threads, eliminating bottlenecks and keeping the UI responsive.
- **Binary Mesh Caching:** Automatically saves processed meshes to the system's temporary directory (`/tmp`) for nearly instantaneous re-loading in future sessions.
- **Modern Shader Pipeline:** Uses GLSL shaders and **Vertex Array Objects (VAOs)** for highly efficient GPU-based rendering.
- **Real-time Benchmarking:** Built-in performance monitoring displaying average draw time (ms) and estimated FPS in the status bar.

### 🛠 Visualization Tools
- **Multi-Mesh Support:** Load any number of meshes side-by-side for comparison.
- **Automatic Horizontal Stacking:** Meshes are automatically arranged along the X-axis with guaranteed separation (no bounding box overlaps).
- **Straight-Line Center Alignment:** The centers of all mesh bounding boxes are perfectly aligned on a single horizontal line, with the entire group centered at the origin `(0,0,0)` for stable rotation.
- **Togglable Scale Normalization:** Automatically resize models of different scales (e.g., millimeters vs. meters) to a uniform size for side-by-side viewing, or toggle off to see true relative dimensions.
- **Global Bounding Box:** Displays a collective **blue** bounding box for the entire scene alongside individual **red** boxes for each mesh.
- **Advanced Shading:** Toggle between modern per-pixel Phong shading and a legacy fallback path.
- **Wireframe & Metadata:** High-visibility wireframe overlays, vertex normals, and index labels for both vertices and faces.

### 🖱 Interactive Controls & UI
- **Intuitive Camera:** Standard trackball rotation, zooming, and panning via libQGLViewer. The center of rotation is automatically set to the center of the global bounding box.
- **Comprehensive UI:** Features a toolbar for quick toggles and a status bar for real-time loading feedback.
- **Multi-File CLI:** Pass multiple file paths as command-line arguments to launch directly into a multi-mesh scene.

## 🎨 Rendering Modes

The application supports two distinct rendering paths that can be toggled in real-time:

### ⚡️ Modern (Shader) Path
*   **Technique:** Per-Pixel Lighting (**Phong Shading**).
*   **Visuals:** Extremely smooth lighting and specular highlights.
*   **Implementation:** Custom GLSL Shaders + VAOs.

### 🏛 Legacy (VBO) Path
*   **Technique:** Per-Vertex Lighting (**Gouraud Shading**).
*   **Visuals:** Classical interpolated lighting; useful for compatibility testing.
*   **Implementation:** Fixed-Function Pipeline emulation.

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

### Compiling
```bash
mkdir build && cd build
cmake .. -DQGLVIEWER_INCLUDE_DIR=/path/to/libQGLViewer -DQGLVIEWER_LIBRARY=/path/to/libQGLViewer.dylib
make
```

## Usage

Run the executable and open mesh files through the UI (multi-select enabled), or pass multiple models as command-line arguments:
```bash
./build/meshviewer model1.obj model2.ply model3.stl
```

## Shortcuts
| Key | Action |
|-----|--------|
| **O** | Open Mesh(es) |
| **N** | Toggle Scale Normalization (Uniform vs. True Size) |
| **L** | Toggle Normals |
| **W** | Toggle Wireframe |
| **V** | Toggle Vertex Labels |
| **F** | Toggle Face Labels |
| **B** | Toggle Bounding Boxes (Individual & Global) |
| **S** | Toggle Modern Shaders |
| **H** | Help (Viewer Controls) |
