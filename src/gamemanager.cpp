// gamemanager.cpp

#include "gamemanager.h"
#include "mainwindow.h"
#include "prius.h"
#include <QTimer>
#include <QDebug>
#include <cmath>         // Для std::max, std::min, round
#include <QRandomGenerator>
#include <QMetaObject>
#include <algorithm> // Для std::min, std::max

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
    dialogInteractionStarted(false)
{
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &GameManager::updateDistanceInternal);
    updateTimer->start(16); // ~60 FPS

    connect(orderManager, &OrderManager::orderGenerated, this, [this](const Order &order) {
        emit showOrder(order);
        qDebug() << "Order generated signal received, emitting showOrder:" << order.distance << "m," << order.price << "AZN," << order.payment;
    });

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
    if(prius) prius->deactivateTurbo(); // Убедимся, что турбо выключено
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
    // Не сбрасываем счетчики wallet, rating, completedOrders при рестарте после аварии
    emit updateCompletedOrdersCount(completedOrders); emit ratingUpdated(rating); emit walletUpdated(wallet);
}

void GameManager::setPrius(Prius* newPrius) {
    this->prius = newPrius;
    qDebug() << "GameManager's Prius pointer updated.";
}

OrderManager* GameManager::get_orderManager() const { return orderManager; }
bool GameManager::hasActiveOrder() const { return totalDistance > 0; }
double GameManager::getWallet() const { return wallet; }

// +++ ДОБАВЛЕНА РЕАЛИЗАЦИЯ +++
int GameManager::getCompletedOrders() const {
    return completedOrders;
}
// +++ КОНЕЦ ДОБАВЛЕНИЯ +++

