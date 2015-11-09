INCLUDEPATH += $$PWD/../include/valobj

VALOBJ_SOURCES = \
    $$PWD/handle.cpp \
    $$PWD/array.cpp \
    $$PWD/object.cpp \
    $$PWD/value.cpp

HEADERS += \
    $$PWD/../include/valobj/handle.h \
    $$PWD/../include/valobj/array.h \
    $$PWD/../include/valobj/object.h \
    $$PWD/../include/valobj/value.h \
    $$PWD/value-macros.h
