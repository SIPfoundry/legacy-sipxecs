AC_PREREQ(2.57)
AC_INIT(sipXpbx, 15.04, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])
AM_INIT_AUTOMAKE(foreign tar-ustar)
SFAC_AUTOMAKE_VERSION([1.6])
AC_PROG_LN_S
AC_PATH_PROG([BASH], [bash])
AC_PATH_PROG([PERL], [perl])
CHECK_APACHE2
CHECK_SELINUX
CHECK_XARGS_REPLACE
CHECK_FREESWITCH
SFAC_LIB_COMMSERVER
CHECK_MSGFMT
AC_ARG_WITH([dailydir],
  AC_HELP_STRING([--with-dailydir=directory],
    [The directory containing scripts that are run every night]),
  [DAILYDIR=${withval}],[DAILYDIR='$(sysconfdir)/cron.daily'])
AC_SUBST([DAILYDIR])
CHECK_POSTGRES
CHECK_SERVICEDIR
AC_CONFIG_FILES([
	Makefile
	bin/Makefile
	etc/Makefile
	src/Makefile
	src/wwwcontent/Makefile
])
AC_OUTPUT
