// gamemanager.h

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QRandomGenerator>
#include "ordermanager.h"
#include <algorithm> // Для std::min, std::max

// Forward declarations
class MainWindow;
class Prius;

class GameManager : public QObject {
    Q_OBJECT
public:
    explicit GameManager(MainWindow *window, Prius *prius, QObject *parent = nullptr);
    void startGame();
    void resetGame(); // Полный сброс
    void resetForRestart(); // Сброс для рестарта после аварии
    void setPrius(Prius* newPrius);
    OrderManager* get_orderManager() const;
    bool hasActiveOrder() const;
    double getRating() const { return rating; }
    // Метод setRating теперь гарантирует, что рейтинг не превысит 5.0
    void setRating(double newRating);
    double getTotalDistanceDriven() const { return totalDistanceDriven; }
    void resetFuelDistance();
    double getWallet() const;
    int getCompletedOrders() const; // <--- ДОБАВЛЕНО ОБЪЯВЛЕНИЕ

public slots:
    void acceptPendingOrder(const Order &order);
    void resumeGame();
    void onCollision();
    // Слот decreaseRatingOnReject теперь будет вычитать 0.02
    void decreaseRatingOnReject();
    void processRestartPenalty();
    void onOrderAccepted(const Order &order);
    void onOrderFailed();
    // Слот processOrderCompletion теперь будет добавлять 0.1 к рейтингу
    void processOrderCompletion();

    // --- Слоты для диалога ---
    void onPlayerInitiatedDialog();
    void onPlayerToldStory();
    void onPlayerComplained();

    // --- СЛОТ для покупки Turbo ---
    void purchaseTurboUpgrade();

signals:
    // Сигналы для UI
    void distanceUpdated(double current, int total);
    void ratingUpdated(double newRating);
    void fuelUpdated(double fuelPercentage);
    void totalDistanceUpdated(double totalDistance);
    void walletUpdated(double newWallet);
    void updateCompletedOrdersCount(int count);

    // Сигналы управления состоянием игры
    void showOrder(const Order &order);
    void newPassenger();
    void pauseGame();
    void pauseForNewPassenger(); // Используется для паузы перед генерацией заказа
    void gameEndedWithMessage(QString message);
    void startObstacles();
    void orderStarted();

    // --- Сигналы для диалога ---
    void updateDialogDisplayHtml(const QString& htmlFragment);
    void updateDialogButtonStates(bool enableStart, bool enableStory, bool enableComplain);

    // --- СИГНАЛ для кнопки Turbo ---
    void turboAvailabilityChanged(bool available);


private slots:
    void updateDistanceInternal();

private:
    MainWindow *mainWindow;
    Prius *prius;
    OrderManager *orderManager;
    bool paused;
    double distance;
    int totalDistance;
    double rating; // Начальное значение 4.5, Макс 5.0
    int completedOrders; // <-- Приватный член, значение которого возвращает getCompletedOrders()
    Order currentOrder;
    Order pendingOrder;
    bool hasPendingOrder; // Флаг, есть ли принятый заказ, ожидающий активации
    double totalDistanceDriven;
    double fuelDistanceDriven; // Расстояние, пройденное на текущем баке
    double fuelLevel; // Уровень топлива в %
    double wallet;
    bool pendingOrderAfter500m; // Флаг для активации заказа после 500м езды
    double distanceSinceNewOrder; // Счетчик для 500м

    // Метод расчета расхода топлива
    void consumeFuel(double distanceIncrement);

    // --- Переменные для автоматических комментариев ---
    QString lastComment;
    bool blockDefaultEndComment; // Блокировать стандартное "спасибо"?
    enum SpeedCommentState { SCS_None, SCS_Speeding, SCS_Radio, SCS_Vomit };
    SpeedCommentState currentSpeedCommentState;

    // --- Переменные состояния диалога ---
    bool dialogStartUsed;
    bool dialogStoryUsed;
    bool dialogComplainUsed;
    bool passengerReceptive;
    bool dialogInteractionStarted; // Был ли инициирован диалог игроком?

    // --- Константы ---
    const double TURBO_COST = 5.0;
    const double MAX_RATING = 5.0; // Максимальный рейтинг
    const double MIN_RATING_GAME_OVER = 1.0; // Порог рейтинга для Game Over
    const double RATING_PER_ORDER = 0.1; // Прибавка за заказ
    const double RATING_PENALTY_REJECT = 0.02; // Штраф за отказ
    const double RATING_PENALTY_CRASH = 0.5; // Штраф за аварию
    const double MIN_WALLET_GAME_OVER = -15.0; // Порог кошелька для Game Over
    const double FUEL_CONSUMPTION_RATE_PER_METER = 0.02; // 100% / 5000m = 0.02% per meter

    // Приватные методы
    void addMoney(double amount);
    void deductMoney(double amount);
    void checkDebtLimit();
    double calculateTip();
    void resetDialogState();
    void checkTurboAvailability();
};

#endif // GAMEMANAGER_H
