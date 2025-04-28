#include "mainwindow.h"
#include "gameview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QDebug>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QApplication>
#include <QtMath>
#include <QGraphicsRectItem>
#include <QProgressBar>
#include <QLabel>
#include <QPixmap>
#include <QTextEdit>
#include <QFrame>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),

    scene(nullptr),
    view(nullptr),
    prius(nullptr),

    gameManager(nullptr),
    centralWidget(nullptr),
    restartButton(nullptr),
    commentDisplayArea(nullptr),
    bottomLeftStatsWidget(nullptr),
    walletLabel(nullptr),
    completedOrdersLabel(nullptr),
    distanceLabel(nullptr),
    totalDistanceLabel(nullptr),
    leftStatusPanelWidget(nullptr),
    speedBar(nullptr),
    turboBar(nullptr),
    fuelBar(nullptr),
    topRightStatusPanelWidget(nullptr),
    ratingIconLabel(nullptr),
    ratingLabel(nullptr),
    orderOverlayWidget(nullptr),
    orderOverlayLabel(nullptr),
    acceptButton(nullptr),
    rejectButton(nullptr),
    dialogButtonContainer(nullptr),
    startDialogButton(nullptr),
    tellStoryButton(nullptr),
    complainButton(nullptr),
    turboUpgradeButton(nullptr),

    gameOver(false),
    gamePaused(false),
    obstaclesActive(false),
    lastFuelCanSpawnDistance(0.0),


    movementTimer(nullptr),
    spawnTimer(nullptr),
    fuelSpawnTimer(nullptr)
{
    qDebug() << "Initializing MainWindow...";



    QString backgroundImagePath = "C:/Users/iskender/Documents/Crazy_Prius_Simulator/pictures/sky_background.jpg";

    this->setStyleSheet(QString("MainWindow { border-image: url(%1) 0 0 0 0 stretch stretch; }")
                            .arg(backgroundImagePath));

    setupUI();
    setupDialogButtons();
    setupUpgradeButton();
    qDebug() << "UI setup complete.";

    qDebug() << "Creating Prius...";
    prius = new Prius();
    qDebug() << "Prius created.";


    gameManager = new GameManager(this, prius, this);
    gameManager->setPrius(prius);

    qDebug() << "Setting up connections...";


    connect(gameManager, &GameManager::distanceUpdated, this, &MainWindow::updateDistanceLabel);
    connect(gameManager, &GameManager::ratingUpdated, this, &MainWindow::updateRatingLabel);
    connect(gameManager, &GameManager::fuelUpdated, this, &MainWindow::updateFuelBar);
    connect(gameManager, &GameManager::totalDistanceUpdated, this, &MainWindow::updateTotalDistanceLabel);
    connect(gameManager, &GameManager::walletUpdated, this, &MainWindow::updateWalletLabel);
    connect(gameManager, &GameManager::updateCompletedOrdersCount, this, &MainWindow::updateCompletedOrdersLabel);
    connect(gameManager, &GameManager::showOrder, this, &MainWindow::showOrderOverlay);
    connect(gameManager, &GameManager::turboAvailabilityChanged, this, &MainWindow::updateTurboButton);


    connect(gameManager, &GameManager::pauseGame, this, &MainWindow::pauseGame);
    connect(gameManager, &GameManager::pauseForNewPassenger, this, &MainWindow::pauseGame);
    connect(gameManager, &GameManager::gameEndedWithMessage, this, &MainWindow::showGameOverMessage);
    connect(gameManager, &GameManager::startObstacles, this, &MainWindow::startObstacles);
    connect(gameManager, &GameManager::newPassenger, this, &MainWindow::showNewPassengerMessage);


    connect(gameManager, &GameManager::updateDialogDisplayHtml, this, &MainWindow::appendDialogText);
    connect(gameManager, &GameManager::updateDialogButtonStates, this, &MainWindow::updateDialogButtons);


    if (restartButton) {
        connect(restartButton, &QPushButton::clicked, this, &MainWindow::restartGame);
    } else {
        qDebug() << "Error: restartButton is null after setupUI!";
    }


    movementTimer = new QTimer(this);
    connect(movementTimer, &QTimer::timeout, this, &MainWindow::updateMovement);
    spawnTimer = new QTimer(this);
    connect(spawnTimer, &QTimer::timeout, this, &MainWindow::spawnObstacleCar);
    fuelSpawnTimer = new QTimer(this);
    connect(fuelSpawnTimer, &QTimer::timeout, this, &MainWindow::spawnFuelCan);

    setFocusPolicy(Qt::StrongFocus);
    qDebug() << "MainWindow constructor finished.";
}

MainWindow::~MainWindow() {
    qDebug() << "Destroying MainWindow...";


}

