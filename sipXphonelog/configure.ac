AC_PREREQ(2.57)
AC_INIT(sipxphonelog, 0.0.4.5.2, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(foreign tar-ustar)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
SFAC_INIT_FLAGS
AM_CONDITIONAL(IS_RPMBUILD, test x$enable_rpmbuild = xyes)
AC_CONFIG_FILES([
    Makefile
])
AC_OUTPUT
