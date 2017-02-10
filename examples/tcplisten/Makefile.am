AM_CXXFLAGS = -I../../libdvbtee
AM_CXXFLAGS += -I../../libdvbtee/decode -I../../libdvbtee/decode/table -I../../libdvbtee/decode/descriptor

if COVERALLS
AM_CXXFLAGS += -g -O0
endif COVERALLS

bin_PROGRAMS = dvbtee-tcplisten
dvbtee_tcplisten_SOURCES = tcplisten.cpp
dvbtee_tcplisten_LDADD = -L../../libdvbtee -ldvbtee

if DVBPSIBUILD
AM_CPPFLAGS = -I../../usr/include/
endif DVBPSIBUILD
