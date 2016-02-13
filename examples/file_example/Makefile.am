AM_CXXFLAGS = -I../../libdvbtee

bin_PROGRAMS = dvbtee-fparser
dvbtee_fparser_SOURCES = fparser.cpp
dvbtee_fparser_LDADD = -L../../libdvbtee -ldvbtee

if DVBPSIBUILD
AM_CPPFLAGS = -I../../usr/include/
endif DVBPSIBUILD

TESTS = ./tests.sh
