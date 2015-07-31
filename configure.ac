#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libdvbtee], [0.3.7], [mkrufky@linuxtv.org])

AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_SRCDIR([libdvbtee/parse.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# libtool...
LT_INIT
AC_CONFIG_MACRO_DIR([m4])

# Checks for libraries.
AC_CHECK_LIB([curl], [curl_easy_init])
AC_CHECK_LIB([dvbpsi], [dvbpsi_pat_attach])
AC_CHECK_LIB([hdhomerun], [hdhomerun_device_destroy])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netdb.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h libhdhomerun/hdhomerun.h linux/dvb/frontend.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRCOLL
AC_CHECK_FUNCS([gethostname inet_ntoa localtime_r memchr memmove memset select socket stpcpy strchr strcspn strdup strerror strpbrk strrchr strspn strstr strtoul])

AC_CONFIG_FILES([Makefile
                 dvbtee/Makefile
                 libdvbtee/Makefile
                 libdvbtee_server/Makefile
                 tunerprovider/Makefile
                 examples/parser_example/Makefile
                 examples/server_example/Makefile
                 examples/walk_hls/Makefile
                 examples/Makefile
                 libdvbtee.pc])
AC_OUTPUT

echo "
libdvbtee configuration
-----------------------
libdvbtee version     : ${VERSION}
compile flags         : ${CFLAGS}
"
