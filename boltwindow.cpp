#include "boltwindow.h"
#include <QVBoxLayout>
#include <QDebug>

BoltWindow::BoltWindow(OrderManager *manager, QWidget *parent) : QDialog(parent), orderManager(manager) {
    qDebug() << "Creating BoltWindow...";
    setWindowTitle("Bolt Order");
    QVBoxLayout *layout = new QVBoxLayout(this);
    orderLabel = new QLabel("No orders");
    acceptButton = new QPushButton("Accept");
    rejectButton = new QPushButton("Reject");
    layout->addWidget(orderLabel);
    layout->addWidget(acceptButton);
    layout->addWidget(rejectButton);

    connect(acceptButton, &QPushButton::clicked, orderManager, &OrderManager::acceptOrder);
    connect(rejectButton, &QPushButton::clicked, orderManager, &OrderManager::rejectOrder);
    connect(acceptButton, &QPushButton::clicked, this, &QDialog::hide);
    connect(rejectButton, &QPushButton::clicked, this, &QDialog::hide);
}

void BoltWindow::showOrder(const Order &order) {
    qDebug() << "Showing order:" << order.distance << "km," << order.price << "AZN";
    orderLabel->setText(QString("Order: %1 km, %2 AZN, %3").arg(order.distance).arg(order.price).arg(order.payment));
    show();
}
