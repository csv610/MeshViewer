#ifndef VIEWER_H
#define VIEWER_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QElapsedTimer>
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
    bool useShaders = true; // Modern path enabled by default

    double getFPS() const { return 1000.0 / lastFrameTime; }
    double getFrameTime() const { return lastFrameTime; }

protected:
    virtual void draw() override;
    virtual void init() override;

private:
    Mesh mesh;
    QOpenGLBuffer vbo;
    QOpenGLBuffer ibo;
    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgram shaderProgram;

    void setupBuffers();
    void setupVAO();
    void drawMeshVBO();    // Legacy
    void drawMeshShaders(bool lighting = true, const QVector3D& color = QVector3D(0.8f, 0.8f, 0.8f)); // Modern
    void drawNormals();
    void drawLabels();
    void drawBB();

    // Benchmarking
    QElapsedTimer perfTimer;
    double lastFrameTime = 0.01;
    int frameCount = 0;
    double accumulatedTime = 0.0;
};

#endif
