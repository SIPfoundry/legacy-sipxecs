## Process this file with automake to produce Makefile.in
include config/project.am

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
    sipxmedialib.spec

include config/conditional_doc.am

SUBDIRS = \
	. \
	src \
	include \
	bin \
	$(doc_SUBDIRS)

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
