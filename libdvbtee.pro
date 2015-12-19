QT =
TEMPLATE = subdirs

SUBDIRS += libdvbtee \
           libdvbtee/value \
           tunerprovider \
           libdvbtee_server \
           dvbtee \
           examples

libdvbtee.target = libdvbtee
valueobj.target = libdvbtee/value
tunerprovider.target = tunerprovider
libdvbtee_server.target = libdvbtee_server
dvbtee.target = dvbtee
examples.target = examples

libdvbtee.depends = libdvbtee/value

tunerprovider.depends = libdvbtee
libdvbtee_server.depends = libdvbtee
dvbtee.depends = libdvbtee_server
examples.depends = libdvbtee_server

QMAKE_EXTRA_TARGETS += libdvbtee \
                       valueobj \
                       tunerprovider \
                       libdvbtee_server \
                       dvbtee \
                       examples

OTHER_FILES += AUTHORS \
               COPYING \
               INSTALL \
               NEWS \
               README \
               README.md \
               ChangeLog \
               configure.ac \
               Makefile.am \
               libdvbtee.pc.in \
               libdvbtee_server.pc.in \
               packaging/libdvbtee.spec.in \
               .travis.yml \
               version.m4
