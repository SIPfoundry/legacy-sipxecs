#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXportLib, 3.7.0, sipx-dev@list.sipfoundry.org)


AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

AC_CONFIG_SRCDIR([src/os/OsConfigDb.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC

CHECK_MSG_NOSIGNAL

# Checks for libraries.
CHECK_LIBRT

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

CHECK_CPPUNIT
CHECK_PCRE
CHECK_SSL

SFAC_INIT_FLAGS

SFAC_FEATURE_SIP_TLS

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)

AC_CONFIG_FILES([
  Makefile 
  include/Makefile 
  bin/Makefile
  src/Makefile 
  src/test/Makefile
  src/test/sipxunit/Makefile
  src/test/testplugin/Makefile
  syslogviewer/Makefile
  doc/Makefile
])

AC_OUTPUT
