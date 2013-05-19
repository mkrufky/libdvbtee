#-------------------------------------------------
#
# Project created by QtCreator 2013-04-08T14:29:53
#
#-------------------------------------------------

QT       -= gui

TARGET = walk_hls
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    hlsinput.cpp \
    hlsfeed.cpp \
    curlhttpget.cpp

HEADERS += \
    hlsinput.h \
    hlsfeed.h \
    curlhttpget.h

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../libdvbtee/libdvbtee/release/ -ldvbtee
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../libdvbtee/libdvbtee/debug/ -ldvbtee
else:symbian: LIBS += -ldvbtee
else:unix: LIBS += -L$$PWD/../../libdvbtee/libdvbtee/ -ldvbtee

INCLUDEPATH += $$PWD/../../libdvbtee/libdvbtee
DEPENDPATH += $$PWD/../../libdvbtee/libdvbtee

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../libdvbtee/usr/lib/release/ -ldvbpsi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../libdvbtee/usr/lib/debug/ -ldvbpsi
else:symbian: LIBS += -ldvbpsi
else:unix: LIBS += -L$$PWD/../../libdvbtee/usr/lib/ -ldvbpsi

INCLUDEPATH += $$PWD/../../libdvbtee/usr/include
DEPENDPATH += $$PWD/../../libdvbtee/usr/include

unix|win32: LIBS += -lcurl

unix|win32: LIBS += -lhdhomerun

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__USE_LARGEFILE64
