#include "MeshViewer.h"
#include <OpenGL/gl.h>
#include <QMatrix4x4>
#include <iostream>

MeshViewer::MeshViewer(QWidget* parent) : QGLViewer(parent) {}

MeshViewer::~MeshViewer() {
    makeCurrent();
    clearScene();
}

void MeshViewer::addMesh(const RenderMesh& m, const QString& name) {
    makeCurrent();
    auto model = std::make_shared<MeshModel>();
    model->data = m;
    model->name = name;
    
    // Initial scale calculation (always stored, but application is togglable)
    float targetSize = 100.0f;
    glm::vec3 dims = m.maxBB - m.minBB;
    float maxDim = std::max({dims.x, dims.y, dims.z});
    model->scale = (maxDim > 0) ? (targetSize / maxDim) : 1.0f;

    if (!isTestMode) {
        setupModelGPU(*model);
    }
    models.push_back(model);
    
    updateLayout();
}

void MeshViewer::updateLayout() {
    if (models.empty()) return;

    float currentLeftEdge = 0.0f;
    
    // Pass 1: Calculate offsets for stacking along X-axis
    for (size_t i = 0; i < models.size(); ++i) {
        auto& model = models[i];
        float s = normalizeScale ? model->scale : 1.0f;
        
        float w = (model->data.maxBB.x - model->data.minBB.x) * s;
        float centerY = (model->data.maxBB.y + model->data.minBB.y) * 0.5f * s;
        float centerZ = (model->data.maxBB.z + model->data.minBB.z) * 0.5f * s;

        if (i > 0) {
            auto& prevModel = models[i-1];
            float prevS = normalizeScale ? prevModel->scale : 1.0f;
            float prevWidth = (prevModel->data.maxBB.x - prevModel->data.minBB.x) * prevS;
            float padding = prevWidth * 0.1f;
            if (padding <= 0.0f) padding = normalizeScale ? 5.0f : 0.5f;
            currentLeftEdge += padding;
        }

        model->offset = glm::vec3(currentLeftEdge - (model->data.minBB.x * s), -centerY, -centerZ);
        currentLeftEdge += w;
    }

    // Pass 2: Shift everything so the entire group is centered at X=0
    float totalWidth = currentLeftEdge;
    float groupShiftX = totalWidth * 0.5f;
    for (auto& model : models) {
        model->offset.x -= groupShiftX;
    }

    updateSceneConstraints();
    update();
}

void MeshViewer::clearScene() {
    makeCurrent();
    for(auto& model : models) {
        model->vao->destroy();
        model->vbo->destroy();
        model->ibo->destroy();
    }
    models.clear();
    update();
}

void MeshViewer::updateSceneConstraints() {
    if (models.empty()) return;

    globalMinBB = glm::vec3(std::numeric_limits<float>::max());
    globalMaxBB = glm::vec3(std::numeric_limits<float>::lowest());

    for (const auto& model : models) {
        float s = normalizeScale ? model->scale : 1.0f;
        globalMinBB = glm::min(globalMinBB, model->data.minBB * s + model->offset);
        globalMaxBB = glm::max(globalMaxBB, model->data.maxBB * s + model->offset);
    }

    glm::vec3 center = (globalMinBB + globalMaxBB) * 0.5f;
    float radius = glm::length(globalMaxBB - globalMinBB) * 0.5f;
    setSceneCenter(qglviewer::Vec(center.x, center.y, center.z));
    setSceneRadius(radius > 0 ? radius : 1.0f);
    showEntireScene();
}

void MeshViewer::init() {
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
}

