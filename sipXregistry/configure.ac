#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXregistry, 4.2.0, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

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
CHECK_DOCBOOKXML
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(GENERATE_DOXYGEN, test x$enable_doxygen = xyes)
AM_CONDITIONAL(GENERATE_DOT, test x$have_dot = xYES)
AM_CONDITIONAL(GENERATE_DOCBOOK_HTML, test x$enable_xml2xhtml = xyes)
AM_CONDITIONAL(GENERATE_DOCBOOK_TXT, test x$enable_xml2txt = xyes)
AM_CONDITIONAL(GENERATE_DOCBOOK_PDF, test x$enable_xml2pdf = xyes)

# Checks for libraries.
# Checks for libraries.
CHECK_LIBRT
CHECK_CPPUNIT
# CHECK_RPM_DEBUG
# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

SFAC_LIB_CALL
SFAC_LIB_COMMSERVER

SFAC_FEATURE_SIP_TLS

# Conditionally use the GIPS audio libraries
CHECK_GIPSNEQ

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/test/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([meta/Makefile])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([lib/Makefile])
AC_CONFIG_FILES([lib/redirect_plugins/Makefile])
AC_CONFIG_FILES([lib/redirect_plugins/test/Makefile])
AC_CONFIG_FILES([lib/register_plugins/Makefile])
AC_CONFIG_FILES([lib/register_plugins/mwi/Makefile])
AC_CONFIG_FILES([lib/register_plugins/mwi/test/Makefile])

AC_OUTPUT
