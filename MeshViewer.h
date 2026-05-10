#ifndef MESH_VIEWER_H
#define MESH_VIEWER_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QElapsedTimer>
#include <QGLViewer/qglviewer.h>
#include <memory>
#include "Mesh.h"

struct MeshModel {
    Mesh data;
    std::unique_ptr<QOpenGLBuffer> vbo;
    std::unique_ptr<QOpenGLBuffer> ibo;
    std::unique_ptr<QOpenGLVertexArrayObject> vao;
    bool gpuReady = false;
    QString name;
    glm::vec3 offset = glm::vec3(0.0f);
    float scale = 1.0f;

    MeshModel() : 
        vbo(std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer)),
        ibo(std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer)),
        vao(std::make_unique<QOpenGLVertexArrayObject>()) {}
};

class MeshViewer : public QGLViewer, protected QOpenGLFunctions {
public:
    MeshViewer(QWidget* parent = nullptr);
    ~MeshViewer();

    void addMesh(const Mesh& mesh, const QString& name = "Mesh");
    void clearScene();
    void updateLayout(); // New method to recalculate offsets
    
    bool showWireframe = false;
    QVector3D edgeColor = QVector3D(0.0f, 0.0f, 0.0f);
    float edgeThickness = 1.0f;
    bool antialiasing = true;
    bool showNormals = false;
    bool showVertexLabels = false;
    bool showFaceLabels = false;
    bool showBoundingBox = false;
    bool useShaders = true;
    bool useFlatShading = false;
    bool normalizeScale = true; // Togglable normalization

    double getFPS() const { return 1000.0 / lastFrameTime; }
    double getFrameTime() const { return lastFrameTime; }

    // Test accessors
    const std::vector<std::shared_ptr<MeshModel>>& getModels() const { return models; }
    glm::vec3 getGlobalMinBB() const { return globalMinBB; }
    glm::vec3 getGlobalMaxBB() const { return globalMaxBB; }
    bool isTestMode = false;

protected:
    virtual void draw() override;
    virtual void init() override;

private:
    std::vector<std::shared_ptr<MeshModel>> models;
    QOpenGLShaderProgram shaderProgram;

    void setupModelGPU(MeshModel& model);
    void drawModelShaders(MeshModel& model, bool lighting = true, const QVector3D& color = QVector3D(0.8f, 0.8f, 0.8f), bool useVertexColors = true);
    void drawModelVBO(MeshModel& model);
    
    void drawNormals();
    void drawLabels();
    void drawBB();

    void updateSceneConstraints();

    glm::vec3 globalMinBB;
    glm::vec3 globalMaxBB;

    // Benchmarking
    QElapsedTimer perfTimer;
    double lastFrameTime = 0.01;
    int frameCount = 0;
    double accumulatedTime = 0.0;
};

#endif