void MainWindow::setupDialogButtons() {

    if (!centralWidget) {
        qDebug() << "Error: centralWidget is null in setupDialogButtons!";
        return;
    }
    dialogButtonContainer = new QWidget(centralWidget);
    dialogButtonContainer->setStyleSheet("background-color: rgba(0, 0, 0, 0); border: none;");
    startDialogButton = new QPushButton("Начать диалог", dialogButtonContainer);
    tellStoryButton = new QPushButton("Рассказать историю", dialogButtonContainer);
    complainButton = new QPushButton("Пожаловаться", dialogButtonContainer);
    QString buttonStyle = "QPushButton { font-size: 10pt; padding: 5px; background-color: #555; color: white; border-radius: 4px; margin: 1px; } QPushButton:hover { background-color: #777; } QPushButton:disabled { background-color: #333; color: #888; }";
    startDialogButton->setStyleSheet(buttonStyle);
    tellStoryButton->setStyleSheet(buttonStyle);
    complainButton->setStyleSheet(buttonStyle);
    QVBoxLayout *buttonLayout = new QVBoxLayout(dialogButtonContainer);
    buttonLayout->setContentsMargins(0, 2, 0, 0);
    buttonLayout->setSpacing(3);
    buttonLayout->addWidget(startDialogButton);
    buttonLayout->addWidget(tellStoryButton);
    buttonLayout->addWidget(complainButton);
    dialogButtonContainer->setLayout(buttonLayout);
    dialogButtonContainer->setVisible(true);
    startDialogButton->setEnabled(true);
    tellStoryButton->setEnabled(false);
    complainButton->setEnabled(false);
    connect(startDialogButton, &QPushButton::clicked, this, &MainWindow::onStartDialogClicked);
    connect(tellStoryButton, &QPushButton::clicked, this, &MainWindow::onTellStoryClicked);
    connect(complainButton, &QPushButton::clicked, this, &MainWindow::onComplainClicked);
    qDebug() << "Dialog buttons UI created.";
}

void MainWindow::setupUpgradeButton() {

    if (!centralWidget) {
        qDebug() << "Error: centralWidget is null in setupUpgradeButton!";
        return;
    }
    turboUpgradeButton = new QPushButton("Turbo Prius (5 AZN)", centralWidget);
    turboUpgradeButton->setStyleSheet("QPushButton { font-size: 9pt; padding: 5px; background-color: #0055A4; color: white; border-radius: 4px; margin: 1px; } QPushButton:hover { background-color: #0077CC; } QPushButton:disabled { background-color: #333; color: #888; }");
    turboUpgradeButton->setEnabled(false);
    turboUpgradeButton->setVisible(true);
    connect(turboUpgradeButton, &QPushButton::clicked, this, &MainWindow::onTurboUpgradeClicked);
    qDebug() << "Turbo Upgrade button created.";
}

void MainWindow::setupSpeedPanel() {

    if (!centralWidget) {
        qDebug() << "Error: centralWidget is null in setupSpeedPanel!";
        return;
    }
    leftStatusPanelWidget = new QWidget(centralWidget);
    leftStatusPanelWidget->setStyleSheet("background-color: rgba(0, 0, 0, 170); border-radius: 5px;");

    QHBoxLayout* speedPanelLayout = new QHBoxLayout();
    speedPanelLayout->setContentsMargins(0, 0, 0, 0);
    speedPanelLayout->setSpacing(2);

    speedBar = new QProgressBar(leftStatusPanelWidget);
    speedBar->setOrientation(Qt::Vertical);
    speedBar->setRange(0, 120);
    speedBar->setValue(0);
    speedBar->setTextVisible(true);
    speedBar->setFormat("%v");
    speedBar->setMinimumHeight(180);
    speedBar->setFixedWidth(45);
    speedBar->setStyleSheet("QProgressBar { border: 1px solid grey; border-radius: 4px; text-align: center; background-color: rgba(50,50,50,180); color: white; } QProgressBar::chunk:vertical { background-color: yellow; border-radius: 3px; margin: 1px; }");

    turboBar = new QProgressBar(leftStatusPanelWidget);
    turboBar->setOrientation(Qt::Vertical);
    turboBar->setRange(0, 60);
    turboBar->setValue(0);
    turboBar->setTextVisible(false);
    turboBar->setMinimumHeight(180);
    turboBar->setFixedWidth(10);
    turboBar->setStyleSheet("QProgressBar { border: 1px solid #444; border-radius: 3px; background-color: rgba(50,50,50,180); } QProgressBar::chunk:vertical { background-color: cyan; border-radius: 2px; margin: 1px; }");
    turboBar->setVisible(false);

    speedPanelLayout->addWidget(speedBar);
    speedPanelLayout->addWidget(turboBar);

    QWidget* speedBarsContainer = new QWidget(leftStatusPanelWidget);
    speedBarsContainer->setLayout(speedPanelLayout);

    fuelBar = new QProgressBar(leftStatusPanelWidget);
    fuelBar->setRange(0, 100);
    fuelBar->setValue(100);
    fuelBar->setTextVisible(true);
    fuelBar->setFormat("F:%p%");
    fuelBar->setFixedHeight(30);
    fuelBar->setStyleSheet("QProgressBar { border: 1px solid grey; border-radius: 4px; text-align: center; background-color: rgba(50,50,50,180); color: white; font-size: 10pt; } QProgressBar::chunk { background-color: red; border-radius: 3px; margin: 1px; }");

    QVBoxLayout* mainLeftPanelLayout = new QVBoxLayout(leftStatusPanelWidget);
    mainLeftPanelLayout->setContentsMargins(5, 5, 5, 5);
    mainLeftPanelLayout->setSpacing(8);
    mainLeftPanelLayout->addWidget(speedBarsContainer);
    mainLeftPanelLayout->addWidget(fuelBar);
    mainLeftPanelLayout->addStretch();

    leftStatusPanelWidget->setLayout(mainLeftPanelLayout);
    leftStatusPanelWidget->adjustSize();
}


