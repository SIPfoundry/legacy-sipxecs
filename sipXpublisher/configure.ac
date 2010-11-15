#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXpublisher, 0.4.4, sipx-dev@list.sipfoundry.org) 

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/statusserver/main.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE

ENABLE_DOC
ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(GENERATE_DOXYGEN, test x$enable_doxygen = xyes)

# Checks for libraries.
# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

CHECK_SSL
# CHECK_RPM_DEBUG

SFAC_LIB_COMMSERVER

SFAC_FEATURE_SIP_TLS

AC_CONFIG_FILES([Makefile include/Makefile src/Makefile])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_OUTPUT
