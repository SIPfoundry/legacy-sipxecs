include $(top_srcdir)/config/subdir.am

SUBDIRS = \
	. \
	man \
	test

LOCAL_SHELLSCRIPTS = \
	expire-spread \
	monitor-spread \
	sipxproc \
	sipxreplicate

EXTRA_DIST = $(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(sipxScript).in)

dist_bin_SCRIPTS = \
	regtimes \
	sipx-stats \
	sipx-alias2dot \
	sipx-trace \
	sipx-readroute \
	sipx-log-cut \
	sipx-servtest \
	sipxcar \
	sipdb-dump \
	polycom-config \
	sipx_reset_cd

install-exec-hook: localize_scripts sipx_reset_cd

.PHONY: localize_scripts
localize_scripts: $(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(DESTDIR)@SIPX_BINDIR@/$(sipxScript))

$(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(DESTDIR)@SIPX_BINDIR@/$(sipxScript)) : $(DESTDIR)@SIPX_BINDIR@/% : %
	$(INSTALL) -D -m 755 $< $@

$(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(sipxScript)) : % : $(srcdir)/%.in
	$(LocalizeSipXconfig) $< > $@
	chmod +x $@

