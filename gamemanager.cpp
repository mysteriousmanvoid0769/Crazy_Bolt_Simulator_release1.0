#include "gamemanager.h"
#include "mainwindow.h"
#include "prius.h"
#include <QTimer>
#include <QDebug>
#include <cmath>
#include <QRandomGenerator>
#include <QMetaObject>
#include <algorithm>
#include <QSoundEffect>
#include <QUrl>

GameManager::GameManager(MainWindow *window, Prius *prius, QObject *parent)
    : QObject(parent), mainWindow(window), prius(prius), orderManager(new OrderManager(this)),
    paused(true), distance(0.0), totalDistance(0), rating(4.5), completedOrders(0), hasPendingOrder(false),
    totalDistanceDriven(0.0), fuelDistanceDriven(0.0), fuelLevel(100.0), wallet(0.0), lastComment(""),
    blockDefaultEndComment(false),
    currentSpeedCommentState(SCS_None),
    pendingOrderAfter500m(false), distanceSinceNewOrder(0.0),
    dialogStartUsed(false),
    dialogStoryUsed(false),
    dialogComplainUsed(false),
    passengerReceptive(false),
    dialogInteractionStarted(false),
    newOrderSound(nullptr),
    winSound(nullptr),
    fatalCrashSound(nullptr),
    debtGameOverSound(nullptr)
{
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &GameManager::updateDistanceInternal);
    updateTimer->start(16);

    connect(orderManager, &OrderManager::orderGenerated, this, [this](const Order &order) {
        if (newOrderSound && newOrderSound->isLoaded()) {
            newOrderSound->play();
        } else {
            qDebug() << "New order sound not loaded or null!";
        }

        emit showOrder(order);
        qDebug() << "Order generated signal received, emitting showOrder:" << order.distance << "m," << order.price << "AZN," << order.payment;
    });

    newOrderSound = new QSoundEffect(this);
    newOrderSound->setSource(QUrl::fromLocalFile("C:/Users/iskender/Documents/Crazy_Prius_Simulator_new/sounds/order_sound.wav"));
    newOrderSound->setVolume(0.7);

    winSound = new QSoundEffect(this);
    winSound->setSource(QUrl::fromLocalFile("C:/Users/iskender/Documents/Crazy_Prius_Simulator_new/sounds/pobeda.wav"));
    winSound->setVolume(0.9);

    fatalCrashSound = new QSoundEffect(this);
    fatalCrashSound->setSource(QUrl::fromLocalFile("C:/Users/iskender/Documents/Crazy_Prius_Simulator_new/sounds/avariya.wav"));
    fatalCrashSound->setVolume(1.0);

    debtGameOverSound = new QSoundEffect(this);
    debtGameOverSound->setSource(QUrl::fromLocalFile("C:/Users/iskender/Documents/Crazy_Prius_Simulator_new/sounds/gde_dengi.wav"));
    debtGameOverSound->setVolume(0.9);

    qDebug() << "GameManager initialized...";
    emit ratingUpdated(rating);
    emit fuelUpdated(fuelLevel);
    emit walletUpdated(wallet);
    emit updateCompletedOrdersCount(completedOrders);

    resetDialogState();
    checkTurboAvailability();
}

void GameManager::resetDialogState() {
    qDebug() << "GameManager: Resetting dialog state.";
    dialogStartUsed = false;
    dialogStoryUsed = false;
    dialogComplainUsed = false;
    passengerReceptive = false;
    dialogInteractionStarted = false;

    emit updateDialogButtonStates(true, false, false);

    if (mainWindow) {
        QMetaObject::invokeMethod(mainWindow, "clearCommentArea", Qt::QueuedConnection);
    }
}

void GameManager::startGame() {
    qDebug() << "GameManager::startGame() called";
    paused = true;
    hasPendingOrder = false;
    pendingOrderAfter500m = false;
    distanceSinceNewOrder = 0.0;
    lastComment = "";
    blockDefaultEndComment = false;
    currentSpeedCommentState = SCS_None;

    resetDialogState();
    if(prius) prius->deactivateTurbo();
    checkTurboAvailability();

    emit updateCompletedOrdersCount(completedOrders);
    emit ratingUpdated(rating);
    emit walletUpdated(wallet);
    if (orderManager) orderManager->generateOrder();
    emit pauseGame();
    qDebug() << "Game started, paused for order selection.";
}

