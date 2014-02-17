AC_PREREQ(2.57)
AC_INIT(sipXsqa, 4.6.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
m4_include([config/java.m4])
m4_include([config/java2.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_DISABLE_STATIC
AC_CONFIG_SRCDIR([src/StateQueueAgent.cpp])
AM_CONFIG_HEADER([src/config.h])
AC_PROG_LIBTOOL
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
CHECK_CPPUNIT
AC_PROG_JAVA_CC([javac])
AC_PROG_JAVA([java])
AC_PATH_PROG([SWIG], [swig])
CHECK_JDK
CHECK_JNI
PROG_JAVA_DEP
SFAC_INIT_FLAGS
SFAC_LIB_PORT
SFAC_LIB_COMMSERVER
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/plugin/Makefile
  src/swig/Makefile
  src/test/Makefile
  include/Makefile
  bin/Makefile
  etc/Makefile
])
AC_OUTPUT
