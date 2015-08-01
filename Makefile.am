ACLOCAL_AMFLAGS= -I m4

SUBDIRS=libdvbtee libdvbtee_server tunerprovider examples dvbtee

EXTRA_DIST = libdvbtee_server.pc.in libdvbtee.pc.in

if HAVE_PKG_CONFIG
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libdvbtee.pc libdvbtee_server.pc
endif