void MainWindow::setupUI() {
    qDebug() << "Setting up UI...";
    centralWidget = new QWidget(this);

    centralWidget->setStyleSheet("background: transparent;");

    this->setCentralWidget(centralWidget);

    scene = new QGraphicsScene(0, 0, 400, 1000, this);


    view = new GameView(scene, centralWidget);

    view->setStyleSheet("background: transparent; border: none;");







    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(view, 1);


    setupSpeedPanel();
    setupOrderOverlay();


    topRightStatusPanelWidget = new QWidget(centralWidget);
    topRightStatusPanelWidget->setStyleSheet("background-color: rgba(0, 0, 0, 170); border-radius: 5px; padding: 2px;");
    ratingIconLabel = new QLabel(topRightStatusPanelWidget);

    QString starIconPath = "C:/Users/iskender/Documents/Crazy_Prius_Simulator/pictures/star.jpg";
    QPixmap starPixmap(starIconPath);
    if (!starPixmap.isNull()) {
        ratingIconLabel->setPixmap(starPixmap.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        qDebug() << "Warning: Failed to load star icon from" << starIconPath << ". Using text fallback.";
        ratingIconLabel->setText("*");
        ratingIconLabel->setStyleSheet("color: yellow; font-size: 16pt; font-weight: bold; background: transparent; border: none;");
    }
    ratingLabel = new QLabel("...", topRightStatusPanelWidget);
    ratingLabel->setStyleSheet("color: white; font-size: 11pt; background-color: transparent; border: none; padding-left: 4px;");
    QHBoxLayout *topRightLayout = new QHBoxLayout(topRightStatusPanelWidget);
    topRightLayout->setContentsMargins(4, 4, 4, 4);
    topRightLayout->setSpacing(3);
    topRightLayout->addWidget(ratingIconLabel);
    topRightLayout->addWidget(ratingLabel);
    topRightStatusPanelWidget->setLayout(topRightLayout);
    topRightStatusPanelWidget->adjustSize();


    bottomLeftStatsWidget = new QWidget(centralWidget);
    bottomLeftStatsWidget->setStyleSheet("background-color: rgba(0, 0, 0, 170); border-radius: 5px; padding: 5px;");
    walletLabel = new QLabel("Wallet: ...", bottomLeftStatsWidget);
    completedOrdersLabel = new QLabel("Orders: ...", bottomLeftStatsWidget);
    distanceLabel = new QLabel("Dist: ...", bottomLeftStatsWidget);
    totalDistanceLabel = new QLabel("Total: ...", bottomLeftStatsWidget);
    QVBoxLayout *bottomLeftStatsLayout = new QVBoxLayout(bottomLeftStatsWidget);
    bottomLeftStatsLayout->setContentsMargins(4, 4, 4, 4);
    bottomLeftStatsLayout->setSpacing(2);
    bottomLeftStatsLayout->addWidget(walletLabel);
    bottomLeftStatsLayout->addWidget(completedOrdersLabel);
    bottomLeftStatsLayout->addWidget(distanceLabel);
    bottomLeftStatsLayout->addWidget(totalDistanceLabel);

    QString statsLabelStyle = "QLabel { font-size: 10pt; color: white; background-color: transparent; border: none; padding: 1px; }";
    walletLabel->setStyleSheet(statsLabelStyle);
    completedOrdersLabel->setStyleSheet(statsLabelStyle);
    distanceLabel->setStyleSheet(statsLabelStyle);
    totalDistanceLabel->setStyleSheet(statsLabelStyle);

    walletLabel->setAlignment(Qt::AlignLeft);
    completedOrdersLabel->setAlignment(Qt::AlignLeft);
    distanceLabel->setAlignment(Qt::AlignLeft);
    totalDistanceLabel->setAlignment(Qt::AlignLeft);
    bottomLeftStatsWidget->setLayout(bottomLeftStatsLayout);
    bottomLeftStatsWidget->adjustSize();


    commentDisplayArea = new QTextEdit(centralWidget);
    commentDisplayArea->setReadOnly(true);
    commentDisplayArea->setStyleSheet("background-color: rgba(0, 0, 0, 170); border-radius: 5px; padding: 5px; color: white; font-size: 10pt; border: 1px solid #444;");
    commentDisplayArea->clear();


    restartButton = new QPushButton("Restart", centralWidget);
    restartButton->setVisible(false);
    restartButton->setMinimumHeight(30);
    restartButton->setStyleSheet("QPushButton { font-size: 12pt; padding: 5px; background-color: #C00000; color: white; border-radius: 5px; } QPushButton:hover { background-color: #E00000; }");



    qDebug() << "UI setup finished.";
}

void MainWindow::setupOrderOverlay() {

    qDebug() << "Setting up Order Overlay UI...";
    if (!centralWidget) { qDebug() << "Error: centralWidget is null in setupOrderOverlay!"; return; }
    orderOverlayWidget = new QWidget(centralWidget);
    orderOverlayWidget->setStyleSheet("background-color: rgba(0, 0, 0, 180); border-radius: 8px; border: 1px solid #555;");
    orderOverlayWidget->setFixedSize(280, 150);
    QVBoxLayout *overlayLayout = new QVBoxLayout(orderOverlayWidget);
    overlayLayout->setContentsMargins(10, 10, 10, 10);
    overlayLayout->setSpacing(6);
    orderOverlayLabel = new QLabel("Order: ...", orderOverlayWidget);
    orderOverlayLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    orderOverlayLabel->setStyleSheet("color: white; font-size: 10pt; background-color: transparent; border: none;");
    orderOverlayLabel->setWordWrap(true);
    overlayLayout->addWidget(orderOverlayLabel, 1);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    acceptButton = new QPushButton("Accept", orderOverlayWidget);
    rejectButton = new QPushButton("Reject", orderOverlayWidget);

    acceptButton->setStyleSheet("QPushButton { font-size: 10pt; padding: 6px; background-color: #00A000; color: white; border-radius: 5px; } QPushButton:hover { background-color: #00C000; }");
    rejectButton->setStyleSheet("QPushButton { font-size: 10pt; padding: 6px; background-color: #C00000; color: white; border-radius: 5px; } QPushButton:hover { background-color: #E00000; }");

    connect(acceptButton, &QPushButton::clicked, this, &MainWindow::acceptOrderClicked);
    connect(rejectButton, &QPushButton::clicked, this, &MainWindow::rejectOrderClicked);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(rejectButton);
    overlayLayout->addLayout(buttonLayout);
    orderOverlayWidget->setLayout(overlayLayout);
    orderOverlayWidget->setVisible(false);
    qDebug() << "Order Overlay UI created.";
}

void MainWindow::positionManuallyPlacedWidgets() {


    if (!centralWidget || !view) return;
    int margin = 10;


    if (orderOverlayWidget) {
        orderOverlayWidget->move(margin, margin);
        orderOverlayWidget->raise();
    }


    if (leftStatusPanelWidget) {
        int yPos = margin;
        if (orderOverlayWidget && orderOverlayWidget->isVisible()) {
            yPos = orderOverlayWidget->geometry().bottom() + margin;
        }
        leftStatusPanelWidget->adjustSize();
        int panelHeight = leftStatusPanelWidget->height();

        if (yPos + panelHeight > centralWidget->height() - margin) {
            yPos = centralWidget->height() - panelHeight - margin;
        }
        if (yPos < margin) yPos = margin;
        leftStatusPanelWidget->move(margin, yPos);
        leftStatusPanelWidget->raise();
        leftStatusPanelWidget->setVisible(true);
    }


    if (topRightStatusPanelWidget) {
        topRightStatusPanelWidget->adjustSize();
        int panelWidth = topRightStatusPanelWidget->width();
        topRightStatusPanelWidget->move(centralWidget->width() - panelWidth - margin, margin);
        topRightStatusPanelWidget->raise();
        topRightStatusPanelWidget->setVisible(true);
    }


    int currentBottomY = centralWidget->height() - margin;


    if(restartButton && restartButton->isVisible()) {
        restartButton->adjustSize();
        int buttonWidth = restartButton->width(); if (buttonWidth <= 0) buttonWidth = 100;
        int buttonHeight = restartButton->height(); if (buttonHeight <= 0) buttonHeight = 30;
        int restartY = centralWidget->height() - buttonHeight - margin;
        restartButton->move((centralWidget->width() - buttonWidth) / 2, restartY);
        restartButton->raise();
        currentBottomY = restartY - margin;
    }


    if (bottomLeftStatsWidget) {
        bottomLeftStatsWidget->adjustSize();
        int panelHeight = bottomLeftStatsWidget->height();
        int panelY = currentBottomY - panelHeight;
        bottomLeftStatsWidget->move(margin, panelY);
        bottomLeftStatsWidget->raise();
        bottomLeftStatsWidget->setVisible(true);
        currentBottomY = panelY - 5;
    }


    if (turboUpgradeButton) {
        turboUpgradeButton->adjustSize();
        int buttonHeight = turboUpgradeButton->height();
        int buttonY = currentBottomY - buttonHeight;

        int topLimitY = margin;
        if (leftStatusPanelWidget && leftStatusPanelWidget->isVisible()) {
            topLimitY = leftStatusPanelWidget->geometry().bottom() + margin;
        }
        if (buttonY < topLimitY) {
            buttonY = topLimitY;
        }
        turboUpgradeButton->move(margin, buttonY);

        turboUpgradeButton->setFixedWidth(bottomLeftStatsWidget ? bottomLeftStatsWidget->width() : 120);
        turboUpgradeButton->raise();
        turboUpgradeButton->setVisible(true);
    }



    if (commentDisplayArea) {
        int panelWidth = 300;

        if (panelWidth > centralWidget->width() - margin * 3) panelWidth = centralWidget->width() - margin * 3;
        if (panelWidth < 100) panelWidth = 100;

        int xPos = centralWidget->width() - panelWidth - margin;
        int yPos = margin;

        if(topRightStatusPanelWidget && topRightStatusPanelWidget->isVisible()) yPos = topRightStatusPanelWidget->geometry().bottom() + margin;

        int buttonContainerHeight = 0;

        if(dialogButtonContainer) {
            dialogButtonContainer->setFixedWidth(panelWidth);
            buttonContainerHeight = dialogButtonContainer->sizeHint().height();
        }


        int bottomLimitYForComments = centralWidget->height() - margin;
        int commentAreaHeight = bottomLimitYForComments - yPos - buttonContainerHeight - 5;
        if (commentAreaHeight < 50) commentAreaHeight = 50;


        commentDisplayArea->setGeometry(xPos, yPos, panelWidth, commentAreaHeight);
        commentDisplayArea->raise(); commentDisplayArea->setVisible(true);


        if (dialogButtonContainer) {
            int buttonContainerY = yPos + commentAreaHeight + 5;
            dialogButtonContainer->setGeometry(xPos, buttonContainerY, panelWidth, buttonContainerHeight);
            dialogButtonContainer->raise(); dialogButtonContainer->setVisible(true);
        }
    } else if (dialogButtonContainer) {

        dialogButtonContainer->setVisible(false);
    }
}


void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    positionManuallyPlacedWidgets();
}

