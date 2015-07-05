QT =
TEMPLATE = subdirs

SUBDIRS += libdvbtee \
           libdvbtee/value \
           tunerprovider \
           libdvbtee_server \
           dvbtee \
           server_example \
           parser_example \
           walk_hls

libdvbtee.target = libdvbtee
valueobj.target = libdvbtee/value
tunerprovider.target = tunerprovider
libdvbtee_server.target = libdvbtee_server
dvbtee.target = dvbtee
server_example.target = server_example
parser_example.target = parser_example
walk_hls.target = walk_hls

libdvbtee.depends = libdvbtee/value

tunerprovider.depends = libdvbtee
libdvbtee_server.depends = libdvbtee
dvbtee.depends = libdvbtee_server
server_example.depends = libdvbtee_server
parser_example.depends = libdvbtee_server
walk_hls.depends = libdvbtee

QMAKE_EXTRA_TARGETS += libdvbtee \
                       valueobj \
                       tunerprovider \
                       libdvbtee_server \
                       dvbtee \
                       server_example \
                       parser_example \
                       walk_hls
