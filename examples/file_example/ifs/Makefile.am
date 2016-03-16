AM_CXXFLAGS = -I../../../libdvbtee -DIFS

if COVERALLS
AM_CXXFLAGS += -g -O0
endif COVERALLS

bin_PROGRAMS = dvbtee-ifsparser
dvbtee_ifsparser_SOURCES = ifsparser.cpp
dvbtee_ifsparser_LDADD = -L../../../libdvbtee -ldvbtee

if DVBPSIBUILD
AM_CPPFLAGS = -I../../../usr/include/
endif DVBPSIBUILD

TESTS = ./tests.sh
