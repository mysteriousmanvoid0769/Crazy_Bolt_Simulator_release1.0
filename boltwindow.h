#ifndef BOLTWINDOW_H
#define BOLTWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include "ordermanager.h"

class BoltWindow : public QDialog {
    Q_OBJECT
public:
    BoltWindow(OrderManager *manager, QWidget *parent = nullptr);

public slots:
    void showOrder(const Order &order);

private:
    OrderManager *orderManager;
    QLabel *orderLabel;
    QPushButton *acceptButton;
    QPushButton *rejectButton;
};

#endif // BOLTWINDOW_H
