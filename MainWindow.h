#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Viewer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    void loadMesh(const QString& filename);

private slots:
    void openFile();
    void toggleWireframe(bool checked);
    void toggleNormals(bool checked);
    void toggleVertexLabels(bool checked);
    void toggleFaceLabels(bool checked);
    void toggleBB(bool checked);

private:
    Viewer* viewer;
};

#endif
