#include "fuelcan.h"
#include <QPixmap>
#include <QDebug>

FuelCan::FuelCan(int lane) : m_lane(lane) {
    QPixmap fuelPixmap("../../pictures/fuel_can.jpg");
    if (fuelPixmap.isNull()) {
        qDebug() << "Error: Failed to load fuel_can.jpg - using default size for collision detection";

        setPixmap(QPixmap(30, 30));
    } else {
        setPixmap(fuelPixmap.scaled(30, 30));
    }
    int xPos;
    if (lane == 0) xPos = 110;
    else if (lane == 1) xPos = 190;
    else xPos = 270;
    setPos(xPos, -50);
}

void FuelCan::move(int speed) {
    setY(y() + speed * 0.06);
}