// ИСПРАВЛЕНО: setRating теперь округляет до ДВУХ знаков после запятой
void GameManager::setRating(double newRating) {
    rating = std::max(0.0, std::min(newRating, MAX_RATING));
    // Округляем до 2 знаков, чтобы 0.02 имело эффект
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

    // --- Логика АВТОМАТИЧЕСКИХ комментариев ---
    // Автокомментарии работают всегда, когда есть активный заказ
    if (hasActiveOrder()) {
        QString autoComment = "";
        QString autoCommentHtmlTag = QString("<font color=\"gray\">[%1]</font>");
        QString vomitCommentHtmlTag = QString("<font color=\"orange\">Пассажир: %1</font>");

        // 1. Рвота (приоритет!)
        if (prius->isTurboActive() && speed >= 175 && currentSpeedCommentState != SCS_Vomit) {
            autoComment = "АААААААА!!! (пассажир вырвал в салон)";
            currentSpeedCommentState = SCS_Vomit;
            blockDefaultEndComment = true;
        }
        // 2. Превышение скорости
        else if (currentSpeedCommentState != SCS_Vomit && speed > 100 && currentSpeedCommentState != SCS_Speeding) {
            autoComment = "помедленнее!";
            currentSpeedCommentState = SCS_Speeding;
            blockDefaultEndComment = true;
        }
        // 3. Снижение после превышения
        else if (currentSpeedCommentState == SCS_Speeding && speed <= 90) {
            autoComment = "да, вот так хорошо";
            currentSpeedCommentState = SCS_None;
        }
        // 4. Радио (удалено или изменено, т.к. 60 - низкая скорость для жалобы на скуку)
        // else if (currentSpeedCommentState == SCS_None && speed == 60 && lastComment != "включите радио, слишком скучно") {
        //     autoComment = "включите радио, слишком скучно";
        //     blockDefaultEndComment = true;
        // }
        // 5. Сброс рвоты
        else if (currentSpeedCommentState == SCS_Vomit && speed < 150) {
            currentSpeedCommentState = SCS_None;
        }

        // Отправка
        if (!autoComment.isEmpty() && autoComment != lastComment) {
            if (currentSpeedCommentState == SCS_Vomit) {
                emit updateDialogDisplayHtml(vomitCommentHtmlTag.arg(autoComment));
            } else {
                emit updateDialogDisplayHtml(autoCommentHtmlTag.arg(autoComment));
            }
            lastComment = autoComment;
        }
    }
    // --- Конец логики АВТОМАТИЧЕСКИХ комментариев ---


    // Логика активации ожидающего заказа после 500м езды
    if (pendingOrderAfter500m) {
        distanceSinceNewOrder += distanceIncrement;
        if (distanceSinceNewOrder >= 500) {
            qDebug() << "500m driven since new order accepted. Activating pending order.";
            if (hasActiveOrder()) {
                qDebug() << "DEBUG: Force processing completion for the *previous* order before activating pending one.";
                distance = totalDistance; // Имитируем завершение дистанции
                processOrderCompletion(); // Обрабатываем завершение предыдущего
            }
            // Активируем ожидающий заказ
            pendingOrderAfter500m = false;
            distanceSinceNewOrder = 0.0;
            currentOrder = pendingOrder;
            pendingOrder = Order(); // Сбрасываем ожидающий
            totalDistance = static_cast<int>(currentOrder.distance);
            distance = 0.0; // Сбрасываем текущую дистанцию
            hasPendingOrder = false;
            emit newPassenger();
            emit startObstacles();
            emit orderStarted();
            resetDialogState();
            lastComment = "";
            currentSpeedCommentState = SCS_None;
            emit distanceUpdated(distance, totalDistance);
            qDebug() << "New order activated after 500m drive, totalDistance set to:" << totalDistance << "m";
            return; // Выходим, чтобы не обработать завершение нового заказа сразу же
        }
    }

    // Логика завершения заказа
    // Проверяем, не активируется ли сейчас заказ после 500м
    if (hasActiveOrder() && distance >= totalDistance && !pendingOrderAfter500m) {
        processOrderCompletion();
    }

    // Генерация нового заказа, если нужно (и игра не на паузе/не закончена)
    if (!hasActiveOrder() && !hasPendingOrder && !pendingOrderAfter500m && !paused && !(mainWindow && mainWindow->isGameOver())) {
        qDebug() << "No active or pending order, pausing game to generate next order.";
        paused = true; // Ставим на паузу для выбора заказа
        emit pauseGame(); // Отправляем сигнал паузы
        if (orderManager) orderManager->generateOrder(); // Генерируем новый заказ
    }
}


void GameManager::consumeFuel(double distanceIncrement) {
    double fuelConsumedPercent = distanceIncrement * FUEL_CONSUMPTION_RATE_PER_METER;

    if (prius && prius->isTurboActive()) {
        fuelConsumedPercent *= 1.25; // Турбо увеличивает расход на 25%
    }

    if (fuelLevel > 0) {
        fuelLevel -= fuelConsumedPercent;
        fuelLevel = std::max(0.0, fuelLevel); // Не уходить ниже 0
        emit fuelUpdated(fuelLevel); // Обновляем UI

        // Проверка на конец игры из-за топлива
        if (fuelLevel <= 0 && !(mainWindow && mainWindow->isGameOver())) {
            emit gameEndedWithMessage("У вас закончилось топливо, вы не заехали на Азпетроль");
            qDebug() << "Game Over: Fuel ran out!";
        }
    }
}