void MainWindow::showEvent(QShowEvent *event) {

    QMainWindow::showEvent(event);
    static bool startScheduled = false;
    if (!startScheduled && gameManager) {
        startScheduled = true;

        QTimer::singleShot(100, this, &MainWindow::delayedStartGame);
    }

    QTimer::singleShot(0, this, &MainWindow::positionManuallyPlacedWidgets);
    activateWindow();
    setFocus();
}

void MainWindow::delayedStartGame() {

    qDebug() << "Executing delayedStartGame()...";
    if(scene) {
        resetScene();
        qDebug() << "Scene reset done.";
    } else {
        qDebug() << "Error: scene is null in delayedStartGame!"; return;
    }
    if (prius && scene) {
        scene->addItem(prius);
        prius->setY(scene->height() * 0.75);
        scene->update();
        qDebug() << "Prius added to scene.";
    } else {
        qDebug() << "Error: prius or scene is null in delayedStartGame!"; return;
    }

    if(movementTimer && !movementTimer->isActive()) movementTimer->start(16);
    if(spawnTimer && !spawnTimer->isActive()) spawnTimer->start(2000);
    if(fuelSpawnTimer && !fuelSpawnTimer->isActive()) fuelSpawnTimer->start(100);
    qDebug() << "Timers started.";
    if (gameManager) {
        gameManager->startGame();
    } else {
        qDebug() << "Error: gameManager is null in delayedStartGame!"; return;
    }
    activateWindow(); setFocus(); qDebug() << "Focus set.";

    QTimer::singleShot(0, this, &MainWindow::positionManuallyPlacedWidgets);
}