void GameManager::resetGame() {
    qDebug() << "GameManager::resetGame() called (FULL RESET)";
    paused = true; distance = 0.0; totalDistance = 0; rating = 4.5; completedOrders = 0;
    hasPendingOrder = false; totalDistanceDriven = 0.0; fuelDistanceDriven = 0.0;
    fuelLevel = 100.0; wallet = 0.0; pendingOrderAfter500m = false; distanceSinceNewOrder = 0.0;
    currentOrder = Order(); pendingOrder = Order();
    lastComment = ""; blockDefaultEndComment = false; currentSpeedCommentState = SCS_None;

    if(prius) prius->deactivateTurbo();
    resetDialogState();
    checkTurboAvailability();

    emit distanceUpdated(distance, totalDistance); emit ratingUpdated(rating); emit fuelUpdated(fuelLevel);
    emit totalDistanceUpdated(totalDistanceDriven); emit walletUpdated(wallet); emit updateCompletedOrdersCount(completedOrders);
}

void GameManager::resetForRestart() {
    qDebug() << "GameManager::resetForRestart() called";
    paused = true; distance = 0.0; totalDistance = 0;

    currentOrder = Order(); pendingOrder = Order(); hasPendingOrder = false;
    pendingOrderAfter500m = false; distanceSinceNewOrder = 0.0; fuelDistanceDriven = 0.0; fuelLevel = 100.0;
    lastComment = ""; blockDefaultEndComment = false; currentSpeedCommentState = SCS_None;

    if(prius) prius->deactivateTurbo();
    resetDialogState();
    checkTurboAvailability();

    emit distanceUpdated(distance, totalDistance); emit fuelUpdated(fuelLevel);
    emit updateCompletedOrdersCount(completedOrders); emit ratingUpdated(rating); emit walletUpdated(wallet);
}

void GameManager::setPrius(Prius* newPrius) {
    this->prius = newPrius;
    qDebug() << "GameManager's Prius pointer updated.";
}

OrderManager* GameManager::get_orderManager() const { return orderManager; }
bool GameManager::hasActiveOrder() const { return totalDistance > 0; }
double GameManager::getWallet() const { return wallet; }

int GameManager::getCompletedOrders() const {
    return completedOrders;
}

void GameManager::setRating(double newRating) {
    rating = std::max(0.0, std::min(newRating, MAX_RATING));
    rating = round(rating * 100.0) / 100.0;
    qDebug() << "Rating set to:" << rating;
    emit ratingUpdated(rating);

    if (rating < MIN_RATING_GAME_OVER && !(mainWindow && mainWindow->isGameOver())) {
        emit gameEndedWithMessage("You are fired, go apply to Uber");
        qDebug() << "Game Over: Rating dropped below " << MIN_RATING_GAME_OVER;
    }
}

void GameManager::processRestartPenalty() {
    qDebug() << "GameManager::processRestartPenalty() called (currently unused)";
}

void GameManager::resetFuelDistance() {
    fuelDistanceDriven = 0.0;
    fuelLevel = 100.0;
    qDebug() << "Fuel distance reset and fuel level restored to 100%.";
    emit fuelUpdated(fuelLevel);
}

