## Process this file with automake to produce Makefile.in
include config/project.am

SUBDIRS = \
	src \
	preflight

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    CONTRIBUTORS \
    LICENSE \
    BUILDSTAMP \
    SVN-VERSION \
    ChangeLog \
    sipxtools.spec

RPMS = $(RPM)

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
