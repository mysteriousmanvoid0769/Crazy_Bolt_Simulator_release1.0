QT += core gui widgets
CONFIG += c++11
SOURCES += main.cpp mainwindow.cpp prius.cpp ordermanager.cpp gamemanager.cpp commentwindow.cpp dialogwindow.cpp \
    fuelcan.cpp \
    gameview.cpp \
    startwindow.cpp
HEADERS += mainwindow.h prius.h ordermanager.h gamemanager.h commentwindow.h dialogwindow.h \
    fuelcan.h \
    gameview.h \
    startwindow.h
QMAKE_CXXFLAGS += -O0
