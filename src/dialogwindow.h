#ifndef DIALOGWINDOW_H
#define DIALOGWINDOW_H

#include <QDialog>
#include <QLabel>

class DialogWindow : public QDialog {
    Q_OBJECT
public:
    DialogWindow(QWidget *parent = nullptr);
};

#endif // DIALOGWINDOW_H
