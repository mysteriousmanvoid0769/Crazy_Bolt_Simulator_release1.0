#ifndef PRIUS_H
#define PRIUS_H

#include <QGraphicsRectItem>
#include <QPixmap>

class Prius : public QGraphicsRectItem {
public:
    Prius();
    void accelerate();
    void brake();
    void moveUp();
    void moveDown();
    int getSpeed() const;

private:
    int speed;
    int lane;
    static const int lanesY[3];
    QPixmap pixmap;
};

#endif // PRIUS_H
