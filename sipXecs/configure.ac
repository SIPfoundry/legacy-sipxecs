AC_PREREQ(2.57)
AC_INIT(sipxecs, 4.1.0, sipx-dev@list.sipfoundry.org)

m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AC_CONFIG_AUX_DIR(config)

SFAC_SIPX_INSTALL_PREFIX
SFAC_INIT_FLAGS
#AC_PROG_LIBTOOL
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(foreign)

AC_CONFIG_FILES([
    Makefile
    bin/Makefile
])

AC_OUTPUT
