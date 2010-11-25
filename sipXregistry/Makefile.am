## Process this file with automake to produce Makefile.in
include config/project.am

include config/conditional_doc.am

SUBDIRS = \
	. \
	include \
	src \
	lib \
	bin \
	meta \
	etc \
	$(doc_SUBDIRS)

EXTRA_DIST = \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-VERSION \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/svn-version \
    config/Doxyfile.in \
    @PACKAGE@-rpmlintrc \
    sipxregistry.spec

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
