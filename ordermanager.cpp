#include "ordermanager.h"
#include <QRandomGenerator>
#include <QDebug>

OrderManager::OrderManager(QObject *parent) : QObject(parent), orderActive(false) {
    qDebug() << "Creating OrderManager...";
}

Order OrderManager::generateOrder() {
    qDebug() << "Generating order...";
    Order order;
    order.distance = QRandomGenerator::global()->bounded(10, 100) / 10.0; // 1.0-10.0 km
    order.price = QRandomGenerator::global()->bounded(30, 200) / 10.0; // 3.0-20.0 AZN
    order.payment = QRandomGenerator::global()->bounded(2) ? "cash" : "card";
    currentOrder = order;
    return order;
}

Order OrderManager::getCurrentOrder() const {
    return currentOrder;
}

void OrderManager::acceptOrder() {
    orderActive = true;
    qDebug() << "Order accepted";
}

void OrderManager::rejectOrder() {
    orderActive = false;
    qDebug() << "Order rejected";
}

bool OrderManager::hasOrder() const {
    return orderActive;
}
