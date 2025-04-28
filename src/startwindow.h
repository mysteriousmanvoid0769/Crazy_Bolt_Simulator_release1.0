#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QWidget>

// Forward declarations
class QLabel;
class QPushButton;
class MainWindow; // Нужно для создания главного окна

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
    void setupUi(); // Приватный метод для настройки интерфейса

    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QPushButton *startButton;
    QPushButton *aboutButton;
    QLabel *authorsLabel;

    MainWindow *mainWindow = nullptr; // Указатель на главное окно игры
};

#endif // STARTWINDOW_H
