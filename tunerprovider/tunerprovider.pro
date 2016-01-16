QT       -= core gui

TARGET = dvbtee_tunerprovider
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += ../libdvbtee/
INCLUDEPATH += ../usr/include

LIBS += -L../libdvbtee/ -ldvbtee
LIBS += -L../usr/lib/ -ldvbpsi

SOURCES += tunerprovider.cpp

HEADERS += tunerprovider.h

QMAKE_CXXFLAGS += -Wno-unused-parameter -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__USE_LARGEFILE64 -D__STDC_FORMAT_MACROS
QMAKE_CXXFLAGS += -DHAVE_ARPA_INET_H

disablehdhr {
} else {
    macx: LIBS += -L/opt/local/lib/ -lhdhomerun

    QMAKE_CXXFLAGS += -DUSE_HDHOMERUN
}

unix:!macx:!symbian: QMAKE_CXXFLAGS += -DUSE_LINUXTV

OTHER_FILES += Makefile.am
