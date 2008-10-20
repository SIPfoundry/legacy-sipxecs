include $(top_srcdir)/config/subdir.am

SUBDIRS = \
	. \
	man \
	test

LOCAL_SHELLSCRIPTS = \
	sipx-alarm \
	sipx-servtest \
	sipx-readroute \
	expire-spread \
	monitor-spread \
	sipxproc \
	sipxreplicate

EXTRA_DIST = $(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(sipxScript).in)

dist_bin_SCRIPTS = \
	${PACKAGE}-config \
	regtimes \
	sipx-stats \
	sipx-alias2dot \
	sipx-trace \
	sipx-log-cut \
	sipx-servtest \
	sipxcar \
	sipdb-dump \
	polycom-config

install-exec-hook: localize_scripts

.PHONY: localize_scripts
localize_scripts: $(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(DESTDIR)@SIPX_BINDIR@/$(sipxScript))

$(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(DESTDIR)@SIPX_BINDIR@/$(sipxScript)) : $(DESTDIR)@SIPX_BINDIR@/% : %
	$(INSTALL) -D -m 755 $< $@

$(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(sipxScript)) : % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
	chmod +x $@

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@echo "making ${PACKAGE}-config"
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config


