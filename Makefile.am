ACLOCAL_AMFLAGS= -I m4

if OLDDECODER
SUBDIRS=
else !OLDDECODER
SUBDIRS=libdvbtee/value libdvbtee/decode
endif !OLDDECODER
SUBDIRS+=libdvbtee libdvbtee_server tunerprovider examples dvbtee

if DVBPSIBUILD
SUBDIRS+=libdvbpsi
endif DVBPSIBUILD

EXTRA_DIST = libdvbtee_server.pc.in libdvbtee.pc.in packaging/libdvbtee.spec.in

if HAVE_PKG_CONFIG
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libdvbtee.pc libdvbtee_server.pc
endif
