#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXvxml, 3.11.7, sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/client/api/OSBclient.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE

ENABLE_DOC
ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(GENERATE_DOXYGEN, test x$enable_doxygen = xyes)

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Check the nature of va_list.
CHECK_VA_LIST
# CHECK_RPM_DEBUG

# Checks for library functions.

SFAC_SRCDIR_EXPAND()

FLAGS="-DOPENVXI -D_GNU_SOURCE -D_PTHREADS -D__STL_PTHREADS -DNDEBUG -D_linux_ -I[$abs_srcdir]/include -I[$abs_srcdir]/src -I[$abs_srcdir]/src/client -I[$abs_srcdir]/js/src"
CFLAGS="$CFLAGS $FLAGS"
CXXFLAGS="$CXXFLAGS $FLAGS"

# Checks for libraries.
CHECK_ZLIB
CHECK_LIBWWW
SFAC_REQUIRE_LIBWWWSSL
CHECK_XERCES

SFAC_LIB_COMMSERVER
SFAC_LIB_CALL

SFAC_FEATURE_SIP_TLS

# Conditionally use the GIPS audio libraries
CHECK_GIPSNEQ

dnl Double the LIBS to deal with recursive dependencies
LIBS="$LIBS $LIBS"

## Common flags for pingtel related source

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([js/Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([src/trd/Makefile])
AC_CONFIG_FILES([src/value/Makefile])
AC_CONFIG_FILES([src/log/Makefile])
AC_CONFIG_FILES([src/jsi/Makefile])
AC_CONFIG_FILES([src/inet/Makefile])
AC_CONFIG_FILES([src/prompt/Makefile])
AC_CONFIG_FILES([src/rec/Makefile])
AC_CONFIG_FILES([src/tel/Makefile])
AC_CONFIG_FILES([src/VXI/Makefile])
AC_CONFIG_FILES([src/client/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([meta/Makefile])
AC_OUTPUT
