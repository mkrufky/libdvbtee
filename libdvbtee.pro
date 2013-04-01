QT =
TEMPLATE = subdirs

SUBDIRS += libdvbtee \
           libdvbtee_server \
           dvbtee \
           server_example \
           parser_example

libdvbtee.target = libdvbtee
libdvbtee_server.target = libdvbtee_server
dvbtee.target = dvbtee
server_example.target = server_example
parser_example.target = parser_example

QMAKE_EXTRA_TARGETS += libdvbtee \
                       libdvbtee_server \
                       dvbtee \
                       server_example \
                       parser_example
