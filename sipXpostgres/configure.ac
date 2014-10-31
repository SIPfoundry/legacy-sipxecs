AC_PREREQ(2.57)
AC_INIT(sipxpostgres, 15.04, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(foreign tar-ustar)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
m4_include([config/sipXlib2.m4])

CHECK_POSTGRES

AC_CONFIG_FILES([
    Makefile
    etc/Makefile
])
AC_OUTPUT
