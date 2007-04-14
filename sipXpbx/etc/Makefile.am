## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am


pkgconfdir = @SIPX_CONFDIR@

pkgconf_DATA = \
    mime.types

pbxconf_files = \
    WatchDog.xml \
    ProcessDefinitions.xml \
    resourcemap.xml \
    voicemail.xml.in \
    cbadmission.xml.in

wwwconf_files = \
    httpd.conf \
    httpd-sipxchange-common-ssl.conf.in \
    httpd-sipxchange-common.conf.in

EXTRA_DIST = \
	$(pkgconf_DATA) \
	$(foreach file,$(pbxconf_files) $(wwwconf_files),$(file).in) \
	sipxchange

if DO_RPMBUILD
RPM_INSTALL=rpm-install
else
RPM_INSTALL=
endif

install-exec-hook :

install-data-hook : $(RPM_INSTALL) \
	$(foreach file,$(pbxconf_files) $(wwwconf_files),$(DESTDIR)$(sysconfdir)/sipxpbx/$(file))

$(foreach file,$(pbxconf_files) $(wwwconf_files),$(DESTDIR)$(sysconfdir)/sipxpbx/$(file)) : $(DESTDIR)$(sysconfdir)/sipxpbx/% : %
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

$(foreach file,$(pbxconf_files) $(wwwconf_files),$(file)) : % : %.in
	$(LocalizeSipXconfig) \
	-e 's,@APACHE2_HOST_ACCESS\@,@APACHE2_HOST_ACCESS@,g' \
	-e 's,@APACHE2_MOD_ACCESS\@,@APACHE2_MOD_ACCESS@,g' \
	$< > $@

.PHONY : rpm-install
rpm-install: $(DESTDIR)$(sysconfdir)/logrotate.d/sipxchange

$(DESTDIR)$(sysconfdir)/logrotate.d/sipxchange: sipxchange
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

