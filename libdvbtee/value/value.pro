QT=

TARGET = valueobj
TEMPLATE = lib
CONFIG += staticlib

include ( value.pri )

SOURCES += $$VALUEOBJ_SOURCES

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-deprecated -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D__USE_LARGEFILE64 -D__STDC_FORMAT_MACROS
