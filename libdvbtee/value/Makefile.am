AM_CXXFLAGS = -I../include/valobj

lib_LTLIBRARIES = libvalueobj.la

libvalueobj_la_SOURCES = value.cpp object.cpp array.cpp

EXTRA_DIST=../include/valobj/array.h \
           ../include/valobj/object.h \
           ../include/valobj/value.h

VALOBJ_LIBRARY_VERSION=1:0:0

libvalueobj_la_LDFLAGS = -version-info $(VALOBJ_LIBRARY_VERSION)

library_includedir=$(includedir)/valobj
library_include_HEADERS = ../include/valobj/array.h \
                          ../include/valobj/object.h \
                          ../include/valobj/value.h
