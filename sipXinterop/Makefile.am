## Process this file with automake to produce Makefile.in
include config/project.am

RPMS = $(DEFAULT_RPMS)

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
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
	 sipxinterop.spec \
    BUILDSTAMP \
    SVN-VERSION

install-data-hook : @SIPX_INTEROP@ @SIPX_INTEROP@/SVN-VERSION @SIPX_INTEROP@/BUILDSTAMP

@SIPX_INTEROP@/BUILDSTAMP : BUILDSTAMP
	$(INSTALL) $< $@

@SIPX_INTEROP@/SVN-VERSION : SVN-VERSION
	$(INSTALL) $< $@

@SIPX_INTEROP@ :
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
