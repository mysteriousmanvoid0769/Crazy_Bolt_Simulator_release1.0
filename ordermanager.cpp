#include "ordermanager.h"
#include <QRandomGenerator>
#include <QDebug>
#include <cmath>

OrderManager::OrderManager(QObject *parent) : QObject(parent), isFirstOrder(true) {
    qDebug() << "OrderManager initialized...";
}

void OrderManager::generateOrder() {
    if (isFirstOrder) {

        generatedOrder.distance = 600;
        generatedOrder.price = 6.0;
        generatedOrder.payment = "Card";
        qDebug() << "Generating first order (600m, 3.0 AZN, Card)";
    } else {


        generatedOrder.distance = QRandomGenerator::global()->bounded(1, 6) * 500;

        generatedOrder.price = generatedOrder.distance / 1000.0 * 1.5;

        generatedOrder.price = round(generatedOrder.price * 10) / 10.0;

        generatedOrder.payment = QRandomGenerator::global()->bounded(2) == 0 ? "Cash" : "Card";
        qDebug() << "Generating random order.";
    }

    emit orderGenerated(generatedOrder);
    qDebug() << "Order generated and emitted:" << generatedOrder.distance << "m," << generatedOrder.price << "AZN," << generatedOrder.payment;
}


void OrderManager::rejectOrder() {
    qDebug() << "OrderManager notified of rejection. Next generateOrder will be random (if not first).";


}




void OrderManager::confirmFirstOrderAccepted() {
    if (isFirstOrder) {
        qDebug() << "OrderManager: First order confirmed as accepted.";
        isFirstOrder = false;
    }
}


bool OrderManager::getIsFirstOrder() const {
    return isFirstOrder;
}
