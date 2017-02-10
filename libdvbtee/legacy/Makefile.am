AM_CXXFLAGS = -I.. -I../decode -I../decode/table -I../decode/descriptor -I/usr/include/libhdhomerun -I/usr/lib/libhdhomerun -I/opt/local/include -pthread -D__STDC_FORMAT_MACROS

if COVERALLS
AM_CXXFLAGS += -g -O0
endif COVERALLS

lib_LTLIBRARIES = libdvbtee_legacy.la

libdvbtee_legacy_la_SOURCES = \
    hdhr_tuner.cpp \
    linuxtv_tuner.cpp \
    tune.cpp

DVBTEE_LIBRARY_VERSION=0:0:0

libdvbtee_legacy_la_LDFLAGS = -version-info $(DVBTEE_LIBRARY_VERSION)

AM_CXXFLAGS += -DDVBTEE_FEED_LEGACY

if DVBPSIBUILD
AM_CPPFLAGS = -I../../usr/include/
endif DVBPSIBUILD
