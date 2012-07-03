AC_PREREQ(2.57)
AC_INIT(sipXacdStatistics, 4.6.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
m4_include([config/java2.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
AC_CONFIG_SRCDIR([agent])
SFAC_INIT_FLAGS
AC_PROG_JAVA_CC([javac])
AC_PROG_JAVA([java])
PROG_JAVA_DEP
AM_CONDITIONAL(IS_RPMBUILD, test x$enable_rpmbuild = xyes)
CHECK_RUBY([1.8.2])
CHECK_RUBY_MODULE([soap/wsdlDriver])
CHECK_RUBYGEMS([0.8.11])
CHECK_POSTGRES
CHECK_CRON
AC_CONFIG_FILES([
  Makefile
  bin/Makefile
  etc/Makefile
  agent/Makefile
  ruby_bindings/Makefile
  java_bindings/Makefile
  report/Makefile
])
AC_OUTPUT
