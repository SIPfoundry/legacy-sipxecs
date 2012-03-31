include $(top_srcdir)/config/utility.am

SUBDIRS = \
	. \
	man \
	syslog2siptrace \
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

EXTRA_DIST = \
	$(LOCAL_SHELLSCRIPTS:=.in)

dist_bin_SCRIPTS = \
	regtimes \
	sipx-stats \
	sipx-alias2dot \
	sipx-trace \
	sipx-dialog-count \
	sipx-logtrans \
	sipx-response-correlator \
	sipdb-dump \
	sipsend \
	polycom-config

$(LOCAL_SHELLSCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(LOCAL_SHELLSCRIPTS)
