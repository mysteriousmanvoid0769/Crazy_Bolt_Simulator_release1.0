// mainwindow.cpp

#include "mainwindow.h"
#include "gameview.h" // <-- Убедитесь, что этот include есть
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
#include <QStyle> // Включен для стилей

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), // Базовый класс первым
    // --- Инициализируем в порядке объявления в .h ---
    scene(nullptr),
    view(nullptr),
    prius(nullptr),
    // dashes, obstacleCars, fuelCans - не указатели, инициализируются по умолчанию
    gameManager(nullptr),
    centralWidget(nullptr), // Будет создан в setupUI
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
    // --- Не-указатели / bool ---
    gameOver(false),
    gamePaused(false), // Изначально не на паузе (хотя startGame поставит)
    obstaclesActive(false),
    lastFuelCanSpawnDistance(0.0),
    // currentDisplayedOrder - инициализируется по умолчанию
    // --- Таймеры ---
    movementTimer(nullptr),
    spawnTimer(nullptr),
    fuelSpawnTimer(nullptr)
{
    qDebug() << "Initializing MainWindow...";

    // *** УСТАНОВКА ФОНА ДЛЯ ОКНА ***
    // ЗАМЕНИТЕ ПУТЬ НА ВАШ АКТУАЛЬНЫЙ!
    QString backgroundImagePath = "C:/Users/iskender/Documents/Crazy_Prius_Simulator/pictures/sky_background.jpg";
    // Используем border-image для растягивания фона на все окно
    this->setStyleSheet(QString("MainWindow { border-image: url(%1) 0 0 0 0 stretch stretch; }")
                            .arg(backgroundImagePath));

    setupUI(); // Основной UI (теперь создает GameView)
    setupDialogButtons(); // Кнопки диалога
    setupUpgradeButton(); // Кнопка Turbo
    qDebug() << "UI setup complete.";

    qDebug() << "Creating Prius...";
    prius = new Prius(); // Создаем машину
    qDebug() << "Prius created.";

    // Создаем GameManager после Prius
    gameManager = new GameManager(this, prius, this);
    gameManager->setPrius(prius); // Передаем указатель на Prius менеджеру

    qDebug() << "Setting up connections...";
    // --- Соединения сигналов и слотов ---
    // От GameManager к MainWindow (обновление UI)
    connect(gameManager, &GameManager::distanceUpdated, this, &MainWindow::updateDistanceLabel);
    connect(gameManager, &GameManager::ratingUpdated, this, &MainWindow::updateRatingLabel);
    connect(gameManager, &GameManager::fuelUpdated, this, &MainWindow::updateFuelBar);
    connect(gameManager, &GameManager::totalDistanceUpdated, this, &MainWindow::updateTotalDistanceLabel);
    connect(gameManager, &GameManager::walletUpdated, this, &MainWindow::updateWalletLabel);
    connect(gameManager, &GameManager::updateCompletedOrdersCount, this, &MainWindow::updateCompletedOrdersLabel);
    connect(gameManager, &GameManager::showOrder, this, &MainWindow::showOrderOverlay); // Показ оверлея заказа
    connect(gameManager, &GameManager::turboAvailabilityChanged, this, &MainWindow::updateTurboButton); // Доступность турбо

    // От GameManager к MainWindow (управление игрой)
    connect(gameManager, &GameManager::pauseGame, this, &MainWindow::pauseGame);
    connect(gameManager, &GameManager::pauseForNewPassenger, this, &MainWindow::pauseGame); // Тоже пауза
    connect(gameManager, &GameManager::gameEndedWithMessage, this, &MainWindow::showGameOverMessage); // Конец игры
    connect(gameManager, &GameManager::startObstacles, this, &MainWindow::startObstacles); // Начало спавна препятствий
    connect(gameManager, &GameManager::newPassenger, this, &MainWindow::showNewPassengerMessage); // Лог нового пассажира

    // От GameManager к MainWindow (диалоги)
    connect(gameManager, &GameManager::updateDialogDisplayHtml, this, &MainWindow::appendDialogText);
    connect(gameManager, &GameManager::updateDialogButtonStates, this, &MainWindow::updateDialogButtons);

    // Кнопка Restart
    if (restartButton) {
        connect(restartButton, &QPushButton::clicked, this, &MainWindow::restartGame);
    } else {
        qDebug() << "Error: restartButton is null after setupUI!";
    }

    // Таймеры
    movementTimer = new QTimer(this);
    connect(movementTimer, &QTimer::timeout, this, &MainWindow::updateMovement);
    spawnTimer = new QTimer(this);
    connect(spawnTimer, &QTimer::timeout, this, &MainWindow::spawnObstacleCar);
    fuelSpawnTimer = new QTimer(this);
    connect(fuelSpawnTimer, &QTimer::timeout, this, &MainWindow::spawnFuelCan);

    setFocusPolicy(Qt::StrongFocus); // Чтобы окно ловило нажатия клавиш
    qDebug() << "MainWindow constructor finished.";
}

MainWindow::~MainWindow() {
    qDebug() << "Destroying MainWindow...";
    // Qt обычно сама удаляет дочерние объекты, но можно явно удалить менеджер, если нужно
    // delete gameManager; // Если он не дочерний объект MainWindow
}

void MainWindow::setupDialogButtons() {
    // (Код setupDialogButtons остается без изменений)
    if (!centralWidget) {
        qDebug() << "Error: centralWidget is null in setupDialogButtons!";
        return;
    }
    dialogButtonContainer = new QWidget(centralWidget);
    dialogButtonContainer->setStyleSheet("background-color: rgba(0, 0, 0, 0); border: none;"); // Прозрачный контейнер
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
    // (Код setupUpgradeButton остается без изменений)
    if (!centralWidget) {
        qDebug() << "Error: centralWidget is null in setupUpgradeButton!";
        return;
    }
    turboUpgradeButton = new QPushButton("Turbo Prius (5 AZN)", centralWidget);
    turboUpgradeButton->setStyleSheet("QPushButton { font-size: 9pt; padding: 5px; background-color: #0055A4; color: white; border-radius: 4px; margin: 1px; } QPushButton:hover { background-color: #0077CC; } QPushButton:disabled { background-color: #333; color: #888; }");
    turboUpgradeButton->setEnabled(false); // Изначально неактивна
    turboUpgradeButton->setVisible(true);
    connect(turboUpgradeButton, &QPushButton::clicked, this, &MainWindow::onTurboUpgradeClicked);
    qDebug() << "Turbo Upgrade button created.";
}

