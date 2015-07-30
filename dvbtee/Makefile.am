AM_CXXFLAGS = -I../libdvbtee -I../libdvbtee_server

bin_PROGRAMS = dvbtee
dvbtee_SOURCES = dvbtee.cpp
dvbtee_LDADD = -L../libdvbtee -L../libdvbtee_server -ldvbtee -ldvbtee_server
#MacOSX needs libiconv
#dvbtee_LDADD += -L/opt/local/lib -liconv
