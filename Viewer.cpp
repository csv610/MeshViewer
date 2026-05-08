#include "Viewer.h"
#include <OpenGL/gl.h>
#include <iostream>

Viewer::Viewer(QWidget* parent) 
    : QGLViewer(parent), 
      vbo(QOpenGLBuffer::VertexBuffer), 
      ibo(QOpenGLBuffer::IndexBuffer) {}

Viewer::~Viewer() {
    makeCurrent();
    vao.destroy();
    vbo.destroy();
    ibo.destroy();
}

void Viewer::setMesh(const Mesh& m) {
    makeCurrent();
    mesh = m;
    if (!mesh.vertices.empty()) {
        glm::vec3 center = (mesh.minBB + mesh.maxBB) * 0.5f;
        float radius = glm::length(mesh.maxBB - mesh.minBB) * 0.5f;
        setSceneCenter(qglviewer::Vec(center.x, center.y, center.z));
        setSceneRadius(radius > 0 ? radius : 1.0f);
        showEntireScene();
        setupBuffers();
        setupVAO();
    }
    update();
}

void Viewer::init() {
    initializeOpenGLFunctions();
    
    // Load shaders
    if (!shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/mesh.vert")) {
        std::cerr << "Vertex shader error: " << shaderProgram.log().toStdString() << std::endl;
    }
    if (!shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/mesh.frag")) {
        std::cerr << "Fragment shader error: " << shaderProgram.log().toStdString() << std::endl;
    }
    if (!shaderProgram.link()) {
        std::cerr << "Shader link error: " << shaderProgram.log().toStdString() << std::endl;
    }

    restoreStateFromFile();
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void Viewer::setupBuffers() {
    if (!vbo.isCreated()) vbo.create();
    vbo.bind();
    vbo.allocate(mesh.vertices.data(), mesh.vertices.size() * sizeof(Mesh::Vertex));
    
    if (!ibo.isCreated()) ibo.create();
    ibo.bind();
    ibo.allocate(mesh.indices.data(), mesh.indices.size() * sizeof(unsigned int));
    
    vbo.release();
    ibo.release();
}

void Viewer::setupVAO() {
    if (!vao.isCreated()) vao.create();
    vao.bind();
    vbo.bind();
    ibo.bind();

    shaderProgram.enableAttributeArray("position");
    shaderProgram.setAttributeBuffer("position", GL_FLOAT, offsetof(Mesh::Vertex, position), 3, sizeof(Mesh::Vertex));
    
    shaderProgram.enableAttributeArray("normal");
    shaderProgram.setAttributeBuffer("normal", GL_FLOAT, offsetof(Mesh::Vertex, normal), 3, sizeof(Mesh::Vertex));

    vao.release();
    vbo.release();
    ibo.release();
}

void Viewer::draw() {
    if (mesh.vertices.empty()) return;

    perfTimer.start();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // 1. Shaded Mesh
    if (useShaders && shaderProgram.isLinked()) {
        drawMeshShaders();
    } else {
        glEnable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3f(0.8f, 0.8f, 0.8f);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);
        drawMeshVBO();
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    // 2. Overlays
    if (showWireframe) {
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(0.0f, 0.0f, 0.0f);
        if (useShaders && shaderProgram.isLinked()) {
             // We can still use shaders for wireframe by disabling lighting uniform
             shaderProgram.bind();
             shaderProgram.setUniformValue("useLighting", false);
             shaderProgram.setUniformValue("color", QVector3D(0,0,0));
             drawMeshShaders();
             shaderProgram.release();
        } else {
            drawMeshVBO();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (showNormals) drawNormals();
    if (showVertexLabels || showFaceLabels) drawLabels();
    if (showBoundingBox) drawBB();

    glPopAttrib();
    
    // Benchmarking update
    double elapsed = perfTimer.nsecsElapsed() / 1000000.0; // ms
    accumulatedTime += elapsed;
    frameCount++;
    if (frameCount >= 30) { // Average over 30 frames
        lastFrameTime = accumulatedTime / frameCount;
        frameCount = 0;
        accumulatedTime = 0;
    }
}

void Viewer::drawMeshShaders() {
    shaderProgram.bind();
    
    // Get matrices from libQGLViewer
    GLdouble modelview[16], projection[16];
    camera()->getModelViewMatrix(modelview);
    camera()->getProjectionMatrix(projection);
    
    QMatrix4x4 mv;
    for(int i=0; i<16; ++i) mv.data()[i] = (float)modelview[i];
    QMatrix4x4 p;
    for(int i=0; i<16; ++i) p.data()[i] = (float)projection[i];
    
    shaderProgram.setUniformValue("mvp", p * mv);
    shaderProgram.setUniformValue("modelview", mv);
    shaderProgram.setUniformValue("normalMatrix", mv.normalMatrix());
    shaderProgram.setUniformValue("lightPos", QVector3D(0, 0, 10)); // Simple head light
    shaderProgram.setUniformValue("color", QVector3D(0.8f, 0.8f, 0.8f));
    shaderProgram.setUniformValue("useLighting", true);
    shaderProgram.setUniformValue("useFlatShading", false);
    shaderProgram.setUniformValue("useMatCap", false);
    shaderProgram.setUniformValue("useCurvature", false);
    shaderProgram.setUniformValue("usePicking", false);

    vao.bind();
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    vao.release();
    
    shaderProgram.release();
}
void Viewer::drawMeshVBO() {
    vbo.bind();
    ibo.bind();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, position));
    glNormalPointer(GL_FLOAT, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, normal));

    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    ibo.release();
    vbo.release();
}