void MainWindow::setupSpeedPanel() {
    // (Код setupSpeedPanel остается без изменений)
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
    turboBar->setRange(0, 60); // Диапазон для скорости > 120 (120 до 180)
    turboBar->setValue(0);
    turboBar->setTextVisible(false); // Не показываем текст на турбо-баре
    turboBar->setMinimumHeight(180);
    turboBar->setFixedWidth(10);
    turboBar->setStyleSheet("QProgressBar { border: 1px solid #444; border-radius: 3px; background-color: rgba(50,50,50,180); } QProgressBar::chunk:vertical { background-color: cyan; border-radius: 2px; margin: 1px; }");
    turboBar->setVisible(false); // Изначально скрыт

    speedPanelLayout->addWidget(speedBar);
    speedPanelLayout->addWidget(turboBar);

    QWidget* speedBarsContainer = new QWidget(leftStatusPanelWidget);
    speedBarsContainer->setLayout(speedPanelLayout);

    fuelBar = new QProgressBar(leftStatusPanelWidget); // Родитель - leftStatusPanelWidget
    fuelBar->setRange(0, 100);
    fuelBar->setValue(100);
    fuelBar->setTextVisible(true);
    fuelBar->setFormat("F:%p%");
    fuelBar->setFixedHeight(30);
    fuelBar->setStyleSheet("QProgressBar { border: 1px solid grey; border-radius: 4px; text-align: center; background-color: rgba(50,50,50,180); color: white; font-size: 10pt; } QProgressBar::chunk { background-color: red; border-radius: 3px; margin: 1px; }");

    QVBoxLayout* mainLeftPanelLayout = new QVBoxLayout(leftStatusPanelWidget);
    mainLeftPanelLayout->setContentsMargins(5, 5, 5, 5);
    mainLeftPanelLayout->setSpacing(8);
    mainLeftPanelLayout->addWidget(speedBarsContainer); // Добавляем контейнер спидометров
    mainLeftPanelLayout->addWidget(fuelBar); // Добавляем топливо
    mainLeftPanelLayout->addStretch(); // Растягиваем, чтобы прижать вверх

    leftStatusPanelWidget->setLayout(mainLeftPanelLayout);
    leftStatusPanelWidget->adjustSize(); // Подгоняем размер панели
}


