AC_PREREQ(2.57)
AC_INIT(sipXtools,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\(.*\)\.\([0-9]\+\)-\([0-9a-z\-]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_NSIS
AC_CONFIG_SRCDIR([src/sipxcar])
AC_PROG_LIBTOOL
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
SFAC_INIT_FLAGS
SFAC_LIB_STACK
CHECK_GENERATE_MANPAGES
CHECK_JDK
CHECK_ANT
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/test/Makefile
  src/man/Makefile
  src/sipviewer/Makefile
  src/syslog2siptrace/Makefile
  preflight/Makefile
])
AC_OUTPUT
