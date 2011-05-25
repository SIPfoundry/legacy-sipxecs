include $(top_srcdir)/config/utility.am

if IS_RPMBUILD
CRON_INSTALL = install-crontab
endif

EXTRA_DIST = \
	$(proc_DATA:=.in) \
	$(cfg_DATA:=.in) \
	$(rpt_DATA) \
	sipxconfig-report-crontab.in

procdir = $(SIPX_DATADIR)/process.d
proc_DATA = \
	sipxconfig-agent-process.xml

cfgdir = $(SIPX_CONFDIR)
cfg_DATA = \
	sipxconfig-report-config

rptdir = $(SIPX_CONFDIR)/report
rpt_DATA = \
	schema.sql \
	enable_upgrades.sql \
	location_fqdn.sql \
	sipxconfig-report-config.vm

sipxconfig-report-crontab $(cfg_DATA) $(proc_DATA) : % : %.in
	@$(call SearchAndReplace, $<, $@)

# Target is not normally run, also, you may have to add sipx user to
# /etc/cron.d/cron.allow file
install-crontab : sipxconfig-report-crontab
	$(INSTALL) -D -m 644 $< $(DESTDIR)$(CRON_D)/$@

CLEANFILES = sipxconfig-report-crontab $(cfg_DATA) $(proc_DATA)
