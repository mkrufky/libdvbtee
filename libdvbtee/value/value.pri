INCLUDEPATH += $$PWD

VALOBJ_SOURCES = \
    $$PWD/handle.cpp \
    $$PWD/array.cpp \
    $$PWD/object.cpp \
    $$PWD/value.cpp

HEADERS += \
    $$PWD/handle.h \
    $$PWD/array.h \
    $$PWD/object.h \
    $$PWD/value-macros.h \
    $$PWD/value.h

OTHER_FILES += $$PWD/Makefile.am
