AM_CXXFLAGS = -I.. -I../value -I./table -I./descriptor

lib_LTLIBRARIES = libdvbtee_decode.la

libdvbtee_decode_la_SOURCES = decoder.cpp \
                              table/table.cpp \
                              table/tabl_00.cpp \
                              table/tabl_02.cpp \
                              table/tabl_40.cpp \
                              table/tabl_42.cpp \
                              table/tabl_4e.cpp \
                              table/tabl_70.cpp \
                              descriptor/descriptor.cpp \
                              descriptor/desc_0a.cpp \
                              descriptor/desc_48.cpp \
                              descriptor/desc_4d.cpp \
                              table/tabl_00.h \
                              table/tabl_02.h \
                              table/tabl_40.h \
                              table/tabl_42.h \
                              table/tabl_4e.h \
                              table/tabl_70.h \
                              descriptor/desc_0a.h \
                              descriptor/desc_48.h \
                              descriptor/desc_4d.h

# requires libdvbpsi v1.x.x
libdvbtee_decode_la_SOURCES += \
                              table/tabl_c7.cpp \
                              table/tabl_c8.cpp \
                              table/tabl_cb.cpp \
                              table/tabl_cc.cpp \
                              table/tabl_cd.cpp \
                              descriptor/desc_62.cpp \
                              descriptor/desc_81.cpp \
                              descriptor/desc_83.cpp \
                              descriptor/desc_86.cpp \
                              descriptor/desc_a0.cpp \
                              descriptor/desc_a1.cpp \
                              table/tabl_c7.h \
                              table/tabl_c8.h \
                              table/tabl_cb.h \
                              table/tabl_cc.h \
                              table/tabl_cd.h \
                              descriptor/desc_62.h \
                              descriptor/desc_81.h \
                              descriptor/desc_83.h \
                              descriptor/desc_86.h \
                              descriptor/desc_a0.h \
                              descriptor/desc_a1.h

EXTRA_DIST = decoder.h \
             table/table.h \
             descriptor/descriptor.h

DVBTEE_DECODE_LIBRARY_VERSION=1:0:0

libdvbtee_decode_la_LDFLAGS = -version-info $(DVBTEE_DECODE_LIBRARY_VERSION)

library_includedir=$(includedir)/dvbtee
library_include_HEADERS = decoder.h \
             table/table.h \
             descriptor/descriptor.h

libdvbtee_decode_la_LIBADD = -L../value -lvalueobj
