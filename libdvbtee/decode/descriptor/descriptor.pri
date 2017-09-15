INCLUDEPATH += $$PWD

DESCRIPTOR_REQ_DVBPSI_V0_2_SOURCES = \
    $$PWD/desc_0a.cpp \
    $$PWD/desc_48.cpp \
    $$PWD/desc_4d.cpp \
    $$PWD/desc_4e.cpp

DESCRIPTOR_REQ_DVBPSI_V1_SOURCES = \
    $$DESCRIPTOR_REQ_DVBPSI_V0_2_SOURCES \
    $$PWD/desc_62.cpp \
    $$PWD/desc_83.cpp

DESCRIPTOR_REQ_DVBPSI_V1_2_SOURCES = \
    $$DESCRIPTOR_REQ_DVBPSI_V1_SOURCES \
    $$PWD/desc_81.cpp \
    $$PWD/desc_86.cpp \
    $$PWD/desc_a0.cpp \
    $$PWD/desc_a1.cpp

DESCRIPTOR_SOURCES = \
    $$PWD/descriptor.cpp

# FIXME: autodetect
DESCRIPTOR_SOURCES += \
    $$DESCRIPTOR_REQ_DVBPSI_V1_2_SOURCES

HEADERS += \
    $$PWD/desc_0a.h \
    $$PWD/desc_48.h \
    $$PWD/desc_4d.h \
    $$PWD/desc_4e.h \
    $$PWD/desc_62.h \
    $$PWD/desc_81.h \
    $$PWD/desc_83.h \
    $$PWD/desc_86.h \
    $$PWD/desc_a0.h \
    $$PWD/desc_a1.h \
    $$PWD/descript.h
