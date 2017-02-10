#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_include([version.m4])
AC_PREREQ([2.50])
AC_INIT([libdvbtee], m4_defn([DVBTEE_VERSION]), [mkrufky@linuxtv.org])
AC_CONFIG_AUX_DIR([.])

# Check target OS
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_SRCDIR([libdvbtee/parse.cpp])
AC_CONFIG_HEADERS([config.h])

# Check configure args
AC_ARG_ENABLE([coveralls],
[  --enable-coveralls    Enable coverage checker],
[case "${enableval}" in
  yes) coveralls=true ;;
  no)  coveralls=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-coveralls]) ;;
esac],[coveralls=false])
AM_CONDITIONAL([COVERALLS], [test x$coveralls = xtrue])

# Check configure args
AC_ARG_ENABLE([olddecoder],
[  --enable-olddecoder    Enable old decoder],
[case "${enableval}" in
  yes) olddecoder=true ;;
  no)  olddecoder=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-olddecoder]) ;;
esac],[olddecoder=false])
AM_CONDITIONAL([OLDDECODER], [test x$olddecoder = xtrue])

AC_ARG_ENABLE([hdhr],
    AS_HELP_STRING([--disable-hdhr], [Disable HDHomeRun support]))

AC_ARG_ENABLE([dvbpsibuild],
[  --enable-dvbpsibuild    Enable dvbpsi build],
[case "${enableval}" in
  yes) dvbpsibuild=true ;;
  no)  dvbpsibuild=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-dvbpsibuild]) ;;
esac],[dvbpsibuild=false])
AM_CONDITIONAL([DVBPSIBUILD], [test x$dvbpsibuild = xtrue])

# Check OS
AC_CANONICAL_BUILD

AM_CONDITIONAL([BUILD_OS_IS_DARWIN], [case $build_os in darwin* ) true ;; * ) false ;; esac])
AM_CONDITIONAL([DARWIN], [case $target_os in darwin* ) true ;; * ) false ;; esac])
AM_CONDITIONAL([CYGWIN], [case $target_os in cygwin* ) true ;; * ) false ;; esac])
AM_CONDITIONAL([MINGW], [case $target_os in mingw* ) true ;; * ) false ;; esac])
AM_CONDITIONAL([MSYS], [case $target_os in msys* ) true ;; * ) false ;; esac])

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

AS_IF([test "x$enable_hdhr" != "xno"], [
  AC_CHECK_LIB([hdhomerun], [hdhomerun_device_destroy])
  AC_CHECK_HEADERS([libhdhomerun/hdhomerun.h])
])

if test -d "$srcdir/libdvbpsi"; then
  AM_COND_IF([DVBPSIBUILD], [
    AC_CONFIG_SUBDIRS([libdvbpsi])
  ])
fi

AM_COND_IF([DVBPSIBUILD], [], [
  AC_CHECK_LIB([dvbpsi], [dvbpsi_pat_attach], [], [AC_CHECK_LIB([dvbpsi], [dvbpsi_AttachPAT])])
])

