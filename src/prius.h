#ifndef PRIUS_H
#define PRIUS_H

#include <QGraphicsPixmapItem>
#include <QObject>

// Enum для полос движения
enum class Lane { Left = 0, Middle = 1, Right = 2 };

class Prius : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    Prius(QGraphicsItem *parent = nullptr);
    void accelerate();
    void brake();
    void moveLeft();
    void moveRight();
    int getSpeed() const;
    Lane getCurrentLane() const;

    // --- Методы и геттеры для Turbo ---
    void activateTurbo();
    void deactivateTurbo();
    bool isTurboActive() const;
    int getMaxSpeed() const;

private:
    int speed;
    Lane currentLane;

    // --- Переменные для Turbo ---
    int maxSpeed;
    bool turboActive;

    // Функция для установки позиции по полосе
    void setLanePosition(Lane lane);
    // Рассчитываем X координату для центра полосы
    qreal getCenterXForLane(Lane lane);

    // Константы для полос
    const qreal LANE_WIDTH = 100;
    const qreal ROAD_LEFT_EDGE = 50;

    // --- Константы скорости (проверяем значения) ---
    // УБЕДИТЕСЬ, ЧТО ЭТИ ЗНАЧЕНИЯ КОРРЕКТНЫ!
    static const int DEFAULT_MAX_SPEED = 120; // Сделаем static const для надежности
    static const int TURBO_MAX_SPEED = 180;   // Сделаем static const для надежности
};

#endif // PRIUS_H
