QT += core gui widgets
QT += core gui widgets multimedia
CONFIG += c++11
SOURCES += main.cpp mainwindow.cpp prius.cpp ordermanager.cpp gamemanager.cpp commentwindow.cpp \
    fuelcan.cpp \
    gameview.cpp \
    startwindow.cpp
HEADERS += mainwindow.h prius.h ordermanager.h gamemanager.h commentwindow.h \
    fuelcan.h \
    gameview.h \
    startwindow.h
    RESOURCES = \
    resources.qrc
QMAKE_CXXFLAGS += -O0