# Checks for individual table support in libdvbpsi.
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([TA00], [true])
], [
  AC_CHECK_FUNC([dvbpsi_pat_attach],[TA00=true],[TA00=false])
  AC_CHECK_FUNC([dvbpsi_AttachPAT],[TA00=true])
  AM_CONDITIONAL([TA00], [test x$TA00 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([TA02], [true])
], [
  AC_CHECK_FUNC([dvbpsi_pmt_attach],[TA02=true],[TA02=false])
  AC_CHECK_FUNC([dvbpsi_AttachPMT],[TA02=true])
  AM_CONDITIONAL([TA02], [test x$TA02 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([TA40], [true])
], [
  AC_CHECK_FUNC([dvbpsi_nit_attach],[TA40=true],[TA40=false])
  AC_CHECK_FUNC([dvbpsi_AttachNIT],[TA40=true])
  AM_CONDITIONAL([TA40], [test x$TA40 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([TA42], [true])
], [
  AC_CHECK_FUNC([dvbpsi_sdt_attach],[TA42=true],[TA42=false])
  AC_CHECK_FUNC([dvbpsi_AttachSDT],[TA42=true])
  AM_CONDITIONAL([TA42], [test x$TA42 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([TA4E], [true])
], [
  AC_CHECK_FUNC([dvbpsi_eit_attach],[TA4E=true],[TA4E=false])
  AC_CHECK_FUNC([dvbpsi_AttachEIT],[TA4E=true])
  AM_CONDITIONAL([TA4E], [test x$TA4E = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([TA70], [true])
], [
  AC_CHECK_FUNC([dvbpsi_tot_attach],[TA70=true],[TA70=false])
  AC_CHECK_FUNC([dvbpsi_AttachTOT],[TA70=true])
  AM_CONDITIONAL([TA70], [test x$TA70 = xtrue])
])

AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([TAC7], [true])
], [
  AC_CHECK_FUNC([dvbpsi_atsc_AttachMGT],[TAC7=true],[TAC7=false])
  AM_CONDITIONAL([TAC7], [test x$TAC7 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([TAC8], [true])
], [
  AC_CHECK_FUNC([dvbpsi_atsc_AttachVCT],[TAC8=true],[TAC8=false])
  AM_CONDITIONAL([TAC8], [test x$TAC8 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([TACB], [true])
], [
  AC_CHECK_FUNC([dvbpsi_atsc_AttachEIT],[TACB=true],[TACB=false])
  AM_CONDITIONAL([TACB], [test x$TACB = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([TACC], [true])
], [
  AC_CHECK_FUNC([dvbpsi_atsc_AttachETT],[TACC=true],[TACC=false])
  AM_CONDITIONAL([TACC], [test x$TACC = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([TACD], [true])
], [
  AC_CHECK_FUNC([dvbpsi_atsc_AttachSTT],[TACD=true],[TACD=false])
  AM_CONDITIONAL([TACD], [test x$TACD = xtrue])
])

# Checks for individual descriptor support in libdvbpsi.
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([DR0A], [true])
], [
  AC_CHECK_FUNC([dvbpsi_DecodeISO639Dr],[DR0A=true],[DR0A=false])
  AM_CONDITIONAL([DR0A], [test x$DR0A = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([DR48], [true])
], [
  AC_CHECK_FUNC([dvbpsi_DecodeServiceDr],[DR48=true],[DR48=false])
  AM_CONDITIONAL([DR48], [test x$DR48 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([DR4D], [true])
], [
  AC_CHECK_FUNC([dvbpsi_DecodeShortEventDr],[DR4D=true],[DR4D=false])
  AM_CONDITIONAL([DR4D], [test x$DR4D = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([DR62], [true])
], [
  AC_CHECK_FUNC([dvbpsi_DecodeFrequencyListDr],[DR62=true],[DR62=false])
  AM_CONDITIONAL([DR62], [test x$DR62 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([DR81], [true])
], [
  AC_CHECK_FUNC([dvbpsi_DecodeAc3AudioDr],[DR81=true],[DR81=false])
  AM_CONDITIONAL([DR81], [test x$DR81 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([DR83], [true])
], [
  AC_CHECK_FUNC([dvbpsi_DecodeLCNDr],[DR83=true],[DR83=false])
  AM_CONDITIONAL([DR83], [test x$DR83 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([DR86], [true])
], [
  AC_CHECK_FUNC([dvbpsi_DecodeCaptionServiceDr],[DR86=true],[DR86=false])
  AM_CONDITIONAL([DR86], [test x$DR86 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([DRA0], [true])
], [
  AC_CHECK_FUNC([dvbpsi_DecodeExtendedChannelNameDr],[DRA0=true],[DRA0=false])
  AC_CHECK_FUNC([dvbpsi_ExtendedChannelNameDr],[DRA0=true])
  AM_CONDITIONAL([DRA0], [test x$DRA0 = xtrue])
])
AM_COND_IF([DVBPSIBUILD], [
  AM_CONDITIONAL([DRA1], [true])
], [
  AC_CHECK_FUNC([dvbpsi_DecodeServiceLocationDr],[DRA1=true],[DRA1=false])
  AM_CONDITIONAL([DRA1], [test x$DRA1 = xtrue])
])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netdb.h stdint.h stdlib.h string.h sys/ioctl.h sys/time.h sys/timeb.h unistd.h ifaddrs.h sys/poll.h sys/socket.h linux/dvb/frontend.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([gethostname inet_ntoa localtime_r memset select socket strchr strdup strstr strtoul memchr memmove stpcpy strcspn strerror strpbrk strrchr strspn fcntl strtok_r timegm _mkgmtime inet_aton inet_pton InetPton])

PKG_PROG_PKG_CONFIG([0.20])
AM_CONDITIONAL([HAVE_PKG_CONFIG], [test "x$PKG_CONFIG" != "x"])
AC_SUBST(HAVE_PKG_CONFIG)

AS_IF([test "x$PKG_CONFIG" != "x"], [
	AC_CONFIG_FILES([libdvbtee.pc
	                 libdvbtee_server.pc])
])

AC_CONFIG_FILES([Makefile
                 dvbtee/Makefile
                 libdvbtee/Makefile
                 libdvbtee/decode/Makefile
                 libdvbtee/value/Makefile
                 libdvbtee/legacy/Makefile
                 libdvbtee_server/Makefile
                 tunerprovider/Makefile
                 examples/file_example/Makefile
                 examples/file_example/ifs/Makefile
                 examples/udplisten/Makefile
                 examples/tcplisten/Makefile
                 examples/parser_example/Makefile
                 examples/server_example/Makefile
                 examples/stdin_example/Makefile
                 examples/walk_hls/Makefile
                 examples/Makefile
                 packaging/libdvbtee.spec])
AC_OUTPUT

echo "
libdvbtee configuration
-----------------------
libdvbtee version     : ${VERSION}
compile flags         : ${CFLAGS}
target os             : ${target_os}
"
