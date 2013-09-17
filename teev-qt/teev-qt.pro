#-------------------------------------------------
#
# Project created by QtCreator 2013-08-13T12:24:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia multimediawidgets

lessThan(QT_MAJOR_VERSION, 5) {
    QT += phonon
}

LIBS += -ljsoncpp

INCLUDEPATH += $$PWD/../libdvbtee
DEPENDPATH += $$PWD/../libdvbtee

INCLUDEPATH += $$PWD/../libdvbtee_server
DEPENDPATH += $$PWD/../libdvbtee_server

LIBS += -L$$PWD/../libdvbtee/ -ldvbtee
LIBS += -L$$PWD/../libdvbtee_server/ -ldvbtee_server

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../libdvbtee/usr/lib/release/ -ldvbpsi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../libdvbtee/usr/lib/debug/ -ldvbpsi
else:symbian: LIBS += -ldvbpsi
else:unix: LIBS += -L$$PWD/../../libdvbtee/usr/lib/ -ldvbpsi

TARGET = teev
TEMPLATE = app


SOURCES += main.cpp\
	mainwindow.cpp \
	tunerprovider.cpp

HEADERS  += mainwindow.h \
	tunerprovider.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -DUSE_HDHOMERUN

unix:!macx:!symbian: QMAKE_CXXFLAGS += -DUSE_LINUXTV
