QT =
TEMPLATE = subdirs

SUBDIRS += libdvbtee \
           tunerprovider \
           libdvbtee_server \
           dvbtee \
           server_example \
           parser_example \
           walk_hls

libdvbtee.target = libdvbtee
tunerprovider.target = tunerprovider
libdvbtee_server.target = libdvbtee_server
dvbtee.target = dvbtee
server_example.target = server_example
parser_example.target = parser_example
walk_hls.target = walk_hls

tunerprovider.depends = libdvbtee
libdvbtee_server.depends = libdvbtee
dvbtee.depends = libdvbtee_server
server_example.depends = libdvbtee_server
parser_example.depends = libdvbtee_server
walk_hls.depends = libdvbtee

QMAKE_EXTRA_TARGETS += libdvbtee \
                       tunerprovider \
                       libdvbtee_server \
                       dvbtee \
                       server_example \
                       parser_example \
                       walk_hls

OTHER_FILES += AUTHORS COPYING INSTALL NEWS README ChangeLog configure.ac Makefile.am
