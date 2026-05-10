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
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    viewer = new MeshViewer(this);
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
        if (!ks.isEmpty()) {
            cb->setShortcut(ks);
            cb->setToolTip(QString("Shortcut: %1").arg(ks.toString()));
        }
        connect(cb, &QCheckBox::toggled, this, slot);
        toolBar->addWidget(cb);
        return cb;
    };

    addToggle("Wireframe", Qt::Key_W, &MainWindow::toggleWireframe);
    
    // Edge controls
    toolBar->addSeparator();
    toolBar->addWidget(new QLabel(" Edge: ", this));
    
    QPushButton* colorBtn = new QPushButton("Color", this);
    connect(colorBtn, &QPushButton::clicked, this, &MainWindow::setEdgeColor);
    toolBar->addWidget(colorBtn);

    QDoubleSpinBox* thicknessSpin = new QDoubleSpinBox(this);
    thicknessSpin->setRange(0.1, 10.0);
    thicknessSpin->setSingleStep(0.1);
    thicknessSpin->setValue(1.0);
    thicknessSpin->setSuffix(" px");
    connect(thicknessSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::setEdgeThickness);
    toolBar->addWidget(thicknessSpin);

    QCheckBox* aaCb = addToggle("AA", QKeySequence(), &MainWindow::toggleAntialiasing);
    aaCb->setChecked(true);

    toolBar->addSeparator();
    addToggle("Flat Shading", Qt::Key_P, &MainWindow::toggleFlatShading);
    addToggle("Normals", Qt::Key_L, &MainWindow::toggleNormals);
    addToggle("Vertex Labels", Qt::Key_V, &MainWindow::toggleVertexLabels);
    addToggle("Face Labels", Qt::Key_F, &MainWindow::toggleFaceLabels);
    addToggle("Bounding Box", Qt::Key_B, &MainWindow::toggleBB);
    QCheckBox* normCb = addToggle("Normalize Scale", Qt::Key_N, &MainWindow::toggleNormalizeScale);
    normCb->setChecked(true);

    toolBar->addSeparator();
    QCheckBox* shaderCb = addToggle("Use Modern Shaders", Qt::Key_S, &MainWindow::toggleShaderMode);
    shaderCb->setChecked(true);

    benchmarkTimer = new QTimer(this);
    connect(benchmarkTimer, &QTimer::timeout, this, &MainWindow::updateBenchmark);
    benchmarkTimer->start(500); // Update status every 500ms

    statusBar()->showMessage("Ready.");
    setWindowTitle("Mesh Viewer");
    resize(1200, 1000);
}

void MainWindow::openFile() {
    QStringList filenames = QFileDialog::getOpenFileNames(this, "Open Mesh(s)", "", "Mesh Files (*.obj *.ply *.stl *.off);;All Files (*)");
    for (const QString& filename : filenames) {
        if (!filename.isEmpty()) {
            loadMesh(filename);
        }
    }
}

void MainWindow::loadMesh(const QString& filename) {
    auto pending = std::make_shared<PendingMesh>();
    pending->filename = filename;
    loadQueue.append(pending);

    QFileInfo fi(filename);
    
    // Check if we already loaded this in the current session to avoid redundant background work
    if (loadedMeshesCache.contains(filename)) {
        pending->mesh = loadedMeshesCache[filename];
        pending->success = true;
        pending->ready = true;
        statusBar()->showMessage("Instant load (cached): " + fi.fileName());
        processReadyMeshes();
        return;
    }

    statusBar()->showMessage("Queued: " + fi.fileName());

    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, pending, filename]() {
        pending->success = watcher->result();
        pending->ready = true;
        if (pending->success) {
            loadedMeshesCache[filename] = pending->mesh;
        }
        watcher->deleteLater();
        processReadyMeshes();
    });

    QFuture<bool> future = QtConcurrent::run([pending, filename]() {
        if (pending->mesh.load(filename.toStdString())) {
            return true;
        }
        return false;
    });
    watcher->setFuture(future);
}

void MainWindow::processReadyMeshes() {
    while (!loadQueue.isEmpty() && loadQueue.first()->ready) {
        auto pending = loadQueue.takeFirst();
        if (pending->success) {
            mesh = pending->mesh; // Update member for benchmarking
            lastLoadedFilename = pending->filename;
            viewer->addMesh(pending->mesh, QFileInfo(pending->filename).fileName());
            statusBar()->showMessage("Loaded: " + QFileInfo(pending->filename).fileName());
        } else {
            statusBar()->showMessage("Failed to load: " + QFileInfo(pending->filename).fileName());
        }
    }
}

void MainWindow::toggleNormalizeScale(bool checked) { 
    viewer->normalizeScale = checked; 
    viewer->updateLayout(); 
}

void MainWindow::toggleWireframe(bool checked) { viewer->showWireframe = checked; viewer->update(); }

void MainWindow::setEdgeColor() {
    QColor color = QColorDialog::getColor(QColor::fromRgbF(viewer->edgeColor.x(), viewer->edgeColor.y(), viewer->edgeColor.z()), this, "Select Edge Color");
    if (color.isValid()) {
        viewer->edgeColor = QVector3D(color.redF(), color.greenF(), color.blueF());
        viewer->update();
    }
}

void MainWindow::setEdgeThickness(double value) {
    viewer->edgeThickness = (float)value;
    viewer->update();
}

void MainWindow::toggleAntialiasing(bool checked) {
    viewer->antialiasing = checked;
    viewer->update();
}

void MainWindow::toggleFlatShading(bool checked) { viewer->useFlatShading = checked; viewer->update(); }
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
    if (lastLoadedFilename.isEmpty()) {
        statusBar()->showMessage("No mesh loaded to clear cache for.");
        return;
    }

    QString cachePath = QString::fromStdString(Mesh::getCachePath(lastLoadedFilename.toStdString()));
    if (QFile::exists(cachePath)) {
        if (QFile::remove(cachePath)) {
            statusBar()->showMessage("Cache cleared for: " + QFileInfo(lastLoadedFilename).fileName());
        } else {
            statusBar()->showMessage("Failed to remove cache file.");
        }
    } else {
        statusBar()->showMessage("No cache file found for this mesh.");
    }
}
