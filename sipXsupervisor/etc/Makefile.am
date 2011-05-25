include $(top_srcdir)/config/utility.am

alarmdir = @SIPX_CONFDIR@/alarms
dist_alarm_DATA = \
	sipXalarms-config.xml

EXTRA_DIST = \
        $(proc_DATA:.xml=.xml.in)

procdir = \
        $(SIPX_DATADIR)/process.d

proc_DATA = \
	sipxsupervisor-process.xml

$(proc_DATA) : % : %.in
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(proc_DATA)
