AM_CXXFLAGS = -I../../libdvbtee

if COVERALLS
AM_CXXFLAGS += -g -O0
endif COVERALLS

bin_PROGRAMS = dvbtee-udplisten
dvbtee_udplisten_SOURCES = udplisten.cpp
dvbtee_udplisten_LDADD = -L../../libdvbtee -ldvbtee

if DVBPSIBUILD
AM_CPPFLAGS = -I../../usr/include/
endif DVBPSIBUILD

TESTS = ./tests.sh
