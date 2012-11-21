AC_PREREQ(2.57)
AC_INIT(sipXcdr, 4.6.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
m4_include([config/java2.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_INIT_FLAGS
AC_PROG_JAVA_CC([javac])
AC_PROG_JAVA([java])
PROG_JAVA_DEP
AM_CONDITIONAL(IS_RPMBUILD, test x$enable_rpmbuild = xyes)
CHECK_RUBY([1.8.2])
# not avail on ruby 1.9, default on fedora 17
#CHECK_RUBY_MODULE([soap/wsdlDriver])
CHECK_RUBYGEMS([0.8.11])
CHECK_POSTGRES
CHECK_ODBC
AC_CONFIG_FILES([
  Makefile
  java-client/Makefile
  etc/Makefile
  bin/Makefile
])
AC_OUTPUT