void MainWindow::setupUI() {
    qDebug() << "Setting up UI...";
    centralWidget = new QWidget(this);
    // *** ДОБАВЛЕНО: Делаем центральный виджет прозрачным, чтобы фон MainWindow был виден ***
    centralWidget->setStyleSheet("background: transparent;");
    // *** КОНЕЦ ДОБАВЛЕНИЯ ***
    this->setCentralWidget(centralWidget);

    scene = new QGraphicsScene(0, 0, 400, 1000, this); // Размер сцены

    // *** ИЗМЕНЕНИЕ: Создаем GameView (он теперь прозрачный по умолчанию) ***
    view = new GameView(scene, centralWidget);
    // Убедимся, что у view нет своей рамки или фона, перекрывающих фон окна
    view->setStyleSheet("background: transparent; border: none;");
    // *** КОНЕЦ ИЗМЕНЕНИЯ ***

    // *** УБРАН вызов view->setBackgroundImage(...) ***
    // *** УБРАН старый код установки фона для сцены ***

    // Основной layout для centralWidget, содержит только view
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(view, 1); // view растягивается на все доступное место

    // --- Создание панелей UI (они будут поверх view) ---
    setupSpeedPanel(); // Левая панель
    setupOrderOverlay(); // Оверлей заказа (изначально скрыт)

    // Правая верхняя панель (Рейтинг)
    topRightStatusPanelWidget = new QWidget(centralWidget); // Родитель - centralWidget
    topRightStatusPanelWidget->setStyleSheet("background-color: rgba(0, 0, 0, 170); border-radius: 5px; padding: 2px;");
    ratingIconLabel = new QLabel(topRightStatusPanelWidget);
    // ЗАМЕНИТЕ ПУТЬ НА ВАШ!
    QString starIconPath = "C:/Users/iskender/Documents/Crazy_Prius_Simulator/pictures/star.jpg";
    QPixmap starPixmap(starIconPath);
    if (!starPixmap.isNull()) {
        ratingIconLabel->setPixmap(starPixmap.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        qDebug() << "Warning: Failed to load star icon from" << starIconPath << ". Using text fallback.";
        ratingIconLabel->setText("*"); // Запасной вариант - текст
        ratingIconLabel->setStyleSheet("color: yellow; font-size: 16pt; font-weight: bold; background: transparent; border: none;");
    }
    ratingLabel = new QLabel("...", topRightStatusPanelWidget); // Начальное значение
    ratingLabel->setStyleSheet("color: white; font-size: 11pt; background-color: transparent; border: none; padding-left: 4px;");
    QHBoxLayout *topRightLayout = new QHBoxLayout(topRightStatusPanelWidget);
    topRightLayout->setContentsMargins(4, 4, 4, 4);
    topRightLayout->setSpacing(3);
    topRightLayout->addWidget(ratingIconLabel);
    topRightLayout->addWidget(ratingLabel);
    topRightStatusPanelWidget->setLayout(topRightLayout); // Установить layout для панели
    topRightStatusPanelWidget->adjustSize(); // Подогнать размер

    // Нижняя левая панель статистики
    bottomLeftStatsWidget = new QWidget(centralWidget); // Родитель - centralWidget
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
    // Стиль для текста статистики
    QString statsLabelStyle = "QLabel { font-size: 10pt; color: white; background-color: transparent; border: none; padding: 1px; }";
    walletLabel->setStyleSheet(statsLabelStyle);
    completedOrdersLabel->setStyleSheet(statsLabelStyle);
    distanceLabel->setStyleSheet(statsLabelStyle);
    totalDistanceLabel->setStyleSheet(statsLabelStyle);
    // Выравнивание текста по левому краю
    walletLabel->setAlignment(Qt::AlignLeft);
    completedOrdersLabel->setAlignment(Qt::AlignLeft);
    distanceLabel->setAlignment(Qt::AlignLeft);
    totalDistanceLabel->setAlignment(Qt::AlignLeft);
    bottomLeftStatsWidget->setLayout(bottomLeftStatsLayout); // Установить layout
    bottomLeftStatsWidget->adjustSize(); // Подогнать размер

    // Панель комментариев (справа)
    commentDisplayArea = new QTextEdit(centralWidget); // Родитель - centralWidget
    commentDisplayArea->setReadOnly(true);
    commentDisplayArea->setStyleSheet("background-color: rgba(0, 0, 0, 170); border-radius: 5px; padding: 5px; color: white; font-size: 10pt; border: 1px solid #444;");
    commentDisplayArea->clear(); // Очистить при старте

    // Кнопка Restart (по центру внизу, изначально скрыта)
    restartButton = new QPushButton("Restart", centralWidget); // Родитель - centralWidget
    restartButton->setVisible(false);
    restartButton->setMinimumHeight(30);
    restartButton->setStyleSheet("QPushButton { font-size: 12pt; padding: 5px; background-color: #C00000; color: white; border-radius: 5px; } QPushButton:hover { background-color: #E00000; }");

    // Не устанавливаем layout для centralWidget здесь, т.к. mainLayout уже установлен
    // centralWidget->setLayout(mainLayout); // ЭТО ЛИШНЕЕ
    qDebug() << "UI setup finished.";
}

void MainWindow::setupOrderOverlay() {
    // (Код setupOrderOverlay остается без изменений)
    qDebug() << "Setting up Order Overlay UI...";
    if (!centralWidget) { qDebug() << "Error: centralWidget is null in setupOrderOverlay!"; return; }
    orderOverlayWidget = new QWidget(centralWidget);
    orderOverlayWidget->setStyleSheet("background-color: rgba(0, 0, 0, 180); border-radius: 8px; border: 1px solid #555;");
    orderOverlayWidget->setFixedSize(280, 150); // Фиксированный размер оверлея
    QVBoxLayout *overlayLayout = new QVBoxLayout(orderOverlayWidget);
    overlayLayout->setContentsMargins(10, 10, 10, 10);
    overlayLayout->setSpacing(6);
    orderOverlayLabel = new QLabel("Order: ...", orderOverlayWidget);
    orderOverlayLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop); // Выравнивание текста
    orderOverlayLabel->setStyleSheet("color: white; font-size: 10pt; background-color: transparent; border: none;");
    orderOverlayLabel->setWordWrap(true); // Перенос слов
    overlayLayout->addWidget(orderOverlayLabel, 1); // Метка занимает основное место
    QHBoxLayout *buttonLayout = new QHBoxLayout(); // Горизонтальный layout для кнопок
    buttonLayout->setSpacing(10);
    acceptButton = new QPushButton("Accept", orderOverlayWidget);
    rejectButton = new QPushButton("Reject", orderOverlayWidget);
    // Стили кнопок
    acceptButton->setStyleSheet("QPushButton { font-size: 10pt; padding: 6px; background-color: #00A000; color: white; border-radius: 5px; } QPushButton:hover { background-color: #00C000; }");
    rejectButton->setStyleSheet("QPushButton { font-size: 10pt; padding: 6px; background-color: #C00000; color: white; border-radius: 5px; } QPushButton:hover { background-color: #E00000; }");
    // Соединяем сигналы кнопок со слотами
    connect(acceptButton, &QPushButton::clicked, this, &MainWindow::acceptOrderClicked);
    connect(rejectButton, &QPushButton::clicked, this, &MainWindow::rejectOrderClicked);
    buttonLayout->addStretch(1); // Растягиваем пространство слева от кнопок
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(rejectButton);
    overlayLayout->addLayout(buttonLayout); // Добавляем layout кнопок вниз
    orderOverlayWidget->setLayout(overlayLayout); // Устанавливаем главный layout для оверлея
    orderOverlayWidget->setVisible(false); // Изначально скрыт
    qDebug() << "Order Overlay UI created.";
}

