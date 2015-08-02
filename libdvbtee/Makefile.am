AM_CXXFLAGS = -I/usr/include/libhdhomerun -I/opt/local/include -pthread

lib_LTLIBRARIES = libdvbtee.la

libdvbtee_la_SOURCES = atsctext.cpp channels.cpp curlhttpget.cpp decode.cpp demux.cpp desc.cpp feed.cpp functions.cpp hdhr_tuner.cpp hlsfeed.cpp linuxtv_tuner.cpp listen.cpp output.cpp parse.cpp rbuf.cpp stats.cpp tune.cpp

EXTRA_DIST = atsctext.h channels.h curlhttpget.h decode.h demux.h feed.h functions.h hdhr_tuner.h hlsfeed.h linuxtv_tuner.h listen.h log.h output.h parse.h rbuf.h stats.h tune.h

DVBTEE_LIBRARY_VERSION=1:0:0

libdvbtee_la_LDFLAGS = -version-info $(DVBTEE_LIBRARY_VERSION)

library_includedir=$(includedir)/dvbtee
library_include_HEADERS = atsctext.h channels.h curlhttpget.h decode.h demux.h feed.h functions.h hdhr_tuner.h hlsfeed.h linuxtv_tuner.h listen.h log.h output.h parse.h rbuf.h stats.h tune.h

libdvbtee_la_LIBADD = -ldvbpsi
