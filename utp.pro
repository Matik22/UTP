QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
TARGET = LibraryFund
TEMPLATE = app

SOURCES += \
main.cpp \
book.cpp \
catalog.cpp \
user.cpp \
issuerecord.cpp \
mainwindow.cpp \
issuedialog.cpp \
piechartwidget.cpp

HEADERS += \
book.h \
catalog.h \
user.h \
issuerecord.h \
mainwindow.h \
issuedialog.h \
piechartwidget.h

win32:CONFIG -= console
QMAKE_CXXFLAGS += -Wall -Wextra -pedantic
