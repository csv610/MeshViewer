#include "Viewer.h"
#include <OpenGL/gl.h>
#include <iostream>

Viewer::Viewer(QWidget* parent) : QGLViewer(parent) {}

void Viewer::setMesh(const Mesh& m) {
    mesh = m;
    if (!mesh.vertices.empty()) {
        glm::vec3 center = (mesh.minBB + mesh.maxBB) * 0.5f;
        float radius = glm::length(mesh.maxBB - mesh.minBB) * 0.5f;
        setSceneCenter(qglviewer::Vec(center.x, center.y, center.z));
        setSceneRadius(radius > 0 ? radius : 1.0f);
        showEntireScene();
    }
    update();
}

void Viewer::init() {
    restoreStateFromFile();
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void Viewer::draw() {
    if (mesh.vertices.empty()) return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // 1. Shaded Mesh
    glEnable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(0.8f, 0.8f, 0.8f);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);
    drawMesh();
    glDisable(GL_POLYGON_OFFSET_FILL);

    // 2. Overlays
    if (showWireframe) {
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(0.0f, 0.0f, 0.0f);
        drawMesh();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (showNormals) drawNormals();
    if (showVertexLabels || showFaceLabels) drawLabels();
    if (showBoundingBox) drawBB();

    glPopAttrib();
}

void Viewer::drawMesh() {
    glBegin(GL_TRIANGLES);
    for (unsigned int i : mesh.indices) {
        const auto& v = mesh.vertices[i];
        glNormal3f(v.normal.x, v.normal.y, v.normal.z);
        glVertex3f(v.position.x, v.position.y, v.position.z);
    }
    glEnd();
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