void MainWindow::positionManuallyPlacedWidgets() {
    // (Код positionManuallyPlacedWidgets остается без изменений)
    // Эта функция отвечает за размещение виджетов, которые не в layout
    // Она должна вызываться при изменении размера окна (resizeEvent) и при показе (showEvent)
    if (!centralWidget || !view) return;
    int margin = 10;

    // Оверлей заказа (слева сверху)
    if (orderOverlayWidget) {
        orderOverlayWidget->move(margin, margin);
        orderOverlayWidget->raise(); // Поверх других
    }

    // Левая панель (под оверлеем заказа или сверху)
    if (leftStatusPanelWidget) {
        int yPos = margin;
        if (orderOverlayWidget && orderOverlayWidget->isVisible()) {
            yPos = orderOverlayWidget->geometry().bottom() + margin;
        }
        leftStatusPanelWidget->adjustSize();
        int panelHeight = leftStatusPanelWidget->height();
        // Предотвращаем выход за нижнюю границу
        if (yPos + panelHeight > centralWidget->height() - margin) {
            yPos = centralWidget->height() - panelHeight - margin;
        }
        if (yPos < margin) yPos = margin; // Не заезжать наверх
        leftStatusPanelWidget->move(margin, yPos);
        leftStatusPanelWidget->raise();
        leftStatusPanelWidget->setVisible(true);
    }

    // Правая верхняя панель (справа сверху)
    if (topRightStatusPanelWidget) {
        topRightStatusPanelWidget->adjustSize();
        int panelWidth = topRightStatusPanelWidget->width();
        topRightStatusPanelWidget->move(centralWidget->width() - panelWidth - margin, margin);
        topRightStatusPanelWidget->raise();
        topRightStatusPanelWidget->setVisible(true);
    }

    // Элементы слева снизу (статистика, кнопка турбо)
    int currentBottomY = centralWidget->height() - margin; // Начинаем с самого низа

    // Кнопка Restart (по центру снизу, если видима)
    if(restartButton && restartButton->isVisible()) {
        restartButton->adjustSize();
        int buttonWidth = restartButton->width(); if (buttonWidth <= 0) buttonWidth = 100;
        int buttonHeight = restartButton->height(); if (buttonHeight <= 0) buttonHeight = 30;
        int restartY = centralWidget->height() - buttonHeight - margin;
        restartButton->move((centralWidget->width() - buttonWidth) / 2, restartY);
        restartButton->raise();
        currentBottomY = restartY - margin; // Обновляем Y для следующих элементов
    }

    // Статистика (над кнопкой Restart или просто снизу слева)
    if (bottomLeftStatsWidget) {
        bottomLeftStatsWidget->adjustSize();
        int panelHeight = bottomLeftStatsWidget->height();
        int panelY = currentBottomY - panelHeight;
        bottomLeftStatsWidget->move(margin, panelY);
        bottomLeftStatsWidget->raise();
        bottomLeftStatsWidget->setVisible(true);
        currentBottomY = panelY - 5; // Обновляем Y для следующих элементов
    }

    // Кнопка Турбо (над статистикой)
    if (turboUpgradeButton) {
        turboUpgradeButton->adjustSize();
        int buttonHeight = turboUpgradeButton->height();
        int buttonY = currentBottomY - buttonHeight;
        // Ограничиваем сверху, чтобы не наезжать на левую панель
        int topLimitY = margin;
        if (leftStatusPanelWidget && leftStatusPanelWidget->isVisible()) {
            topLimitY = leftStatusPanelWidget->geometry().bottom() + margin;
        }
        if (buttonY < topLimitY) {
            buttonY = topLimitY;
        }
        turboUpgradeButton->move(margin, buttonY);
        // Делаем кнопку той же ширины, что и панель статистики для аккуратности
        turboUpgradeButton->setFixedWidth(bottomLeftStatsWidget ? bottomLeftStatsWidget->width() : 120);
        turboUpgradeButton->raise();
        turboUpgradeButton->setVisible(true);
    }


    // Панель комментариев и кнопки диалога (справа)
    if (commentDisplayArea) {
        int panelWidth = 300; // Ширина панели комментариев
        // Корректируем ширину, если окно слишком узкое
        if (panelWidth > centralWidget->width() - margin * 3) panelWidth = centralWidget->width() - margin * 3;
        if (panelWidth < 100) panelWidth = 100;

        int xPos = centralWidget->width() - panelWidth - margin; // Позиция X справа
        int yPos = margin; // Позиция Y сверху
        // Смещаем ниже, если есть верхняя правая панель
        if(topRightStatusPanelWidget && topRightStatusPanelWidget->isVisible()) yPos = topRightStatusPanelWidget->geometry().bottom() + margin;

        int buttonContainerHeight = 0;
        // Рассчитываем высоту кнопок диалога
        if(dialogButtonContainer) {
            dialogButtonContainer->setFixedWidth(panelWidth); // Задаем ширину
            buttonContainerHeight = dialogButtonContainer->sizeHint().height();
        }

        // Высота панели комментариев = оставшееся место между Y верха и Y кнопок диалога
        int bottomLimitYForComments = centralWidget->height() - margin;
        int commentAreaHeight = bottomLimitYForComments - yPos - buttonContainerHeight - 5; // 5 - отступ
        if (commentAreaHeight < 50) commentAreaHeight = 50; // Минимальная высота

        // Устанавливаем геометрию панели комментариев
        commentDisplayArea->setGeometry(xPos, yPos, panelWidth, commentAreaHeight);
        commentDisplayArea->raise(); commentDisplayArea->setVisible(true);

        // Устанавливаем геометрию кнопок диалога под панелью комментариев
        if (dialogButtonContainer) {
            int buttonContainerY = yPos + commentAreaHeight + 5; // 5 - отступ
            dialogButtonContainer->setGeometry(xPos, buttonContainerY, panelWidth, buttonContainerHeight);
            dialogButtonContainer->raise(); dialogButtonContainer->setVisible(true);
        }
    } else if (dialogButtonContainer) {
        // Если нет области комментариев, кнопки диалога тоже не показываем
        dialogButtonContainer->setVisible(false);
    }
}


void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    // Перемещаем все вручную размещенные виджеты при изменении размера окна
    positionManuallyPlacedWidgets();
}

void MainWindow::showEvent(QShowEvent *event) {
    // (Код showEvent остается без изменений)
    QMainWindow::showEvent(event);
    static bool startScheduled = false;
    if (!startScheduled && gameManager) {
        startScheduled = true;
        // Запускаем игру с небольшой задержкой после показа окна
        QTimer::singleShot(100, this, &MainWindow::delayedStartGame);
    }
    // Размещаем виджеты сразу после показа
    QTimer::singleShot(0, this, &MainWindow::positionManuallyPlacedWidgets);
    activateWindow(); // Делаем окно активным
    setFocus();       // Устанавливаем фокус для обработки клавиш
}

