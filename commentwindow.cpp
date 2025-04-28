#include "commentwindow.h"
#include <QVBoxLayout>
#include <QDebug>

CommentWindow::CommentWindow(QWidget *parent) : QDialog(parent) {
    qDebug() << "Creating CommentWindow...";
    setWindowTitle("Passenger Comments");
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Trip started");
    layout->addWidget(label);
    setFixedSize(200, 100);
}
