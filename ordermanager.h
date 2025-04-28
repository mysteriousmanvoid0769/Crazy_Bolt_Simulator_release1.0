#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <QObject>
#include <QString>

struct Order {
    double distance = 0;
    double price = 0;
    QString payment = "";
};

class OrderManager : public QObject {
    Q_OBJECT
public:
    explicit OrderManager(QObject *parent = nullptr);
    void generateOrder();
    void rejectOrder();

    void confirmFirstOrderAccepted();
    bool getIsFirstOrder() const;

signals:


    void orderGenerated(const Order &order);


private:
    Order generatedOrder;
    bool isFirstOrder;
};

#endif // ORDERMANAGER_H
