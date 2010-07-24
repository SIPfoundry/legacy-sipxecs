## Process this file with automake to produce Makefile.in
include config/project.am

RPMS = $(DEFAULT_RPMS) $(DEBUG_RPM)

include config/conditional_doc.am

SUBDIRS = \
	. \
	src \
	examples \
	include \
	bin \
	$(doc_SUBDIRS)

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    CONTRIBUTORS \
    SVN-VERSION \
    BUILDSTAMP \
    @PACKAGE@-rpmlintrc \
    sipxcalllib.spec


DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES) \
	BUILDSTAMP \
	Makefile.in \
	aclocal.m4 \
	autom4te.cache/* \
	config.h.in \
	configure
