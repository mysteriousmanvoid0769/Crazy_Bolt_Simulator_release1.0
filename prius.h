#ifndef PRIUS_H
#define PRIUS_H

#include <QGraphicsPixmapItem>
#include <QObject>


enum class Lane { Left = 0, Middle = 1, Right = 2 };

class Prius : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    Prius(QGraphicsItem *parent = nullptr);
    void accelerate();
    void brake();
    void moveLeft();
    void moveRight();
    int getSpeed() const;
    Lane getCurrentLane() const;


    void activateTurbo();
    void deactivateTurbo();
    bool isTurboActive() const;
    int getMaxSpeed() const;

private:
    int speed;
    Lane currentLane;


    int maxSpeed;
    bool turboActive;


    void setLanePosition(Lane lane);

    qreal getCenterXForLane(Lane lane);


    static constexpr qreal LANE_WIDTH = 100;
    static constexpr qreal ROAD_LEFT_EDGE = 50;


    static constexpr int DEFAULT_MAX_SPEED = 120;
    static constexpr int TURBO_MAX_SPEED = 180;
};

#endif // PRIUS_H
