#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXmediaAdapterLib, 0.4.4, sipx-dev@list.sipfoundry.org) 

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

AC_CONFIG_SRCDIR([sipXmediaMediaProcessing/src/sipXmediaFactoryImpl.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE

# Checks for libraries.
CHECK_LIBRT
###AC_SEARCH_LIBS([res_init], [resolv])

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

CHECK_CPPUNIT

CHECK_PCRE
CHECK_SSL
# CHECK_RPM_DEBUG

# Note that SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED must be called before
# SFAC_INIT_FLAGS.
SFAC_STRICT_COMPILE_NO_WARNINGS_ALLOWED
SFAC_INIT_FLAGS
SFAC_LIB_MEDIA

# Conditionally use the GIPS audio libraries
CHECK_GIPSNEQ
CHECK_GIPSVE
CHECK_GIPSCE

ENABLE_DOC
ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(GENERATE_DOXYGEN, test x$enable_doxygen = xyes)

AC_CONFIG_FILES([
  Makefile
  interface/Makefile
  sipXmediaMediaProcessing/Makefile
  sipXmediaMediaProcessing/src/Makefile
  sipXmediaMediaProcessing/include/Makefile
  sipXmediaVoiceEngine/Makefile
  sipXmediaVoiceEngine/src/Makefile
  sipXmediaVoiceEngine/include/Makefile
  sipXmediaConferenceEngine/Makefile
  sipXmediaConferenceEngine/src/Makefile
  sipXmediaConferenceEngine/include/Makefile
  doc/Makefile
])

AC_OUTPUT
