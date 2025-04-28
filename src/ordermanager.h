#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <QObject>
#include <QString>

struct Order {
    double distance = 0; // Инициализация по умолчанию
    double price = 0;
    QString payment = "";
};

class OrderManager : public QObject {
    Q_OBJECT
public:
    explicit OrderManager(QObject *parent = nullptr);
    void generateOrder(); // Генерирует и испускает orderGenerated
    void rejectOrder(); // Вызывается из GameManager при отклонении
    // void completeOrder(); // Убрали, чаевые и сигнал в GameManager
    void confirmFirstOrderAccepted(); // Вызывается GameManager'ом после принятия первого заказа
    bool getIsFirstOrder() const; // Getter для проверки первого заказа

signals:
    // void orderCompleted(double payment, double tip); // Убран
    // void orderFailed(); // Убран (используется rejectOrder)
    void orderGenerated(const Order &order);
    // void orderAccepted(const Order &order); // Убран

private:
    Order generatedOrder; // Храним последний сгенерированный заказ
    bool isFirstOrder;
};

#endif // ORDERMANAGER_H
