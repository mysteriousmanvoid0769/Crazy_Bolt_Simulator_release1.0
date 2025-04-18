#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <QObject>
#include <QString>

struct Order {
    double distance; // km
    double price; // AZN
    QString payment; // "cash" or "card"
};

class OrderManager : public QObject {
    Q_OBJECT
public:
    OrderManager(QObject *parent = nullptr);
    Order generateOrder();
    Order getCurrentOrder() const;
    void acceptOrder();
    void rejectOrder();
    bool hasOrder() const;

signals:
    void newOrder(const Order &order);

private:
    Order currentOrder;
    bool orderActive;
};

#endif // ORDERMANAGER_H
