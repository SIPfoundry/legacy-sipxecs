## Process this file with automake to produce Makefile.in
include config/project.am

RPMS = $(DEFAULT_RPMS) $(DEBUG_RPM)

SUBDIRS = \
	src \
	etc \
	bin \
	$(doc_SUBDIRS)

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    BUILDSTAMP \
    SVN-VERSION \
    sipxacd.spec

install-exec-hook :

install-data-hook :
