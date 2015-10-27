INCLUDEPATH += $$PWD

DESCRIPTOR_REQ_DVBPSI_V1_SOURCES = \
    $$PWD/desc_62.cpp \
    $$PWD/desc_81.cpp \
    $$PWD/desc_83.cpp \
    $$PWD/desc_86.cpp \
    $$PWD/desc_a0.cpp \
    $$PWD/desc_a1.cpp

DESCRIPTOR_SOURCES = \
    $$DESCRIPTOR_REQ_DVBPSI_V1_SOURCES \
    $$PWD/desc_0a.cpp \
    $$PWD/desc_48.cpp \
    $$PWD/desc_4d.cpp \
    $$PWD/descriptor.cpp

HEADERS += \
    $$PWD/desc_0a.h \
    $$PWD/desc_48.h \
    $$PWD/desc_4d.h \
    $$PWD/desc_62.h \
    $$PWD/desc_81.h \
    $$PWD/desc_83.h \
    $$PWD/desc_86.h \
    $$PWD/desc_a0.h \
    $$PWD/desc_a1.h \
    $$PWD/descriptor.h
