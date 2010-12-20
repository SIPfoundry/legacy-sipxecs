AC_PREREQ(2.57)
AC_INIT(sipxecs,
  m4_esyscmd([config/git-version-gen .tarball-version | sed 's/\(.*\)\.\([0-9]\+\)-\([0-9a-z\-]\+\)/\1/g']),
  sipx-dev@list.sipfoundry.org)
m4_include([config/sipXlib2.m4])
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(foreign tar-ustar)
AC_CONFIG_FILES([
    Makefile
])
AC_OUTPUT
