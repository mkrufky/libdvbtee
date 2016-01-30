INCLUDEPATH += $$PWD

FEEDER_SOURCES = \
    $$PWD/feed.cpp

HEADERS += \
    $$PWD/feed.h \
    $$PWD/feeder.h \
    $$PWD/file.h \
    $$PWD/fdfeeder.h

OTHER_FILES += $$PWD/Makefile.am

SOURCES += \
    $$PWD/feeder.cpp \
    $$PWD/file.cpp \
    $$PWD/fdfeeder.cpp
