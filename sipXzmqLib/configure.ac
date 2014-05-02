AC_PREREQ(2.57)
AC_INIT(sipXzmqLib, 14.10, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/ax_boost_base.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/ZMQMessage.cpp])
AM_CONFIG_HEADER([src/config.h])
AC_PROG_LIBTOOL
AC_LTDL_SHLIBEXT
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
CHECK_MSG_NOSIGNAL
CHECK_LIBRT
AX_BOOST_BASE([1.39],,
  AC_MSG_ERROR([no compatible boost library found]))
AC_SEARCH_LIBS([dlopen], [dl])
CHECK_CPPUNIT
CHECK_PCRE
CHECK_SSL
SFAC_INIT_FLAGS
AC_CHECK_FUNCS(setenv)
AC_CONFIG_FILES([
  Makefile
  include/Makefile
  src/Makefile
])

AC_OUTPUT
