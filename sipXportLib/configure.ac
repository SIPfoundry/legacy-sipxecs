#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXportLib, 0.0.4.3.2, sipx-dev@list.sipfoundry.org) 


AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

AC_CONFIG_SRCDIR([src/os/OsConfigDb.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL
# check the shared library file extension - sets LTDL_SHLIB_EXT
AC_LTDL_SHLIBEXT

# Checks for programs.
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE

CHECK_MSG_NOSIGNAL

# Checks for libraries.
CHECK_LIBRT
AC_SEARCH_LIBS([dlopen], [dl])

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

CHECK_CPPUNIT
CHECK_PCRE
CHECK_SSL
# CHECK_RPM_DEBUG

SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED
SFAC_INIT_FLAGS

SFAC_FEATURE_SIP_TLS

AC_MSG_CHECKING(for system type for process iterator ($host))
case $host in
    *-*-freebsd*)
    PROCESS_ITERATOR=src/os/freebsd/OsProcessIteratorFreeBSD.cpp
    LIBS="$(LIBS) -lkvm"
    ;;
    *-*-hpux*)
    PROCESS_ITERATOR=src/os/hpux/OsProcessIteratorHpux.cpp
    ;;
    *-*-linux*)
    PROCESS_ITERATOR=src/os/linux/OsProcessIteratorLinux.cpp
    ;;
esac
AC_MSG_RESULT($PROCESS_ITERATOR)
AC_CONFIG_LINKS(src/os/OsProcessIterator.cpp:$PROCESS_ITERATOR)

AC_CHECK_FUNCS(setenv)

ENABLE_DOC
ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(GENERATE_DOXYGEN, test x$enable_doxygen = xyes)

AC_CONFIG_FILES([
  Makefile
  include/Makefile
  bin/Makefile
  src/Makefile
  src/test/Makefile
  src/test/sipxunit/Makefile
  src/test/testplugin/Makefile
  syslogviewer/Makefile
  doc/Makefile
])

AC_OUTPUT
