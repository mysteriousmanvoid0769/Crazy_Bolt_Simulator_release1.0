#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QPainter>
#include <QDebug>


class GameView : public QGraphicsView {
    Q_OBJECT
public:

    explicit GameView(QWidget *parent = nullptr);
    explicit GameView(QGraphicsScene *scene, QWidget *parent = nullptr);
};

#endif // GAMEVIEW_H
