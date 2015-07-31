QT =
TEMPLATE = subdirs

SUBDIRS += libdvbtee \
           tunerprovider \
           libdvbtee_server \
           dvbtee \
           examples

libdvbtee.target = libdvbtee
tunerprovider.target = tunerprovider
libdvbtee_server.target = libdvbtee_server
dvbtee.target = dvbtee
examples.target = examples

tunerprovider.depends = libdvbtee
libdvbtee_server.depends = libdvbtee
dvbtee.depends = libdvbtee_server
examples.depends = libdvbtee_server

QMAKE_EXTRA_TARGETS += libdvbtee \
                       tunerprovider \
                       libdvbtee_server \
                       dvbtee \
                       examples

OTHER_FILES += AUTHORS COPYING INSTALL NEWS README ChangeLog configure.ac Makefile.am
