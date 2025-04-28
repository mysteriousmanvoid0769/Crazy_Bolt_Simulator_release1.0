// startwindow.cpp

#include "startwindow.h"
#include "mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFont>
#include <QSpacerItem>
#include <QApplication>
#include <QPalette> // Для установки цвета фона и текста
#include <QLinearGradient> // Для градиентного фона
#include <QPainter> // Может понадобиться для градиента (но попробуем через стили)

StartWindow::StartWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setWindowTitle("Crazy Prius Simulator - Меню");
    setMinimumSize(600, 450); // Немного увеличим высоту
    // Устанавливаем стиль для всего окна (фон и базовый цвет текста)
    this->setStyleSheet(
        "QWidget {"
        "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,"
        "                                   stop:0 rgba(40, 40, 40, 255),"    // Темно-серый сверху
        "                                   stop:1 rgba(60, 60, 60, 255));"   // Чуть светлее снизу
        "   color: #FFFFFF;" // Белый цвет текста по умолчанию
        "}"
        );
}

StartWindow::~StartWindow()
{
}

void StartWindow::setupUi()
{
    // --- Создание виджетов ---
    titleLabel = new QLabel("Crazy Prius Simulator", this);
    subtitleLabel = new QLabel("(игра про болт такси в Азербайджане)", this);
    startButton = new QPushButton("Старт", this);
    aboutButton = new QPushButton("Об игре", this);
    authorsLabel = new QLabel("Авторы: Iskender Guliyev, Mehraj", this);

    // --- Настройка шрифтов ---
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(32); // Увеличим шрифт заголовка
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    // Можно добавить тень или другой цвет для заголовка, если хочется
    // titleLabel->setStyleSheet("color: #FFEEAA;"); // Пример: желтоватый цвет

    QFont subtitleFont = subtitleLabel->font();
    subtitleFont.setPointSize(12);
    subtitleFont.setItalic(true); // Сделаем подзаголовок курсивом
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #DDDDDD;"); // Чуть темнее белого

    QFont buttonFont = startButton->font();
    buttonFont.setPointSize(14);
    startButton->setFont(buttonFont);
    aboutButton->setFont(buttonFont);
    startButton->setMinimumHeight(45); // Сделать кнопки еще чуть больше
    aboutButton->setMinimumHeight(45);
    startButton->setCursor(Qt::PointingHandCursor); // Курсор-рука для кнопок
    aboutButton->setCursor(Qt::PointingHandCursor);

    authorsLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    authorsLabel->setStyleSheet("color: #AAAAAA; background-color: transparent;"); // Светло-серый, прозрачный фон

    // --- Стили кнопок ---
    QString buttonStyle =
        "QPushButton {"
        "   background-color: #0078D7;" // Синий цвет (можно взять зеленый #28A745)
        "   color: white;"
        "   border: none;"
        "   padding: 10px 25px;" // Отступы внутри кнопки
        "   border-radius: 15px;" // Скругленные углы
        "   font-size: 14pt;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005BAA;" // Более темный синий при наведении
        "}"
        "QPushButton:pressed {"
        "   background-color: #004C8C;" // Еще темнее при нажатии
        "}";
    startButton->setStyleSheet(buttonStyle);
    aboutButton->setStyleSheet(buttonStyle);

    // --- Компоновка ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 20); // Увеличим отступы по краям
    mainLayout->setSpacing(15); // Расстояние между элементами

    mainLayout->addStretch(1); // Растягиваем пространство сверху
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addSpacing(40); // Больший отступ после подзаголовка

    // Горизонтальная компоновка для кнопок
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(); // Пробел слева
    buttonLayout->addWidget(startButton);
    buttonLayout->addSpacing(20); // Расстояние между кнопками
    buttonLayout->addWidget(aboutButton);
    buttonLayout->addStretch(); // Пробел справа
    mainLayout->addLayout(buttonLayout);

    mainLayout->addStretch(2); // Больше растягивающегося пространства снизу

    // Добавляем авторов вниз
    mainLayout->addWidget(authorsLabel, 0, Qt::AlignRight); // Выравниваем по правому краю

    setLayout(mainLayout);

    // --- Соединение сигналов и слотов ---
    connect(startButton, &QPushButton::clicked, this, &StartWindow::onStartClicked);
    connect(aboutButton, &QPushButton::clicked, this, &StartWindow::onAboutClicked);
}

// Методы onStartClicked() и onAboutClicked() остаются без изменений...
void StartWindow::onStartClicked()
{
    mainWindow = new MainWindow();
    mainWindow->resize(1200, 900);
    mainWindow->show();
    this->close();
}

void StartWindow::onAboutClicked()
{
    // Используем стили для QMessageBox, чтобы он тоже был темным (опционально)
    QMessageBox aboutBox(this);
    aboutBox.setWindowTitle("Об игре");
    aboutBox.setTextFormat(Qt::RichText); // Чтобы работал HTML
    aboutBox.setText("<b>Crazy Prius Simulator</b> - игра про непростые будни таксиста Bolt в Баку!\n\n"
                     "<b>Приколы:</b>\n"
                     "<ul>" // Используем список
                     "<li>Реалистичная (почти) физика Prius!</li>"
                     "<li>Возможность установки TURBO!</li>"
                     "<li>Диалоги с пассажирами (иногда они могут вырвать...).</li>"
                     "<li>Штрафы за аварии и падение рейтинга.</li>"
                     "<li>Необходимость заправляться.</li>"
                     "<li>Чаевые от довольных клиентов (и недовольство от остальных).</li>"
                     "</ul>\n" // Конец списка
                     "Удачи на дорогах!");
    aboutBox.setIcon(QMessageBox::Information);
    aboutBox.setStandardButtons(QMessageBox::Ok);
    // Применяем темный стиль к QMessageBox
    aboutBox.setStyleSheet(
        "QMessageBox {"
        "    background-color: #444444;" // Фон окна сообщения
        "}"
        "QMessageBox QLabel {" // Текст сообщения
        "    color: #FFFFFF;"
        "    background-color: transparent;" // Убедимся, что фон метки прозрачный
        "}"
        "QMessageBox QPushButton {" // Кнопки в окне сообщения
        "    background-color: #0078D7;"
        "    color: white;"
        "    border: none;"
        "    padding: 5px 15px;"
        "    border-radius: 5px;"
        "    min-width: 60px;" // Минимальная ширина кнопки
        "}"
        "QMessageBox QPushButton:hover {"
        "    background-color: #005BAA;"
        "}"
        "QMessageBox QPushButton:pressed {"
        "    background-color: #004C8C;"
        "}"
        );
    aboutBox.exec();
}
