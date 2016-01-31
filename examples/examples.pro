QT =
TEMPLATE = subdirs

SUBDIRS += \
           file_example \
           parser_example \
           stdin_example \
           walk_hls

parser_example.target = parser_example
walk_hls.target = walk_hls

QMAKE_EXTRA_TARGETS += \
                       file_example \
                       parser_example \
                       stdin_example \
                       walk_hls

!macx {
    SUBDIRS += server_example
    server_example.target = server_example
    QMAKE_EXTRA_TARGETS += server_example
}


OTHER_FILES += Makefile.am
