// gameview.h

#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QPainter> // Может все еще быть нужен для других переопределений
#include <QDebug> // Для отладки
// #include <QPixmap> // Больше не нужен здесь

class GameView : public QGraphicsView {
    Q_OBJECT
public:
    // Конструкторы, аналогичные QGraphicsView
    explicit GameView(QWidget *parent = nullptr);
    explicit GameView(QGraphicsScene *scene, QWidget *parent = nullptr);

    // Метод для установки пути к фоновому изображению - УДАЛЕН
    // void setBackgroundImage(const QString &imagePath);

    // protected: // УДАЛЕНА секция protected, если в ней больше ничего нет
    // Переопределяем виртуальный метод отрисовки фона - УДАЛЕН
    // void drawBackground(QPainter *painter, const QRectF &rect) override;

    // private: // УДАЛЕНЫ private переменные для фона
    // QPixmap backgroundImage;
    // bool backgroundLoaded = false;
};

#endif // GAMEVIEW_H