void MainWindow::resetScene() {

    qDebug() << "Resetting scene visuals...";

    dashes.clear(); obstacleCars.clear(); fuelCans.clear();
    if (scene) {

        QList<QGraphicsItem*> items = scene->items();
        for (QGraphicsItem* item : items) {
            if (item == prius) continue;
            scene->removeItem(item);
            delete item;
        }
        qDebug() << "Old items cleared from scene.";


        qreal sceneHeight = scene->sceneRect().height();
        qreal roadWidth = 300;
        qreal roadX = (scene->width() - roadWidth) / 2.0;

        QGraphicsRectItem *road = new QGraphicsRectItem(roadX, 0, roadWidth, sceneHeight);
        road->setBrush(QColor(80, 80, 80));
        road->setPen(Qt::NoPen);
        road->setZValue(-1);
        scene->addItem(road);


        qreal dashLength = 40; qreal gapLength = 30;
        qreal totalSegmentLength = dashLength + gapLength;

        int numDashPairsNeeded = qCeil((sceneHeight * 2.0 + totalSegmentLength) / totalSegmentLength);
        qreal totalPatternHeight = numDashPairsNeeded * totalSegmentLength;
        qreal line1X = roadX + roadWidth / 3.0;
        qreal line2X = roadX + 2.0 * roadWidth / 3.0;
        qreal dashWidth = 5;

        qreal startY = sceneHeight - totalPatternHeight;

        for (int i = 0; i < numDashPairsNeeded; ++i) {
            qreal yPos = startY + i * totalSegmentLength;

            QGraphicsRectItem *dashL = new QGraphicsRectItem(line1X - dashWidth / 2, yPos, dashWidth, dashLength);
            dashL->setBrush(Qt::white); dashL->setPen(Qt::NoPen); dashL->setZValue(0);
            scene->addItem(dashL); dashes.append(dashL);

            QGraphicsRectItem *dashR = new QGraphicsRectItem(line2X - dashWidth / 2, yPos, dashWidth, dashLength);
            dashR->setBrush(Qt::white); dashR->setPen(Qt::NoPen); dashR->setZValue(0);
            scene->addItem(dashR); dashes.append(dashR);
        }
        qDebug() << "Road and dashes recreated.";
    } else {
        qDebug() << "Error: Scene is null in resetScene()!";
    }
}


