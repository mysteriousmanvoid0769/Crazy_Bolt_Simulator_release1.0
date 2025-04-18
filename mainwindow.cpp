#include "mainwindow.h"
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QTimer>
#include <QDebug>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), gameOver(false) {
    qDebug() << "Initializing MainWindow...";
    setupUI();
    qDebug() << "UI complete...";

    qDebug() << "Creating Prius...";
    prius = new Prius();
    qDebug() << "Prius created...";
    if (scene && prius) {
        scene->addItem(prius);
        prius->setPos(100, 190);
        scene->update();
        qDebug() << "Prius added to scene...";
    } else {
        qDebug() << "Error: scene or prius is null!";
    }

    // Добавляем прерывистую разметку
    for (int x = -2400; x < 4800; x += 20) {
        QGraphicsLineItem *dashTop = new QGraphicsLineItem(x, 175, x + 10, 175);
        dashTop->setPen(QPen(Qt::white, 5));
        scene->addItem(dashTop);
        dashes.append(dashTop);

        QGraphicsLineItem *dashBottom = new QGraphicsLineItem(x, 225, x + 10, 225);
        dashBottom->setPen(QPen(Qt::white, 5));
        scene->addItem(dashBottom);
        dashes.append(dashBottom);
    }
    qDebug() << "Dashes added to scene...";

    // Таймер для движения
    QTimer *movementTimer = new QTimer(this);
    connect(movementTimer, &QTimer::timeout, this, &MainWindow::updateMovement);
    movementTimer->start(16); // 60 FPS
    if (!movementTimer->isActive()) {
        qDebug() << "Error: Movement timer not active!";
    }
    qDebug() << "Movement timer started...";

    // Таймер для генерации машин-препятствий
    QTimer *spawnTimer = new QTimer(this);
    connect(spawnTimer, &QTimer::timeout, this, &MainWindow::spawnObstacleCar);
    spawnTimer->start(2000);
    qDebug() << "Spawn timer started...";

    setFocus();
    qDebug() << "Focus set on MainWindow at startup";

    qDebug() << "UI fully initialized...";

    QMainWindow::show();
    qDebug() << "MainWindow shown...";
}

MainWindow::~MainWindow() {
    qDebug() << "Destroying MainWindow...";
}

void MainWindow::setupUI() {
    qDebug() << "Setting up UI...";
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    scene = new QGraphicsScene(-2400, 0, 4800, 400);
    view = new QGraphicsView(scene);
    view->setMinimumSize(600, 400);
    view->setSceneRect(0, 0, 600, 400);
    view->setFocusPolicy(Qt::NoFocus);
    scene->setBackgroundBrush(Qt::blue);
    QGraphicsLineItem *road = new QGraphicsLineItem(-2400, 200, 4800, 200);
    road->setPen(QPen(Qt::gray, 150));
    scene->addItem(road);
    qDebug() << "Scene created...";

    speedLabel = new QLabel("Speed: 0 km/h");
    speedLabel->setMinimumHeight(30);
    layout->addWidget(view);
    layout->addWidget(speedLabel);
    qDebug() << "Speed label added to layout...";

    setCentralWidget(centralWidget);
    resize(620, 480);
    qDebug() << "Central widget set...";
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (gameOver) return;

    if (!prius) {
        qDebug() << "Error: scene or prius is null in keyPressEvent!";
        return;
    }
    setFocus();
    qDebug() << "Focus set on MainWindow";
    qDebug() << "Key pressed:" << event->key();
    switch (event->key()) {
    case Qt::Key_Right:
        prius->accelerate();
        speedLabel->setText(QString("Speed: %1 km/h").arg(prius->getSpeed()));
        qDebug() << "Speed updated to:" << prius->getSpeed();
        break;
    case Qt::Key_Left:
        prius->brake();
        speedLabel->setText(QString("Speed: %1 km/h").arg(prius->getSpeed()));
        qDebug() << "Speed updated to:" << prius->getSpeed();
        break;
    case Qt::Key_Up:
        prius->moveUp();
        break;
    case Qt::Key_Down:
        prius->moveDown();
        break;
    }
    scene->update();
    view->update();
}

int MainWindow::getPriusSpeed() const {
    return prius ? prius->getSpeed() : 0;
}

void MainWindow::updateMovement() {
    if (gameOver) return;

    qDebug() << "Updating movement...";
    if (!prius) return;

    int speed = prius->getSpeed();
    qDebug() << "Current speed:" << speed;
    if (speed > 0) {
        for (QGraphicsLineItem *dash : dashes) {
            dash->setX(dash->x() - speed * 0.06);
            if (dash->x() < -2400) dash->setX(dash->x() + 4000);
        }
    }

    for (ObstacleCar *car : obstacleCars) {
        car->move(speed); // Передаём скорость Prius
        if (car->x() < -2400) {
            scene->removeItem(car);
            obstacleCars.removeOne(car);
            delete car;
            qDebug() << "ObstacleCar removed from scene...";
        }
    }

    for (ObstacleCar *car : obstacleCars) {
        if (prius->collidesWithItem(car)) {
            qDebug() << "Game Over: Collision detected!";
            gameOver = true;
            speedLabel->setText("Game Over!");
            return;
        }
    }

    scene->update();
    view->update();
}

void MainWindow::spawnObstacleCar() {
    if (gameOver) return;

    int lane = QRandomGenerator::global()->bounded(3);
    ObstacleCar *car = new ObstacleCar(lane);
    scene->addItem(car);
    obstacleCars.append(car);
    qDebug() << "Spawned ObstacleCar at lane:" << lane;
}
