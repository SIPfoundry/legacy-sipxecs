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
	sipxreplicate \
	sipx-network-trace

bin_SCRIPTS = \
   $(LOCAL_SHELLSCRIPTS)

EXTRA_DIST = $(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(sipxScript).in)

dist_bin_SCRIPTS = \
	regtimes \
	sipx-stats \
	sipx-alias2dot \
	sipx-trace \
	sipx-dialog-count \
	sipx-log-cut \
	sipx-logtrans \
	sipx-response-correlator \
	sipdb-dump \
	sipsend \
	polycom-config

$(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(sipxScript)) : % : $(srcdir)/%.in Makefile
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
	chmod +x $@