void MainWindow::keyPressEvent(QKeyEvent *event) {

    if (gameOver) {
        if (event->key() == Qt::Key_Escape) QApplication::quit();
        return;
    }
    if (event->key() == Qt::Key_Escape) QApplication::quit();


    if (gamePaused || !prius) return;


    switch (event->key()) {
    case Qt::Key_Up: case Qt::Key_W: prius->accelerate(); break;
    case Qt::Key_Down: case Qt::Key_S: prius->brake(); break;
    case Qt::Key_Left: case Qt::Key_A: prius->moveLeft(); break;
    case Qt::Key_Right: case Qt::Key_D: prius->moveRight(); break;
    default: QMainWindow::keyPressEvent(event); return;
    }
}

void MainWindow::updateMovement() {


    if (gameOver || gamePaused || !prius || !view || !scene) return;

    int speed = prius->getSpeed();
    updateSpeedBar(speed, prius->isTurboActive());


    qreal roadSpeedFactor = 0.07;
    qreal deltaY = speed * roadSpeedFactor;


    int numDashPairs = dashes.count() / 2;
    if (numDashPairs > 0) {
        qreal dashLength = 40; qreal gapLength = 30;
        qreal totalSegmentLength = dashLength + gapLength;
        qreal totalPatternHeight = numDashPairs * totalSegmentLength;
        qreal sceneHeight = scene->sceneRect().height();
        for (QGraphicsRectItem *dash : dashes) {
            dash->moveBy(0, deltaY);

            if (dash->sceneBoundingRect().top() >= sceneHeight) {
                if (totalPatternHeight > 0) dash->moveBy(0, -totalPatternHeight);
                else qDebug() << "Error: totalPatternHeight is zero!";
            }
        }
    }


    if (obstaclesActive) {
        qreal sceneHeight = scene->sceneRect().height();


        QMutableListIterator<ObstacleCar*> carIter(obstacleCars);
        while (carIter.hasNext()) {
            ObstacleCar *car = carIter.next();
            car->move(speed);


            if (prius->collidesWithItem(car)) {
                if(gameManager) gameManager->onCollision();
                return;
            }


            if (car->sceneBoundingRect().top() > sceneHeight + 50) {
                scene->removeItem(car); carIter.remove(); delete car;
            }
        }


        QMutableListIterator<FuelCan*> canIter(fuelCans);
        while (canIter.hasNext()) {
            FuelCan *can = canIter.next();
            can->move(speed);


            if (prius->collidesWithItem(can)) {
                if(gameManager) gameManager->resetFuelDistance();
                scene->removeItem(can); canIter.remove(); delete can;
                continue;
            }


            if (can->sceneBoundingRect().top() > sceneHeight + 50) {
                scene->removeItem(can); canIter.remove(); delete can;
            }
        }
    }


    prius->setY(scene->height() * 0.75);
}


void MainWindow::spawnObstacleCar() {


    if (!obstaclesActive || gameOver || gamePaused || !scene) return;

    int maxObstaclesOnScreen = 3;
    if (obstacleCars.size() >= maxObstaclesOnScreen) return;


    if (QRandomGenerator::global()->bounded(100) < 70) return;

    int lane = QRandomGenerator::global()->bounded(3);
    qreal spawnY = -100.0;


    bool isLaneFreeNearSpawn = true;
    for (ObstacleCar *existingCar : obstacleCars) {

        if (existingCar->lane() == lane && qAbs(existingCar->y() - spawnY) < existingCar->boundingRect().height() * 2.5) {
            isLaneFreeNearSpawn = false; break;
        }
    }

    if (isLaneFreeNearSpawn) {
        ObstacleCar *car = new ObstacleCar(lane);
        scene->addItem(car);
        obstacleCars.append(car);
    }
}