void GameManager::updateDistanceInternal() {
    if (paused || (mainWindow && mainWindow->isGameOver()) || !prius) {
        return;
    }

    int speed = prius->getSpeed();
    double distanceIncrement = (speed / 3.6) * (16.0 / 1000.0);

    totalDistanceDriven += distanceIncrement;
    fuelDistanceDriven += distanceIncrement;
    consumeFuel(distanceIncrement);

    if (hasActiveOrder()) {
        distance += distanceIncrement;
        emit distanceUpdated(distance, totalDistance);
    }
    emit totalDistanceUpdated(totalDistanceDriven);

    if (hasActiveOrder()) {
        QString autoComment = "";
        QString autoCommentHtmlTag = QString("<font color=\"gray\">[%1]</font>");
        QString vomitCommentHtmlTag = QString("<font color=\"orange\">Пассажир: %1</font>");

        if (prius->isTurboActive() && speed >= 175 && currentSpeedCommentState != SCS_Vomit) {
            autoComment = "АААААААА!!! (пассажир вырвал в салон)";
            currentSpeedCommentState = SCS_Vomit;
            blockDefaultEndComment = true;
        }
        else if (currentSpeedCommentState != SCS_Vomit && speed > 100 && currentSpeedCommentState != SCS_Speeding) {
            autoComment = "помедленнее!";
            currentSpeedCommentState = SCS_Speeding;
            blockDefaultEndComment = true;
        }
        else if (currentSpeedCommentState == SCS_Speeding && speed <= 90) {
            autoComment = "да, вот так хорошо";
            currentSpeedCommentState = SCS_None;
        }
        else if (currentSpeedCommentState == SCS_Vomit && speed < 150) {
            currentSpeedCommentState = SCS_None;
        }

        if (!autoComment.isEmpty() && autoComment != lastComment) {
            if (currentSpeedCommentState == SCS_Vomit) {
                emit updateDialogDisplayHtml(vomitCommentHtmlTag.arg(autoComment));
            } else {
                emit updateDialogDisplayHtml(autoCommentHtmlTag.arg(autoComment));
            }
            lastComment = autoComment;
        }
    }

    if (pendingOrderAfter500m) {
        distanceSinceNewOrder += distanceIncrement;
        if (distanceSinceNewOrder >= 500) {
            qDebug() << "500m driven since new order accepted. Activating pending order.";
            if (hasActiveOrder()) {
                qDebug() << "DEBUG: Force processing completion for the *previous* order before activating pending one.";
                distance = totalDistance;
                processOrderCompletion();
            }
            pendingOrderAfter500m = false;
            distanceSinceNewOrder = 0.0;
            currentOrder = pendingOrder;
            pendingOrder = Order();
            totalDistance = static_cast<int>(currentOrder.distance);
            distance = 0.0;
            hasPendingOrder = false;
            emit newPassenger();
            emit startObstacles();
            emit orderStarted();
            resetDialogState();
            lastComment = "";
            currentSpeedCommentState = SCS_None;
            emit distanceUpdated(distance, totalDistance);
            qDebug() << "New order activated after 500m drive, totalDistance set to:" << totalDistance << "m";
            return;
        }
    }

    if (hasActiveOrder() && distance >= totalDistance && !pendingOrderAfter500m) {
        processOrderCompletion();
    }

    if (!hasActiveOrder() && !hasPendingOrder && !pendingOrderAfter500m && !paused && !(mainWindow && mainWindow->isGameOver())) {
        qDebug() << "No active or pending order, pausing game to generate next order.";
        paused = true;
        emit pauseGame();
        if (orderManager) orderManager->generateOrder();
    }
}


void GameManager::consumeFuel(double distanceIncrement) {
    double fuelConsumedPercent = distanceIncrement * FUEL_CONSUMPTION_RATE_PER_METER;

    if (prius && prius->isTurboActive()) {
        fuelConsumedPercent *= 1.25;
    }

    if (fuelLevel > 0) {
        fuelLevel -= fuelConsumedPercent;
        fuelLevel = std::max(0.0, fuelLevel);
        emit fuelUpdated(fuelLevel);

        if (fuelLevel <= 0 && !(mainWindow && mainWindow->isGameOver())) {
            emit gameEndedWithMessage("У вас закончилось топливо, вы не заехали на Азпетроль");
            qDebug() << "Game Over: Fuel ran out!";
        }
    }
}

void GameManager::acceptPendingOrder(const Order &order) {
    if (!hasPendingOrder) {
        pendingOrder = order;
        hasPendingOrder = true;
        qDebug() << "Pending order saved via acceptPendingOrder: distance=" << order.distance << "m, price=" << order.price << "AZN";
    } else {
        qDebug() << "Warning: Tried to accept a pending order via acceptPendingOrder while another one was already pending.";
    }
}

