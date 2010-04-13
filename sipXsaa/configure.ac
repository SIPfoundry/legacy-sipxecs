#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)
AC_INIT(sipXsaa, 4.2.0, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED
SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PATH_PROG([BASH], [bash])
AC_PATH_PROG([PERL], [perl])
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

CHECK_APACHE2
CHECK_SELINUX
CHECK_XARGS_REPLACE
# CHECK_RPM_DEBUG

SFAC_LIB_COMMSERVER
SFAC_LIB_CALL
SFAC_LIB_MEDIA
SFAC_LIB_STACK

# Conditionally use the GIPS audio libraries
CHECK_GIPSNEQ

# Java
CHECK_JDK
CHECK_ANT

# This allows configuring where the periodic scripts get installed
AC_ARG_WITH([dailydir],
  AC_HELP_STRING([--with-dailydir=directory],
    [The directory containing scripts that are run every night]),
  [DAILYDIR=${withval}],[DAILYDIR='$(sysconfdir)/cron.daily'])
AC_SUBST([DAILYDIR])

CHECK_POSTGRES
CHECK_SERVICEDIR

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/test/Makefile])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([meta/Makefile])
AC_CONFIG_FILES([doc/Makefile])

AM_CONDITIONAL(DO_RPMBUILD, test x$enable_rpmbuild = xyes)

AC_OUTPUT
