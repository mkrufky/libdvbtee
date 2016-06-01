AM_CXXFLAGS = -I../include/valobj

if COVERALLS
AM_CXXFLAGS += -g -O0
endif COVERALLS

lib_LTLIBRARIES = libvalueobj.la

libvalueobj_la_SOURCES = value.cpp object.cpp array.cpp handle.cpp

EXTRA_DIST=value.h object.h array.h handle.h

VALOBJ_LIBRARY_VERSION=1:0:0

libvalueobj_la_LDFLAGS = -version-info $(VALOBJ_LIBRARY_VERSION)

library_includedir=$(includedir)/valobj
library_include_HEADERS = value.h object.h array.h handle.h
