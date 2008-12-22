## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

pkgconfdir = @SIPX_CONFDIR@

pkgconf_DATA = \
    mime.types

wwwconf_files = \
    httpd.conf \
    httpd-sipxchange-common-ssl.conf.in \
    httpd-sipxchange-common.conf.in

EXTRA_DIST = \
	$(pkgconf_DATA) \
	$(foreach file,$(wwwconf_files),$(file).in) \
	init.d/sipxpbx.in \
	logrotate.d/sipxchange.in

install-exec-hook : $(DESTDIR)@SERVICEDIR@/sipxpbx

$(DESTDIR)@SERVICEDIR@/sipxpbx: $(DESTDIR)@SERVICEDIR@ init.d/sipxpbx
	$(INSTALL) -D -m 755 init.d/sipxpbx $(DESTDIR)@SERVICEDIR@/sipxpbx

init.d/sipxpbx: init.d/sipxpbx.in
	test -d init.d || mkdir init.d
	@echo "Localizing $(srcdir)/init.d/sipxpbx.in -> init.d/sipxpbx"
	@$(LocalizeSipXconfig) $(srcdir)/init.d/sipxpbx.in > init.d/sipxpbx

$(DESTDIR)@SERVICEDIR@:
	$(INSTALL) -d $(DESTDIR)@SERVICEDIR@

install-data-hook : \
	$(DESTDIR)$(sysconfdir)/sipxpbx/httpd.conf.d \
	$(DESTDIR)$(sysconfdir)/logrotate.d/sipxchange \
	$(foreach file, $(wwwconf_files),$(DESTDIR)$(sysconfdir)/sipxpbx/$(file))

$(DESTDIR)$(sysconfdir)/sipxpbx/httpd.conf.d:
	$(INSTALL) -d -m 755 $@

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
