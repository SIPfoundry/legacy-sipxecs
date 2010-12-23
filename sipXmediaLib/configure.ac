#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXmediaLib,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\(.*\)\.\([0-9]\+\)-\([0-9a-z\-]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/mp/MpCallFlowGraph.cpp])
AM_CONFIG_HEADER([src/config.h])
AC_C_BIGENDIAN
AC_PROG_LIBTOOL
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
CHECK_COREAUDIO
CHECK_CPPUNIT
SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED
SFAC_INIT_FLAGS
SFAC_LIB_STACK
SFAC_FEATURE_SIP_TLS
# Enable local audio support (microphone / speaker)
AC_ARG_ENABLE(local_audio,
[  --enable-local-audio    Enable local audio (speaker and microphone)],
[ case "${enableval}" in
  yes) enable_local_audio=true ;;
  no) enable_local_audio=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-local-audio) ;;
esac],[enable_local_audio=false])
AM_CONDITIONAL(ENABLE_LOCAL_AUDIO, test x$enable_local_audio = xtrue)
CHECK_GIPSNEQ
AC_CONFIG_FILES([
  Makefile
  include/Makefile
  src/Makefile
  src/test/Makefile
])
AC_OUTPUT
