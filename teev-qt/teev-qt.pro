#-------------------------------------------------
#
# Project created by QtCreator 2013-08-13T12:24:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia multimediawidgets

lessThan(QT_MAJOR_VERSION, 5) {
    QT += phonon
    QMAKE_CXXFLAGS += -DUSE_JSONCPP
    LIBS += -ljsoncpp
}

unix|win32: LIBS += -lcurl

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libdvbtee/release/ -ldvbtee
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libdvbtee/debug/ -ldvbtee
else:symbian: LIBS += -ldvbtee
else:unix: LIBS += -L$$PWD/../libdvbtee/ -ldvbtee

INCLUDEPATH += $$PWD/../libdvbtee
DEPENDPATH += $$PWD/../libdvbtee

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libdvbtee_server/release/ -ldvbtee_server
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libdvbtee_server/debug/ -ldvbtee_server
else:symbian: LIBS += -ldvbtee_server
else:unix: LIBS += -L$$PWD/../libdvbtee_server/ -ldvbtee_server

INCLUDEPATH += $$PWD/../libdvbtee_server
DEPENDPATH += $$PWD/../libdvbtee_server

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../usr/lib/release/ -ldvbpsi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../usr/lib/debug/ -ldvbpsi
else:symbian: LIBS += -ldvbpsi
else:unix: LIBS += -L$$PWD/../usr/lib/ -ldvbpsi

INCLUDEPATH += $$PWD/../usr/include
DEPENDPATH += $$PWD/../usr/include

TARGET = teev
TEMPLATE = app

INCLUDEPATH += $$PWD/../tunerprovider/

LIBS += -L$$PWD/../tunerprovider/ -ldvbtee_tunerprovider

SOURCES += main.cpp\
	mainwindow.cpp \
	serverprovider.cpp

HEADERS  += mainwindow.h \
	serverprovider.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-deprecated -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__USE_LARGEFILE64 -D__STDC_FORMAT_MACROS -DUSE_HDHOMERUN

macx: INCLUDEPATH += /opt/local/include/

unix:!macx:!symbian: QMAKE_CXXFLAGS += -DUSE_LINUXTV
