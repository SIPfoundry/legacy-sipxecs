include $(top_srcdir)/config/subdir.am

if IS_RPMBUILD
CRON_INSTALL = install-crontab
endif

EXTRA_DIST = \
	sipxconfig-report-crontab.in \
	sipxconfig-report-config.in

cfgdir = $(SIPX_CONFDIR)/report

cfg_DATA = \
	schema.sql \
	enable_upgrades.sql \
	location_fqdn.sql \
	sipxconfig-report-config.vm

install-data-hook : $(CRON_INSTALL) $(DESTDIR)@SIPX_CONFDIR@/sipxconfig-report-config

sipxconfig-report-config : sipxconfig-report-config.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

$(DESTDIR)$(sysconfdir)/sipxpbx/sipxconfig-report-config: sipxconfig-report-config
	@if ! test -f $@; \
	then \
		echo "Installing default $@"; \
		$(INSTALL) -D -m 644 $< $@; \
	else \
		diff $< $@ > /dev/null 2>&1 \
		|| (  echo "Using existing $@ - new file is $@.NEW" \
		    ; $(INSTALL) -D -m 644 $< $@.NEW; \
			); \
	fi

# Target is not normally run, also, you may have to add sipx user to
# /etc/cron.d/cron.allow file
.PHONY: install-crontab
install-crontab : sipxconfig-report-crontab.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/sipxconfig-report-crontab.in > sipxconfig-report-crontab
	$(INSTALL) -D -m 644 sipxconfig-report-crontab $(DESTDIR)$(CRON_D)/sipxconfig-report-crontab
