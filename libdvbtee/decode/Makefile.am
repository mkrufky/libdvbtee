AM_CXXFLAGS = -I.. -I../value -I./table -I./descriptor

if DVBPSIBUILD
AM_CXXFLAGS += -DFORCE_DECODER_LINKAGE -I../../usr/include/
endif DVBPSIBUILD

if CYGWIN
AM_CXXFLAGS += -DFORCE_DECODER_LINKAGE
endif CYGWIN

if MINGW
AM_CXXFLAGS += -DFORCE_DECODER_LINKAGE
endif MINGW

if MSYS
AM_CXXFLAGS += -DFORCE_DECODER_LINKAGE
endif MSYS

lib_LTLIBRARIES = libdvbtee_decode.la

libdvbtee_decode_la_SOURCES = decoder.cpp \
                              table/table.cpp \
                              descriptor/descriptor.cpp

# table decoders
if TA00
libdvbtee_decode_la_SOURCES += table/tabl_00.cpp table/tabl_00.h
endif TA00
if TA02
libdvbtee_decode_la_SOURCES += table/tabl_02.cpp table/tabl_02.h
endif TA02
if TA40
libdvbtee_decode_la_SOURCES += table/tabl_40.cpp table/tabl_40.h
endif TA40
if TA42
libdvbtee_decode_la_SOURCES += table/tabl_42.cpp table/tabl_42.h
endif TA42
if TA4E
libdvbtee_decode_la_SOURCES += table/tabl_4e.cpp table/tabl_4e.h
endif TA4E
if TA70
libdvbtee_decode_la_SOURCES += table/tabl_70.cpp table/tabl_70.h
endif TA70

# requires libdvbpsi v1.x.x
if TAC7
libdvbtee_decode_la_SOURCES += table/tabl_c7.cpp table/tabl_c7.h
endif TAC7
if TAC8
libdvbtee_decode_la_SOURCES += table/tabl_c8.cpp table/tabl_c8.h
endif TAC8
if TACB
libdvbtee_decode_la_SOURCES += table/tabl_cb.cpp table/tabl_cb.h
endif TACB
if TACC
libdvbtee_decode_la_SOURCES += table/tabl_cc.cpp table/tabl_cc.h
endif TACC
if TACD
libdvbtee_decode_la_SOURCES += table/tabl_cd.cpp table/tabl_cd.h
endif TACD

# descriptor decoders
if DR0A
libdvbtee_decode_la_SOURCES += descriptor/desc_0a.cpp descriptor/desc_0a.h
endif DR0A
if DR48
libdvbtee_decode_la_SOURCES += descriptor/desc_48.cpp descriptor/desc_48.h
endif DR48
if DR4D
libdvbtee_decode_la_SOURCES += descriptor/desc_4d.cpp descriptor/desc_4d.h
endif DR4D
if DR4E
libdvbtee_decode_la_SOURCES += descriptor/desc_4e.cpp descriptor/desc_4e.h
endif DR4E

# requires libdvbpsi v1.x.x
if DR62
libdvbtee_decode_la_SOURCES += descriptor/desc_62.cpp descriptor/desc_62.h
endif DR62
if DR83
libdvbtee_decode_la_SOURCES += descriptor/desc_83.cpp descriptor/desc_83.h
endif DR83

# requires libdvbpsi v1.2.x
if DR81
libdvbtee_decode_la_SOURCES += descriptor/desc_81.cpp descriptor/desc_81.h
endif DR81
if DR86
libdvbtee_decode_la_SOURCES += descriptor/desc_86.cpp descriptor/desc_86.h
endif DR86
if DRA0
libdvbtee_decode_la_SOURCES += descriptor/desc_a0.cpp descriptor/desc_a0.h
endif DRA0
if DRA1
libdvbtee_decode_la_SOURCES += descriptor/desc_a1.cpp descriptor/desc_a1.h
endif DRA1


EXTRA_DIST = decoder.h \
             table/table.h \
             descriptor/descript.h

DVBTEE_DECODE_LIBRARY_VERSION=0:0:0

libdvbtee_decode_la_LDFLAGS = -version-info $(DVBTEE_DECODE_LIBRARY_VERSION)

library_includedir=$(includedir)/dvbtee
library_include_HEADERS = decoder.h \
             table/table.h \
             descriptor/descript.h

libdvbtee_decode_la_LIBADD = -L../value -lvalueobj
