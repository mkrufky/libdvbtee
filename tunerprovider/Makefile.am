AM_CXXFLAGS = -I../libdvbtee -I/usr/include/libhdhomerun
AM_CXXFLAGS += -I../libdvbtee/decode -I../libdvbtee/decode/table

lib_LTLIBRARIES = libdvbtee_tunerprovider.la

libdvbtee_tunerprovider_la_SOURCES = tunerprovider.cpp

EXTRA_DIST = tunerprovider.h

if DVBPSIBUILD
AM_CPPFLAGS = -I../usr/include/
endif DVBPSIBUILD

DVBTEE_TUNERPROVIDER_LIBRARY_VERSION=0:0:0

libdvbtee_tunerprovider_la_LDFLAGS = -version-info $(DVBTEE_TUNERPROVIDER_LIBRARY_VERSION)

library_includedir=$(includedir)/dvbtee
library_include_HEADERS = tunerprovider.h
