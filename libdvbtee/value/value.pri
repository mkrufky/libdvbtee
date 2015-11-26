INCLUDEPATH += $$PWD/../include/valobj

VALOBJ_SOURCES = \
    $$PWD/array.cpp \
    $$PWD/object.cpp \
    $$PWD/value.cpp

HEADERS += \
    $$PWD/../include/valobj/array.h \
    $$PWD/../include/valobj/object.h \
    $$PWD/../include/valobj/value.h \
    $$PWD/value-macros.h
