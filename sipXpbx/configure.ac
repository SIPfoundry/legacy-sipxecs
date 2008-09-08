#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)
AC_INIT(sipXpbx, 3.11.6, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/mailboxmgr/MailboxManager.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PATH_PROG([BASH], [bash])
AC_PATH_PROG([PERL], [perl])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE

ENABLE_DOC
ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(GENERATE_DOXYGEN, test x$enable_doxygen = xyes)

# Checks for libraries.
# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

CHECK_APACHE2
CHECK_SELINUX
CHECK_CGICC
CHECK_XARGS_REPLACE
# CHECK_RPM_DEBUG

SFAC_LIB_COMMSERVER
SFAC_LIB_CALL
SFAC_LIB_MEDIA
SFAC_LIB_STACK

# Conditionally use the GIPS audio libraries
CHECK_GIPSNEQ

# Java
CHECK_JDK
CHECK_ANT

# This allows configuring where the periodic scripts get installed
AC_ARG_WITH([dailydir],
  AC_HELP_STRING([--with-dailydir=directory],
    [The directory containing scripts that are run every night]),
  [DAILYDIR=${withval}],[DAILYDIR='$(sysconfdir)/cron.daily'])
AC_SUBST([DAILYDIR])

CHECK_POSTGRES
CHECK_SERVICEDIR

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([doc/developer/Makefile.config])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([meta/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([include/sipxcgi/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/mailboxmgr/Makefile])
AC_CONFIG_FILES([src/mediaservercgi/Makefile])
AC_CONFIG_FILES([src/sipxcgi/Makefile])
AC_CONFIG_FILES([src/test/Makefile])
AC_CONFIG_FILES([src/test/sipxcgi/Makefile])
AC_CONFIG_FILES([src/wwwcontent/Makefile])
AC_CONFIG_FILES([sipXpark/Makefile])
AC_CONFIG_FILES([sipXpark/src/Makefile])
AC_CONFIG_FILES([sipXpark/etc/Makefile])
AC_CONFIG_FILES([sipXpark/bin/Makefile])
AC_CONFIG_FILES([sipXpark/doc/Makefile])
AC_CONFIG_FILES([sipXpresence/Makefile])
AC_CONFIG_FILES([sipXpresence/src/Makefile])
AC_CONFIG_FILES([sipXpresence/etc/Makefile])
AC_CONFIG_FILES([sipXpresence/bin/Makefile])
AC_CONFIG_FILES([sipXpresence/doc/Makefile])
AC_CONFIG_FILES([sipXpresence/meta/Makefile])
AC_CONFIG_FILES([sipXrls/Makefile])
AC_CONFIG_FILES([sipXrls/src/Makefile])
AC_CONFIG_FILES([sipXrls/etc/Makefile])
AC_CONFIG_FILES([sipXrls/bin/Makefile])
AC_CONFIG_FILES([sipXrls/meta/Makefile])
AC_CONFIG_FILES([sipXrls/doc/Makefile])
AC_CONFIG_FILES([sipXdialog/Makefile])
AC_CONFIG_FILES([sipXdialog/src/Makefile])
AC_CONFIG_FILES([sipXdialog/etc/Makefile])
AC_CONFIG_FILES([sipXdialog/bin/Makefile])
AC_CONFIG_FILES([sipXdialog/doc/Makefile])

AM_CONDITIONAL(DO_RPMBUILD, test x$enable_rpmbuild = xyes)

AC_OUTPUT
