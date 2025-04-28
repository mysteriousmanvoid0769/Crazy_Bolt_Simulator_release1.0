#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QWidget>


class QLabel;
class QPushButton;
class MainWindow;

class StartWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StartWindow(QWidget *parent = nullptr);
    ~StartWindow();

private slots:
    void onStartClicked();
    void onAboutClicked();

private:
    void setupUi();

    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QPushButton *startButton;
    QPushButton *aboutButton;
    QLabel *authorsLabel;

    MainWindow *mainWindow = nullptr;
};

#endif // STARTWINDOW_H
