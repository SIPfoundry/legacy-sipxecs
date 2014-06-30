AC_PREREQ(2.57)
AC_INIT(sipXyard, 14.10, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_DISABLE_STATIC
AC_CONFIG_SRCDIR([src/sipXyard.cpp])
AM_CONFIG_HEADER([src/config.h])
AC_PROG_LIBTOOL
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
AM_CONDITIONAL(IS_RPMBUILD, test x$enable_rpmbuild = xyes)
SFAC_INIT_FLAGS
SFAC_LIB_COMMSERVER


AC_CONFIG_FILES([
  Makefile
  include/Makefile
  src/Makefile
])
AC_OUTPUT
