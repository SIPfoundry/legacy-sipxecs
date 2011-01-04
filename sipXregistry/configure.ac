AC_PREREQ(2.57)
AC_INIT(sipXregistry,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\([0-9.]\+\)-\([0-9]\+\)-\([0-9a-f]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AM_CONFIG_HEADER([src/config.h])
AC_PROG_LIBTOOL
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AX_COMPILER_VENDOR
AC_PROG_CC
CHECK_XARGS_REPLACE
CHECK_LIBRT
CHECK_CPPUNIT
SFAC_LIB_CALL
SFAC_LIB_COMMSERVER
SFAC_FEATURE_SIP_TLS
CHECK_GIPSNEQ
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/test/Makefile])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([meta/Makefile])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([lib/Makefile])
AC_CONFIG_FILES([lib/redirect_plugins/Makefile])
AC_CONFIG_FILES([lib/redirect_plugins/test/Makefile])
AC_CONFIG_FILES([lib/register_plugins/Makefile])
AC_CONFIG_FILES([lib/register_plugins/mwi/Makefile])
AC_CONFIG_FILES([lib/register_plugins/mwi/test/Makefile])
AC_OUTPUT
