#include "gameview.h"
#include <QScrollBar>
#include <QGraphicsScene>

GameView::GameView(QWidget *parent) : QGraphicsView(parent)
{

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);

    setStyleSheet("background: transparent; border: 0px;");

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

}

GameView::GameView(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene, parent)
{

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);

    setStyleSheet("background: transparent; border: 0px;");

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

}
