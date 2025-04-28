#ifndef COMMENTWINDOW_H
#define COMMENTWINDOW_H

#include <QDialog>
#include <QLabel>

class CommentWindow : public QDialog {
    Q_OBJECT
public:
    CommentWindow(QWidget *parent = nullptr);
};

#endif // COMMENTWINDOW_H
