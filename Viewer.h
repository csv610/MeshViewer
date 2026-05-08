#ifndef VIEWER_H
#define VIEWER_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QGLViewer/qglviewer.h>
#include "Mesh.h"

class Viewer : public QGLViewer, protected QOpenGLFunctions {
public:
    Viewer(QWidget* parent = nullptr);
    ~Viewer();

    void setMesh(const Mesh& mesh);
    
    bool showWireframe = false;
    bool showNormals = false;
    bool showVertexLabels = false;
    bool showFaceLabels = false;
    bool showBoundingBox = false;

protected:
    virtual void draw() override;
    virtual void init() override;

private:
    Mesh mesh;
    QOpenGLBuffer vbo;
    QOpenGLBuffer ibo;

    void setupBuffers();
    void drawMeshVBO();
    void drawNormals();
    void drawLabels();
    void drawBB();
};

#endif
