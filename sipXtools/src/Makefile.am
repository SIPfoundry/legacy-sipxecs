include $(top_srcdir)/config/utility.am

SUBDIRS = \
	. \
	syslog2siptrace

LOCAL_SHELLSCRIPTS = \
	sipx-network-trace \
	sipx-readroute \
	sipx-servtest \
	sipx-trace

bin_SCRIPTS = \
   $(LOCAL_SHELLSCRIPTS)

EXTRA_DIST = \
	$(LOCAL_SHELLSCRIPTS:=.in)

dist_bin_SCRIPTS = \
	sipx-dbutil \
	sipx-sipsend \
	sipx-audit-time \
	sipx-dialog-count \
	sipx-response-correlator \
	sipx-stats \
	sipx-tcpdump2siptrace

$(LOCAL_SHELLSCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(LOCAL_SHELLSCRIPTS)