void GameManager::onOrderAccepted(const Order &order) {
    qDebug() << "GameManager::onOrderAccepted received order: " << order.distance << "m";
    if (orderManager && orderManager->getIsFirstOrder()) {
        orderManager->confirmFirstOrderAccepted();
    }

    if (!hasActiveOrder() && !pendingOrderAfter500m) {
        currentOrder = order;
        totalDistance = static_cast<int>(order.distance);
        distance = 0.0;
        emit newPassenger();
        emit startObstacles();
        emit orderStarted();
        emit distanceUpdated(distance, totalDistance);
        qDebug() << "Order accepted in GameManager (no previous active), totalDistance set to:" << totalDistance << "m";
        resetDialogState();
        lastComment = "";
        currentSpeedCommentState = SCS_None;
    }
    else if (hasActiveOrder() && !hasPendingOrder) {
        pendingOrder = order;
        hasPendingOrder = true;
        pendingOrderAfter500m = true;
        distanceSinceNewOrder = 0.0;
        qDebug() << "New order accepted while previous active, will activate after driving 500m.";
    }
    else {
        qDebug() << "Warning: Order accepted in unexpected state (hasActive=" << hasActiveOrder()
        << ", hasPending=" << hasPendingOrder << ", pending500m=" << pendingOrderAfter500m << ").";
        if(!hasPendingOrder) {
            pendingOrder = order;
            hasPendingOrder = true;
            qDebug() << "Corrected state: Saved as pending order.";
        }
    }
}

void GameManager::processOrderCompletion() {
    qDebug() << "DEBUG: processOrderCompletion START";
    if (!hasActiveOrder()) {
        qDebug() << "DEBUG: processOrderCompletion aborted (no active order).";
        return;
    }
    qDebug() << "Processing completion for order: " << currentOrder.distance << "m, price:" << currentOrder.price;

    double tip = calculateTip();
    addMoney(currentOrder.price + tip);
    setRating(rating + RATING_PER_ORDER);
    completedOrders++;
    emit updateCompletedOrdersCount(completedOrders);

    Order completedOrder = currentOrder;
    distance = 0.0;
    totalDistance = 0;
    currentOrder = Order();

    if (!blockDefaultEndComment && !dialogInteractionStarted) {
        QString endComment = "спасибо. до свидания!";
        emit updateDialogDisplayHtml(QString("<font color=\"white\">Пассажир: %1</font>").arg(endComment));
        lastComment = endComment;
    }
    blockDefaultEndComment = false;
    dialogInteractionStarted = false;
    currentSpeedCommentState = SCS_None;

    emit distanceUpdated(distance, totalDistance);

    qDebug() << "Order completed log. Payment:" << completedOrder.price << "AZN, Tip:" << tip << "AZN."
             << " New Rating:" << rating << ". Total completed orders:" << completedOrders;
    qDebug() << "DEBUG: processOrderCompletion END";

    if (!hasPendingOrder) {
        qDebug() << "Order completed. No pending order, will pause game to generate next in update loop.";
    } else {
        qDebug() << "Order completed. Pending order exists, will activate after 500m.";
    }
}


double GameManager::calculateTip() {
    double tip = 0;
    if (QRandomGenerator::global()->bounded(4) == 0) {
        tip = QRandomGenerator::global()->bounded(1, 4) * 0.5;
    }
    return tip;
}

void GameManager::onOrderFailed() {
    qDebug() << "Order failed / rejected in GameManager.";
    if (orderManager) {
        orderManager->rejectOrder();
    }
    decreaseRatingOnReject();
    if (orderManager) {
        orderManager->generateOrder();
    }
}

