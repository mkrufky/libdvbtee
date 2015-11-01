AM_CXXFLAGS = -I../libdvbtee

lib_LTLIBRARIES = libdvbtee_server.la

libdvbtee_server_la_SOURCES = \
    serve.cpp \
    text.cpp \
    text.h

EXTRA_DIST = serve.h

DVBTEE_SERVER_LIBRARY_VERSION=0:0:0

libdvbtee_server_la_LDFLAGS = -version-info $(DVBTEE_SERVER_LIBRARY_VERSION)

library_includedir=$(includedir)/dvbtee
library_include_HEADERS = serve.h

libdvbtee_server_la_LIBADD = -L../libdvbtee -ldvbtee
