#include "prius.h"
#include <QDebug>
#include <QPixmap>


// Убедимся, что инициализация в списке верная
Prius::Prius(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent),
    speed(0),
    currentLane(Lane::Middle),
    maxSpeed(DEFAULT_MAX_SPEED),
    turboActive(false)
{

    QPixmap pixmap("C:/Users/iskender/Documents/Crazy_Prius_Simulator/pictures/prius2.jpg");
    if (pixmap.isNull()) {
        qDebug() << "Error: Failed to load prius.jpg";
        pixmap = QPixmap(30, 50);
        pixmap.fill(Qt::red);
    }
    setPixmap(pixmap.scaled(30, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    setLanePosition(currentLane);
    qDebug() << "Prius initialized in lane:" << static_cast<int>(currentLane) << "at X:" << x() << "Max Speed:" << maxSpeed;
}

void Prius::accelerate() {
    if (speed < maxSpeed) speed += 5;
    if (speed > maxSpeed) speed = maxSpeed;
}

void Prius::brake() {
    if (speed > 0) speed -= 5;
    if (speed < 0) speed = 0;
}

void Prius::moveLeft() {
    if (currentLane == Lane::Right) {
        setLanePosition(Lane::Middle);
    } else if (currentLane == Lane::Middle) {
        setLanePosition(Lane::Left);
    }
}

void Prius::moveRight() {
    if (currentLane == Lane::Left) {
        setLanePosition(Lane::Middle);
    } else if (currentLane == Lane::Middle) {
        setLanePosition(Lane::Right);
    }
}

int Prius::getSpeed() const {
    return speed;
}

Lane Prius::getCurrentLane() const {
    return currentLane;
}


void Prius::activateTurbo() {
    if (!turboActive) {
        maxSpeed = TURBO_MAX_SPEED;
        turboActive = true;
        qDebug() << "Prius: Turbo Activated! Max speed:" << maxSpeed;
    }
}

void Prius::deactivateTurbo() {
    if (turboActive) {
        maxSpeed = DEFAULT_MAX_SPEED;
        turboActive = false;
        if (speed > maxSpeed) {
            speed = maxSpeed;
        }
        qDebug() << "Prius: Turbo Deactivated. Max speed:" << maxSpeed << "Current speed:" << speed;
    } else {


        if (maxSpeed != DEFAULT_MAX_SPEED) {
            qDebug() << "Prius: Warning - deactivateTurbo called but maxSpeed was not DEFAULT. Resetting.";
            maxSpeed = DEFAULT_MAX_SPEED;
        }
        if (speed > maxSpeed) {
            speed = maxSpeed;
        }
    }
}

bool Prius::isTurboActive() const {
    return turboActive;
}

int Prius::getMaxSpeed() const {
    return maxSpeed;
}



void Prius::setLanePosition(Lane lane) {
    currentLane = lane;
    qreal targetXCenter = getCenterXForLane(lane);
    setX(targetXCenter - boundingRect().width() / 2.0);
}

qreal Prius::getCenterXForLane(Lane lane) {
    return ROAD_LEFT_EDGE + LANE_WIDTH / 2.0 + static_cast<int>(lane) * LANE_WIDTH;
}