void MainWindow::spawnFuelCan() {


    if (gameOver || gamePaused || !gameManager || !scene) return;

    double totalDistance = gameManager->getTotalDistanceDriven();

    if (totalDistance - lastFuelCanSpawnDistance >= 1000 && fuelCans.isEmpty()) {
        int lane = QRandomGenerator::global()->bounded(3);
        FuelCan *can = new FuelCan(lane);
        scene->addItem(can);
        fuelCans.append(can);
        lastFuelCanSpawnDistance = totalDistance;
        qDebug() << "Spawned FuelCan in lane:" << lane;
    }
}

void MainWindow::startObstacles() { obstaclesActive = true; qDebug() << "Obstacles activated!"; }

void MainWindow::restartGame() {
    qDebug() << "Restarting game...";
    if (!gameManager || !scene) { qDebug() << "Error: GM or Scene null!"; return; }
    gameOver = false; gamePaused = false; obstaclesActive = false;
    lastFuelCanSpawnDistance = 0.0;

    if(restartButton) restartButton->setVisible(false);

    if(speedBar) speedBar->setValue(0);
    if(turboBar) { turboBar->setValue(0); turboBar->setVisible(false); }
    if(speedBar) speedBar->setStyleSheet("QProgressBar { border: 1px solid grey; border-radius: 4px; text-align: center; background-color: rgba(50,50,50,180); color: white; } QProgressBar::chunk:vertical { background-color: yellow; border-radius: 3px; margin: 1px; }");
    if(turboUpgradeButton) turboUpgradeButton->setEnabled(false);
    clearCommentArea();


    if (prius) { scene->removeItem(prius); delete prius; prius = nullptr; }
    resetScene();
    prius = new Prius();
    scene->addItem(prius); prius->setY(scene->height() * 0.75); scene->update();
    gameManager->setPrius(prius);

    gameManager->resetForRestart();


    if(gameManager) {
        if(fuelBar) updateFuelBar(100.0);
        if(ratingLabel) updateRatingLabel(gameManager->getRating());
        if(walletLabel) updateWalletLabel(gameManager->getWallet());

        if(completedOrdersLabel) updateCompletedOrdersLabel(gameManager->getCompletedOrders());
        if(distanceLabel) updateDistanceLabel(0, 0);
        if(totalDistanceLabel) updateTotalDistanceLabel(gameManager->getTotalDistanceDriven());
    }


    if (movementTimer && !movementTimer->isActive()) movementTimer->start(16);
    if (spawnTimer && !spawnTimer->isActive()) spawnTimer->start(2000);
    if (fuelSpawnTimer && !fuelSpawnTimer->isActive()) fuelSpawnTimer->start(100);

    qDebug() << "Game restarted.";
    gameManager->startGame();
    activateWindow(); setFocus();
    QTimer::singleShot(0, this, &MainWindow::positionManuallyPlacedWidgets);
}


void MainWindow::updateDistanceLabel(double current, int total) {
    if(distanceLabel) distanceLabel->setText(QString("Dist: %1/%2 m").arg(static_cast<int>(current)).arg(total));
}
void MainWindow::updateRatingLabel(double rating) {
    if(ratingLabel) ratingLabel->setText(QString("%1").arg(rating, 0, 'f', 2));
}
void MainWindow::updateCompletedOrdersLabel(int count) {
    if(completedOrdersLabel) completedOrdersLabel->setText(QString("Orders: %1").arg(count));
}
void MainWindow::updateTotalDistanceLabel(double totalDistance) {
    if(totalDistanceLabel) totalDistanceLabel->setText(QString("Total: %1 m").arg(static_cast<int>(totalDistance)));
}
void MainWindow::updateFuelBar(double fuelPercentage) {
    if(fuelBar) fuelBar->setValue(qBound(0, static_cast<int>(fuelPercentage), 100));
}
void MainWindow::updateWalletLabel(double newWallet) {
    if(walletLabel) walletLabel->setText(QString("Wallet: %1 AZN").arg(newWallet, 0, 'f', 2));
}


void MainWindow::updateSpeedBar(int speed, bool turboActive) {

    if (speedBar) {
        speedBar->setValue(qBound(0, speed, 120));

        speedBar->setStyleSheet(QString("QProgressBar { border: 1px solid grey; border-radius: 4px; text-align: center; background-color: rgba(50,50,50,180); color: white; } QProgressBar::chunk:vertical { background-color: %1; border-radius: 3px; margin: 1px; }")
                                    .arg(turboActive ? "orange" : "yellow"));
    }
    if (turboBar) {
        if (turboActive) {
            turboBar->setValue(qBound(0, speed - 120, 60));
            turboBar->setVisible(true);
        } else {
            turboBar->setValue(0);
            turboBar->setVisible(false);
        }
    }
}



