#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXopenacd, 0.0.4.5.1, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/sipXlib2.m4])

AM_INIT_AUTOMAKE(foreign)

AC_ARG_VAR(OPENACD_DIR, [Directory containing your OpenACD installation. Default /opt/openacd])
test -n "$OPENACD_DIR" || OPENACD_DIR=/opt/openacd

AC_CONFIG_FILES([
  Makefile
])

AC_OUTPUT
