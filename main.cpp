#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window; // Окно покажется автоматически из конструктора
    return app.exec();
}
