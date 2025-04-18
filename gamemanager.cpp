#include "gamemanager.h"
#include <QMessageBox>
#include <QDebug>

GameManager::GameManager(QObject *parent) : QObject(parent), currentDistance(0), totalDistance(0), orderPending(false) {
    qDebug() << "Creating GameManager...";
    prius = new Prius();
    orderManager = new OrderManager(this);
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &GameManager::updateGame);
    qDebug() << "GameManager initialized...";
}

void GameManager::startGame() {
    qDebug() << "Starting game...";
    Order order = orderManager->generateOrder();
    orderManager->acceptOrder();
    totalDistance = order.distance;
    emit showOrder(order);
    gameTimer->start(16); // 60 FPS, moved here
}

void GameManager::updateGame() {
    if (orderManager->hasOrder()) {
        double speedKmH = prius->getSpeed();
        currentDistance += (speedKmH * 16) / (3600 * 1000); // km per 16ms
        emit updateDistance(currentDistance * 1000, totalDistance * 1000); // meters
        emit updateSpeed(speedKmH);
        qDebug() << "Game update: distance" << currentDistance << "km, speed" << speedKmH << "km/h";

        if (totalDistance - currentDistance <= 0.5 && !orderPending) {
            Order newOrder = orderManager->generateOrder();
            emit showOrder(newOrder);
            orderPending = true;
            qDebug() << "New order emitted";
        }

        if (currentDistance >= totalDistance) {
            QMessageBox::information(nullptr, "Trip Complete", "Client dropped off!");
            currentDistance = 0;
            totalDistance = 0;
            orderManager->rejectOrder();
            orderPending = false;
            qDebug() << "Trip completed";
        }
    }
}
