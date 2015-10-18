#-------------------------------------------------
#
# Project created by QtCreator 2013-03-06T00:21:30
#
#-------------------------------------------------

QT       -= gui
QT       -= core

TARGET = dvbtee
TEMPLATE = lib

DEFINES += DVBTEE_LIBRARY

SOURCES += channels.cpp \
    decode.cpp \
    demux.cpp \
    desc.cpp \
    feed.cpp \
    functions.cpp \
    listen.cpp \
    output.cpp \
    parse.cpp \
    rbuf.cpp \
    stats.cpp \
    tune.cpp \
    hdhr_tuner.cpp \
    atsctext.cpp \
    hlsfeed.cpp \
    curlhttpget.cpp \
    log.cpp

HEADERS += atsctext.h \
    channels.h \
    decode.h \
    demux.h \
    desc.h \
    feed.h \
    functions.h \
    listen.h \
    log.h \
    output.h \
    parse.h \
    rbuf.h \
    stats.h \
    tune.h \
    hdhr_tuner.h \
    hlsfeed.h \
    curlhttpget.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE1EA1848
    TARGET.CAPABILITY =
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = dvbtee.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!macx:!symbian {
    HEADERS += linuxtv_tuner.h
    SOURCES += linuxtv_tuner.cpp
}

unix:!symbian {
    maemo5 {
	target.path = /opt/usr/lib
    } else {
	target.path = /usr/lib
    }
    INSTALLS += target
}

unix|win32: LIBS += -lcurl

LIBS += -L../usr/include/

LIBS += -ldvbpsi

symbian: LIBS += -ldvbpsi
else:unix|win32: LIBS += -L$$PWD/../usr/lib/ -ldvbpsi

INCLUDEPATH += $$PWD/../usr/include
DEPENDPATH += $$PWD/../usr/include

unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/../usr/lib/libdvbpsi.a

macx: LIBS += -L/opt/local/lib -liconv

symbian: LIBS += -lhdhomerun
macx: LIBS += -L/opt/local/lib/ -lhdhomerun
else:unix|win32: LIBS += -L/usr/lib/ -lhdhomerun

# some distros use a different location for libhdhomerun headers :-/
INCLUDEPATH += /opt/local/include
INCLUDEPATH += /usr/include/libhdhomerun
INCLUDEPATH += /usr/lib/libhdhomerun
DEPENDPATH += /usr/lib/libhdhomerun

QMAKE_CXXFLAGS += -Wno-unused-parameter -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__USE_LARGEFILE64 -D__STDC_FORMAT_MACROS
QMAKE_CXXFLAGS += -DHAVE_LIBCURL
QMAKE_CXXFLAGS += -DUSE_HDHOMERUN

unix:!macx:!symbian {
    QMAKE_CXXFLAGS += -DUSE_LINUXTV
}

OTHER_FILES += Makefile.am