void GameManager::onCollision() {
    qDebug() << "Collision detected in GameManager.";
    if (!prius || (mainWindow && mainWindow->isGameOver())) return;

    bool turboWasActive = prius->isTurboActive();
    int collisionSpeed = prius->getSpeed();

    if (turboWasActive && collisionSpeed >= 175) {
        qDebug() << "FATAL CRASH with Turbo at speed:" << collisionSpeed;
        if (fatalCrashSound && fatalCrashSound->isLoaded()) {
            fatalCrashSound->play();
        } else {
            qDebug() << "Fatal crash sound not loaded or null!";
        }
        emit gameEndedWithMessage("Вы разбились насмерть");
        return;
    }

    QString collisionComment = "эта поездка была ужасная!";
    emit updateDialogDisplayHtml(QString("<font color=\"white\">Пассажир: %1</font>").arg(collisionComment));
    lastComment = collisionComment;
    blockDefaultEndComment = true;
    currentSpeedCommentState = SCS_None;

    emit updateDialogButtonStates(false, false, false);
    dialogInteractionStarted = true;
    passengerReceptive = false;

    setRating(rating - RATING_PENALTY_CRASH);
    deductMoney(5.0);

    if (turboWasActive) {
        qDebug() << "Collision: Turbo upgrade deactivated.";
        prius->deactivateTurbo();
        checkTurboAvailability();
    }

    if (rating >= MIN_RATING_GAME_OVER && wallet >= MIN_WALLET_GAME_OVER && !(mainWindow && mainWindow->isGameOver())) {
        emit gameEndedWithMessage("Crashed! Restart to continue. Passenger is unhappy.");
        qDebug() << "Collision occurred, game paused for restart.";
    }
}

void GameManager::decreaseRatingOnReject() {
    qDebug() << "Decreasing rating due to order rejection.";
    setRating(rating - RATING_PENALTY_REJECT);
}

void GameManager::resumeGame() {
    paused = false;
    qDebug() << "GameManager::resumeGame called, internal paused flag set to false.";
}

void GameManager::addMoney(double amount) {
    if (amount > 0) {
        wallet += amount;
        wallet = round(wallet * 100.0) / 100.0;
        qDebug() << "Wallet updated: +" << amount << " AZN. New balance:" << wallet;
        emit walletUpdated(wallet);

        if (wallet >= 15.0 && !(mainWindow && mainWindow->isGameOver())) {
            qDebug() << "Win condition met! Wallet >= 15 AZN.";
            if (winSound && winSound->isLoaded()) {
                winSound->play();
            } else {
                qDebug() << "Win sound not loaded or null!";
            }
            emit gameEndedWithMessage("Поздравляем! Ваши будни в такси окончены. Возвращайтесь на Рублевку");
            return;
        }

        checkTurboAvailability();
    }
}

void GameManager::deductMoney(double amount) {
    if (amount > 0) {
        wallet -= amount;
        wallet = round(wallet * 100.0) / 100.0;
        qDebug() << "Wallet updated: -" << amount << " AZN. New balance:" << wallet;
        emit walletUpdated(wallet);
        checkTurboAvailability();
        checkDebtLimit();
    }
}

void GameManager::checkDebtLimit() {
    if (wallet < MIN_WALLET_GAME_OVER && !(mainWindow && mainWindow->isGameOver())) {
        if (debtGameOverSound && debtGameOverSound->isLoaded()) {
            debtGameOverSound->play();
        } else {
            qDebug() << "Debt game over sound not loaded or null!";
        }
        emit gameEndedWithMessage("Ахмед пришел возвращать долги, ваш приус сломали и вы остаетесь без машины");
        qDebug() << "Game Over: Debt exceeded " << MIN_WALLET_GAME_OVER << " AZN.";
        return;
    }
}

void GameManager::checkTurboAvailability() {
    if (prius) {
        bool canAfford = wallet >= TURBO_COST;
        bool alreadyActive = prius->isTurboActive();
        bool available = canAfford && !alreadyActive;
        emit turboAvailabilityChanged(available);
    } else {
        emit turboAvailabilityChanged(false);
    }
}

