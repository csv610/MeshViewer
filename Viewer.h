#ifndef VIEWER_H
#define VIEWER_H

#include <QGLViewer/qglviewer.h>
#include "Mesh.h"

class Viewer : public QGLViewer {
public:
    Viewer(QWidget* parent = nullptr);

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
    void drawMesh();
    void drawNormals();
    void drawLabels();
    void drawBB();
};

#endif
