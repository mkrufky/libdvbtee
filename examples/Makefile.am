#SUBDIRS = file_example udplisten parser_example stdin_example walk_hls
SUBDIRS = file_example udplisten stdin_example walk_hls

if !BUILD_OS_IS_DARWIN
if !MINGW
if !CYGWIN
SUBDIRS += server_example
endif !CYGWIN
endif !MINGW
endif !BUILD_OS_IS_DARWIN