void Viewer::drawNormals() {
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 1.0f, 0.0f);
    float length = sceneRadius() * 0.05f;
    glBegin(GL_LINES);
    for (const auto& v : mesh.vertices) {
        glVertex3f(v.position.x, v.position.y, v.position.z);
        glVertex3f(v.position.x + v.normal.x * length, v.position.y + v.normal.y * length, v.position.z + v.normal.z * length);
    }
    glEnd();
}

void Viewer::drawLabels() {
    glDisable(GL_LIGHTING);
    const size_t maxLabels = 1000;
    size_t labelCount = 0;

    if (showVertexLabels) {
        glColor3f(1.0f, 1.0f, 0.0f);
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            const auto& v = mesh.vertices[i];
            qglviewer::Vec screenPos = camera()->projectedCoordinatesOf(qglviewer::Vec(v.position.x, v.position.y, v.position.z));
            if (screenPos.z >= 0.0 && screenPos.z <= 1.0) {
                drawText((int)screenPos.x, (int)screenPos.y, QString::number(i));
                if (++labelCount >= maxLabels) break;
            }
        }
    }

    if (showFaceLabels) {
        glColor3f(0.0f, 1.0f, 1.0f);
        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            if (i + 2 >= mesh.indices.size()) break;
            glm::vec3 center = (mesh.vertices[mesh.indices[i]].position + mesh.vertices[mesh.indices[i+1]].position + mesh.vertices[mesh.indices[i+2]].position) / 3.0f;
            qglviewer::Vec screenPos = camera()->projectedCoordinatesOf(qglviewer::Vec(center.x, center.y, center.z));
            if (screenPos.z >= 0.0 && screenPos.z <= 1.0) {
                drawText((int)screenPos.x, (int)screenPos.y, QString::number(i / 3));
                if (++labelCount >= maxLabels) break;
            }
        }
    }
}

void Viewer::drawBB() {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.0f, 0.0f);
    glm::vec3 min = mesh.minBB;
    glm::vec3 max = mesh.maxBB;
    glBegin(GL_LINE_LOOP); glVertex3f(min.x, min.y, min.z); glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, max.y, min.z); glVertex3f(min.x, max.y, min.z); glEnd();
    glBegin(GL_LINE_LOOP); glVertex3f(min.x, min.y, max.z); glVertex3f(max.x, min.y, max.z); glVertex3f(max.x, max.y, max.z); glVertex3f(min.x, max.y, max.z); glEnd();
    glBegin(GL_LINES); glVertex3f(min.x, min.y, min.z); glVertex3f(min.x, min.y, max.z); glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, min.y, max.z); glVertex3f(max.x, max.y, min.z); glVertex3f(max.x, max.y, max.z); glVertex3f(min.x, max.y, min.z); glVertex3f(min.x, max.y, max.z); glEnd();
}
