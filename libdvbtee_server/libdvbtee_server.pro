#-------------------------------------------------
#
# Project created by QtCreator 2013-03-06T00:21:30
#
#-------------------------------------------------

QT       -= gui
QT       -= core

TARGET = dvbtee_server
TEMPLATE = lib

DEFINES += DVBTEE_LIBRARY

SOURCES += serve.cpp \
    text.cpp

HEADERS += serve.h \
    text.h

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

disablehdhr {
} else {
    symbian: LIBS += -lhdhomerun
    macx: LIBS += -L/opt/local/lib/ -lhdhomerun
    else:unix|win32: LIBS += -L/usr/lib/ -lhdhomerun
}

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libdvbtee/release/ -ldvbtee
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libdvbtee/debug/ -ldvbtee
else:symbian: LIBS += -ldvbtee
else:unix: LIBS += -L$$PWD/../libdvbtee/ -ldvbtee

INCLUDEPATH += $$PWD/../libdvbtee
DEPENDPATH += $$PWD/../libdvbtee

QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-parameter -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__USE_LARGEFILE64

OTHER_FILES += Makefile.am
