#-------------------------------------------------
#
# Project created by QtCreator 2013-03-06T00:21:30
#
#-------------------------------------------------

QT       -= gui

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
    linuxtv_tuner.cpp \
    atsctext.cpp

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
    linuxtv_tuner.h \
    hdhr_tuner.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE1EA1848
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = dvbtee.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

LIBS += -L../usr/include/

LIBS += -ldvbpsi

symbian: LIBS += -ldvbpsi
else:unix|win32: LIBS += -L$$PWD/../usr/lib/ -ldvbpsi

INCLUDEPATH += $$PWD/../usr/include
DEPENDPATH += $$PWD/../usr/include

unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/../usr/lib/libdvbpsi.a

symbian: LIBS += -lhdhomerun
else:unix|win32: LIBS += -L/usr/lib/ -lhdhomerun

INCLUDEPATH += /usr/lib/libhdhomerun
DEPENDPATH += /usr/lib/libhdhomerun