void MeshViewer::setupModelGPU(MeshModel& model) {
    model.vbo->create();
    model.vbo->bind();
    model.vbo->allocate(model.data.vertices.data(), model.data.vertices.size() * sizeof(RenderVertex));
    
    std::vector<unsigned int> allIndices;
    for (const auto& f : model.data.faces) {
        for (unsigned int idx : f.nodes) {
            allIndices.push_back(idx);
        }
    }

    model.ibo->create();
    model.ibo->bind();
    model.ibo->allocate(allIndices.data(), allIndices.size() * sizeof(unsigned int));

    model.vao->create();
    model.vao->bind();
    model.vbo->bind();
    model.ibo->bind();

    shaderProgram.enableAttributeArray("position");
    shaderProgram.setAttributeBuffer("position", GL_FLOAT, offsetof(RenderVertex, position), 3, sizeof(RenderVertex));
    
    shaderProgram.enableAttributeArray("normal");
    shaderProgram.setAttributeBuffer("normal", GL_FLOAT, offsetof(RenderVertex, normal), 3, sizeof(RenderVertex));

    shaderProgram.enableAttributeArray("color");
    shaderProgram.setAttributeBuffer("color", GL_FLOAT, offsetof(RenderVertex, color), 4, sizeof(RenderVertex));

    model.vao->release();
    model.vbo->release();
    model.ibo->release();
    model.gpuReady = true;
}

void MeshViewer::rebuildWithFaceColors(size_t modelIndex) {
    if (modelIndex >= models.size()) return;
    auto& model = models[modelIndex];
    if (model->data.faces.empty()) return;
    
    makeCurrent();
    
    model->vao->destroy();
    model->vbo->destroy();
    model->ibo->destroy();
    
    std::vector<RenderVertex> faceColorVertices;
    std::vector<unsigned int> newIndices;
    
    for (const auto& f : model->data.faces) {
        for (unsigned int idx : f.nodes) {
            RenderVertex v = model->data.vertices[idx];
            v.color = f.color;
            newIndices.push_back(static_cast<unsigned int>(faceColorVertices.size()));
            faceColorVertices.push_back(v);
        }
    }
    
    model->vbo->create();
    model->vbo->bind();
    model->vbo->allocate(faceColorVertices.data(), faceColorVertices.size() * sizeof(RenderVertex));
    
    model->ibo->create();
    model->ibo->bind();
    model->ibo->allocate(newIndices.data(), newIndices.size() * sizeof(unsigned int));

    model->vao->create();
    model->vao->bind();
    model->vbo->bind();
    model->ibo->bind();

    shaderProgram.enableAttributeArray("position");
    shaderProgram.setAttributeBuffer("position", GL_FLOAT, offsetof(RenderVertex, position), 3, sizeof(RenderVertex));
    
    shaderProgram.enableAttributeArray("normal");
    shaderProgram.setAttributeBuffer("normal", GL_FLOAT, offsetof(RenderVertex, normal), 3, sizeof(RenderVertex));

    shaderProgram.enableAttributeArray("color");
    shaderProgram.setAttributeBuffer("color", GL_FLOAT, offsetof(RenderVertex, color), 4, sizeof(RenderVertex));

    model->vao->release();
    model->vbo->release();
    model->ibo->release();
    model->gpuReady = true;
}

void MeshViewer::draw() {
    if (models.empty()) return;

    perfTimer.start();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    for (auto& model : models) {
        if (!model->gpuReady) continue;

        bool useFaceCol = useFaceColors && model->data.hasFaceColors;
        bool useVertCol = useVertexColors && model->data.hasVertexColors && !useFaceCol;

        // 1. Main render based on toggle flags
        if (showFaces) {
            if (useShaders && shaderProgram.isLinked()) {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(1.0f, 1.0f);
                drawModelShaders(*model, true, QVector3D(0.7f, 0.7f, 0.7f), useVertCol || useFaceCol);
                glDisable(GL_POLYGON_OFFSET_FILL);
            } else {
                glEnable(GL_LIGHTING);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                if (!useVertCol && !useFaceCol) glColor3f(0.7f, 0.7f, 0.7f);
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(1.0f, 1.0f);
                drawModelVBO(*model);
                glDisable(GL_POLYGON_OFFSET_FILL);
            }
        }
        if (showPoints) {
            glDisable(GL_LIGHTING);
glColor3f(0.0f, 0.0f, 1.0f);
            glPointSize(pointSize);
            glPushMatrix();
            glTranslatef(model->offset.x, model->offset.y, model->offset.z);
            if (normalizeScale) glScalef(model->scale, model->scale, model->scale);
            model->vbo->bind();
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, sizeof(RenderVertex), (void*)offsetof(RenderVertex, position));
            glDrawArrays(GL_POINTS, 0, model->data.vertices.size());
            glDisableClientState(GL_VERTEX_ARRAY);
            model->vbo->release();
            glPopMatrix();
        }
        if (showEdges) {
            glDisable(GL_LIGHTING);
            if (antialiasing) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_LINE_SMOOTH);
                glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            }
            glLineWidth(edgeThickness);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            if (useShaders && shaderProgram.isLinked()) {
                drawModelShaders(*model, false, edgeColor, false);
            } else {
                glColor3f(edgeColor.x(), edgeColor.y(), edgeColor.z());
                drawModelVBO(*model);
            }
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glLineWidth(1.0f);
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_BLEND);
        }
    }

    if (showNormals) drawNormals();
    if (showVertexLabels || showFaceLabels) drawLabels();
    if (showBoundingBox) drawBB();

    glPopAttrib();
    
    // Benchmarking update
    double elapsed = perfTimer.nsecsElapsed() / 1000000.0; // ms
    accumulatedTime += elapsed;
    frameCount++;
    if (frameCount >= 30) {
        lastFrameTime = accumulatedTime / frameCount;
        frameCount = 0;
        accumulatedTime = 0;
    }
}

