#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFutureWatcher>
#include <QMutexLocker>
#include <memory>
#include <QMap>
#include "Viewer.h"

struct PendingMesh {
    Mesh mesh;
    QString filename;
    bool ready = false;
    bool success = false;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    void loadMesh(const QString& filename);

private slots:
    void openFile();
    void toggleWireframe(bool checked);
    void setEdgeColor();
    void setEdgeThickness(double value);
    void toggleAntialiasing(bool checked);
    void toggleFlatShading(bool checked);
    void toggleNormals(bool checked);
    void toggleVertexLabels(bool checked);
    void toggleFaceLabels(bool checked);
    void toggleBB(bool checked);
    void toggleNormalizeScale(bool checked);
    void processReadyMeshes();
    void clearCache();
    void toggleShaderMode(bool checked);
    void updateBenchmark();

private:
    Viewer* viewer;
    Mesh mesh;
    QMutex meshMutex;
    QList<std::shared_ptr<PendingMesh>> loadQueue;
    QMap<QString, Mesh> loadedMeshesCache; // Cache for the current session
    QTimer* benchmarkTimer;
    QString lastLoadedFilename;
};

#endif
