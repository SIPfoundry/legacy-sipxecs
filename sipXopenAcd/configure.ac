#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXopenAcd, 0.0.4.3.2, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

SFAC_INIT_FLAGS
CHECK_GENERATE_MANPAGES

AC_CONFIG_FILES([
  Makefile
  bin/Makefile
])

AC_OUTPUT