void GameManager::acceptPendingOrder(const Order &order) {
    // Этот метод кажется избыточным, т.к. onOrderAccepted обрабатывает это
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
        orderManager->confirmFirstOrderAccepted(); // Отмечаем, что первый заказ принят
    }

    // Если нет активного заказа и не ждем 500м для активации следующего
    if (!hasActiveOrder() && !pendingOrderAfter500m) {
        currentOrder = order; // Делаем заказ текущим
        totalDistance = static_cast<int>(order.distance);
        distance = 0.0;
        // Эмитим сигналы начала заказа
        emit newPassenger();
        emit startObstacles();
        emit orderStarted();
        emit distanceUpdated(distance, totalDistance); // Обновляем UI дистанции
        qDebug() << "Order accepted in GameManager (no previous active), totalDistance set to:" << totalDistance << "m";
        resetDialogState(); // Сбрасываем состояние диалога
        lastComment = "";
        currentSpeedCommentState = SCS_None;
    }
    // Если есть активный заказ, но нет ожидающего
    else if (hasActiveOrder() && !hasPendingOrder) {
        pendingOrder = order; // Сохраняем как ожидающий
        hasPendingOrder = true;
        pendingOrderAfter500m = true; // Устанавливаем флаг активации через 500м
        distanceSinceNewOrder = 0.0; // Сбрасываем счетчик 500м
        qDebug() << "New order accepted while previous active, will activate after driving 500m.";
    }
    // Неожиданное состояние (например, два ожидающих заказа)
    else {
        qDebug() << "Warning: Order accepted in unexpected state (hasActive=" << hasActiveOrder()
        << ", hasPending=" << hasPendingOrder << ", pending500m=" << pendingOrderAfter500m << ").";
        // Попытка исправить: если нет ожидающего, сохраняем как ожидающий
        if(!hasPendingOrder) {
            pendingOrder = order;
            hasPendingOrder = true;
            qDebug() << "Corrected state: Saved as pending order.";
        }
    }
}

void GameManager::processOrderCompletion() {
    qDebug() << "DEBUG: processOrderCompletion START";
    if (!hasActiveOrder()) { // Проверка, есть ли что завершать
        qDebug() << "DEBUG: processOrderCompletion aborted (no active order).";
        return;
    }
    qDebug() << "Processing completion for order: " << currentOrder.distance << "m, price:" << currentOrder.price;

    // --- Начисление денег и рейтинга ---
    double tip = calculateTip(); // Расчет чаевых
    addMoney(currentOrder.price + tip); // Добавляем оплату и чаевые
    setRating(rating + RATING_PER_ORDER); // Повышаем рейтинг
    completedOrders++; // Увеличиваем счетчик заказов
    emit updateCompletedOrdersCount(completedOrders); // Обновляем UI счетчика
    // --- Конец начисления ---

    Order completedOrder = currentOrder; // Сохраняем для лога
    // Сбрасываем данные текущего заказа
    distance = 0.0;
    totalDistance = 0;
    currentOrder = Order(); // Сброс

    // --- Комментарий в конце поездки ---
    // Показываем стандартное "спасибо", если не было заблокировано (аварией, диалогом и т.д.)
    if (!blockDefaultEndComment && !dialogInteractionStarted) {
        QString endComment = "спасибо. до свидания!";
        emit updateDialogDisplayHtml(QString("<font color=\"white\">Пассажир: %1</font>").arg(endComment));
        lastComment = endComment;
    }
    // Сбрасываем флаги для следующего заказа
    blockDefaultEndComment = false;
    dialogInteractionStarted = false; // Сбрасываем флаг диалога
    currentSpeedCommentState = SCS_None;

    emit distanceUpdated(distance, totalDistance); // Обновляем UI (0/0)

    qDebug() << "Order completed log. Payment:" << completedOrder.price << "AZN, Tip:" << tip << "AZN."
             << " New Rating:" << rating << ". Total completed orders:" << completedOrders;
    qDebug() << "DEBUG: processOrderCompletion END";

    // Если нет ожидающего заказа, игра сама перейдет в паузу для генерации нового в updateDistanceInternal
    if (!hasPendingOrder) {
        qDebug() << "Order completed. No pending order, will pause game to generate next in update loop.";
            // paused = true; // Ставим на паузу здесь, чтобы избежать гонки состояний
            // emit pauseGame(); // Сигнал паузы
            // if (orderManager) orderManager->generateOrder(); // Генерируем следующий
    } else {
        qDebug() << "Order completed. Pending order exists, will activate after 500m.";
        // Ничего не делаем, updateDistanceInternal активирует pendingOrderAfter500m
    }
}


