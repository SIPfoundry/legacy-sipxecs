include $(top_srcdir)/config/subdir.am

bin_SCRIPTS = \
	 ${PACKAGE}-config \
	 sipxecs-config \
	 sipxecs-setup \
	 sipxecs-upgrade \
	 sipxecs-setup-system \
	 analyze_483s \
	 sipx-snapshot \
	 sipx-backup \
	 sipx-restore \
	 sipx-package.py \
	 sipx-swadmin.py \
	 sipx-zoneadmin.sh \
	 sipx-sendmail-configure \
	 sipxlocalization \
	 check-fqdn \
	 sipx-archive-common \
	 ipcclean.pl \
	 sipx-test-dnsIp \
	 sipx-test-ssl \
	 sipx-test-selinux \
	 sipx-test-rpmnew \
	 sipx-test-localhostConfigured \
	 sipx-test-localhostShared \
	 sipx-test-tmp \
	 sipx-test-apache \
	 sipx-test-hostname \
	 sipx-test-maxfds

SipxLibexecScripts = \
	ibm_jvm_tmpdir \
	sipx-activate-ntp \
	sipx-config-httpd-access \
	sipx-config-vxml-domain \
	initial-config \
	sipx-strip-db-secrets.pl \
	sipxkeystoregen

SipxLibexecPythonScripts = \
	 sipxecs_setup_common.py

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
		 $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

SCRIPT_TEMPLATES = \
	sipx-snapshot.in \
	sipx-test-dnsIp.in \
	sipx-test-ssl.in \
	sipx-test-selinux.in \
	sipx-test-rpmnew.in \
	sipx-test-localhostConfigured.in \
	sipx-test-localhostShared.in \
	sipx-test-apache.in \
	sipx-test-hostname.in \
	sipx-backup.in \
	sipx-restore.in \
	sipx-sendmail-configure.in \
	sipxlocalization.in \
	check-fqdn.in \
	sipx-archive-common.in \
	sipx-test-tmp.in \
	sipx-test-maxfds.in \
	sipx-zoneadmin.sh.in

PythonScripts = \
	sipxecs-setup.in \
	sipxecs-upgrade.in \
	sipxecs-setup-system.in \
	sipx-package.py.in \
	sipx-swadmin.py.in

EXTRA_DIST = \
	sipx-core-clean.in \
	voicemail_clean.in \
	sipx-chkspace.in \
	${PACKAGE}-config \
	sipxecs-config \
	analyze_483s \
	ipcclean.pl \
	sipxecs_setup_common.py.in \
	$(foreach libexecScript, $(SipxLibexecScripts), $(libexecScript).in) \
	$(PythonScripts) \
	$(SCRIPT_TEMPLATES)

$(foreach sipxScript,$(SCRIPT_TEMPLATES),$(sipxScript:.in=)) : % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
	chmod +x $@

install-exec-hook : \
	$(DESTDIR)@DAILYDIR@/voicemail_clean \
	$(DESTDIR)@DAILYDIR@/sipx-core-clean \
	$(DESTDIR)@DAILYDIR@/sipx-chkspace \
	$(foreach sipxScript,$(SipxLibexecScripts),$(DESTDIR)@SIPX_LIBEXECDIR@/$(sipxScript)) \
	$(foreach sipxScript,$(SipxLibexecPythonScripts),$(DESTDIR)@SIPX_LIBEXECDIR@/$(sipxScript)) \
	$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/010_sipx-activate-ntp \
	$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/011_sipx-config-httpd-access \
	$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/012_sipx-config-vxml-domain \
	$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/013_sipxkeystoregen \
	$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/020_ibm_jvm_tmpdir \
	remove-obsolete-bins

.PHONY: remove-obsolete-bins

remove-obsolete-bins:
	rm -f $(DESTDIR)$(bindir)/backup-mailstore.sh
	rm -f $(DESTDIR)$(bindir)/restore-configs.sh
	rm -f $(DESTDIR)$(bindir)/restore-mailstore.sh
	rm -f $(DESTDIR)$(bindir)/backup-configs.sh

$(foreach pythonScript,$(PythonScripts),$(pythonScript:.in=)) : % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
	chmod +x $@
	python -m py_compile $@

$(foreach sipxScript,$(SipxLibexecScripts),$(sipxScript)) : % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
	chmod +x $@

$(foreach sipxScript,$(SipxLibexecPythonScripts),$(sipxScript)) : % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
	chmod +x $@
	python -m py_compile $@

$(foreach sipxScript,$(SipxLibexecScripts),$(DESTDIR)@SIPX_LIBEXECDIR@/$(sipxScript)) : $(DESTDIR)@SIPX_LIBEXECDIR@/% : %
	$(INSTALL) -D -m 755 $< $@

$(foreach sipxScript,$(SipxLibexecPythonScripts),$(DESTDIR)@SIPX_LIBEXECDIR@/$(sipxScript)) : $(DESTDIR)@SIPX_LIBEXECDIR@/% : %
	$(INSTALL) -D -m 755 $< $@

$(DESTDIR)@DAILYDIR@/voicemail_clean: voicemail_clean.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/voicemail_clean.in > voicemail_clean
	$(INSTALL) -D -m 755 voicemail_clean $(DESTDIR)@DAILYDIR@/voicemail_clean

$(DESTDIR)@DAILYDIR@/sipx-core-clean: sipx-core-clean.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/sipx-core-clean.in > sipx-core-clean
	$(INSTALL) -D -m 755 sipx-core-clean $(DESTDIR)@DAILYDIR@/sipx-core-clean

$(DESTDIR)@DAILYDIR@/sipx-chkspace: sipx-chkspace.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $(srcdir)/sipx-chkspace.in > sipx-chkspace
	$(INSTALL) -D -m 755 sipx-chkspace $(DESTDIR)@DAILYDIR@/sipx-chkspace

$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/010_sipx-activate-ntp: $(DESTDIR)@SIPX_LIBEXECDIR@/sipx-activate-ntp $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d
	cd $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d \
	&& ln -sf ../sipx-activate-ntp 010_sipx-activate-ntp

$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/011_sipx-config-httpd-access: $(DESTDIR)@SIPX_LIBEXECDIR@/sipx-config-httpd-access $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d
	cd $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d \
	&& ln -sf ../sipx-config-httpd-access 011_sipx-config-httpd-access

$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/012_sipx-config-vxml-domain: $(DESTDIR)@SIPX_LIBEXECDIR@/sipx-config-vxml-domain $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d
	cd $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d \
	&& ln -sf ../sipx-config-vxml-domain 012_sipx-config-vxml-domain

$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/013_sipxkeystoregen: $(DESTDIR)@SIPX_LIBEXECDIR@/sipxkeystoregen $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d
	cd $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d \
	&& ln -sf ../sipxkeystoregen 013_sipxkeystoregen

$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d/020_ibm_jvm_tmpdir: $(DESTDIR)@SIPX_LIBEXECDIR@/ibm_jvm_tmpdir $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d
	cd $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d \
	&& ln -sf ../ibm_jvm_tmpdir 020_ibm_jvm_tmpdir

$(DESTDIR)@SIPX_LIBEXECDIR@/setup.d:
	$(INSTALL) -d $(DESTDIR)@SIPX_LIBEXECDIR@/setup.d

# sipxecs-config is generated from sipxecs-config.in by the
# top-level configure.ac, as it is the only one that knows the names
# of all the components, and so can substitute "@subdirs@".
