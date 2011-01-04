AC_PREREQ(2.57)
AC_INIT(sipXcdrLog,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\([0-9.]\+\)-\([0-9]\+\)-\([0-9a-f]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_CONFIG_SRCDIR([src])
SFAC_INIT_FLAGS
CHECK_JDK
CHECK_ANT
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
