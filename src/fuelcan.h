#ifndef FUELCAN_H
#define FUELCAN_H

#include <QGraphicsPixmapItem>

class FuelCan : public QGraphicsPixmapItem {
public:
    FuelCan(int lane);
    void move(int speed);
    int lane() const { return m_lane; }

private:
    int m_lane;
};

#endif // FUELCAN_H
