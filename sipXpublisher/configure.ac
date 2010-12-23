AC_PREREQ(2.57)
AC_INIT(sipXpublisher,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\(.*\)\.\([0-9]\+\)-\([0-9a-z\-]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/statusserver/main.cpp])
AM_CONFIG_HEADER([src/statusserver/config.h])
AC_PROG_LIBTOOL
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
CHECK_SSL
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
