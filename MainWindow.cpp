#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QFileDialog>
#include <QToolBar>
#include <QStatusBar>
#include <QKeyEvent>
#include <QtConcurrent/QtConcurrent>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    viewer = new Viewer(this);
    setCentralWidget(viewer);

    // Menu Bar
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QAction* openAction = fileMenu->addAction("&Open Mesh", this, &MainWindow::openFile, QKeySequence::Open);
    
    QMenu* toolsMenu = menuBar()->addMenu("&Tools");
    toolsMenu->addAction("Clear &Current Cache", this, &MainWindow::clearCache);

    // Tool Bar
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

    toolBar->addSeparator();
    QCheckBox* shaderCb = addToggle("Use Modern Shaders", Qt::Key_S, &MainWindow::toggleShaderMode);
    shaderCb->setChecked(true);

    connect(&watcher, &QFutureWatcher<bool>::finished, this, &MainWindow::onMeshLoaded);

    progressDialog = new QProgressDialog("Loading mesh...", "Cancel", 0, 0, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(500); // Show only if it takes more than 0.5s
    progressDialog->close();

    benchmarkTimer = new QTimer(this);
    connect(benchmarkTimer, &QTimer::timeout, this, &MainWindow::updateBenchmark);
    benchmarkTimer->start(500); // Update status every 500ms

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
    if (watcher.isRunning()) {
        statusBar()->showMessage("Wait! Another mesh is still loading...");
        return;
    }

    pendingFilename = filename;
    statusBar()->showMessage("Loading mesh: " + filename + " ...");
    
    progressDialog->setLabelText("Loading mesh: " + QFileInfo(filename).fileName() + " ...");
    progressDialog->show();

    QFuture<bool> future = QtConcurrent::run([this, filename]() {
        return mesh.load(filename.toStdString());
    });
    watcher.setFuture(future);
}

void MainWindow::onMeshLoaded() {
    progressDialog->close();
    if (watcher.result()) {
        viewer->setMesh(mesh);
        setWindowTitle("Mesh Viewer - " + pendingFilename);
        statusBar()->showMessage(QString("Loaded: %1 | Vertices: %2 | Faces: %3")
            .arg(pendingFilename)
            .arg(mesh.vertices.size())
            .arg(mesh.indices.size() / 3));
    } else {
        statusBar()->showMessage("Failed to load mesh: " + pendingFilename);
    }
}

void MainWindow::toggleWireframe(bool checked) { viewer->showWireframe = checked; viewer->update(); }
void MainWindow::toggleNormals(bool checked) { viewer->showNormals = checked; viewer->update(); }
void MainWindow::toggleVertexLabels(bool checked) { viewer->showVertexLabels = checked; viewer->update(); }
void MainWindow::toggleFaceLabels(bool checked) { viewer->showFaceLabels = checked; viewer->update(); }
void MainWindow::toggleBB(bool checked) { viewer->showBoundingBox = checked; viewer->update(); }
void MainWindow::toggleShaderMode(bool checked) { viewer->useShaders = checked; viewer->update(); }

void MainWindow::updateBenchmark() {
    if (mesh.vertices.empty()) return;

    QString mode = viewer->useShaders ? "MODERN (Shaders)" : "LEGACY (VBO)";
    QString stats = QString("[%1] Draw: %2 ms | Est. FPS: %3")
        .arg(mode)
        .arg(viewer->getFrameTime(), 0, 'f', 2)
        .arg(viewer->getFPS(), 0, 'f', 1);
    
    statusBar()->showMessage(stats);
}

void MainWindow::clearCache() {
    if (pendingFilename.isEmpty()) {
        statusBar()->showMessage("No mesh loaded to clear cache for.");
        return;
    }

    QString cachePath = QString::fromStdString(Mesh::getCachePath(pendingFilename.toStdString()));
    if (QFile::exists(cachePath)) {
        if (QFile::remove(cachePath)) {
            statusBar()->showMessage("Cache cleared for: " + QFileInfo(pendingFilename).fileName());
        } else {
            statusBar()->showMessage("Failed to remove cache file.");
        }
    } else {
        statusBar()->showMessage("No cache file found for this mesh.");
    }
}
