## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

pkgconfdir = @SIPX_CONFDIR@

pkgconf_DATA = \
    mime.types

wwwconf_files = \
    httpd.conf \
    httpd-sipxchange-common-ssl.conf \
    httpd-sipxchange-common.conf

EXTRA_DIST = \
	$(pkgconf_DATA) \
	$(foreach file,$(wwwconf_files),$(file).in) \
	init.d/sipxecs.in \
	logrotate.d/sipxchange.in

install-exec-hook : $(DESTDIR)@SERVICEDIR@/sipxecs

$(DESTDIR)@SERVICEDIR@/sipxecs: $(DESTDIR)@SERVICEDIR@ init.d/sipxecs
	$(INSTALL) -D -m 755 init.d/sipxecs $(DESTDIR)@SERVICEDIR@/sipxecs

init.d/sipxecs: init.d/sipxecs.in
	test -d init.d || mkdir init.d
	@echo "Localizing $(srcdir)/init.d/sipxecs.in -> init.d/sipxecs"
	@$(LocalizeSipXconfig) $(srcdir)/init.d/sipxecs.in > init.d/sipxecs

$(DESTDIR)@SERVICEDIR@:
	$(INSTALL) -d $(DESTDIR)@SERVICEDIR@

install-data-hook : \
	$(DESTDIR)$(sysconfdir)/logrotate.d/sipxchange \
	$(foreach file, $(wwwconf_files),$(DESTDIR)$(sysconfdir)/sipxpbx/$(file))

$(foreach file,$(wwwconf_files),$(DESTDIR)$(sysconfdir)/sipxpbx/$(file)) : $(DESTDIR)$(sysconfdir)/sipxpbx/% : %
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

$(foreach file,$(wwwconf_files),$(file)) : % : %.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) \
	-e 's,@APACHE2_HOST_ACCESS\@,@APACHE2_HOST_ACCESS@,g' \
	-e 's,@APACHE2_MOD_ACCESS\@,@APACHE2_MOD_ACCESS@,g' \
	$< > $@

logrotate.d/sipxchange: logrotate.d/sipxchange.in
	test -d logrotate.d || mkdir logrotate.d
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

$(DESTDIR)$(sysconfdir)/logrotate.d/sipxchange: logrotate.d/sipxchange
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

DISTCLEANFILES = sipxpbx