void MainWindow::showNewPassengerMessage() { qDebug() << "New passenger/order started in MainWindow."; }
void MainWindow::pauseGame() { if (!gameOver && !gamePaused) { gamePaused = true; qDebug() << "MainWindow: Game paused."; } }
void MainWindow::resumeGame() { if (gamePaused) { gamePaused = false; if (orderOverlayWidget) orderOverlayWidget->setVisible(false); activateWindow(); setFocus(); qDebug() << "MainWindow: Game resumed."; if(gameManager) gameManager->resumeGame(); } }


void MainWindow::onStartDialogClicked() { if (gameManager) QMetaObject::invokeMethod(gameManager, "onPlayerInitiatedDialog", Qt::QueuedConnection); }
void MainWindow::onTellStoryClicked() { if (gameManager) QMetaObject::invokeMethod(gameManager, "onPlayerToldStory", Qt::QueuedConnection); }
void MainWindow::onComplainClicked() { if (gameManager) QMetaObject::invokeMethod(gameManager, "onPlayerComplained", Qt::QueuedConnection); }
void MainWindow::updateDialogButtons(bool enableStart, bool enableStory, bool enableComplain) { if(startDialogButton) startDialogButton->setEnabled(enableStart); if(tellStoryButton) tellStoryButton->setEnabled(enableStory); if(complainButton) complainButton->setEnabled(enableComplain); }
void MainWindow::appendDialogText(const QString &htmlText) { if (commentDisplayArea) { commentDisplayArea->append(htmlText); } }
void MainWindow::clearCommentArea() { if (commentDisplayArea) { commentDisplayArea->clear(); qDebug() << "MainWindow: Comment area cleared."; } }


void MainWindow::showGameOverMessage(QString message) {

    qDebug() << "Game over message received:" << message;
    if (gameOver) return;
    gameOver = true; gamePaused = true;


    if (orderOverlayWidget) orderOverlayWidget->setVisible(false);
    if (commentDisplayArea) commentDisplayArea->setVisible(false);
    if (dialogButtonContainer) dialogButtonContainer->setVisible(false);
    if (turboUpgradeButton) turboUpgradeButton->setVisible(false);
    if (turboBar) turboBar->setVisible(false);
    if (movementTimer) movementTimer->stop();
    if (spawnTimer) spawnTimer->stop();
    if (fuelSpawnTimer) fuelSpawnTimer->stop();


    bool fatalGameOver = (message.contains("Ахмед") || message.contains("fired") || message.contains("закончилось топливо") || message.contains("насмерть"));

    if (!fatalGameOver && restartButton) {
        restartButton->setVisible(true); positionManuallyPlacedWidgets(); restartButton->setFocus();
    } else if(restartButton) {
        restartButton->setVisible(false);
    }


    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning); msgBox.setWindowTitle("Game Over"); msgBox.setText(message); msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();

    if (fatalGameOver) {
        QApplication::quit();
    }
}


void MainWindow::showOrderOverlay(const Order &order) {

    qDebug() << "Showing Order Overlay for distance:" << order.distance;
    if (!orderOverlayWidget || !orderOverlayLabel || !acceptButton || !rejectButton) { qDebug() << "Error: Order overlay UI elements are null!"; return; }
    currentDisplayedOrder = order;

    QString orderText = QString("<b>New Order!</b><br>Distance: %1 m<br>Price: %2 AZN<br>Payment: %3").arg(order.distance).arg(order.price, 0, 'f', 2).arg(order.payment);
    orderOverlayLabel->setText(orderText);
    positionManuallyPlacedWidgets();
    orderOverlayWidget->setVisible(true); orderOverlayWidget->raise();
    acceptButton->setEnabled(true); rejectButton->setEnabled(true); acceptButton->setFocus();
    if (!gamePaused) { pauseGame(); qDebug() << "Order overlay shown, pausing game."; }
}
void MainWindow::acceptOrderClicked() {

    qDebug() << "Accept clicked.";
    if (gameManager && orderOverlayWidget && orderOverlayWidget->isVisible()) {
        acceptButton->setEnabled(false); rejectButton->setEnabled(false);
        gameManager->onOrderAccepted(currentDisplayedOrder);
        resumeGame();
    }
}
void MainWindow::rejectOrderClicked() {

    qDebug() << "Reject clicked.";
    if (gameManager && orderOverlayWidget && orderOverlayWidget->isVisible()) {
        acceptButton->setEnabled(false); rejectButton->setEnabled(false);
        orderOverlayWidget->setVisible(false);
        gameManager->onOrderFailed();

        qDebug() << "Order rejected, waiting for next order generation from GameManager.";
    }
}


void MainWindow::onTurboUpgradeClicked() {

    qDebug() << "MainWindow: Turbo Upgrade button clicked.";
    if (gameManager) {

        QMetaObject::invokeMethod(gameManager, "purchaseTurboUpgrade", Qt::QueuedConnection);
    }
}
void MainWindow::updateTurboButton(bool available) {


    if (turboUpgradeButton) {
        turboUpgradeButton->setEnabled(available);
    }
}
