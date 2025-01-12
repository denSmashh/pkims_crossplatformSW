#include "GraphLoader.hpp"
#include "GraphWidget.hpp"

#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    if (argc > 1) {
        window.loadFile(argv[1]);
    }
    window.show();
    return app.exec();
}
