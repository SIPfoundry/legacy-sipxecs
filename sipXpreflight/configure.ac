AC_PREREQ(2.57)
AC_INIT(sipXpreflight, 4.7.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
m4_include([config/java2.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/org/sipfoundry/preflight/ConsoleTestRunner.java])
AC_PATH_PROG([BASH], [bash])
SFAC_INIT_FLAGS
AC_PROG_JAVA_CC([javac])
AC_PROG_JAVA([java])
PROG_JAVA_DEP
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  etc/Makefile
  bin/Makefile
])
AC_OUTPUT
