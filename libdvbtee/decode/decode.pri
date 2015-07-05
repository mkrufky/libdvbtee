INCLUDEPATH += $$PWD

include ( table/table.pri )
include ( descriptor/descriptor.pri )

DECODER_SOURCES = \
    $$TABLE_SOURCES \
    $$DESCRIPTOR_SOURCES \
    $$PWD/decoder.cpp

HEADERS += \
    $$PWD/decoder.h
