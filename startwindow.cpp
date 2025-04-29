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
#include <QPalette>
#include <QLinearGradient>
#include <QPainter>

StartWindow::StartWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setWindowTitle("Crazy Prius Simulator - Меню");
    setMinimumSize(600, 450);

    this->setStyleSheet(
        "QWidget {"
        "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,"
        "                                   stop:0 rgba(40, 40, 40, 255),"
        "                                   stop:1 rgba(60, 60, 60, 255));"
        "   color: #FFFFFF;"
        "}"
        );
}

StartWindow::~StartWindow()
{
}

void StartWindow::setupUi()
{

    titleLabel = new QLabel("Crazy Prius Simulator", this);
    subtitleLabel = new QLabel("(игра про болт такси в Азербайджане)", this);
    startButton = new QPushButton("Старт", this);
    aboutButton = new QPushButton("Об игре", this);
    authorsLabel = new QLabel("Авторы: Iskender Guliyev, Mehraj Chalabov", this);


    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(32);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);


    QFont subtitleFont = subtitleLabel->font();
    subtitleFont.setPointSize(12);
    subtitleFont.setItalic(true);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #DDDDDD;");

    QFont buttonFont = startButton->font();
    buttonFont.setPointSize(14);
    startButton->setFont(buttonFont);
    aboutButton->setFont(buttonFont);
    startButton->setMinimumHeight(45);
    aboutButton->setMinimumHeight(45);
    startButton->setCursor(Qt::PointingHandCursor);
    aboutButton->setCursor(Qt::PointingHandCursor);

    authorsLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    authorsLabel->setStyleSheet("color: #AAAAAA; background-color: transparent;");


    QString buttonStyle =
        "QPushButton {"
        "   background-color: #0078D7;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 25px;"
        "   border-radius: 15px;"
        "   font-size: 14pt;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005BAA;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004C8C;"
        "}";
    startButton->setStyleSheet(buttonStyle);
    aboutButton->setStyleSheet(buttonStyle);


    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 20);
    mainLayout->setSpacing(15);

    mainLayout->addStretch(1);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addSpacing(40);


    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(startButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(aboutButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    mainLayout->addStretch(2);


    mainLayout->addWidget(authorsLabel, 0, Qt::AlignRight);

    setLayout(mainLayout);


    connect(startButton, &QPushButton::clicked, this, &StartWindow::onStartClicked);
    connect(aboutButton, &QPushButton::clicked, this, &StartWindow::onAboutClicked);
}


void StartWindow::onStartClicked()
{
    mainWindow = new MainWindow();
    mainWindow->resize(1200, 900);
    mainWindow->show();
    this->close();
}

void StartWindow::onAboutClicked()
{

    QMessageBox aboutBox(this);
    aboutBox.setWindowTitle("Об игре");
    aboutBox.setTextFormat(Qt::RichText);
    aboutBox.setText("<b>Crazy Prius Simulator</b> - игра про непростые будни таксиста Bolt в Баку!\n\n"
                     "<b>Приколы:</b>\n"
                     "<ul>"
                     "<li>Реалистичная (почти) физика Prius!</li>"
                     "<li>Возможность установки TURBO!</li>"
                     "<li>Диалоги с пассажирами (иногда они могут вырвать...).</li>"
                     "<li>Штрафы за аварии и падение рейтинга.</li>"
                     "<li>Необходимость заправляться.</li>"
                     "<li>Чаевые от довольных клиентов (и недовольство от остальных).</li>"
                     "</ul>\n"
                     "Удачи на дорогах!");
    aboutBox.setIcon(QMessageBox::Information);
    aboutBox.setStandardButtons(QMessageBox::Ok);

    aboutBox.setStyleSheet(
        "QMessageBox {"
        "    background-color: #444444;"
        "}"
        "QMessageBox QLabel {"
        "    color: #FFFFFF;"
        "    background-color: transparent;"
        "}"
        "QMessageBox QPushButton {"
        "    background-color: #0078D7;"
        "    color: white;"
        "    border: none;"
        "    padding: 5px 15px;"
        "    border-radius: 5px;"
        "    min-width: 60px;"
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
