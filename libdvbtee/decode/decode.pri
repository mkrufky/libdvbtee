include ( decode_include.pri )

include ( table/table.pri )
include ( descriptor/descriptor.pri )

SOURCES += \
    $$PWD/decode.cpp \
    $$PWD/desc.cpp \
    $$PWD/value.cpp \
    $$PWD/decoder.cpp

HEADERS += \
    $$PWD/decode.h \
    $$PWD/desc.h \
    $$PWD/value.h \
    $$PWD/decoder.h
