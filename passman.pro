#-------------------------------------------------
#
# Project created by QtCreator 2015-05-20T22:13:17
#
#-------------------------------------------------

CONFIG += c++11 stl_off
QT += core gui widgets

TARGET = passman
TEMPLATE = app

SOURCES += main.cc\
        mainwindow.cc \
    searchwidget.cc \
    clineedit.cpp \
    passmanapp.cc \
    utils.cc \
    passwordmanager.cc \
    passwordmodel.cc \
    platforms/globalshortcut.cc \
    platforms/platformhelper.cc

HEADERS  += mainwindow.h \
    searchwidget.h \
    clineedit.h \
    platforms/platformhelper.h \
    platforms/globalshortcut.h \
    passmanapp.h \
    utils.h \
    passwordmanager.h \
    passwordmodel.h

unix {
        QT += x11extras
        LIBS += -lX11 -lXtst -ldl
        QMAKE_CXXFLAGS += -Wno-deprecated-declarations
        HEADERS += \
                platforms/x11/platformhelper_x11.h \
                platforms/x11/globalshortcut_x11.h \
                platforms/x11/fakekey_x11.h
        SOURCES += \
                platforms/x11/platformhelper_x11.cc \
                platforms/x11/globalshortcut_x11.cc \
                platforms/x11/fakekey_x11.cc
}
