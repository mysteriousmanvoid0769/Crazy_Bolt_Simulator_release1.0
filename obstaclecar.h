#ifndef OBSTACLECAR_H
#define OBSTACLECAR_H

#include <QGraphicsRectItem>
#include <QPixmap>

class ObstacleCar : public QGraphicsRectItem {
public:
    ObstacleCar(int lane);
    void move(int priusSpeed); // Теперь принимает скорость Prius
    static const int lanesY[3];

private:
    QPixmap pixmap;
};

#endif // OBSTACLECAR_H