void MainWindow::delayedStartGame() {
    // (Код delayedStartGame остается без изменений)
    qDebug() << "Executing delayedStartGame()...";
    if(scene) {
        resetScene(); // Очищаем и перерисовываем сцену (дорогу, разметку)
        qDebug() << "Scene reset done.";
    } else {
        qDebug() << "Error: scene is null in delayedStartGame!"; return;
    }
    if (prius && scene) {
        scene->addItem(prius); // Добавляем машину на сцену
        prius->setY(scene->height() * 0.75); // Позиция машины по Y
        scene->update();
        qDebug() << "Prius added to scene.";
    } else {
        qDebug() << "Error: prius or scene is null in delayedStartGame!"; return;
    }
    // Запускаем таймеры движения и спавна
    if(movementTimer && !movementTimer->isActive()) movementTimer->start(16); // ~60 FPS
    if(spawnTimer && !spawnTimer->isActive()) spawnTimer->start(2000); // Спавн машин каждые 2 сек (примерно)
    if(fuelSpawnTimer && !fuelSpawnTimer->isActive()) fuelSpawnTimer->start(100); // Проверка спавна топлива часто
    qDebug() << "Timers started.";
    if (gameManager) {
        gameManager->startGame(); // Начинаем логику игры (генерация заказа)
    } else {
        qDebug() << "Error: gameManager is null in delayedStartGame!"; return;
    }
    activateWindow(); setFocus(); qDebug() << "Focus set.";
    // Еще раз обновляем позиции виджетов на всякий случай
    QTimer::singleShot(0, this, &MainWindow::positionManuallyPlacedWidgets);
}

void MainWindow::resetScene() {
    // (Код resetScene остается без изменений)
    qDebug() << "Resetting scene visuals...";
    // Очищаем списки старых объектов
    dashes.clear(); obstacleCars.clear(); fuelCans.clear();
    if (scene) {
        // Удаляем все элементы со сцены, кроме машины (prius)
        QList<QGraphicsItem*> items = scene->items();
        for (QGraphicsItem* item : items) {
            if (item == prius) continue; // Машину не удаляем
            scene->removeItem(item);
            delete item; // Освобождаем память
        }
        qDebug() << "Old items cleared from scene.";

        // Перерисовываем дорогу и разметку
        qreal sceneHeight = scene->sceneRect().height();
        qreal roadWidth = 300; // Ширина дороги
        qreal roadX = (scene->width() - roadWidth) / 2.0; // X координата дороги для центрирования
        // Рисуем серый прямоугольник дороги
        QGraphicsRectItem *road = new QGraphicsRectItem(roadX, 0, roadWidth, sceneHeight);
        road->setBrush(QColor(80, 80, 80)); // Темно-серый цвет
        road->setPen(Qt::NoPen); // Без рамки
        road->setZValue(-1); // Помещаем позади других элементов
        scene->addItem(road);

        // Рисуем разметку (белые пунктирные линии)
        qreal dashLength = 40; qreal gapLength = 30;
        qreal totalSegmentLength = dashLength + gapLength;
        // Рассчитываем, сколько пар линий нужно, чтобы покрыть двойную высоту сцены (для прокрутки)
        int numDashPairsNeeded = qCeil((sceneHeight * 2.0 + totalSegmentLength) / totalSegmentLength);
        qreal totalPatternHeight = numDashPairsNeeded * totalSegmentLength;
        qreal line1X = roadX + roadWidth / 3.0; // X первой линии разметки
        qreal line2X = roadX + 2.0 * roadWidth / 3.0; // X второй линии разметки
        qreal dashWidth = 5; // Ширина линии разметки
        // Начинаем рисовать линии "за экраном" сверху, чтобы они выезжали
        qreal startY = sceneHeight - totalPatternHeight;

        for (int i = 0; i < numDashPairsNeeded; ++i) {
            qreal yPos = startY + i * totalSegmentLength;
            // Левая линия
            QGraphicsRectItem *dashL = new QGraphicsRectItem(line1X - dashWidth / 2, yPos, dashWidth, dashLength);
            dashL->setBrush(Qt::white); dashL->setPen(Qt::NoPen); dashL->setZValue(0);
            scene->addItem(dashL); dashes.append(dashL); // Добавляем в список для анимации
            // Правая линия
            QGraphicsRectItem *dashR = new QGraphicsRectItem(line2X - dashWidth / 2, yPos, dashWidth, dashLength);
            dashR->setBrush(Qt::white); dashR->setPen(Qt::NoPen); dashR->setZValue(0);
            scene->addItem(dashR); dashes.append(dashR); // Добавляем в список для анимации
        }
        qDebug() << "Road and dashes recreated.";
    } else {
        qDebug() << "Error: Scene is null in resetScene()!";
    }
}


void MainWindow::keyPressEvent(QKeyEvent *event) {
    // (Код keyPressEvent остается без изменений)
    if (gameOver) { // Если игра окончена, выходим по Esc
        if (event->key() == Qt::Key_Escape) QApplication::quit();
        return;
    }
    if (event->key() == Qt::Key_Escape) QApplication::quit(); // Выход по Esc в любое время

    // Если игра на паузе или нет машины, не обрабатываем управление
    if (gamePaused || !prius) return;

    // Обработка клавиш управления машиной
    switch (event->key()) {
    case Qt::Key_Up: case Qt::Key_W: prius->accelerate(); break; // Газ
    case Qt::Key_Down: case Qt::Key_S: prius->brake(); break;    // Тормоз
    case Qt::Key_Left: case Qt::Key_A: prius->moveLeft(); break; // Влево
    case Qt::Key_Right: case Qt::Key_D: prius->moveRight(); break;// Вправо
    default: QMainWindow::keyPressEvent(event); return; // Другие клавиши - стандартная обработка
    }
}

