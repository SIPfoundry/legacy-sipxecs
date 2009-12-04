include $(top_srcdir)/config/subdir.am

SUBDIRS = \
	. \
	man \
	syslog2siptrace \
	sipviewer \
	test

LOCAL_SHELLSCRIPTS = \
	sipx-alarm \
	sipx-servtest \
	sipx-mrtg-threshexceeded \
	sipx-mrtg-threshok \
	sipx-readroute \
	sipx-trace \
	expire-spread \
	monitor-spread \
	sipxproc \
	sipxreplicate

bin_SCRIPTS = \
   $(LOCAL_SHELLSCRIPTS)

EXTRA_DIST = $(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(sipxScript).in)

dist_bin_SCRIPTS = \
	${PACKAGE}-config \
	regtimes \
	sipx-stats \
	sipx-alias2dot \
	sipx-trace \
	sipx-dialog-count \
	sipx-log-cut \
	sipx-response-correlator \
	sipxcar \
	sipdb-dump \
	sipsend \
	polycom-config

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
