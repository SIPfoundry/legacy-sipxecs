AC_PREREQ(2.57)
AC_INIT(sipXmediaAdapterLib,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\(.*\)\.\([0-9]\+\)-\([0-9a-z\-]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)

AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])

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