double GameManager::calculateTip() {
    double tip = 0;
    if (QRandomGenerator::global()->bounded(4) == 0) { // 25% шанс на чаевые
        tip = QRandomGenerator::global()->bounded(1, 4) * 0.5; // Чаевые от 0.5 до 1.5 AZN
    }
    return tip;
}

void GameManager::onOrderFailed() {
    qDebug() << "Order failed / rejected in GameManager.";
    if (orderManager) {
        // Сообщаем менеджеру (хотя он сам ничего не делает с этим)
        orderManager->rejectOrder();
    }
    decreaseRatingOnReject(); // Снижаем рейтинг за отказ
    // Генерируем новый заказ СРАЗУ ЖЕ (игра уже на паузе из MainWindow)
    if (orderManager) {
        orderManager->generateOrder();
    }
}

void GameManager::onCollision() {
    qDebug() << "Collision detected in GameManager.";
    if (!prius || (mainWindow && mainWindow->isGameOver())) return; // Не обрабатываем, если уже Game Over

    bool turboWasActive = prius->isTurboActive();
    int collisionSpeed = prius->getSpeed();

    // Фатальная авария с турбо на высокой скорости
    if (turboWasActive && collisionSpeed >= 175) {
        qDebug() << "FATAL CRASH with Turbo at speed:" << collisionSpeed;
        emit gameEndedWithMessage("Вы разбились насмерть");
        return;
    }

    // Комментарий пассажира при аварии
    QString collisionComment = "эта поездка была ужасная!";
    emit updateDialogDisplayHtml(QString("<font color=\"white\">Пассажир: %1</font>").arg(collisionComment));
    lastComment = collisionComment;
    blockDefaultEndComment = true; // Блокируем стандартное "спасибо"
    currentSpeedCommentState = SCS_None; // Сбрасываем состояние комментариев о скорости

    // Блокируем кнопки диалога после аварии
    emit updateDialogButtonStates(false, false, false);
    dialogInteractionStarted = true; // Считаем, что взаимодействие было (негативное)
    passengerReceptive = false;

    // Штрафы
    setRating(rating - RATING_PENALTY_CRASH); // Снижаем рейтинг
    deductMoney(5.0); // Вычитаем деньги за ремонт (это вызовет checkTurboAvailability)

    // Отключаем турбо, если оно было активно
    if (turboWasActive) {
        qDebug() << "Collision: Turbo upgrade deactivated.";
        prius->deactivateTurbo();
        checkTurboAvailability(); // Обновляем доступность кнопки турбо
    }

    // Завершаем игру с возможностью рестарта (если рейтинг/деньги позволяют)
    if (rating >= MIN_RATING_GAME_OVER && wallet >= MIN_WALLET_GAME_OVER && !(mainWindow && mainWindow->isGameOver())) {
        emit gameEndedWithMessage("Crashed! Restart to continue. Passenger is unhappy.");
        qDebug() << "Collision occurred, game paused for restart.";
    }
    // Если рейтинг или кошелек упали ниже плинтуса, игра завершится фатально через setRating/deductMoney
}

// Используем константу RATING_PENALTY_REJECT
void GameManager::decreaseRatingOnReject() {
    qDebug() << "Decreasing rating due to order rejection.";
    setRating(rating - RATING_PENALTY_REJECT); // Снижение рейтинга на 0.02
}

void GameManager::resumeGame() {
    // Вызывается из MainWindow, когда игрок принимает заказ или закрывает оверлей
    paused = false;
    qDebug() << "GameManager::resumeGame called, internal paused flag set to false.";
}

void GameManager::addMoney(double amount) {
    if (amount > 0) {
        wallet += amount;
        wallet = round(wallet * 100.0) / 100.0; // Округляем до 2 знаков
        qDebug() << "Wallet updated: +" << amount << " AZN. New balance:" << wallet;
        emit walletUpdated(wallet); // Обновляем UI кошелька
        checkTurboAvailability(); // Проверяем, можно ли купить турбо
    }
}

