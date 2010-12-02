#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXopenfire, 0.0.4.5.1, sipx-dev@list.sipfoundry.org) 

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

# CHECK_RPM_DEBUG

AC_CONFIG_SRCDIR([src])

SFAC_INIT_FLAGS
CHECK_GENERATE_MANPAGES

#Check for postgres
CHECK_POSTGRES

#Check if the openfire base installation exists and is enabled.
CHECK_OPENFIRE
AM_CONDITIONAL(BUILD_OPENFIRE, test x$enable_openfire = xyes)

# Java
CHECK_JDK
CHECK_ANT

AC_CONFIG_FILES([
  Makefile 
])

AC_OUTPUT