void MeshViewer::drawModelShaders(MeshModel& model, bool lighting, const QVector3D& color, bool useVertexColors) {
    shaderProgram.bind();
    
    GLdouble modelview[16], projection[16];
    camera()->getModelViewMatrix(modelview);
    camera()->getProjectionMatrix(projection);
    
    QMatrix4x4 mv;
    for(int i=0; i<16; ++i) mv.data()[i] = (float)modelview[i];
    mv.translate(model.offset.x, model.offset.y, model.offset.z);
    if (normalizeScale) mv.scale(model.scale);

    QMatrix4x4 p;
    for(int i=0; i<16; ++i) p.data()[i] = (float)projection[i];
    
    shaderProgram.setUniformValue("mvp", p * mv);
    shaderProgram.setUniformValue("modelview", mv);
    shaderProgram.setUniformValue("normalMatrix", mv.normalMatrix());
    shaderProgram.setUniformValue("lightPos", QVector3D(100, 100, 100));
    shaderProgram.setUniformValue("color", color);
    shaderProgram.setUniformValue("useVertexColor", useVertexColors);
    shaderProgram.setUniformValue("useLighting", lighting);
    shaderProgram.setUniformValue("useFlatShading", useFlatShading);
    shaderProgram.setUniformValue("useMatCap", false);
    shaderProgram.setUniformValue("useCurvature", false);
    shaderProgram.setUniformValue("usePicking", false);

    model.vao->bind();
    model.ibo->bind();
    glDrawElements(GL_TRIANGLES, model.ibo->size() / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
    model.vao->release();
    
    shaderProgram.release();
}

void MeshViewer::drawModelVBO(MeshModel& model) {
    glPushMatrix();
    glTranslatef(model.offset.x, model.offset.y, model.offset.z);
    if (normalizeScale) glScalef(model.scale, model.scale, model.scale);
    
    model.vbo->bind();
    model.ibo->bind();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    if (model.data.hasVertexColors) glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(RenderVertex), (void*)offsetof(RenderVertex, position));
    glNormalPointer(GL_FLOAT, sizeof(RenderVertex), (void*)offsetof(RenderVertex, normal));
    if (model.data.hasVertexColors) glColorPointer(4, GL_FLOAT, sizeof(RenderVertex), (void*)offsetof(RenderVertex, color));

    glDrawElements(GL_TRIANGLES, model.ibo->size() / sizeof(unsigned int), GL_UNSIGNED_INT, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    if (model.data.hasVertexColors) glDisableClientState(GL_COLOR_ARRAY);

    model.ibo->release();
    model.vbo->release();
    
    glPopMatrix();
}

void MeshViewer::drawNormals() {
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    for (const auto& model : models) {
        float s = normalizeScale ? model->scale : 1.0f;
        float length = sceneRadius() * 0.05f;
        for (const auto& v : model->data.vertices) {
            glm::vec3 p = v.position * s + model->offset;
            glVertex3f(p.x, p.y, p.z);
            glVertex3f(p.x + v.normal.x * length, p.y + v.normal.y * length, p.z + v.normal.z * length);
        }
    }
    glEnd();
}

void MeshViewer::drawLabels() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    const size_t maxLabels = 500;
    size_t totalLabelCount = 0;

    qglviewer::Vec camPos = camera()->position();
    
    for (const auto& model : models) {
        if (!model->gpuReady) continue;
        float s = normalizeScale ? model->scale : 1.0f;
        
        if (showVertexLabels) {
            glColor3f(1.0f, 1.0f, 0.0f);
            for (size_t i = 0; i < model->data.vertices.size(); ++i) {
                if (totalLabelCount >= maxLabels) break;
                
                const auto& v = model->data.vertices[i];
                glm::vec3 worldPos = v.position * s + model->offset;
                
                qglviewer::Vec screenPos = camera()->projectedCoordinatesOf(qglviewer::Vec(worldPos.x, worldPos.y, worldPos.z));
                
                if (screenPos.z > 0.0 && screenPos.z < 1.0) {
                    qglviewer::Vec worldVec(worldPos.x, worldPos.y, worldPos.z);
                    float dist = (worldVec - camPos).norm();
                    if (dist > sceneRadius() * 3.0) continue;
                    
                    int x = static_cast<int>(screenPos.x);
                    int y = static_cast<int>(screenPos.y);
                    if (x >= 0 && x < width() && y >= 0 && y < height()) {
                        drawText(x, y + 10, QString::number(i));
                        totalLabelCount++;
                    }
                }
            }
        }

        if (showFaceLabels) {
            glColor3f(0.0f, 1.0f, 1.0f);
            for (size_t i = 0; i < model->data.faces.size(); ++i) {
                if (totalLabelCount >= maxLabels) break;
                const auto& f = model->data.faces[i];
                if (f.nodes.size() < 3) continue;
                
                glm::vec3 center(0.0f);
                for (unsigned int idx : f.nodes) {
                    center += model->data.vertices[idx].position;
                }
                center /= (float)f.nodes.size();
                center = center * s + model->offset;
                
                qglviewer::Vec screenPos = camera()->projectedCoordinatesOf(qglviewer::Vec(center.x, center.y, center.z));
                
                if (screenPos.z > 0.0 && screenPos.z < 1.0) {
                    qglviewer::Vec centerVec(center.x, center.y, center.z);
                    float dist = (centerVec - camPos).norm();
                    if (dist > sceneRadius() * 3.0) continue;
                    
                    int x = static_cast<int>(screenPos.x);
                    int y = static_cast<int>(screenPos.y);
                    if (x >= 0 && x < width() && y >= 0 && y < height()) {
                        drawText(x, y + 10, QString::number(i));
                        totalLabelCount++;
                    }
                }
            }
        }
    }
    
    glEnable(GL_DEPTH_TEST);
}