void GameManager::deductMoney(double amount) {
    if (amount > 0) {
        wallet -= amount;
        wallet = round(wallet * 100.0) / 100.0; // Округляем до 2 знаков
        qDebug() << "Wallet updated: -" << amount << " AZN. New balance:" << wallet;
        emit walletUpdated(wallet); // Обновляем UI кошелька
        checkTurboAvailability(); // Проверяем доступность турбо
        checkDebtLimit(); // Проверяем, не ушел ли в слишком большой минус
    }
}

void GameManager::checkDebtLimit() {
    // Проверка на Game Over из-за долгов
    if (wallet < MIN_WALLET_GAME_OVER && !(mainWindow && mainWindow->isGameOver())) {
        emit gameEndedWithMessage("Ахмед пришел возвращать долги, ваш приус сломали и вы остаетесь без машины");
        qDebug() << "Game Over: Debt exceeded " << MIN_WALLET_GAME_OVER << " AZN.";
    }
}

void GameManager::checkTurboAvailability() {
    if (prius) {
        bool canAfford = wallet >= TURBO_COST; // Хватает ли денег
        bool alreadyActive = prius->isTurboActive(); // Уже куплено?
        bool available = canAfford && !alreadyActive; // Доступно, если хватает денег и еще не куплено
        emit turboAvailabilityChanged(available); // Обновляем состояние кнопки в UI
    } else {
        emit turboAvailabilityChanged(false); // Если машины нет, турбо недоступно
    }
}

void GameManager::purchaseTurboUpgrade() {
    qDebug() << "GM Slot: purchaseTurboUpgrade called.";
    // Проверяем, есть ли машина, не активно ли турбо и хватает ли денег
    if (prius && !prius->isTurboActive() && wallet >= TURBO_COST) {
        qDebug() << "Attempting to purchase Turbo...";
        deductMoney(TURBO_COST); // Списываем деньги (это вызовет checkTurboAvailability и обновит кнопку)
        prius->activateTurbo(); // Активируем турбо в машине
        qDebug() << "Turbo Upgrade Purchased!";
    } else {
        qDebug() << "Cannot purchase Turbo. Prius exists:" << (prius != nullptr)
        << " Turbo Active:" << (prius ? prius->isTurboActive() : -1)
        << " Wallet:" << wallet;
    }
}

// --- Слоты диалога ---
void GameManager::onPlayerInitiatedDialog() {
    qDebug() << "GM Slot: onPlayerInitiatedDialog called.";
    // Нельзя начать диалог, если он уже использован, нет заказа или игра на паузе
    if (dialogStartUsed || !hasActiveOrder() || paused ) {
        qDebug() << "GM Slot: Cannot start dialog now (Used/NoOrder/Paused):"
                 << dialogStartUsed << !hasActiveOrder() << paused;
        return;
    }
    dialogStartUsed = true; // Отмечаем использование
    dialogInteractionStarted = true; // Флаг, что диалог начат (влияет на коммент в конце)
    QString driverLine = "Блин, эта работа в такси так надоела";
    emit updateDialogDisplayHtml(QString("<font color=\"red\">Я: %1</font>").arg(driverLine)); // Показываем реплику водителя
    emit updateDialogButtonStates(false, false, false); // Блокируем все кнопки диалога

    // Ответ пассажира через 1.5 секунды
    QTimer::singleShot(1500, this, [this](){
        if (!hasActiveOrder()) return; // Если заказ закончился за это время, выходим
        int chance = QRandomGenerator::global()->bounded(100); // Случайный шанс 0-99
        QString passengerResponse;
        bool enableOthers; // Флаг, разблокировать ли другие кнопки
        if (chance < 20) { // 20% шанс негативной реакции
            passengerReceptive = false; // Пассажир не настроен на диалог
            passengerResponse = "Мне это не интересно";
            enableOthers = false; // Другие кнопки не разблокируем
            qDebug() << "GM: Passenger response - Not Interested";
            dialogInteractionStarted = false; // Считаем диалог проваленным, сбрасываем флаг
            // Можно вернуть кнопку "Начать диалог", если хотим дать еще шанс (но пока не будем)
            // emit updateDialogButtonStates(true, false, false); // Вернуть кнопку "Начать" ?
        } else { // 80% шанс позитивной/нейтральной реакции
            passengerReceptive = true; // Пассажир готов слушать дальше
            passengerResponse = "Понимаю";
            enableOthers = true; // Разблокируем кнопки "История" и "Пожаловаться"
            qDebug() << "GM: Passenger response - Interested";
        }
        // Показываем ответ пассажира
        emit updateDialogDisplayHtml(QString("<font color=\"white\">Пассажир: %1</font>").arg(passengerResponse));
        // Обновляем состояние кнопок (разблокируем другие, если enableOthers = true)
        emit updateDialogButtonStates(false, enableOthers && !dialogStoryUsed, enableOthers && !dialogComplainUsed);
    });
}

