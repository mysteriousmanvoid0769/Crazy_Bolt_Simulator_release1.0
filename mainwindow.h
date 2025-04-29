#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

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
#include <QStyle>


class Prius;
class GameManager;
class FuelCan;
class GameView;
struct Order;
class QKeyEvent;
class QShowEvent;
class QResizeEvent;


class ObstacleCar : public QGraphicsPixmapItem {
public:
    ObstacleCar(int lane) : carLane(lane) {

        QPixmap pixmap("../../pictures/obstacle_car.png");
        if (pixmap.isNull()) {
            qDebug() << "Error: Failed to load obstacle_car.jpg";
            pixmap = QPixmap(30, 50); pixmap.fill(Qt::blue);
        }
        setPixmap(pixmap.scaled(30, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        qreal laneCenterX;
        if (lane == 0) laneCenterX = 100;
        else if (lane == 1) laneCenterX = 200;
        else laneCenterX = 300;

        setPos(laneCenterX - boundingRect().width()/2.0, -100);
    }
    void move(int playerSpeed) {

        double obstacle_base_speed = 5.0;
        double relative_speed_factor = 0.04;
        setY(y() + obstacle_base_speed + playerSpeed * relative_speed_factor);
    }
    int lane() const { return carLane; }
private:
    int carLane;
};



#include "prius.h"
#include "gamemanager.h"
#include "fuelcan.h"
#include "gameview.h"


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


    void updateDistanceLabel(double current, int total);
    void updateRatingLabel(double rating);
    void updateCompletedOrdersLabel(int count);
    void updateTotalDistanceLabel(double totalDistance);
    void updateFuelBar(double fuelPercentage);
    void updateWalletLabel(double newWallet);

    void updateSpeedBar(int speed, bool turboActive);


    void showOrderOverlay(const Order &order);
    void acceptOrderClicked();
    void rejectOrderClicked();


    void onStartDialogClicked();
    void onTellStoryClicked();
    void onComplainClicked();
    void updateDialogButtons(bool enableStart, bool enableStory, bool enableComplain);
    void appendDialogText(const QString& htmlText);


    void onTurboUpgradeClicked();
    void updateTurboButton(bool available);


private:

    void setupUI();
    void resetScene();
    void setupOrderOverlay();
    void positionManuallyPlacedWidgets();
    void setupDialogButtons();
    void setupUpgradeButton();
    void setupSpeedPanel();


    QGraphicsScene *scene;
    GameView *view;
    Prius *prius;
    QList<QGraphicsRectItem*> dashes;
    QList<ObstacleCar*> obstacleCars;
    QList<FuelCan*> fuelCans;


    GameManager *gameManager;


    QWidget* centralWidget;
    QPushButton *restartButton;
    QTextEdit* commentDisplayArea;


    QWidget *bottomLeftStatsWidget;
    QLabel *walletLabel;
    QLabel *completedOrdersLabel;
    QLabel *distanceLabel;
    QLabel *totalDistanceLabel;


    QWidget *leftStatusPanelWidget;
    QProgressBar *speedBar;
    QProgressBar *turboBar;
    QProgressBar *fuelBar;


    QWidget *topRightStatusPanelWidget;
    QLabel *ratingIconLabel;
    QLabel *ratingLabel;


    QWidget *orderOverlayWidget;
    QLabel *orderOverlayLabel;
    QPushButton *acceptButton;
    QPushButton *rejectButton;


    QWidget* dialogButtonContainer;
    QPushButton* startDialogButton;
    QPushButton* tellStoryButton;
    QPushButton* complainButton;


    QPushButton* turboUpgradeButton;


    bool gameOver;
    bool gamePaused;
    bool obstaclesActive;
    double lastFuelCanSpawnDistance;
    Order currentDisplayedOrder;


    QTimer *movementTimer;
    QTimer *spawnTimer;
    QTimer *fuelSpawnTimer;
};

#endif // MAINWINDOW_H
