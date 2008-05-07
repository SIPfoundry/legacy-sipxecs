AC_PREREQ(2.57)
AC_INIT(java-sun, 0.0.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign)
SFAC_DIST_DIR
SFAC_DOWNLOAD_DEPENDENCIES

AM_CONDITIONAL([IS_AMD64], [test -n "`uname -m | egrep 'x86_64|amd64'`"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

