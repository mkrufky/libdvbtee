#-------------------------------------------------
#
# Project created by QtCreator 2013-03-31T23:14:10
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = parser
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    parser.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libdvbtee/release/ -ldvbtee
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libdvbtee/debug/ -ldvbtee
else:symbian: LIBS += -ldvbtee
else:unix: LIBS += -L$$PWD/../libdvbtee/ -ldvbtee

INCLUDEPATH += $$PWD/../libdvbtee
DEPENDPATH += $$PWD/../libdvbtee

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../usr/lib/release/ -ldvbpsi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../usr/lib/debug/ -ldvbpsi
else:symbian: LIBS += -ldvbpsi
else:unix: LIBS += -L$$PWD/../usr/lib/ -ldvbpsi

INCLUDEPATH += $$PWD/../usr/include
DEPENDPATH += $$PWD/../usr/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libdvbtee_server/release/ -ldvbtee_server
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libdvbtee_server/debug/ -ldvbtee_server
else:symbian: LIBS += -ldvbtee_server
else:unix: LIBS += -L$$PWD/../libdvbtee_server/ -ldvbtee_server

INCLUDEPATH += $$PWD/../libdvbtee_server
DEPENDPATH += $$PWD/../libdvbtee_server

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-deprecated -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__USE_LARGEFILE64
