## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

pkgconfdir = @SIPX_CONFDIR@

pkgconf_DATA = \
    mime.types

wwwconf_files = \
    httpd.conf \
    httpd-sipxchange-common-ssl.conf \
    httpd-dav.conf  \
    httpd-sipxchange-common.conf.in

EXTRA_DIST = \
	$(pkgconf_DATA) \
	$(foreach file,$(wwwconf_files),$(file).in) \
	init.d/sipxecs.in \
	init.d/phonelogd.in \
	phonelog.conf \
	logrotate.d/sipxchange.in

install-exec-hook : $(DESTDIR)@SERVICEDIR@/sipxecs $(DESTDIR)@SERVICEDIR@/phonelogd

$(DESTDIR)@SERVICEDIR@/sipxecs: $(DESTDIR)@SERVICEDIR@ init.d/sipxecs
	$(INSTALL) -D -m 755 init.d/sipxecs $(DESTDIR)@SERVICEDIR@/sipxecs

init.d/sipxecs: init.d/sipxecs.in
	test -d init.d || mkdir init.d
	@echo "Localizing $(srcdir)/init.d/sipxecs.in -> init.d/sipxecs"
	@$(LocalizeSipXconfig) $(srcdir)/init.d/sipxecs.in > init.d/sipxecs

$(DESTDIR)@SERVICEDIR@/phonelogd: $(DESTDIR)@SERVICEDIR@ init.d/phonelogd
	$(INSTALL) -D -m 755 init.d/phonelogd $(DESTDIR)@SERVICEDIR@/phonelogd

init.d/phonelogd: init.d/phonelogd.in
	test -d init.d || mkdir init.d
	@echo "Localizing $(srcdir)/init.d/phonelogd.in -> init.d/phonelogd"
	@$(LocalizeSipXconfig) \
		-e 's,@SYSLOG_CMD\@,@SYSLOG_CMD@,g' \
		-e 's,@SYSLOG_LIBS\@,@SYSLOG_LIBS@,g' \
		-e 's,@SYSLOG_ARGS\@,@SYSLOG_ARGS@,g' \
		$(srcdir)/init.d/phonelogd.in > init.d/phonelogd

$(DESTDIR)${sysconfdir}/sipxpbx/phonelog.conf: phonelog.conf
	$(INSTALL) -D -m 755 $< $@

$(DESTDIR)@SERVICEDIR@:
	$(INSTALL) -d $(DESTDIR)@SERVICEDIR@

install-data-hook : \
	$(DESTDIR)$(sysconfdir)/sipxpbx/phonelog.conf \
	$(DESTDIR)$(localstatedir)/log/sipxpbx/phone \
	$(DESTDIR)$(sysconfdir)/sipxpbx/httpd.conf.d \
	$(DESTDIR)$(wwwdir)/var \
	$(DESTDIR)$(wwwdir)/webdav \
	$(DESTDIR)$(sysconfdir)/logrotate.d/sipxchange \
	$(foreach file, $(wwwconf_files),$(DESTDIR)$(sysconfdir)/sipxpbx/$(file))

$(DESTDIR)$(sysconfdir)/sipxpbx/httpd.conf.d:
	$(INSTALL) -d -m 755 $@

$(DESTDIR)$(wwwdir)/var:
	$(INSTALL) -d -m 755 $@

$(DESTDIR)$(wwwdir)/webdav:
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

$(DESTDIR)$(localstatedir)/log/sipxpbx/phone:
	$(INSTALL) -d -m 755 $@

DISTCLEANFILES = sipxpbx
