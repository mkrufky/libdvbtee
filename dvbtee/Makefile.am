AM_CXXFLAGS = -I../libdvbtee -I../libdvbtee_server -D__STDC_FORMAT_MACROS

if COVERALLS
AM_CXXFLAGS += -g -O0
endif COVERALLS

bin_PROGRAMS = dvbtee
dvbtee_SOURCES = dvbtee.cpp
dvbtee_LDADD = -L../libdvbtee -L../libdvbtee_server -ldvbtee -ldvbtee_server

#MacOSX needs libiconv
#dvbtee_LDADD += -L/opt/local/lib -liconv

if DVBPSIBUILD
AM_CPPFLAGS = -I../usr/include/
endif DVBPSIBUILD
