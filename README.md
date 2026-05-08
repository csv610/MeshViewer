# Mesh Viewer

A 3D mesh visualization tool built with C++20, Qt6, and OpenGL. It uses Assimp for model importing and libQGLViewer for camera control.

## Core Capabilities

### Multi-Mesh Visualization
- **Simultaneous Loading:** Supports opening multiple mesh files (OBJ, PLY, STL, OFF, etc.) in a single scene.
- **Horizontal Stacking:** Automatically arranges meshes side-by-side along the X-axis with uniform padding.
- **Center Alignment:** Aligns the bounding box centers of all loaded meshes along a straight line. The entire scene is centered at `(0,0,0)` to provide a consistent center of rotation.
- **Global Bounding Box:** Displays a blue bounding box encompassing the entire scene, while individual meshes are bounded in red.

### Scale Management
- **Normalize Scale Toggle:** Optionally resizes all models to a standard height (100 units) to allow side-by-side comparison of models with different intrinsic units (e.g., millimeters vs. meters).
- **True Scale Mode:** When normalization is disabled, models are displayed at their original coordinate dimensions.

### Performance and Feedback
- **Parallel Loading:** Uses background threads to load multiple meshes concurrently, preventing UI freezes during large file imports.
- **Binary Caching:** Stores processed mesh data in the system temporary directory to reduce loading times for previously viewed models.
- **Status Reporting:** Provides real-time feedback in the status bar regarding loading progress, vertex/face counts, and rendering performance (FPS/draw time).

### Overlays and Shading
- **Rendering Modes:** Toggle between a modern GLSL-based shader path (per-pixel lighting) and a legacy VBO-based path.
- **Overlays:** Toggleable visualization for wireframes, vertex normals, and numerical labels for vertices and faces.

## Dependencies

- **Qt 6.x**
- **Assimp**
- **GLM**
- **libQGLViewer**

## Build Instructions

### Installation (macOS)
```bash
brew install qt assimp glm
```

### Compilation
```bash
mkdir build && cd build
cmake .. -DQGLVIEWER_INCLUDE_DIR=/path/to/libQGLViewer -DQGLVIEWER_LIBRARY=/path/to/libQGLViewer.dylib
make
```

## Usage

### Command Line
Launch with any number of model files:
```bash
./meshviewer model1.obj model2.ply model3.off
```

### Key Shortcuts
| Key | Action |
|-----|--------|
| **O** | Open mesh files |
| **N** | Toggle scale normalization |
| **L** | Toggle vertex normals |
| **W** | Toggle wireframe overlay |
| **V** | Toggle vertex index labels |
| **F** | Toggle face index labels |
| **B** | Toggle bounding boxes |
| **S** | Toggle modern shader mode |
| **H** | Show help dialog |
