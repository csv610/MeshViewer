#include <QApplication>
#include "MainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    MainWindow win;
    win.show();
    
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            win.loadMesh(argv[i]);
        }
    }

    return app.exec();
}
