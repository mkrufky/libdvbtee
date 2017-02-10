AM_CXXFLAGS = -I../../libdvbtee -I../../libdvbtee_server
AM_CXXFLAGS += -I../../libdvbtee/decode -I../../libdvbtee/decode/table -I../../libdvbtee/decode/descriptor

bin_PROGRAMS = dvbtee-server
dvbtee_server_SOURCES = server.cpp
dvbtee_server_LDADD = -L../../libdvbtee -L../../libdvbtee_server -ldvbtee -ldvbtee_server
dvbtee_server_LDADD += -L../../libdvbtee/legacy -ldvbtee_legacy

AM_CXXFLAGS += -DDVBTEE_FEED_LEGACY

if DVBPSIBUILD
AM_CPPFLAGS = -I../../usr/include/
endif DVBPSIBUILD
