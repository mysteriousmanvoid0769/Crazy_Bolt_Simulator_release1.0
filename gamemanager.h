#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QRandomGenerator>
#include "ordermanager.h"
#include <algorithm>


class MainWindow;
class Prius;

class GameManager : public QObject {
    Q_OBJECT
public:
    explicit GameManager(MainWindow *window, Prius *prius, QObject *parent = nullptr);
    void startGame();
    void resetGame();
    void resetForRestart();
    void setPrius(Prius* newPrius);
    OrderManager* get_orderManager() const;
    bool hasActiveOrder() const;
    double getRating() const { return rating; }

    void setRating(double newRating);
    double getTotalDistanceDriven() const { return totalDistanceDriven; }
    void resetFuelDistance();
    double getWallet() const;
    int getCompletedOrders() const;

public slots:
    void acceptPendingOrder(const Order &order);
    void resumeGame();
    void onCollision();

    void decreaseRatingOnReject();
    void processRestartPenalty();
    void onOrderAccepted(const Order &order);
    void onOrderFailed();

    void processOrderCompletion();


    void onPlayerInitiatedDialog();
    void onPlayerToldStory();
    void onPlayerComplained();


    void purchaseTurboUpgrade();

signals:

    void distanceUpdated(double current, int total);
    void ratingUpdated(double newRating);
    void fuelUpdated(double fuelPercentage);
    void totalDistanceUpdated(double totalDistance);
    void walletUpdated(double newWallet);
    void updateCompletedOrdersCount(int count);


    void showOrder(const Order &order);
    void newPassenger();
    void pauseGame();
    void pauseForNewPassenger();
    void gameEndedWithMessage(QString message);
    void startObstacles();
    void orderStarted();


    void updateDialogDisplayHtml(const QString& htmlFragment);
    void updateDialogButtonStates(bool enableStart, bool enableStory, bool enableComplain);


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
    double rating;
    int completedOrders;
    Order currentOrder;
    Order pendingOrder;
    bool hasPendingOrder;
    double totalDistanceDriven;
    double fuelDistanceDriven;
    double fuelLevel;
    double wallet;
    bool pendingOrderAfter500m;
    double distanceSinceNewOrder;


    void consumeFuel(double distanceIncrement);


    QString lastComment;
    bool blockDefaultEndComment;
    enum SpeedCommentState { SCS_None, SCS_Speeding, SCS_Radio, SCS_Vomit };
    SpeedCommentState currentSpeedCommentState;


    bool dialogStartUsed;
    bool dialogStoryUsed;
    bool dialogComplainUsed;
    bool passengerReceptive;
    bool dialogInteractionStarted;


    const double TURBO_COST = 5.0;
    const double MAX_RATING = 5.0;
    const double MIN_RATING_GAME_OVER = 1.0;
    const double RATING_PER_ORDER = 0.1;
    const double RATING_PENALTY_REJECT = 0.02;
    const double RATING_PENALTY_CRASH = 0.5;
    const double MIN_WALLET_GAME_OVER = -15.0;
    const double FUEL_CONSUMPTION_RATE_PER_METER = 0.02;


    void addMoney(double amount);
    void deductMoney(double amount);
    void checkDebtLimit();
    double calculateTip();
    void resetDialogState();
    void checkTurboAvailability();
};

#endif // GAMEMANAGER_H
