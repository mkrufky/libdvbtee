AM_CXXFLAGS = -I./decode -I./decode/table -I/usr/include/libhdhomerun -I/opt/local/include -pthread

lib_LTLIBRARIES = libdvbtee.la

libdvbtee_la_SOURCES = \
    atsctext.cpp \
    channels.cpp \
    curlhttpget.cpp \
    decode.cpp \
    demux.cpp \
    feed.cpp \
    functions.cpp \
    hdhr_tuner.cpp \
    hlsfeed.cpp \
    linuxtv_tuner.cpp \
    listen.cpp \
    log.cpp \
    output.cpp \
    parse.cpp \
    rbuf.cpp \
    stats.cpp \
    tune.cpp

EXTRA_DIST = \
    atsctext.h \
    channels.h \
    curlhttpget.h \
    decode.h \
    demux.h \
    feed.h \
    functions.h \
    hdhr_tuner.h \
    hlsfeed.h \
    linuxtv_tuner.h \
    listen.h \
    log.h \
    output.h \
    parse.h \
    rbuf.h \
    stats.h \
    tune.h

DVBTEE_LIBRARY_VERSION=0:0:0

libdvbtee_la_LDFLAGS = -version-info $(DVBTEE_LIBRARY_VERSION)

library_includedir=$(includedir)/dvbtee
library_include_HEADERS = \
    atsctext.h \
    channels.h \
    curlhttpget.h \
    decode.h \
    demux.h \
    feed.h \
    functions.h \
    hdhr_tuner.h \
    hlsfeed.h \
    linuxtv_tuner.h \
    listen.h \
    log.h \
    output.h \
    parse.h \
    rbuf.h \
    stats.h \
    tune.h

# enable the next two lines and disable the one following in order to use the old decoder
if OLDDECODER
libdvbtee_la_SOURCES += desc.cpp
AM_CXXFLAGS += -DOLD_DECODER=1
libdvbtee_la_LIBADD =
else !OLDDECODER
libdvbtee_la_LIBADD = -L./value -L./decode -lvalueobj -ldvbtee_decode
endif !OLDDECODER

if BUILD_OS_IS_DARWIN
libdvbtee_la_LIBADD += -L/opt/local/lib -liconv
endif BUILD_OS_IS_DARWIN
