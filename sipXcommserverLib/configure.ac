#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXcommserverLib, 4.3.2, sipx-dev@list.sipfoundry.org) 

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

AC_CONFIG_SRCDIR([src/sipdb/SIPDBManager.cpp])
AM_CONFIG_HEADER([config.h])

AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PATH_PROG([BASH], [bash])
AC_PATH_PROG([PERL],[perl])
AC_PATH_PROG([OPENSSL],[openssl])
AC_PATH_PROG([CP_BINARY],[cp],"/bin/cp")
AC_PATH_PROG([MV_BINARY],[mv],"/bin/mv")
CHECK_XARGS_REPLACE
CHECK_DATE_PARSE_ARGS

# Checks for libraries.
# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

CHECK_APACHE2
CHECK_CPPUNIT
CHECK_XERCES
CHECK_ODBC
# CHECK_RPM_DEBUG

SFAC_LIB_STACK

SFAC_FEATURE_SIP_TLS

ENABLE_DOC
ENABLE_DOXYGEN
CHECK_GENERATE_MANPAGES
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(GENERATE_DOXYGEN, test x$enable_doxygen = xyes)

AC_ARG_ENABLE(odbc-tests,
  AC_HELP_STRING([--enable-odbc-tests], [Run unit tests that test ODBC connection to postgres]),
  enable_odbc_tests=yes)
if test x$enable_odbc_tests = xyes
then
  AC_SUBST(SIPXTEST_DATABASE, SIPXDB-TEST)
fi
AM_CONDITIONAL(ENABLE_ODBC_TESTS, test x$enable_odbc_tests = xyes)

CHECK_POSTGRES

AC_CONFIG_FILES([
  Makefile
  include/Makefile
  etc/Makefile
  bin/Makefile
  bin/ssl-cert/Makefile
  src/Makefile
  src/sipdb/Makefile
  src/fastdb/Makefile
  src/xsdvalid/Makefile
  src/test/Makefile
  src/test/testlib/Makefile
  doc/Makefile
  meta/Makefile
  src/sipxrestorebinary/Makefile
  src/sipxpackage/Makefile
  src/sipxswadmin/Makefile
  src/sipxzoneadmin/Makefile
  src/sipxtimemanager/Makefile
  src/sipxchangetimezone/Makefile
  src/sipxcdrpostgres/Makefile
])
AC_OUTPUT
