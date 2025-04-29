#include "mainwindow.h"
#include "startwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);


    StartWindow startScreen;

    startScreen.show();

    return a.exec();
}