void GameManager::onPlayerToldStory() {
    qDebug() << "GM Slot: onPlayerToldStory called.";
    // Нельзя рассказать историю, если она уже использована, диалог не начат успешно, пассажир не восприимчив, нет заказа или пауза
    if (dialogStoryUsed || !dialogInteractionStarted || !passengerReceptive || !hasActiveOrder() || paused) {
        qDebug() << "GM Slot: Cannot tell story now. State:" << dialogStoryUsed << !dialogInteractionStarted << !passengerReceptive << !hasActiveOrder() << paused;
        return;
    }
    dialogStoryUsed = true; // Отмечаем использование
    QString driverLine = "Вообще-то у меня есть свой бизнес на Рублевке, такси это просто хобби";
    emit updateDialogDisplayHtml(QString("<font color=\"red\">Я: %1</font>").arg(driverLine)); // Показываем реплику
    // Обновляем кнопки: блокируем "Историю", оставляем "Пожаловаться", если доступна
    emit updateDialogButtonStates(false, false, !dialogComplainUsed && passengerReceptive);
    // Ответ пассажира через 0.8 секунды
    QTimer::singleShot(800, this, [this](){
        if (!hasActiveOrder()) return;
        QString passengerResponse = "Очень круто, что у вас есть страсть!";
        emit updateDialogDisplayHtml(QString("<font color=\"white\">Пассажир: %1</font>").arg(passengerResponse));
    });
}

void GameManager::onPlayerComplained() {
    qDebug() << "GM Slot: onPlayerComplained called.";
    // Нельзя пожаловаться, если уже использовано, диалог не начат успешно, пассажир не восприимчив, нет заказа или пауза
    if (dialogComplainUsed || !dialogInteractionStarted || !passengerReceptive || !hasActiveOrder() || paused) {
        qDebug() << "GM Slot: Cannot complain now. State:" << dialogComplainUsed << !dialogInteractionStarted << !passengerReceptive << !hasActiveOrder() << paused;
        return;
    }
    dialogComplainUsed = true; // Отмечаем использование
    QString driverLine = "Все машины на дороге такие твари, не люблю водить машину";
    emit updateDialogDisplayHtml(QString("<font color=\"red\">Я: %1</font>").arg(driverLine)); // Показываем реплику
    // Обновляем кнопки: блокируем "Пожаловаться", оставляем "Историю", если доступна
    emit updateDialogButtonStates(false, !dialogStoryUsed && passengerReceptive, false);
    // Ответ пассажира через 0.8 секунды
    QTimer::singleShot(800, this, [this](){
        if (!hasActiveOrder()) return;
        QString passengerResponse = "Тяжелая у вас работа...";
        emit updateDialogDisplayHtml(QString("<font color=\"white\">Пассажир: %1</font>").arg(passengerResponse));
    });
}
