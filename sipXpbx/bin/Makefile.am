include $(top_srcdir)/config/subdir.am


bin_SCRIPTS = \
    ${PACKAGE}-config \
    analyze_483s \
    ipcclean.pl

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
	blat.in \
	check-fqdn.in \
	sipx-archive-common.in

EXTRA_DIST = \
	sipxpbx.in \
   sipx-core-clean.in \
   voicemail_clean.in \
   sipx-chkspace.in \
	$(bin_SCRIPTS) \
	$(SCRIPT_TEMPLATES)

install-exec-hook : $(DESTDIR)@SERVICEDIR@/sipxpbx bin-scripts \
                    $(DESTDIR)@DAILYDIR@/voicemail_clean \
                    $(DESTDIR)@DAILYDIR@/sipx-core-clean \
                    $(DESTDIR)@DAILYDIR@/sipx-chkspace
	rm -f $(DESTDIR)$(bindir)/backup-mailstore.sh
	rm -f $(DESTDIR)$(bindir)/restore-configs.sh
	rm -f $(DESTDIR)$(bindir)/restore-mailstore.sh
	rm -f $(DESTDIR)$(bindir)/backup-configs.sh


$(DESTDIR)@SERVICEDIR@/sipxpbx: $(DESTDIR)@SERVICEDIR@ sipxpbx
	$(INSTALL) -D -m 755 sipxpbx $(DESTDIR)@SERVICEDIR@/sipxpbx

sipxpbx: sipxpbx.in
	$(LocalizeSipXconfig) $(srcdir)/sipxpbx.in > sipxpbx

$(DESTDIR)@DAILYDIR@/voicemail_clean: voicemail_clean.in
	$(LocalizeSipXconfig) $(srcdir)/voicemail_clean.in > voicemail_clean
	$(INSTALL) -D -m 755 voicemail_clean $(DESTDIR)@DAILYDIR@/voicemail_clean

$(DESTDIR)@DAILYDIR@/sipx-core-clean: sipx-core-clean.in
	$(LocalizeSipXconfig) $(srcdir)/sipx-core-clean.in > sipx-core-clean
	$(INSTALL) -D -m 755 sipx-core-clean $(DESTDIR)@DAILYDIR@/sipx-core-clean

$(DESTDIR)@DAILYDIR@/sipx-chkspace: sipx-chkspace.in
	$(LocalizeSipXconfig) $(srcdir)/sipx-chkspace.in > sipx-chkspace
	$(INSTALL) -D -m 755 sipx-chkspace $(DESTDIR)@DAILYDIR@/sipx-chkspace

$(DESTDIR)@SERVICEDIR@:
	$(INSTALL) -d $(DESTDIR)@SERVICEDIR@

# XXX - Why do we depend on servicedir (/etc/init.d) but install in bindir?
bin-scripts: $(DESTDIR)@SERVICEDIR@
	for s in $(SCRIPT_TEMPLATES); \
	do \
		tgt=`basename $$s .in`; \
		$(LocalizeSipXconfig) $(srcdir)/$$s > $$tgt; \
		$(INSTALL) -D -m 755 $$tgt $(DESTDIR)$(bindir)/$$tgt; \
	done

.PHONY: bin-scripts

DISTCLEANFILES = sipxpbx