void MainWindow::updateMovement() {
    // (Код updateMovement остается без изменений)
    // Этот слот вызывается таймером movementTimer ~60 раз в секунду
    if (gameOver || gamePaused || !prius || !view || !scene) return; // Не обновляем, если игра окончена/пауза

    int speed = prius->getSpeed(); // Текущая скорость машины
    updateSpeedBar(speed, prius->isTurboActive()); // Обновляем спидометр (maxSpeed убран)

    // Рассчитываем смещение для анимации дороги/объектов на основе скорости
    qreal roadSpeedFactor = 0.07; // Коэффициент для "скорости дороги"
    qreal deltaY = speed * roadSpeedFactor; // Смещение по Y за кадр

    // Движение разметки
    int numDashPairs = dashes.count() / 2;
    if (numDashPairs > 0) {
        qreal dashLength = 40; qreal gapLength = 30;
        qreal totalSegmentLength = dashLength + gapLength;
        qreal totalPatternHeight = numDashPairs * totalSegmentLength; // Общая высота паттерна разметки
        qreal sceneHeight = scene->sceneRect().height();
        for (QGraphicsRectItem *dash : dashes) {
            dash->moveBy(0, deltaY); // Двигаем линию вниз
            // Если линия ушла за нижний край экрана, перемещаем ее наверх
            if (dash->sceneBoundingRect().top() >= sceneHeight) {
                if (totalPatternHeight > 0) dash->moveBy(0, -totalPatternHeight);
                else qDebug() << "Error: totalPatternHeight is zero!";
            }
        }
    }

    // Движение и проверка коллизий с препятствиями и топливом
    if (obstaclesActive) { // Если спавн препятствий активен
        qreal sceneHeight = scene->sceneRect().height();

        // Обработка машин-препятствий
        QMutableListIterator<ObstacleCar*> carIter(obstacleCars);
        while (carIter.hasNext()) {
            ObstacleCar *car = carIter.next();
            car->move(speed); // Двигаем машину-препятствие

            // Проверка столкновения с машиной игрока
            if (prius->collidesWithItem(car)) {
                if(gameManager) gameManager->onCollision(); // Сообщаем менеджеру об аварии
                return; // Прекращаем обработку этого кадра
            }

            // Удаление машины, если она ушла далеко за экран
            if (car->sceneBoundingRect().top() > sceneHeight + 50) {
                scene->removeItem(car); carIter.remove(); delete car;
            }
        }

        // Обработка канистр с топливом
        QMutableListIterator<FuelCan*> canIter(fuelCans);
        while (canIter.hasNext()) {
            FuelCan *can = canIter.next();
            can->move(speed); // Двигаем канистру

            // Проверка столкновения (сбора канистры)
            if (prius->collidesWithItem(can)) {
                if(gameManager) gameManager->resetFuelDistance(); // Восстанавливаем топливо
                scene->removeItem(can); canIter.remove(); delete can; // Удаляем канистру
                continue; // Переходим к следующей канистре
            }

            // Удаление канистры, если она ушла далеко за экран
            if (can->sceneBoundingRect().top() > sceneHeight + 50) {
                scene->removeItem(can); canIter.remove(); delete can;
            }
        }
    }

    // Удерживаем машину игрока на фиксированной позиции Y
    prius->setY(scene->height() * 0.75);
}


void MainWindow::spawnObstacleCar() {
    // (Код spawnObstacleCar остается без изменений)
    // Спавн машин-препятствий (вызывается таймером spawnTimer)
    if (!obstaclesActive || gameOver || gamePaused || !scene) return; // Не спавним если не нужно

    int maxObstaclesOnScreen = 3; // Макс. кол-во препятствий одновременно
    if (obstacleCars.size() >= maxObstaclesOnScreen) return; // Уже максимум

    // Шанс спавна (например, 30%)
    if (QRandomGenerator::global()->bounded(100) < 70) return; // Не спавним в этот раз

    int lane = QRandomGenerator::global()->bounded(3); // Случайная полоса (0, 1, 2)
    qreal spawnY = -100.0; // Позиция спавна над экраном

    // Проверяем, свободна ли полоса в месте спавна (чтобы машины не появлялись одна в другой)
    bool isLaneFreeNearSpawn = true;
    for (ObstacleCar *existingCar : obstacleCars) {
        // Если на той же полосе есть машина слишком близко к точке спавна
        if (existingCar->lane() == lane && qAbs(existingCar->y() - spawnY) < existingCar->boundingRect().height() * 2.5) {
            isLaneFreeNearSpawn = false; break;
        }
    }

    if (isLaneFreeNearSpawn) { // Если свободно
        ObstacleCar *car = new ObstacleCar(lane); // Создаем машину
        scene->addItem(car); // Добавляем на сцену
        obstacleCars.append(car); // Добавляем в список для обработки
    }
}

void MainWindow::spawnFuelCan() {
    // (Код spawnFuelCan остается без изменений)
    // Спавн канистр (вызывается таймером fuelSpawnTimer)
    if (gameOver || gamePaused || !gameManager || !scene) return;

    double totalDistance = gameManager->getTotalDistanceDriven(); // Общее пройденное расстояние
    // Спавним канистру каждые 1000 метров, если на экране еще нет канистр
    if (totalDistance - lastFuelCanSpawnDistance >= 1000 && fuelCans.isEmpty()) {
        int lane = QRandomGenerator::global()->bounded(3); // Случайная полоса
        FuelCan *can = new FuelCan(lane); // Создаем канистру
        scene->addItem(can); // Добавляем на сцену
        fuelCans.append(can); // Добавляем в список
        lastFuelCanSpawnDistance = totalDistance; // Запоминаем дистанцию последнего спавна
        qDebug() << "Spawned FuelCan in lane:" << lane;
    }
}

void MainWindow::startObstacles() { obstaclesActive = true; qDebug() << "Obstacles activated!"; } // Включаем спавн препятствий

