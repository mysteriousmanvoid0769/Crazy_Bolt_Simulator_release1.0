QT += core gui widgets
CONFIG += c++11
SOURCES += main.cpp mainwindow.cpp prius.cpp ordermanager.cpp gamemanager.cpp commentwindow.cpp \
    fuelcan.cpp \
    gameview.cpp \
    startwindow.cpp
HEADERS += mainwindow.h prius.h ordermanager.h gamemanager.h commentwindow.h \
    fuelcan.h \
    gameview.h \
    startwindow.h


folder_Images.files = $$PWD/pictures
folder_Images.path = pictures
INSTALLS += folder_Images

QMAKE_CXXFLAGS += -O0
