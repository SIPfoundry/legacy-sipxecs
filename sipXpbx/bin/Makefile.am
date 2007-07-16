include $(top_srcdir)/config/subdir.am


bin_SCRIPTS = \
    ${PACKAGE}-config \
    analyze_483s \
    ipcclean.pl \
    blat \
    check-fqdn \
    sipx-archive-common

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

SCRIPT_TEMPLATES = \
	keepalive.sh.in \
	sipx-snapshot.in \
	sipx-backup.in \
	sipx-restore.in

EXTRA_DIST = \
	sipxpbx.in \
        voicemail_clean.in \
        sipx-chkspace.in \
	$(bin_SCRIPTS) \
	$(SCRIPT_TEMPLATES)

install-exec-hook : $(DESTDIR)$(sysconfdir)/init.d/sipxpbx bin-scripts \
                    $(DESTDIR)$(sysconfdir)/cron.daily/voicemail_clean \
                    $(DESTDIR)$(sysconfdir)/cron.daily/sipx-chkspace
	rm -f $(DESTDIR)$(bindir)/backup-mailstore.sh
	rm -f $(DESTDIR)$(bindir)/restore-configs.sh
	rm -f $(DESTDIR)$(bindir)/restore-mailstore.sh
	rm -f $(DESTDIR)$(bindir)/backup-configs.sh


$(DESTDIR)$(sysconfdir)/init.d/sipxpbx: $(sysconfdir)/init.d sipxpbx
	$(INSTALL) -D -m 755 sipxpbx $(DESTDIR)$(sysconfdir)/init.d/sipxpbx

sipxpbx: sipxpbx.in
	$(LocalizeSipXconfig) $(srcdir)/sipxpbx.in > sipxpbx

$(DESTDIR)$(sysconfdir)/cron.daily/voicemail_clean: voicemail_clean.in
	$(LocalizeSipXconfig) $(srcdir)/voicemail_clean.in > voicemail_clean
	$(INSTALL) -D -m 755 voicemail_clean $(DESTDIR)$(sysconfdir)/cron.daily/voicemail_clean

$(DESTDIR)$(sysconfdir)/cron.daily/sipx-chkspace: sipx-chkspace.in
	$(LocalizeSipXconfig) $(srcdir)/sipx-chkspace.in > sipx-chkspace
	$(INSTALL) -D -m 755 sipx-chkspace $(DESTDIR)$(sysconfdir)/cron.daily/sipx-chkspace

$(sysconfdir)/init.d:
	$(INSTALL) -d $(DESTDIR)$(sysconfdir)/init.d 

bin-scripts: $(sysconfdir)/init.d
	for s in $(SCRIPT_TEMPLATES); \
	do \
		tgt=`basename $$s .in`; \
		$(LocalizeSipXconfig) $(srcdir)/$$s > $$tgt; \
		$(INSTALL) -D -m 755 $$tgt $(DESTDIR)$(bindir)/$$tgt; \
	done

.PHONY: bin-scripts

DISTCLEANFILES = sipxpbx
