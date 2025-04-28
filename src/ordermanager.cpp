#include "ordermanager.h"
#include <QRandomGenerator>
#include <QDebug>
#include <cmath> // Для round

OrderManager::OrderManager(QObject *parent) : QObject(parent), isFirstOrder(true) {
    qDebug() << "OrderManager initialized...";
}

void OrderManager::generateOrder() {
    if (isFirstOrder) {
        // Генерируем стандартный первый заказ
        generatedOrder.distance = 600;
        generatedOrder.price = 6.0; // <-- ИСПРАВЛЕНО: Цена первого заказа 3.0 AZN
        generatedOrder.payment = "Card";
        qDebug() << "Generating first order (600m, 3.0 AZN, Card)";
    } else {
        // Генерируем случайный заказ
        // Дистанция от 500м до 2500м (шаг 500м) -> [1..5]*500
        generatedOrder.distance = QRandomGenerator::global()->bounded(1, 6) * 500;
        // Цена: ~1.5 AZN за километр
        generatedOrder.price = generatedOrder.distance / 1000.0 * 1.5;
        // Округляем до 1 знака после запятой (1.5, 2.3, 3.8 и т.д.)
        generatedOrder.price = round(generatedOrder.price * 10) / 10.0;
        // Случайный способ оплаты
        generatedOrder.payment = QRandomGenerator::global()->bounded(2) == 0 ? "Cash" : "Card";
        qDebug() << "Generating random order.";
    }
    // Отправляем сигнал с сгенерированным заказом
    emit orderGenerated(generatedOrder);
    qDebug() << "Order generated and emitted:" << generatedOrder.distance << "m," << generatedOrder.price << "AZN," << generatedOrder.payment;
}

// Вызывается из GameManager при отклонении заказа
void OrderManager::rejectOrder() {
    qDebug() << "OrderManager notified of rejection. Next generateOrder will be random (if not first).";
    // Не генерируем новый заказ здесь, GameManager сделает это сам после штрафа
    // generateOrder();
}

// Метод completeOrder больше не используется OrderManager'ом
// void OrderManager::completeOrder() { ... }

// Вызывается GameManager'ом после принятия ПЕРВОГО заказа
void OrderManager::confirmFirstOrderAccepted() {
    if (isFirstOrder) {
        qDebug() << "OrderManager: First order confirmed as accepted.";
        isFirstOrder = false; // Следующий generateOrder будет случайным
    }
}

// Getter для isFirstOrder
bool OrderManager::getIsFirstOrder() const {
    return isFirstOrder;
}