void GameManager::purchaseTurboUpgrade() {
    qDebug() << "GM Slot: purchaseTurboUpgrade called.";
    if (prius && !prius->isTurboActive() && wallet >= TURBO_COST) {
        qDebug() << "Attempting to purchase Turbo...";
        deductMoney(TURBO_COST);
        prius->activateTurbo();
        qDebug() << "Turbo Upgrade Purchased!";
    } else {
        qDebug() << "Cannot purchase Turbo. Prius exists:" << (prius != nullptr)
        << " Turbo Active:" << (prius ? prius->isTurboActive() : -1)
        << " Wallet:" << wallet;
    }
}

void GameManager::onPlayerInitiatedDialog() {
    qDebug() << "GM Slot: onPlayerInitiatedDialog called.";
    if (dialogStartUsed || !hasActiveOrder() || paused ) {
        qDebug() << "GM Slot: Cannot start dialog now (Used/NoOrder/Paused):"
                 << dialogStartUsed << !hasActiveOrder() << paused;
        return;
    }
    dialogStartUsed = true;
    dialogInteractionStarted = true;
    QString driverLine = "Блин, эта работа в такси так надоела";
    emit updateDialogDisplayHtml(QString("<font color=\"red\">Я: %1</font>").arg(driverLine));
    emit updateDialogButtonStates(false, false, false);

    QTimer::singleShot(1500, this, [this](){
        if (!hasActiveOrder()) return;
        int chance = QRandomGenerator::global()->bounded(100);
        QString passengerResponse;
        bool enableOthers;
        if (chance < 20) {
            passengerReceptive = false;
            passengerResponse = "Мне это не интересно";
            enableOthers = false;
            qDebug() << "GM: Passenger response - Not Interested";
            dialogInteractionStarted = false;
        } else {
            passengerReceptive = true;
            passengerResponse = "Понимаю";
            enableOthers = true;
            qDebug() << "GM: Passenger response - Interested";
        }
        emit updateDialogDisplayHtml(QString("<font color=\"white\">Пассажир: %1</font>").arg(passengerResponse));
        emit updateDialogButtonStates(false, enableOthers && !dialogStoryUsed, enableOthers && !dialogComplainUsed);
    });
}

void GameManager::onPlayerToldStory() {
    qDebug() << "GM Slot: onPlayerToldStory called.";
    if (dialogStoryUsed || !dialogInteractionStarted || !passengerReceptive || !hasActiveOrder() || paused) {
        qDebug() << "GM Slot: Cannot tell story now. State:" << dialogStoryUsed << !dialogInteractionStarted << !passengerReceptive << !hasActiveOrder() << paused;
        return;
    }
    dialogStoryUsed = true;
    QString driverLine = "Вообще-то у меня есть свой бизнес на Рублевке, такси это просто хобби";
    emit updateDialogDisplayHtml(QString("<font color=\"red\">Я: %1</font>").arg(driverLine));
    emit updateDialogButtonStates(false, false, !dialogComplainUsed && passengerReceptive);
    QTimer::singleShot(800, this, [this](){
        if (!hasActiveOrder()) return;
        QString passengerResponse = "Очень круто, что у вас есть страсть!";
        emit updateDialogDisplayHtml(QString("<font color=\"white\">Пассажир: %1</font>").arg(passengerResponse));
    });
}

void GameManager::onPlayerComplained() {
    qDebug() << "GM Slot: onPlayerComplained called.";
    if (dialogComplainUsed || !dialogInteractionStarted || !passengerReceptive || !hasActiveOrder() || paused) {
        qDebug() << "GM Slot: Cannot complain now. State:" << dialogComplainUsed << !dialogInteractionStarted << !passengerReceptive << !hasActiveOrder() << paused;
        return;
    }
    dialogComplainUsed = true;
    QString driverLine = "Все машины на дороге такие твари, не люблю водить машину";
    emit updateDialogDisplayHtml(QString("<font color=\"red\">Я: %1</font>").arg(driverLine));
    emit updateDialogButtonStates(false, !dialogStoryUsed && passengerReceptive, false);
    QTimer::singleShot(800, this, [this](){
        if (!hasActiveOrder()) return;
        QString passengerResponse = "Тяжелая у вас работа...";
        emit updateDialogDisplayHtml(QString("<font color=\"white\">Пассажир: %1</font>").arg(passengerResponse));
    });
}
