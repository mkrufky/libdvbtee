AM_CXXFLAGS = -I../libdvbtee -I../libdvbtee_server -D__STDC_FORMAT_MACROS

bin_PROGRAMS = dvbtee
dvbtee_SOURCES = dvbtee.cpp
dvbtee_LDADD = -L../libdvbtee -L../libdvbtee_server -ldvbtee -ldvbtee_server
#MacOSX needs libiconv
#dvbtee_LDADD += -L/opt/local/lib -liconv
