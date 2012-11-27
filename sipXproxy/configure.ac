AC_PREREQ(2.57)
AC_INIT(sipXproxy, 4.7.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_DISABLE_STATIC
AC_CONFIG_SRCDIR([src/sipXproxymain.cpp])
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
SFAC_FEATURE_SIP_TLS
CHECK_ODBC
AC_ARG_ENABLE(cse-tests,
  AC_HELP_STRING([--enable-cse-tests], [Run unit tests that test saving CSE data to postgres db setup]),
  enable_cse_tests=yes)
if test x$enable_cse_tests = xyes
then
  AC_SUBST(SIPXTEST_DATABASE, SIPXDB-TEST)
fi
AM_CONDITIONAL(ENABLE_CSE_TESTS, test x$enable_cse_tests = xyes)
CHECK_POSTGRES
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/test/Makefile
  include/Makefile
  lib/Makefile
  lib/authplugins/Makefile
  lib/authplugins/NatTraversalAgent/Makefile
  lib/authplugins/test/Makefile
  bin/Makefile
  meta/Makefile
  etc/Makefile
])
AC_OUTPUT
