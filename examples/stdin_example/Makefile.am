AM_CXXFLAGS = -I../../libdvbtee

if COVERALLS
AM_CXXFLAGS += -g -O0
endif COVERALLS

bin_PROGRAMS = conparse
conparse_SOURCES = conparse.cpp
conparse_LDADD = -L../../libdvbtee -ldvbtee

if DVBPSIBUILD
AM_CPPFLAGS = -I../../usr/include/
endif DVBPSIBUILD

TESTS = ./tests.sh
