AC_PREREQ(2.57)
AC_INIT(sipXrtcbridge, 4.6.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
m4_include([rtcbridge.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_DISABLE_STATIC
AC_CONFIG_SRCDIR([src/WebRtcBridge.cpp])
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
SFAC_FREESWITCH_DEVEL
SFAC_RESIP_DEVEL
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  include/Makefile
])
AC_OUTPUT
