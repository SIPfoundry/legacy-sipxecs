include $(top_srcdir)/config/subdir.am

bin_SCRIPTS = \
    ${PACKAGE}-config \
	 sipx-setup \
	 sipx-setup-system \
    analyze_483s \
	 sipx-snapshot \
	 sipx-backup \
	 sipx-restore \
	 sipx-sendmail-configure \
	 sipxlocalization \
	 check-fqdn \
	 sipx-archive-common \
    ipcclean.pl

SipxLibexecScripts = \
    sipx-strip-db-secrets.pl

SipxLibexecPythonScripts = \
	 sipx_setup_common.py

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
	sipx-backup.in \
	sipx-restore.in \
	sipx-sendmail-configure.in \
	sipxlocalization.in \
	check-fqdn.in \
	sipx-archive-common.in

PythonScripts = \
	sipx-setup.in \
	sipx-setup-system.in

EXTRA_DIST = \
   sipx-core-clean.in \
   voicemail_clean.in \
   sipx-chkspace.in \
   ${PACKAGE}-config \
   analyze_483s \
   ipcclean.pl \
   sipx-strip-db-secrets.pl.in \
	sipx_setup_common.py.in \
	$(PythonScripts) \
	$(SCRIPT_TEMPLATES)

$(foreach sipxScript,$(SCRIPT_TEMPLATES),$(sipxScript:.in=)) : % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
	chmod +x $@

install-exec-hook : $(DESTDIR)@DAILYDIR@/voicemail_clean \
                    $(DESTDIR)@DAILYDIR@/sipx-core-clean \
                    $(DESTDIR)@DAILYDIR@/sipx-chkspace \
						  $(foreach sipxScript,$(SipxLibexecScripts),$(DESTDIR)@SIPX_LIBEXECDIR@/$(sipxScript)) \
						  $(foreach sipxScript,$(SipxLibexecPythonScripts),$(DESTDIR)@SIPX_LIBEXECDIR@/$(sipxScript)) \
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

