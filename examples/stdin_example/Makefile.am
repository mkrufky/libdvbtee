AM_CXXFLAGS = -I../../libdvbtee

bin_PROGRAMS = conparse
conparse_SOURCES = conparse.cpp
conparse_LDADD = -L../../libdvbtee -ldvbtee

if DVBPSIBUILD
AM_CPPFLAGS = -I../../usr/include/
endif DVBPSIBUILD
