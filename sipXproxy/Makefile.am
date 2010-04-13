## Process this file with automake to produce Makefile.in
include config/project.am

RPMS = $(DEFAULT_RPMS) $(DEBUG_RPM) \
	$(DEST_RPM)/@PACKAGE@-cdr-$(SIPX_VERSION).$(RPM_TARGET_ARCH).rpm

include config/conditional_doc.am

SUBDIRS = \
	. \
	include \
	lib \
	src \
	bin \
	meta \
	etc \
	$(doc_SUBDIRS)

EXTRA_DIST = \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-VERSION \
    src/cdr \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    sipxproxy.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
