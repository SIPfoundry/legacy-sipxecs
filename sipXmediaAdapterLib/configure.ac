AC_PREREQ(2.57)
AC_INIT(sipXmediaAdapterLib, 4.6.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([sipXmediaMediaProcessing/src/sipXmediaFactoryImpl.cpp])
AC_PROG_LIBTOOL
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
CHECK_LIBRT
CHECK_CPPUNIT
CHECK_PCRE
CHECK_SSL
SFAC_INIT_FLAGS
SFAC_LIB_MEDIA
CHECK_GIPSNEQ
CHECK_GIPSVE
CHECK_GIPSCE
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
])

AC_OUTPUT
