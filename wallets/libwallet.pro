#-------------------------------------------------
#
# Project created by QtCreator 2015-05-22T00:41:53
#
#-------------------------------------------------

CONFIG += plugin
QT -= gui

TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++11

DEFINES += LIBWALLET_LIBRARY

HEADERS += wallet.h \
    wallet_stringlist.h \
    wallet_buffer.h \
    wallet_stringmap.h

SOURCES += wallet.cc

TARGET = wallet_dummy

kwallet4 {
	SOURCES = kwallet4/wallet.cc
	LIBS += -lkwalletbackend -lkdeui
	TARGET = wallet_kw4
}

kwallet5 {
	SOURCES = kwallet5/wallet.cc
	LIBS += -lkwalletbackend5 -lkdecore
	TARGET = wallet_kw5
}
