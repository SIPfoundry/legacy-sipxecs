## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

pkgconfdir = @SIPX_CONFDIR@

EXTRA_DIST = \
	logrotate.d/freeswitch.in

install-data-hook : \
	$(DESTDIR)$(sysconfdir)/logrotate.d/freeswitch

logrotate.d/freeswitch: logrotate.d/freeswitch.in
	test -d logrotate.d || mkdir logrotate.d
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

$(DESTDIR)$(sysconfdir)/logrotate.d/freeswitch: logrotate.d/freeswitch
	@if [ -f $@ ]; \
	then \
		diff $< $@ > /dev/null 2>&1 \
		|| ( echo "Using existing $@; new version copied as .NEW"; \
			$(INSTALL) -D -m 644 $< $@.NEW \
			); \
	else \
		echo "Installing default $@"; \
		$(INSTALL) -D -m 644 $< $@; \
	fi
