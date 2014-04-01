AC_PREREQ(2.57)
AC_INIT(sipXeslLib, 14.04, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/EslConnection.cpp])

AC_PROG_LIBTOOL
AC_LTDL_SHLIBEXT
AC_PATH_PROG([BASH], [bash])

AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC

AC_CHECK_FUNCS(setenv)
AC_CONFIG_FILES([
  Makefile
  include/Makefile
  src/Makefile
])

SFAC_INIT_FLAGS

AC_OUTPUT
