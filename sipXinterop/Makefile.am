## Process this file with automake to produce Makefile.in
include config/project.am

include interop.am

SUBDIRS = \
	. \
	bin \
	http_conf \
	http_rootdir \
	sipx_confdir \
	sipx_dbdir

EXTRA_DIST = \
    config/sipX-config.in \
    config/svn-version \
	 sipxinterop.spec \
    BUILDSTAMP \
    SVN-VERSION

install-data-hook : $(DESTDIR)@SIPX_INTEROP@ $(DESTDIR)@SIPX_INTEROP@/SVN-VERSION $(DESTDIR)@SIPX_INTEROP@/BUILDSTAMP

$(DESTDIR)@SIPX_INTEROP@/BUILDSTAMP : BUILDSTAMP
	$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/SVN-VERSION : SVN-VERSION
	$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@ :
	$(INSTALL) -d $@

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES) \
	BUILDSTAMP \
	Makefile.in \
	SVN-VERSION \
	aclocal.m4 \
	autom4te.cache/output.0 \
	autom4te.cache/output.1 \
	autom4te.cache/requests \
	autom4te.cache/traces.0 \
	autom4te.cache/traces.1
