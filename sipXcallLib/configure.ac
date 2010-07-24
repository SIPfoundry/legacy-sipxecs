#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXcallLib, 4.3.0, sipx-dev@list.sipfoundry.org) 

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

SFAC_SIPX_INSTALL_PREFIX
AM_INIT_AUTOMAKE(foreign)
CHECK_AUTOCONF

SFAC_AUTOMAKE_VERSION([1.6])

AC_CONFIG_SRCDIR([src/cp/CallManager.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE

# Checks for libraries.

# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.

CHECK_LIBOBJC
CHECK_CPPUNIT
# CHECK_RPM_DEBUG

SFAC_LIB_MEDIA

SFAC_FEATURE_SIP_TLS

ENABLE_DOC
ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(GENERATE_DOXYGEN, test x$enable_doxygen = xyes)

# Conditionally use the GIPS audio libraries
CHECK_GIPSNEQ
CHECK_GIPSVE
CHECK_GIPSCE

AC_ARG_ENABLE(call-examples,
             [  --enable-call-examples    build sipXtapi examples PlaceCall and ReceiveCall],
             [enable_call_examples=yes], [enable_call_examples=no])
AM_CONDITIONAL(CALL_EXAMPLES, test x$enable_call_examples = xyes)

AC_CONFIG_FILES([
  Makefile
  bin/Makefile
  doc/Makefile
  examples/CallScript/src/Makefile
  examples/Makefile
  examples/PlaceCall/Makefile
  examples/PlaceCall/src/Makefile
  examples/ReceiveCall/Makefile
  examples/ReceiveCall/src/Makefile
  examples/SendAudio/Makefile
  examples/SendAudio/src/Makefile
  examples/musicServer/Makefile
  include/Makefile
  src/Makefile
  src/test/Makefile
  src/test/tapi/Makefile
])

AC_OUTPUT
