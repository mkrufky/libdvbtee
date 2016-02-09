QT       -= gui
QT       -= core

TARGET = tcplisten
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    tcplisten.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libdvbtee/release/ -ldvbtee
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libdvbtee/debug/ -ldvbtee
else:symbian: LIBS += -ldvbtee
else:unix: LIBS += -L$$PWD/../../libdvbtee/ -ldvbtee

INCLUDEPATH += $$PWD/../../libdvbtee
DEPENDPATH += $$PWD/../../libdvbtee

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../usr/lib/release/ -ldvbpsi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../usr/lib/debug/ -ldvbpsi
else:symbian: LIBS += -ldvbpsi
else:unix: LIBS += -L$$PWD/../../usr/lib/ -ldvbpsi

INCLUDEPATH += $$PWD/../../usr/include
DEPENDPATH += $$PWD/../../usr/include

QMAKE_CXXFLAGS += -Wno-unused-parameter -D_TCP_OFFSET_BITS=64 -D_GNU_SOURCE -D__USE_LARGETCP64

OTHER_TCPS += Maketcp.am
