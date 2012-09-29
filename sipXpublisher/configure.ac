AC_PREREQ(2.57)
AC_INIT(sipXpublisher, 4.4.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/statusserver/main.cpp])
AM_CONFIG_HEADER([src/statusserver/config.h])
AC_PROG_LIBTOOL
AC_CHECK_LIB(boost_thread, main, [BOOST_LDADD="-lboost_thread"],
    [AC_CHECK_LIB(boost_thread-mt, main,
    [BOOST_LDADD="-lboost_thread-mt"],
    [AC_MSG_ERROR("no boost thread found")])])
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
CHECK_SSL
AC_SUBST(BOOST_LDADD)
SFAC_LIB_COMMSERVER
SFAC_FEATURE_SIP_TLS
AC_CONFIG_FILES([
  Makefile
  include/Makefile
  src/Makefile
  etc/Makefile
  bin/Makefile
])
AC_OUTPUT
