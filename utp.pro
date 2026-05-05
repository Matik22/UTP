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
    chartwidget.cpp \
    logindialog.cpp


HEADERS += \
    book.h \
    catalog.h \
    user.h \
    issuerecord.h \
    mainwindow.h \
    issuedialog.h \
    chartwidget.h \
    librarian.h \
    logindialog.h \

win32:CONFIG -= console
QMAKE_CXXFLAGS += -Wall -Wextra -pedantic
CONFIG += utf8_source
CODECFORTR = UTF-8
CODECFORSRC = UTF-8
