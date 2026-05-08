#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QFileDialog>
#include <QToolBar>
#include <QStatusBar>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    viewer = new Viewer(this);
    setCentralWidget(viewer);

    QToolBar* toolBar = addToolBar("Controls");
    
    QPushButton* openBtn = new QPushButton("Open Mesh", this);
    openBtn->setShortcut(QKeySequence::Open);
    connect(openBtn, &QPushButton::clicked, this, &MainWindow::openFile);
    toolBar->addWidget(openBtn);

    toolBar->addSeparator();

    auto addToggle = [&](const QString& label, const QKeySequence& ks, auto slot) {
        QCheckBox* cb = new QCheckBox(label, this);
        cb->setShortcut(ks);
        cb->setToolTip(QString("Shortcut: %1").arg(ks.toString()));
        connect(cb, &QCheckBox::toggled, this, slot);
        toolBar->addWidget(cb);
        return cb;
    };

    addToggle("Wireframe", Qt::Key_W, &MainWindow::toggleWireframe);
    addToggle("Normals", Qt::Key_N, &MainWindow::toggleNormals);
    addToggle("Vertex Labels", Qt::Key_V, &MainWindow::toggleVertexLabels);
    addToggle("Face Labels", Qt::Key_F, &MainWindow::toggleFaceLabels);
    addToggle("Bounding Box", Qt::Key_B, &MainWindow::toggleBB);

    statusBar()->showMessage("Ready.");
    setWindowTitle("Mesh Viewer");
    resize(1000, 1000);
}

void MainWindow::openFile() {
    QString filename = QFileDialog::getOpenFileName(this, "Open Mesh", "", "Mesh Files (*.obj *.ply *.stl *.off);;All Files (*)");
    if (!filename.isEmpty()) {
        loadMesh(filename);
    }
}

void MainWindow::loadMesh(const QString& filename) {
    Mesh mesh;
    if (mesh.load(filename.toStdString())) {
        viewer->setMesh(mesh);
        setWindowTitle("Mesh Viewer - " + filename);
        statusBar()->showMessage(QString("Vertices: %1 | Faces: %2").arg(mesh.vertices.size()).arg(mesh.indices.size() / 3));
    } else {
        statusBar()->showMessage("Failed to load mesh: " + filename);
    }
}

void MainWindow::toggleWireframe(bool checked) { viewer->showWireframe = checked; viewer->update(); }
void MainWindow::toggleNormals(bool checked) { viewer->showNormals = checked; viewer->update(); }
void MainWindow::toggleVertexLabels(bool checked) { viewer->showVertexLabels = checked; viewer->update(); }
void MainWindow::toggleFaceLabels(bool checked) { viewer->showFaceLabels = checked; viewer->update(); }
void MainWindow::toggleBB(bool checked) { viewer->showBoundingBox = checked; viewer->update(); }
