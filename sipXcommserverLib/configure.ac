AC_PREREQ(2.57)
AC_INIT(sipXcommserverLib,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\(.*\)\.\([0-9]\+\)-\([0-9a-z\-]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/sipdb/SIPDBManager.cpp])
AM_CONFIG_HEADER([src/config.h])
AC_PROG_LIBTOOL
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PATH_PROG([BASH], [bash])
AC_PATH_PROG([PERL],[perl])
AC_PATH_PROG([OPENSSL],[openssl])
AC_PATH_PROG([CP_BINARY],[cp],"/bin/cp")
AC_PATH_PROG([MV_BINARY],[mv],"/bin/mv")
CHECK_XARGS_REPLACE
CHECK_DATE_PARSE_ARGS
CHECK_APACHE2
CHECK_CPPUNIT
CHECK_XERCES
CHECK_ODBC
SFAC_LIB_STACK
SFAC_FEATURE_SIP_TLS
AC_ARG_ENABLE(odbc-tests,
  AC_HELP_STRING([--enable-odbc-tests], [Run unit tests that test ODBC connection to postgres]),
  enable_odbc_tests=yes)
if test x$enable_odbc_tests = xyes
then
  AC_SUBST(SIPXTEST_DATABASE, SIPXDB-TEST)
fi
AM_CONDITIONAL(ENABLE_ODBC_TESTS, test x$enable_odbc_tests = xyes)
CHECK_GENERATE_MANPAGES
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
