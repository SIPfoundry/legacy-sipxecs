## Process this file with automake to produce Makefile.in

include config/project.am

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-VERSION \
    sipxportlib.spec

RPMS = $(DEFAULT_RPMS) $(DEBUG_RPM)

include config/conditional_doc.am

SUBDIRS = \
	. \
	bin \
	src \
	include \
	syslogviewer \
	$(doc_SUBDIRS)

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
