#include "dialogwindow.h"
#include <QVBoxLayout>
#include <QDebug>

DialogWindow::DialogWindow(QWidget *parent) : QDialog(parent) {
    qDebug() << "Creating DialogWindow...";
    setWindowTitle("Passenger Dialog");
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Dialogs coming soon");
    layout->addWidget(label);
    setFixedSize(200, 100);
}
