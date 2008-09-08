#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXtools, 3.11.6, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF
# CHECK_RPM_DEBUG

AC_CONFIG_SRCDIR([src/sipxcar])

SFAC_INIT_FLAGS
CHECK_GENERATE_MANPAGES

# Java
CHECK_JDK
CHECK_ANT

AC_CONFIG_FILES([
  Makefile 
  src/Makefile 
  src/test/Makefile 
  src/man/Makefile 
  preflight/Makefile
])

AC_OUTPUT