void MeshViewer::drawBB() {
    glDisable(GL_LIGHTING);
    
    auto drawBox = [](const glm::vec3& min, const glm::vec3& max) {
        glBegin(GL_LINE_LOOP); glVertex3f(min.x, min.y, min.z); glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, max.y, min.z); glVertex3f(min.x, max.y, min.z); glEnd();
        glBegin(GL_LINE_LOOP); glVertex3f(min.x, min.y, max.z); glVertex3f(max.x, min.y, max.z); glVertex3f(max.x, max.y, max.z); glVertex3f(min.x, max.y, max.z); glEnd();
        glBegin(GL_LINES); 
        glVertex3f(min.x, min.y, min.z); glVertex3f(min.x, min.y, max.z); 
        glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, min.y, max.z); 
        glVertex3f(max.x, max.y, min.z); glVertex3f(max.x, max.y, max.z); 
        glVertex3f(min.x, max.y, min.z); glVertex3f(min.x, max.y, max.z); 
        glEnd();
    };

    // Per-mesh red boxes
    glColor3f(1.0f, 0.0f, 0.0f);
    for (const auto& model : models) {
        float s = normalizeScale ? model->scale : 1.0f;
        drawBox(model->data.minBB * s + model->offset, model->data.maxBB * s + model->offset);
    }

    // Global blue box
    if (models.size() > 1) {
        glColor3f(0.0f, 0.0f, 1.0f);
        glLineWidth(2.0f);
        drawBox(globalMinBB, globalMaxBB);
        glLineWidth(1.0f);
    }
}