void MainWindow::restartGame() {
    qDebug() << "Restarting game...";
    if (!gameManager || !scene) { qDebug() << "Error: GM or Scene null!"; return; }
    gameOver = false; gamePaused = false; obstaclesActive = false;
    lastFuelCanSpawnDistance = 0.0; // Сбрасываем счетчик спавна топлива

    if(restartButton) restartButton->setVisible(false); // Скрываем кнопку рестарта
    // Сбрасываем индикаторы
    if(speedBar) speedBar->setValue(0);
    if(turboBar) { turboBar->setValue(0); turboBar->setVisible(false); }
    if(speedBar) speedBar->setStyleSheet("QProgressBar { border: 1px solid grey; border-radius: 4px; text-align: center; background-color: rgba(50,50,50,180); color: white; } QProgressBar::chunk:vertical { background-color: yellow; border-radius: 3px; margin: 1px; }"); // Стандартный цвет спидометра
    if(turboUpgradeButton) turboUpgradeButton->setEnabled(false); // Отключаем кнопку турбо
    clearCommentArea(); // Очищаем область комментариев

    // Пересоздаем машину игрока
    if (prius) { scene->removeItem(prius); delete prius; prius = nullptr; }
    resetScene(); // Перерисовываем дорогу и разметку
    prius = new Prius(); // Создаем новую машину
    scene->addItem(prius); prius->setY(scene->height() * 0.75); scene->update();
    gameManager->setPrius(prius); // Обновляем указатель на машину в GameManager

    gameManager->resetForRestart(); // Сбрасываем состояние GameManager (топливо, текущий заказ и т.д.)

    // Обновляем UI начальными значениями
    if(gameManager) {
        if(fuelBar) updateFuelBar(100.0);
        if(ratingLabel) updateRatingLabel(gameManager->getRating());
        if(walletLabel) updateWalletLabel(gameManager->getWallet());
        // *** ИСПРАВЛЕНА ОШИБКА: Используем добавленный getter ***
        if(completedOrdersLabel) updateCompletedOrdersLabel(gameManager->getCompletedOrders());
        if(distanceLabel) updateDistanceLabel(0, 0);
        if(totalDistanceLabel) updateTotalDistanceLabel(gameManager->getTotalDistanceDriven());
    }

    // Перезапускаем таймеры
    if (movementTimer && !movementTimer->isActive()) movementTimer->start(16);
    if (spawnTimer && !spawnTimer->isActive()) spawnTimer->start(2000);
    if (fuelSpawnTimer && !fuelSpawnTimer->isActive()) fuelSpawnTimer->start(100);

    qDebug() << "Game restarted.";
    gameManager->startGame(); // Начинаем новую игру (ждем заказ)
    activateWindow(); setFocus(); // Устанавливаем фокус
    QTimer::singleShot(0, this, &MainWindow::positionManuallyPlacedWidgets); // Обновляем расположение UI
}

// --- Слоты обновления UI ---
void MainWindow::updateDistanceLabel(double current, int total) {
    if(distanceLabel) distanceLabel->setText(QString("Dist: %1/%2 m").arg(static_cast<int>(current)).arg(total));
}
void MainWindow::updateRatingLabel(double rating) {
    if(ratingLabel) ratingLabel->setText(QString("%1").arg(rating, 0, 'f', 2)); // Формат с 2 знаками после запятой
}
void MainWindow::updateCompletedOrdersLabel(int count) {
    if(completedOrdersLabel) completedOrdersLabel->setText(QString("Orders: %1").arg(count));
}
void MainWindow::updateTotalDistanceLabel(double totalDistance) {
    if(totalDistanceLabel) totalDistanceLabel->setText(QString("Total: %1 m").arg(static_cast<int>(totalDistance)));
}
void MainWindow::updateFuelBar(double fuelPercentage) {
    if(fuelBar) fuelBar->setValue(qBound(0, static_cast<int>(fuelPercentage), 100)); // Значение от 0 до 100
}
void MainWindow::updateWalletLabel(double newWallet) {
    if(walletLabel) walletLabel->setText(QString("Wallet: %1 AZN").arg(newWallet, 0, 'f', 2)); // Формат с 2 знаками
}

// *** ИСПРАВЛЕНО: Убран неиспользуемый параметр maxSpeed ***
void MainWindow::updateSpeedBar(int speed, bool turboActive) {
    // Обновление спидометра и индикатора турбо
    if (speedBar) {
        speedBar->setValue(qBound(0, speed, 120)); // Основной спидометр до 120
        // Меняем цвет полосы спидометра, если активно турбо
        speedBar->setStyleSheet(QString("QProgressBar { border: 1px solid grey; border-radius: 4px; text-align: center; background-color: rgba(50,50,50,180); color: white; } QProgressBar::chunk:vertical { background-color: %1; border-radius: 3px; margin: 1px; }")
                                    .arg(turboActive ? "orange" : "yellow")); // Оранжевый при турбо, желтый обычно
    }
    if (turboBar) {
        if (turboActive) { // Если турбо активно
            turboBar->setValue(qBound(0, speed - 120, 60)); // Показываем скорость > 120 (макс 180)
            turboBar->setVisible(true); // Показываем индикатор турбо
        } else {
            turboBar->setValue(0); // Сбрасываем значение
            turboBar->setVisible(false); // Скрываем индикатор турбо
        }
    }
}


// --- Слоты управления состоянием игры ---
void MainWindow::showNewPassengerMessage() { qDebug() << "New passenger/order started in MainWindow."; } // Лог нового заказа
void MainWindow::pauseGame() { if (!gameOver && !gamePaused) { gamePaused = true; qDebug() << "MainWindow: Game paused."; } } // Пауза игры
void MainWindow::resumeGame() { if (gamePaused) { gamePaused = false; if (orderOverlayWidget) orderOverlayWidget->setVisible(false); activateWindow(); setFocus(); qDebug() << "MainWindow: Game resumed."; if(gameManager) gameManager->resumeGame(); } } // Возобновление игры

