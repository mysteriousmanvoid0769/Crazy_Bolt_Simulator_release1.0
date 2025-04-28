#include "mainwindow.h" // Оставляем, т.к. StartWindow его использует
#include "startwindow.h" // Добавляем заголовок нашего нового окна
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // --- ИЗМЕНЕНИЕ: Создаем и показываем StartWindow вместо MainWindow ---
    StartWindow startScreen;
    // w.resize(1200, 900); // Убрали, размер задается в StartWindow или MainWindow
    startScreen.show();
    // --- КОНЕЦ ИЗМЕНЕНИЯ ---

    return a.exec();
}
