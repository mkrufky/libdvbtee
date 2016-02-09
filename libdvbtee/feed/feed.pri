INCLUDEPATH += $$PWD

FEEDER_SOURCES = \
    $$PWD/feed.cpp

HEADERS += \
    $$PWD/feed.h \
    $$PWD/feeder.h \
    $$PWD/file.h \
    $$PWD/ifsfeeder.h \
    $$PWD/fdfeeder.h \
    $$PWD/tcp.h \
    $$PWD/udp.h \
    $$PWD/stdin.h

OTHER_FILES += $$PWD/Makefile.am

SOURCES += \
    $$PWD/feeder.cpp \
    $$PWD/file.cpp \
    $$PWD/ifsfeeder.cpp \
    $$PWD/fdfeeder.cpp \
    $$PWD/tcp.cpp \
    $$PWD/udp.cpp \
    $$PWD/stdin.cpp