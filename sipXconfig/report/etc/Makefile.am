include $(top_srcdir)/config/subdir.am

if IS_RPMBUILD
CRON_INSTALL = install-crontab
endif

EXTRA_DIST = \
	sipxconfig-report-crontab.in

cfgdir = $(SIPX_CONFDIR)/report

cfg_DATA = \
	schema.sql \
	sipxconfig-report-config

install-data-hook : $(CRON_INSTALL)

# Target is not normally run, also, you may have to add sipx user to
# /etc/cron.d/cron.allow file
.PHONY: install-crontab
install-crontab : sipxconfig-report-crontab.in
	$(LocalizeSipXconfig) $(srcdir)/sipxconfig-report-crontab.in > sipxconfig-report-crontab
	$(INSTALL) -D -m 644 sipxconfig-report-crontab $(DESTDIR)$(CRON_D)/sipxconfig-report-crontab

