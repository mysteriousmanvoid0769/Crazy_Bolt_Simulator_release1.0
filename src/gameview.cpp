// gameview.cpp

#include "gameview.h"
#include <QScrollBar>
#include <QGraphicsScene> // Для scene()

GameView::GameView(QWidget *parent) : QGraphicsView(parent)
{
    // Общие настройки
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);
    // Устанавливаем прозрачный фон и убираем рамку
    setStyleSheet("background: transparent; border: 0px;");
    // setCacheMode(QGraphicsView::CacheBackground); // Больше не нужно
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    // Центрирование сцены по умолчанию (setAlignment не используется)
}

GameView::GameView(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene, parent)
{
    // Общие настройки
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::NoFocus);
        // Устанавливаем прозрачный фон и убираем рамку
    setStyleSheet("background: transparent; border: 0px;");
    // setCacheMode(QGraphicsView::CacheBackground); // Больше не нужно
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    // Центрирование сцены по умолчанию (setAlignment не используется)
}


// // УДАЛЕНА реализация setBackgroundImage
// void GameView::setBackgroundImage(const QString &imagePath)
// {
//     // ... старый код ...
// }

// // УДАЛЕНА реализация drawBackground
// void GameView::drawBackground(QPainter *painter, const QRectF &rect)
// {
//     // ... старый код ...
// }
