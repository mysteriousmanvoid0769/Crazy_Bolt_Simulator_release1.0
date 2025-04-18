#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QObject>
#include <QTimer>
#include "prius.h"
#include "ordermanager.h"

class GameManager : public QObject {
    Q_OBJECT
public:
    GameManager(QObject *parent = nullptr);
    void startGame();
    OrderManager* getOrderManager() const { return orderManager; }

signals:
    void updateDistance(int distance, int total);
    void updateSpeed(int speed);
    void showOrder(const Order &order);

private slots:
    void updateGame();

private:
    Prius *prius;
    OrderManager *orderManager;
    QTimer *gameTimer;
    double currentDistance; 
    double totalDistance; 
    bool orderPending;
};

#endif // GAMEMANAGER_H
