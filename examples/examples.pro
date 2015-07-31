QT =
TEMPLATE = subdirs

SUBDIRS += \
           server_example \
           parser_example \
           walk_hls

server_example.target = server_example
parser_example.target = parser_example
walk_hls.target = walk_hls

QMAKE_EXTRA_TARGETS += \
                       server_example \
                       parser_example \
                       walk_hls

OTHER_FILES += Makefile.am
