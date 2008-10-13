## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

pkgconfdir = @SIPX_CONFDIR@

pbxconf_files = \
    WatchDog.xml \
    ProcessDefinitions.xml \
    resourcemap.xml \
    sipxsupervisor-config.in

alarmdir = @SIPX_CONFDIR@/alarms
alarm_DATA = \
    sipXalarms-config.xml

EXTRA_DIST = \
	$(foreach file,$(pbxconf_files),$(file).in) $(alarm_DATA)

install-data-hook : \
	$(foreach file,$(pbxconf_files),$(DESTDIR)$(sysconfdir)/sipxpbx/$(file))

$(foreach file,$(pbxconf_files),$(DESTDIR)$(sysconfdir)/sipxpbx/$(file)) : $(DESTDIR)$(sysconfdir)/sipxpbx/% : %
	@if [ -f $@ ]; \
	then \
		diff $< $@ > /dev/null 2>&1 \
		|| ( echo "Using existing $@; new version copied as .NEW"; \
			$(INSTALL) -D -m 644  $< $@.NEW \
			); \
	else \
		echo "Installing default $@"; \
		$(INSTALL) -D -m 644 $< $@; \
	fi

$(foreach file,$(pbxconf_files),$(file)) : % : %.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@


