#include "obstaclecar.h"
#include <QDebug>
#include <QBrush>

const int ObstacleCar::lanesY[3] = {140, 190, 240};

ObstacleCar::ObstacleCar(int lane) {
    qDebug() << "Creating ObstacleCar...";
    pixmap = QPixmap("C:/Users/iskender/Documents/Crazy_Prius_Simulator/pictures/ObstacleCar.jpg");
    if (pixmap.isNull()) {
        qDebug() << "Error: Failed to load ObstacleCar.jpg! Using rectangle instead.";
        setRect(0, 0, 50, 30);
        setBrush(QBrush(Qt::red));
    } else {
        setRect(0, 0, 50, 30);
        QBrush brush(pixmap.scaled(50, 30, Qt::KeepAspectRatio));
        setBrush(brush);
    }
    setPos(600, lanesY[lane]);
    qDebug() << "ObstacleCar created at lane:" << lane << ", y:" << lanesY[lane];
}

void ObstacleCar::move(int priusSpeed) {
    // Машины движутся навстречу Prius с базовой скоростью 20 + скорость Prius
    setX(x() - (20 + priusSpeed) * 0.016);
}
