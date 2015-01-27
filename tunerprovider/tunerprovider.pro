QT       -= core gui

TARGET = dvbtee_tunerprovider
TEMPLATE = lib
CONFIG += staticlib

include ( ../libdvbtee/libdvbtee_include.pri )

INCLUDEPATH += ../libdvbtee/
INCLUDEPATH += ../usr/include

LIBS += -L../libdvbtee/ -ldvbtee
LIBS += -L../usr/lib/ -ldvbpsi

SOURCES += tunerprovider.cpp

HEADERS += tunerprovider.h

macx: LIBS += -L/opt/local/lib/ -lhdhomerun

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-deprecated -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__USE_LARGEFILE64 -D__STDC_FORMAT_MACROS -DUSE_HDHOMERUN
unix:!macx:!symbian: QMAKE_CXXFLAGS += -DUSE_LINUXTV
