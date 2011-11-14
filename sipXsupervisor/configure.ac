AC_PREREQ(2.57)
AC_INIT(sipXsupervisor, 0.0.4.5.2, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/sipXsupervisor.cpp])
AM_CONFIG_HEADER([src/config.h])
AC_PROG_LIBTOOL
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
CHECK_NET_SNMP
SFAC_LIB_COMMSERVER
CHECK_SERVICEDIR
AC_CONFIG_FILES([
	Makefile
	bin/Makefile
	src/Makefile
	src/test/Makefile
	etc/Makefile
])
AC_OUTPUT
