#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXinterop, 4.1.7, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

AC_CONFIG_SRCDIR([bin/sipxinterop-setup.in])

CHECK_SERVICEDIR
CHECK_APACHE2

SFAC_INIT_FLAGS

AC_CONFIG_FILES([
  Makefile 
  bin/Makefile
  sipx_confdir/Makefile
  sipx_dbdir/Makefile
  http_conf/Makefile
  http_rootdir/Makefile
])

AC_OUTPUT
