// mainwindow.h

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
// #include <QGraphicsView> // Заменено на gameview.h
#include <QGraphicsRectItem>
#include <QLabel>
#include <QPushButton>
#include <QList>
#include <QDebug>
#include <QProgressBar>
#include <QTimer>
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle> // Включен, может быть полезен для стилей

// Forward declarations
class Prius;
class GameManager;
class FuelCan;
class GameView; // <-- Добавлено Forward declaration для GameView
struct Order;
class QKeyEvent;
class QShowEvent;
class QResizeEvent;

// --- Определение ObstacleCar ---
// (Код ObstacleCar остается без изменений)
class ObstacleCar : public QGraphicsPixmapItem {
public:
    ObstacleCar(int lane) : carLane(lane) {
        // ЗАМЕНИТЕ ПУТЬ НА ВАШ!
        QPixmap pixmap("C:/Users/iskender/Documents/Crazy_Prius_Simulator_new/pictures/obstacle_car.png");
        if (pixmap.isNull()) {
            qDebug() << "Error: Failed to load obstacle_car.jpg";
            pixmap = QPixmap(30, 50); pixmap.fill(Qt::blue);
        }
        setPixmap(pixmap.scaled(30, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        qreal laneCenterX;
        if (lane == 0) laneCenterX = 100; // Центр левой полосы
        else if (lane == 1) laneCenterX = 200; // Центр средней полосы
        else laneCenterX = 300; // Центр правой полосы
        // Устанавливаем позицию так, чтобы центр машины был в центре полосы
        setPos(laneCenterX - boundingRect().width()/2.0, -100); // Спавн над экраном
    }
    void move(int playerSpeed) {
        // Скорость препятствия = базовая + % от скорости игрока
        double obstacle_base_speed = 5.0; // Базовая скорость "ползучести"
        double relative_speed_factor = 0.04; // Коэффициент относительной скорости
        setY(y() + obstacle_base_speed + playerSpeed * relative_speed_factor);
    }
    int lane() const { return carLane; }
private:
    int carLane;
};
// --- Конец ObstacleCar ---


#include "prius.h"
#include "gamemanager.h"
#include "fuelcan.h"
#include "gameview.h" // <-- Подключаем заголовок нового класса


class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool isGameOver() const { return gameOver; }

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void clearCommentArea();

private slots:
    // Основные слоты обновления и управления
    void updateMovement();
    void spawnObstacleCar();
    void restartGame();
    void pauseGame();
    void resumeGame();
    void startObstacles();
    void spawnFuelCan();
    void showGameOverMessage(QString message);
    void showNewPassengerMessage();
    void delayedStartGame();

    // Слоты для обновления UI
    void updateDistanceLabel(double current, int total);
    void updateRatingLabel(double rating); // Этот слот обновляет только текст
    void updateCompletedOrdersLabel(int count);
    void updateTotalDistanceLabel(double totalDistance);
    void updateFuelBar(double fuelPercentage);
    void updateWalletLabel(double newWallet);
    // Слот для обновления баров скорости и турбо
    void updateSpeedBar(int speed, bool turboActive); // <--- УБРАН maxSpeed

    // Слоты для оверлея заказа
    void showOrderOverlay(const Order &order);
    void acceptOrderClicked();
    void rejectOrderClicked();

    // --- Слоты для диалога ---
    void onStartDialogClicked();
    void onTellStoryClicked();
    void onComplainClicked();
    void updateDialogButtons(bool enableStart, bool enableStory, bool enableComplain);
    void appendDialogText(const QString& htmlText);

    // --- Слоты Turbo ---
    void onTurboUpgradeClicked();
    void updateTurboButton(bool available);


private:
    // Порядок объявления важен для -Wreorder
    void setupUI();
    void resetScene();
    void setupOrderOverlay();
    void positionManuallyPlacedWidgets();
    void setupDialogButtons();
    void setupUpgradeButton();
    void setupSpeedPanel();

    // Графические элементы
    QGraphicsScene *scene;
    GameView *view; // <-- Тип указателя GameView
    Prius *prius;
    QList<QGraphicsRectItem*> dashes;
    QList<ObstacleCar*> obstacleCars;
    QList<FuelCan*> fuelCans;

    // Менеджер игры
    GameManager *gameManager;

    // --- Элементы основного UI ---
    QWidget* centralWidget;
    QPushButton *restartButton;
    QTextEdit* commentDisplayArea;

    // Нижняя левая панель
    QWidget *bottomLeftStatsWidget;
    QLabel *walletLabel;
    QLabel *completedOrdersLabel;
    QLabel *distanceLabel;
    QLabel *totalDistanceLabel;

    // Левая панель
    QWidget *leftStatusPanelWidget;
    QProgressBar *speedBar;
    QProgressBar *turboBar;
    QProgressBar *fuelBar;

    // Правая верхняя панель
    QWidget *topRightStatusPanelWidget;
    QLabel *ratingIconLabel;
    QLabel *ratingLabel;

    // Оверлей заказа
    QWidget *orderOverlayWidget;
    QLabel *orderOverlayLabel;
    QPushButton *acceptButton;
    QPushButton *rejectButton;

    // --- Элементы UI диалога ---
    QWidget* dialogButtonContainer;
    QPushButton* startDialogButton;
    QPushButton* tellStoryButton;
    QPushButton* complainButton;

    // --- Элемент UI Turbo ---
    QPushButton* turboUpgradeButton;

    // --- Состояние игры ---
    bool gameOver;
    bool gamePaused;
    bool obstaclesActive;
    double lastFuelCanSpawnDistance;
    Order currentDisplayedOrder; // Хранит данные заказа, показанного в оверлее

    // --- Таймеры ---
    QTimer *movementTimer;
    QTimer *spawnTimer;
    QTimer *fuelSpawnTimer;
};

#endif // MAINWINDOW_H
