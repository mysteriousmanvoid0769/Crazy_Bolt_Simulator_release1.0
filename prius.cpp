#include "prius.h"
#include <QPixmap>
#include <QDebug>
#include <QBrush>

const int Prius::lanesY[3] = {140, 190, 240}; // Обновили позиции

Prius::Prius() : speed(0), lane(1) {
    qDebug() << "Creating Prius...";
    pixmap = QPixmap("C:/Users/iskender/Documents/Crazy_Prius_Simulator/pictures/Prius.jpg");
    if (pixmap.isNull()) {
        qDebug() << "Error: Failed to load Prius.jpg! Using rectangle instead.";
        setRect(0, 0, 50, 30);
        setBrush(QBrush(Qt::green));
    } else {
        setRect(0, 0, 50, 30);
        QBrush brush(pixmap.scaled(50, 30, Qt::KeepAspectRatio));
        setBrush(brush);
    }
    qreal initialY = lanesY[lane];
    if (initialY < 140) initialY = 140;
    if (initialY > 240) initialY = 240;
    setPos(100, initialY);
    qDebug() << "Prius initial position, y:" << initialY;
}

void Prius::accelerate() {
    if (speed < 160) {
        speed += 5;
        qDebug() << "Accelerate called, speed:" << speed;
    }
}

void Prius::brake() {
    if (speed > 0) {
        speed -= 5;
        qDebug() << "Brake called, speed:" << speed;
    }
}

void Prius::moveUp() {
    if (lane > 0) {
        lane--;
        qreal newY = lanesY[lane];
        if (newY < 140) newY = 140;
        setY(newY);
        qDebug() << "Move up, lane:" << lane << ", y:" << newY;
    }
}

void Prius::moveDown() {
    if (lane < 2) {
        lane++;
        qreal newY = lanesY[lane];
        if (newY > 240) newY = 240;
        setY(newY);
        qDebug() << "Move down, lane:" << lane << ", y:" << newY;
    }
}

int Prius::getSpeed() const {
    return speed;
}
