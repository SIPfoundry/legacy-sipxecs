#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)
AC_INIT(sipXsupervisor, 4.1.6, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED
SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/sipXsupervisor.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE

ENABLE_DOC
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
CHECK_DOCBOOKXML
AM_CONDITIONAL(GENERATE_DOT, test x$have_dot = xYES)
AM_CONDITIONAL(GENERATE_DOCBOOK_HTML, test x$enable_xml2xhtml = xyes)
AM_CONDITIONAL(GENERATE_DOCBOOK_TXT, test x$enable_xml2xtxt = xyes)
AM_CONDITIONAL(GENERATE_DOCBOOK_PDF, test x$enable_xml2pdf = xyes)
ENABLE_DOXYGEN
AM_CONDITIONAL(GENERATE_DOXYGEN, test x$enable_doxygen = xyes)

# Checks for libraries.
# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

SFAC_LIB_COMMSERVER

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/test/Makefile])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([meta/Makefile])
AC_CONFIG_FILES([meta/alarms/Makefile])
AC_CONFIG_FILES([meta/mibs/Makefile])
AC_CONFIG_FILES([doc/Makefile])

AM_CONDITIONAL(DO_RPMBUILD, test x$enable_rpmbuild = xyes)

AC_OUTPUT
