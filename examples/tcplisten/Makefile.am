AM_CXXFLAGS = -I../../libdvbtee

bin_PROGRAMS = dvbtee-tcplisten
dvbtee_tcplisten_SOURCES = tcplisten.cpp
dvbtee_tcplisten_LDADD = -L../../libdvbtee -ldvbtee

if DVBPSIBUILD
AM_CPPFLAGS = -I../../usr/include/
endif DVBPSIBUILD
