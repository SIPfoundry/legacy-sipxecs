## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

pkgconfdir = @SIPX_CONFDIR@

alarmdir = @SIPX_CONFDIR@/alarms
alarm_DATA = \
    sipXalarms-config.xml

EXTRA_DIST = \
	sipxsupervisor-process.xml.in \
	$(alarm_DATA)

SIPX_PROCESS_DEFS=\
	sipxsupervisor

install-data-hook :

