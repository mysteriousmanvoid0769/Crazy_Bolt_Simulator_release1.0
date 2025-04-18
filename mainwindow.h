#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QList>
#include "prius.h"
#include "obstaclecar.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateMovement();
    void spawnObstacleCar();

private:
    void setupUI();
    QGraphicsScene *scene;
    QGraphicsView *view;
    Prius *prius;
    QLabel *speedLabel;
    QList<QGraphicsLineItem*> dashes;
    QList<ObstacleCar*> obstacleCars;
    bool gameOver;
    int getPriusSpeed() const; // Добавляем метод для получения скорости Prius
};

#endif // MAINWINDOW_H
