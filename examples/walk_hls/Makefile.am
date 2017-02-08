AM_CXXFLAGS = -I../../libdvbtee
AM_CXXFLAGS += -I../../libdvbtee/decode/table

bin_PROGRAMS = walk-hls
walk_hls_SOURCES = main.cpp hlsinput.cpp
walk_hls_LDADD = -L../../libdvbtee -ldvbtee

if DVBPSIBUILD
AM_CPPFLAGS = -I../../usr/include/
endif DVBPSIBUILD
