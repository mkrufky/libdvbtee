AM_CXXFLAGS = -I../../libdvbtee -I../../libdvbtee_server

bin_PROGRAMS = dvbtee-server
dvbtee_server_SOURCES = server.cpp
dvbtee_server_LDADD = -L../../libdvbtee/legacy -L../../libdvbtee -L../../libdvbtee_server -ldvbtee -ldvbtee_server -ldvbtee_legacy

if DVBPSIBUILD
AM_CPPFLAGS = -I../../usr/include/
endif DVBPSIBUILD

AM_CXXFLAGS += -DDVBTEE_FEED_LEGACY
