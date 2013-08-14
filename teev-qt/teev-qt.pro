#-------------------------------------------------
#
# Project created by QtCreator 2013-08-13T12:24:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia multimediawidgets

lessThan(QT_MAJOR_VERSION, 5) {
    QT += phonon
    QMAKE_CXXFLAGS += -DUSE_PHONON
}

TARGET = teev
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
