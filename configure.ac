#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_include([version.m4])
AC_PREREQ([2.69])
AC_INIT([libdvbtee], m4_defn([DVBTEE_VERSION]), [mkrufky@linuxtv.org])

AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_SRCDIR([libdvbtee/parse.cpp])
AC_CONFIG_HEADERS([config.h])

# Check configure args
AC_ARG_ENABLE([olddecoder],
[  --enable-olddecoder    Enable old decoder],
[case "${enableval}" in
  yes) olddecoder=true ;;
  no)  olddecoder=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-olddecoder]) ;;
esac],[olddecoder=false])
AM_CONDITIONAL([OLDDECODER], [test x$olddecoder = xtrue])

AM_CONDITIONAL([BUILD_OS_IS_DARWIN], [test x"$build_os" = darwin])

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

# Checks for individual table support in libdvbpsi.
AC_CHECK_FUNC([dvbpsi_pat_attach],[TA00=true],[TA00=false])
AC_CHECK_FUNC([dvbpsi_AttachPAT],[TA00=true])
AM_CONDITIONAL([TA00], [test x$TA00 = xtrue])
AC_CHECK_FUNC([dvbpsi_pmt_attach],[TA02=true],[TA02=false])
AC_CHECK_FUNC([dvbpsi_AttachPMT],[TA02=true])
AM_CONDITIONAL([TA02], [test x$TA02 = xtrue])
AC_CHECK_FUNC([dvbpsi_nit_attach],[TA40=true],[TA40=false])
AC_CHECK_FUNC([dvbpsi_AttachNIT],[TA40=true])
AM_CONDITIONAL([TA40], [test x$TA40 = xtrue])
AC_CHECK_FUNC([dvbpsi_sdt_attach],[TA42=true],[TA42=false])
AC_CHECK_FUNC([dvbpsi_AttachSDT],[TA42=true])
AM_CONDITIONAL([TA42], [test x$TA42 = xtrue])
AC_CHECK_FUNC([dvbpsi_eit_attach],[TA4E=true],[TA4E=false])
AC_CHECK_FUNC([dvbpsi_AttachEIT],[TA4E=true])
AM_CONDITIONAL([TA4E], [test x$TA4E = xtrue])
AC_CHECK_FUNC([dvbpsi_tot_attach],[TA70=true],[TA70=false])
AC_CHECK_FUNC([dvbpsi_AttachTOT],[TA70=true])
AM_CONDITIONAL([TA70], [test x$TA70 = xtrue])

AC_CHECK_FUNC([dvbpsi_atsc_AttachMGT],[TAC7=true],[TAC7=false])
AM_CONDITIONAL([TAC7], [test x$TAC7 = xtrue])
AC_CHECK_FUNC([dvbpsi_atsc_AttachVCT],[TAC8=true],[TAC8=false])
AM_CONDITIONAL([TAC8], [test x$TAC8 = xtrue])
AC_CHECK_FUNC([dvbpsi_atsc_AttachEIT],[TACB=true],[TACB=false])
AM_CONDITIONAL([TACB], [test x$TACB = xtrue])
AC_CHECK_FUNC([dvbpsi_atsc_AttachETT],[TACC=true],[TACC=false])
AM_CONDITIONAL([TACC], [test x$TACC = xtrue])
AC_CHECK_FUNC([dvbpsi_atsc_AttachSTT],[TACD=true],[TACD=false])
AM_CONDITIONAL([TACD], [test x$TACD = xtrue])

# Checks for individual descriptor support in libdvbpsi.
AC_CHECK_FUNC([dvbpsi_DecodeISO639Dr],[DR0A=true],[DR0A=false])
AM_CONDITIONAL([DR0A], [test x$DR0A = xtrue])
AC_CHECK_FUNC([dvbpsi_DecodeServiceDr],[DR48=true],[DR48=false])
AM_CONDITIONAL([DR48], [test x$DR48 = xtrue])
AC_CHECK_FUNC([dvbpsi_DecodeShortEventDr],[DR4D=true],[DR4D=false])
AM_CONDITIONAL([DR4D], [test x$DR4D = xtrue])
AC_CHECK_FUNC([dvbpsi_DecodeFrequencyListDr],[DR62=true],[DR62=false])
AM_CONDITIONAL([DR62], [test x$DR62 = xtrue])
AC_CHECK_FUNC([dvbpsi_DecodeAc3AudioDr],[DR81=true],[DR81=false])
AM_CONDITIONAL([DR81], [test x$DR81 = xtrue])
AC_CHECK_FUNC([dvbpsi_DecodeLCNDr],[DR83=true],[DR83=false])
AM_CONDITIONAL([DR83], [test x$DR83 = xtrue])
AC_CHECK_FUNC([dvbpsi_DecodeCaptionServiceDr],[DR86=true],[DR86=false])
AM_CONDITIONAL([DR86], [test x$DR86 = xtrue])
AC_CHECK_FUNC([dvbpsi_DecodeExtendedChannelNameDr],[DRA0=true],[DRA0=false])
AC_CHECK_FUNC([dvbpsi_ExtendedChannelNameDr],[DRA0=true])
AM_CONDITIONAL([DRA0], [test x$DRA0 = xtrue])
AC_CHECK_FUNC([dvbpsi_DecodeServiceLocationDr],[DRA1=true],[DRA1=false])
AM_CONDITIONAL([DRA1], [test x$DRA1 = xtrue])

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
                 libdvbtee_server/Makefile
                 tunerprovider/Makefile
                 examples/parser_example/Makefile
                 examples/server_example/Makefile
                 examples/walk_hls/Makefile
                 examples/Makefile
                 packaging/libdvbtee.spec])
AC_OUTPUT

echo "
libdvbtee configuration
-----------------------
libdvbtee version     : ${VERSION}
compile flags         : ${CFLAGS}
"
