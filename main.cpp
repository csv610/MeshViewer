#include <QApplication>
#include "MainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    MainWindow win;
    win.show();
    
    if (argc > 1) {
        // We need a way to trigger file loading in MainWindow from here
        // or just expose the load logic.
        // For now, let's just use a timer to call it after the event loop starts
        // or call a public method.
        win.loadMesh(argv[1]);
    }

    return app.exec();
}
