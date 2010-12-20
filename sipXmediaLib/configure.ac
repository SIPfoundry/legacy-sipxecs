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
AM_CONFIG_HEADER([config.h])
AC_C_BIGENDIAN
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE

# Checks for libraries.
CHECK_COREAUDIO

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

CHECK_CPPUNIT
# CHECK_RPM_DEBUG

# Note that SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED must be called before
# SFAC_INIT_FLAGS, but also that SFAC_LIB_STACK will call SFAC_INIT_FLAGS also.
SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED
SFAC_INIT_FLAGS

# sipX dependancies

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
