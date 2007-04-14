#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXproxy, 3.7.0, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/sipauthproxy/sipauthproxymain.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)

SFAC_INIT_FLAGS
AM_CONDITIONAL(IS_RPMBUILD, test x$enable_rpmbuild = xyes)

# Checks for libraries.

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

SFAC_LIB_COMMSERVER

SFAC_FEATURE_SIP_TLS

AC_ARG_ENABLE(cdr,
  AC_HELP_STRING([--enable-cdr], [Build CDR capability, requires ruby and deve tools like gem and rake]),
  enable_cdr=yes)

if test x$enable_cdr = xyes
then
  CHECK_RUBY([1.8.2])
  CHECK_RAKE([0.5.3])
  CHECK_GEM([0.8.11])
fi
AM_CONDITIONAL(ENABLE_CDR, test x$enable_cdr = xyes)


AC_ARG_ENABLE(cse-tests,
  AC_HELP_STRING([--enable-cse-tests], [Run unit tests that test saving CSE data to postgres db setup]),
  enable_cse_tests=yes)
if test x$enable_cse_tests = xyes
then
  AC_SUBST(SIPXTEST_DATABASE, SIPXDB-TEST)
fi
AM_CONDITIONAL(ENABLE_CSE_TESTS, test x$enable_cse_tests = xyes)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([src/sipforkingproxy/Makefile])
AC_CONFIG_FILES([src/sipauthproxy/Makefile])
AC_CONFIG_FILES([src/cse/Makefile])
AC_CONFIG_FILES([src/test/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([meta/Makefile])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([etc/database/Makefile])
AC_CONFIG_FILES([src/cdr/Makefile])
AC_CONFIG_FILES([src/cse/test/Makefile])

AC_OUTPUT

