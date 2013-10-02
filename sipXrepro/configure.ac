AC_PREREQ(2.57)
AC_INIT(sipXtackLib, 4.6.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/ReproGlue.cpp])
AC_PROG_LIBTOOL
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
AC_MSG_CHECKING(for res_query)

CHECK_CPPUNIT
CHECK_SSL
SFAC_INIT_FLAGS
SFAC_LIB_PORT


AC_CONFIG_FILES([
  Makefile
  include/Makefile
  src/Makefile
])

AC_OUTPUT
