include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(proc_DATA:=.in) \
	$(cfg_DATA:=.in) \
	$(rpt_DATA) \
	$(cron_FILES:=.in)

cron_FILES = sipxconfig-report-crontab
if IS_RPMBUILD
# Target is not normally run, also, you may have to add sipx user to
# /etc/cron.d/cron.allow file
crondir = $(CRON_D)
cron_SCRIPTS = $(cron_FILES)
else
noinst_SCRIPTS = $(cron_FILES)
endif

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

$(cron_FILES) $(cfg_DATA) $(proc_DATA) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@)

CLEANFILES = $(cron_FILES) $(cfg_DATA) $(proc_DATA)