// --- Слоты диалога ---
void MainWindow::onStartDialogClicked() { if (gameManager) QMetaObject::invokeMethod(gameManager, "onPlayerInitiatedDialog", Qt::QueuedConnection); } // Клик "Начать диалог"
void MainWindow::onTellStoryClicked() { if (gameManager) QMetaObject::invokeMethod(gameManager, "onPlayerToldStory", Qt::QueuedConnection); }       // Клик "Рассказать историю"
void MainWindow::onComplainClicked() { if (gameManager) QMetaObject::invokeMethod(gameManager, "onPlayerComplained", Qt::QueuedConnection); }      // Клик "Пожаловаться"
void MainWindow::updateDialogButtons(bool enableStart, bool enableStory, bool enableComplain) { if(startDialogButton) startDialogButton->setEnabled(enableStart); if(tellStoryButton) tellStoryButton->setEnabled(enableStory); if(complainButton) complainButton->setEnabled(enableComplain); } // Обновление доступности кнопок диалога
void MainWindow::appendDialogText(const QString &htmlText) { if (commentDisplayArea) { commentDisplayArea->append(htmlText); } } // Добавление текста в область комментариев
void MainWindow::clearCommentArea() { if (commentDisplayArea) { commentDisplayArea->clear(); qDebug() << "MainWindow: Comment area cleared."; } } // Очистка области комментариев

// --- Game Over ---
void MainWindow::showGameOverMessage(QString message) {
    // (Код showGameOverMessage остается без изменений)
    qDebug() << "Game over message received:" << message;
    if (gameOver) return; // Если уже Game Over, ничего не делаем
    gameOver = true; gamePaused = true; // Устанавливаем флаги

    // Скрываем/останавливаем ненужные элементы
    if (orderOverlayWidget) orderOverlayWidget->setVisible(false);
    if (commentDisplayArea) commentDisplayArea->setVisible(false);
    if (dialogButtonContainer) dialogButtonContainer->setVisible(false);
    if (turboUpgradeButton) turboUpgradeButton->setVisible(false);
    if (turboBar) turboBar->setVisible(false);
    if (movementTimer) movementTimer->stop();
    if (spawnTimer) spawnTimer->stop();
    if (fuelSpawnTimer) fuelSpawnTimer->stop();

    // Проверяем, фатальный ли Game Over (без рестарта)
    bool fatalGameOver = (message.contains("Ахмед") || message.contains("fired") || message.contains("закончилось топливо") || message.contains("насмерть"));

    if (!fatalGameOver && restartButton) { // Если не фатальный, показываем кнопку Restart
        restartButton->setVisible(true); positionManuallyPlacedWidgets(); restartButton->setFocus();
    } else if(restartButton) { // Если фатальный, скрываем кнопку Restart
        restartButton->setVisible(false);
    }

    // Показываем сообщение о Game Over
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning); msgBox.setWindowTitle("Game Over"); msgBox.setText(message); msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec(); // Показываем окно сообщения и ждем закрытия

    if (fatalGameOver) { // Если фатальный, закрываем приложение
        QApplication::quit();
    }
}

// --- Оверлей заказа ---
void MainWindow::showOrderOverlay(const Order &order) {
    // (Код showOrderOverlay остается без изменений)
    qDebug() << "Showing Order Overlay for distance:" << order.distance;
    if (!orderOverlayWidget || !orderOverlayLabel || !acceptButton || !rejectButton) { qDebug() << "Error: Order overlay UI elements are null!"; return; }
    currentDisplayedOrder = order; // Сохраняем текущий заказ
    // Формируем текст для оверлея
    QString orderText = QString("<b>New Order!</b><br>Distance: %1 m<br>Price: %2 AZN<br>Payment: %3").arg(order.distance).arg(order.price, 0, 'f', 2).arg(order.payment);
    orderOverlayLabel->setText(orderText);
    positionManuallyPlacedWidgets(); // Обновляем позицию (на всякий случай)
    orderOverlayWidget->setVisible(true); orderOverlayWidget->raise(); // Показываем оверлей поверх всего
    acceptButton->setEnabled(true); rejectButton->setEnabled(true); acceptButton->setFocus(); // Активируем кнопки
    if (!gamePaused) { pauseGame(); qDebug() << "Order overlay shown, pausing game."; } // Ставим игру на паузу
}
void MainWindow::acceptOrderClicked() {
    // (Код acceptOrderClicked остается без изменений)
    qDebug() << "Accept clicked.";
    if (gameManager && orderOverlayWidget && orderOverlayWidget->isVisible()) {
        acceptButton->setEnabled(false); rejectButton->setEnabled(false); // Блокируем кнопки
        gameManager->onOrderAccepted(currentDisplayedOrder); // Сообщаем менеджеру о принятии
        resumeGame(); // Возобновляем игру
    }
}
void MainWindow::rejectOrderClicked() {
    // (Код rejectOrderClicked остается без изменений)
    qDebug() << "Reject clicked.";
    if (gameManager && orderOverlayWidget && orderOverlayWidget->isVisible()) {
        acceptButton->setEnabled(false); rejectButton->setEnabled(false); // Блокируем кнопки
        orderOverlayWidget->setVisible(false); // Скрываем оверлей
        gameManager->onOrderFailed(); // Сообщаем менеджеру об отказе
        // Игра остается на паузе, GameManager сгенерирует новый заказ
        qDebug() << "Order rejected, waiting for next order generation from GameManager.";
    }
}

// --- Слоты Turbo ---
void MainWindow::onTurboUpgradeClicked() {
    // (Код onTurboUpgradeClicked остается без изменений)
    qDebug() << "MainWindow: Turbo Upgrade button clicked.";
    if (gameManager) {
        // Вызываем слот покупки турбо в GameManager через очередь событий
        QMetaObject::invokeMethod(gameManager, "purchaseTurboUpgrade", Qt::QueuedConnection);
    }
}
void MainWindow::updateTurboButton(bool available) {
    // (Код updateTurboButton остается без изменений)
    // Обновляем доступность кнопки покупки турбо
    if (turboUpgradeButton) {
        turboUpgradeButton->setEnabled(available);
    }
}
