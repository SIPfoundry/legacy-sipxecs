AC_PREREQ(2.57)
AC_INIT(sipXcallLib,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\(.*\)\.\([0-9]\+\)-\([0-9a-z\-]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src/cp/CallManager.cpp])
AM_CONFIG_HEADER([src/config.h])
AC_PROG_LIBTOOL
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
CHECK_LIBOBJC
CHECK_CPPUNIT
SFAC_LIB_MEDIA
SFAC_FEATURE_SIP_TLS
CHECK_GIPSNEQ
CHECK_GIPSVE
CHECK_GIPSCE

AC_ARG_ENABLE(call-examples,
             [  --enable-call-examples    build sipXtapi examples PlaceCall and ReceiveCall],
             [enable_call_examples=yes], [enable_call_examples=no])
AM_CONDITIONAL(CALL_EXAMPLES, test x$enable_call_examples = xyes)

AC_CONFIG_FILES([
  Makefile
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
