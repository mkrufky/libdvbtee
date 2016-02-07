AM_CXXFLAGS = -I../../libdvbtee

bin_PROGRAMS = dvbtee-udplisten
dvbtee_udplisten_SOURCES = udplisten.cpp
dvbtee_udplisten_LDADD = -L../../libdvbtee -ldvbtee

if DVBPSIBUILD
AM_CPPFLAGS = -I../../usr/include/
endif DVBPSIBUILD
